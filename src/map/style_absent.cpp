// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les styles hors de vue de moonclient. Voir style_absent.hpp.

#include "style_absent.hpp"

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <common/grfio.hpp> // encode_zip
#include <common/nullpo.hpp>
#include <common/socket.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "guild.hpp"
#include "map.hpp" // map_charid2sd
#include "packets.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "style_layers.hpp"

namespace {

using bytes = std::vector<uint8>;

// La fenêtre du plafond d'octets, en millisecondes.
constexpr t_tick BYTES_WINDOW_MS = 60000;
constexpr size_t CZ_HEAD = sizeof(PACKET_CZ_MOONCLIENT_STYLE_ABSENT);
constexpr size_t ZC_HEAD = sizeof(PACKET_ZC_MOONCLIENT_STYLE_ABSENT);
constexpr size_t CHAR_ID_BYTES = sizeof(uint32);
// L'en-tête d'un bloc : char_id, état.
constexpr size_t BLOCK_HEAD = CHAR_ID_BYTES + 1;
// Le plus d'entrées de calque qu'un bloc du 0x0F5F porte : la tête, la peau du
// visage, quatre exemplaires, la suite et la peau de chaque corps.
constexpr size_t SHOWN_ENTRIES_MAX = 1 + 1 + 4 + 2 * BOURGEON_STYLE_MAX_VARIANTS;
// Le pire bloc : l'apparence, toutes les variantes, toutes les entrées pleines.
constexpr size_t BLOCK_MAX = BLOCK_HEAD + MOONCLIENT_STYLE_ABSENT_LOOK_BYTES
	+ 1 + BOURGEON_STYLE_MAX_VARIANTS * MOONCLIENT_STYLE_ABSENT_BODY_BYTES
	+ 1 + SHOWN_ENTRIES_MAX * MOONCLIENT_STYLE_LAYER_ENTRY_MAX;

static_assert(CZ_HEAD == 5, "l'en-tête du CZ 0x0F63 ne vaut plus ce que le client attend");
static_assert(ZC_HEAD == 10, "l'en-tête du ZC 0x0F64 ne vaut plus ce que le client attend");
static_assert(MOONCLIENT_STYLE_ABSENT_BODY_BYTES == 52, "un corps v7 ne vaut plus une entrée du 0x0F27 sans gid");
static_assert(BLOCK_MAX <= MOONCLIENT_STYLE_LAYERS_RAW_MAX, "un bloc doit tenir dans une trame");
// Même incompressible, la trame garde une longueur positive.
static_assert(ZC_HEAD + MOONCLIENT_STYLE_LAYERS_RAW_MAX < INT16_MAX, "une trame doit tenir sa longueur");

// La cadence et le plafond d'octets d'une session, par char_id.
struct s_absent_state {
	t_tick next_allowed = 0;
	t_tick window_start = 0;
	uint32 window_bytes = 0;
};

std::unordered_map<uint32, s_absent_state> s_states;

void push_u16(bytes& out, uint16 value) {
	out.push_back(static_cast<uint8>(value & 0xFF));
	out.push_back(static_cast<uint8>(value >> 8));
}

void push_u32(bytes& out, uint32 value) {
	for (int32 shift = 0; shift < 32; shift += 8)
		out.push_back(static_cast<uint8>((value >> shift) & 0xFF));
}

// 🔴 LES char_id QUE LA SESSION EST EN DROIT DE VOIR, relus ici à chaque demande et
// jamais d'après le client : sa guilde, son groupe, ses amis.
std::unordered_set<uint32> visible_char_ids(const map_session_data& sd) {
	std::unordered_set<uint32> out;
	if (sd.guild != nullptr) {
		for (const guild_member& m : sd.guild->guild.member)
			if (m.char_id != 0)
				out.insert(m.char_id);
	}
	if (sd.status.party_id != 0) {
		if (const party_data* p = party_search(sd.status.party_id)) {
			for (const party_member& m : p->party.member)
				if (m.char_id != 0)
					out.insert(m.char_id);
		}
	}
	for (const s_friend& f : sd.status.friends)
		if (f.char_id != 0)
			out.insert(f.char_id);
	return out;
}

// Caché à `viewer` : la garde de la diffusion, et `@hide` quoi que dise
// `hide_cloaked_units`.
bool concealed(const map_session_data& target, const map_session_data& viewer) {
	if (clif_hidden_unit_withheld(target, viewer))
		return true;
	return pc_isinvisible(&target) && pc_get_group_level(&viewer) < battle_config.hidden_units_view_level;
}

// Le bloc d'un char_id : l'état 0, ou ce qu'un joueur en vue recevrait.
bytes block_of(const map_session_data& sd, uint32 char_id, const std::unordered_set<uint32>& visible) {
	bytes block;
	push_u32(block, char_id);
	map_session_data* target = visible.count(char_id) != 0 ? map_charid2sd(char_id) : nullptr;
	if (target == nullptr || target->state.spectator || concealed(*target, sd)) {
		block.push_back(MOONCLIENT_STYLE_ABSENT_NOTHING);
		return block;
	}
	block.push_back(MOONCLIENT_STYLE_ABSENT_SHOWN);
	push_u16(block, static_cast<uint16>(target->status.class_));
	block.push_back(static_cast<uint8>(target->status.sex));
	push_u16(block, static_cast<uint16>(target->status.hair));
	push_u16(block, static_cast<uint16>(target->status.hair_color));
	clif_bourgeon_style_bodies(*target, block);
	const bytes entries = style_layers_shown_entries(*target);
	block.insert(block.end(), entries.begin(), entries.end());
	return block;
}

// Une trame à `sd` : le résultat, et la charge, compressée au-delà du seuil.
void reply(const map_session_data& sd, uint8 result, uint16 wait_ms, const bytes& raw) {
	const int32 fd = sd.fd;
	if (!session_isActive(fd))
		return;
	bytes body = raw;
	uint8 flags = 0;
	if (raw.size() > MOONCLIENT_STYLE_LAYERS_ZIP_ABOVE) {
		unsigned long zipped_size = static_cast<unsigned long>(raw.size() + raw.size() / 100 + 64);
		bytes zipped(zipped_size);
		if (encode_zip(zipped.data(), &zipped_size, raw.data(), static_cast<unsigned long>(raw.size())) == 0 && zipped_size < raw.size()) {
			zipped.resize(zipped_size);
			body = std::move(zipped);
			flags = MOONCLIENT_STYLE_LAYERS_ZIPPED;
		}
	}
	const size_t length = ZC_HEAD + body.size();
	WFIFOHEAD(fd, length);
	PACKET_ZC_MOONCLIENT_STYLE_ABSENT* p = reinterpret_cast<PACKET_ZC_MOONCLIENT_STYLE_ABSENT*>(WFIFOP(fd, 0));
	p->packetType = HEADER_ZC_MOONCLIENT_STYLE_ABSENT;
	p->packetLength = static_cast<int16>(length);
	p->result = result;
	p->wait_ms = wait_ms;
	p->flags = flags;
	p->raw_size = static_cast<uint16>(raw.size());
	if (!body.empty())
		memcpy(WFIFOP(fd, ZC_HEAD), body.data(), body.size());
	WFIFOSET(fd, length);
}

uint16 wait_of(t_tick until, t_tick now) {
	return static_cast<uint16>(std::min<t_tick>(UINT16_MAX, std::max<t_tick>(0, DIFF_TICK(until, now))));
}

} // namespace

void style_absent_on_logout(const map_session_data& sd) {
	s_states.erase(sd.status.char_id);
}

void clif_parse_moonclient_style_absent(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!style_layers_receiver(*sd))
		return;
	const size_t length = RFIFOW(fd, 2);
	s_absent_state& state = s_states[sd->status.char_id];
	const t_tick now = gettick();

	// ── Le plafond d'octets : chaque trame compte, refusée ou non ──────────
	if (state.window_start == 0 || DIFF_TICK(now, state.window_start) >= BYTES_WINDOW_MS) {
		state.window_start = now;
		state.window_bytes = 0;
	}
	state.window_bytes += static_cast<uint32>(length);
	if (state.window_bytes > MOONCLIENT_STYLE_ABSENT_BYTES_PER_MINUTE) {
		reply(*sd, MOONCLIENT_STYLE_CAP, wait_of(state.window_start + BYTES_WINDOW_MS, now), bytes());
		return;
	}

	// ── La structure : la longueur exacte, ni zéro ni doublon ──────────────
	const size_t count = length >= CZ_HEAD ? RFIFOB(fd, 4) : 0;
	if (count == 0 || count > MOONCLIENT_STYLE_ABSENT_IDS_MAX || length != CZ_HEAD + count * CHAR_ID_BYTES) {
		reply(*sd, MOONCLIENT_STYLE_MALFORMED, 0, bytes());
		return;
	}
	std::vector<uint32> asked;
	asked.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		const uint32 char_id = RFIFOL(fd, CZ_HEAD + i * CHAR_ID_BYTES);
		if (char_id == 0 || std::find(asked.begin(), asked.end(), char_id) != asked.end()) {
			reply(*sd, MOONCLIENT_STYLE_MALFORMED, 0, bytes());
			return;
		}
		asked.push_back(char_id);
	}

	// ── La cadence ─────────────────────────────────────────────────────────
	if (state.next_allowed != 0 && DIFF_TICK(state.next_allowed, now) > 0) {
		reply(*sd, MOONCLIENT_STYLE_TOO_SOON, wait_of(state.next_allowed, now), bytes());
		return;
	}
	state.next_allowed = now + MOONCLIENT_STYLE_ABSENT_CADENCE_MS;

	// ── Un bloc par char_id, jamais coupé entre deux trames ────────────────
	const std::unordered_set<uint32> visible = visible_char_ids(*sd);
	bytes raw;
	for (const uint32 char_id : asked) {
		const bytes block = block_of(*sd, char_id, visible);
		if (raw.size() + block.size() > MOONCLIENT_STYLE_LAYERS_RAW_MAX) {
			reply(*sd, MOONCLIENT_STYLE_ACCEPTED, 0, raw);
			raw.clear();
		}
		raw.insert(raw.end(), block.begin(), block.end());
	}
	reply(*sd, MOONCLIENT_STYLE_ACCEPTED, 0, raw);
}
