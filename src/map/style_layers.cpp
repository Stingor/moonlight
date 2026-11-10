// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les teintures de calques de moonclient. Voir style_layers.hpp.

#include "style_layers.hpp"

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <common/grfio.hpp> // encode_zip
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "itemdb.hpp"
#include "map.hpp" // mmysql_handle, map_id2nd, map_foreachinallrange
#include "npc.hpp"
#include "packets.hpp"
#include "path.hpp" // distance_xy
#include "pc.hpp"

namespace {

using bytes = std::vector<uint8>;

// La version du BLOB rangé en base : une suite d'entrées, telles qu'elles
// partent dans un ZC.
constexpr uint8 STORED_VERSION = 1;
// La fenêtre du plafond d'octets, en millisecondes.
constexpr t_tick BYTES_WINDOW_MS = 60000;
// Les profils de détection, et les rangs qu'ils emploient.
constexpr uint8 PROFILE_RAMPS = 1;
constexpr uint8 PROFILE_HUE_CLASSES = 2;
constexpr int32 RAMP_RANKS = 8;
// Une coupe dans le mot haut d'une clé de tête, une couleur dans le mot bas.
constexpr uint32 HEAD_KEY_SHIFT = 16;
constexpr uint32 HEAD_KEY_LOW = 0xFFFF;
// Un CZ ne décrit pas plus que ce qu'un personnage range, plus ses huit
// emplacements.
constexpr int32 REQUEST_ENTRIES_MAX = MOONCLIENT_STYLE_LAYERS_MAX + (MOONCLIENT_STYLE_SLOT_LAST - MOONCLIENT_STYLE_SLOT_FIRST + 1);
// Le plus d'exemplaires qu'un joueur affiche : trois coiffes et une cape.
constexpr int32 SHOWN_ITEMS_MAX = 4;
// Les octets d'en-tête d'un bloc : gid, nombre.
constexpr size_t BLOCK_HEAD = 5;
// L'en-tête d'un ZC.
constexpr size_t ZC_HEAD = sizeof(PACKET_ZC_MOONCLIENT_STYLE_LAYERS);

// Ce qu'une session tient en mémoire.
struct s_style_layer_state {
	bool loaded = false;
	bool load_failed = false;
	// La tête (une teinture par coupe et couleur) et la peau, la plus récemment
	// écrite au bout.
	std::vector<bytes> character;
	// Le dernier bloc diffusé à la zone, pour ne pas le redire.
	bytes last_block;
	bool broadcast = false;
	// La cadence et le plafond d'octets.
	t_tick next_allowed = 0;
	t_tick window_start = 0;
	uint32 window_bytes = 0;
	// L'atelier du styliste.
	bool atelier_open = false;
	int32 atelier_npc = 0;
	int16 atelier_m = -1;
	int16 atelier_x = 0;
	int16 atelier_y = 0;
	t_tick atelier_until = 0;
};

// La teinture d'un exemplaire : l'objet qu'elle teint, et son entrée.
struct s_item_dye {
	t_itemid nameid = 0;
	bytes entry;
};

std::unordered_map<uint32, s_style_layer_state> s_states; // par char_id
// Le cache des teintures d'exemplaires, par `unique_id`. Une absence se garde
// aussi : elle épargne une requête à chaque apparition.
std::unordered_map<uint64, std::optional<s_item_dye>> s_item_dyes;

// Un emplacement du CZ : où le chercher, et ce qu'il peint.
struct s_slot {
	equip_index eqi;
	uint32 eqp;
	uint8 layer;
};

std::optional<s_slot> slot_of(uint8 slot) {
	switch (slot) {
		case 10: return s_slot{ EQI_HEAD_TOP, EQP_HEAD_TOP, MOONCLIENT_STYLE_LAYER_HEADGEAR };
		case 11: return s_slot{ EQI_HEAD_MID, EQP_HEAD_MID, MOONCLIENT_STYLE_LAYER_HEADGEAR };
		case 12: return s_slot{ EQI_HEAD_LOW, EQP_HEAD_LOW, MOONCLIENT_STYLE_LAYER_HEADGEAR };
		case 13: return s_slot{ EQI_GARMENT, EQP_GARMENT, MOONCLIENT_STYLE_LAYER_GARMENT };
		case 14: return s_slot{ EQI_COSTUME_HEAD_TOP, EQP_COSTUME_HEAD_TOP, MOONCLIENT_STYLE_LAYER_HEADGEAR };
		case 15: return s_slot{ EQI_COSTUME_HEAD_MID, EQP_COSTUME_HEAD_MID, MOONCLIENT_STYLE_LAYER_HEADGEAR };
		case 16: return s_slot{ EQI_COSTUME_HEAD_LOW, EQP_COSTUME_HEAD_LOW, MOONCLIENT_STYLE_LAYER_HEADGEAR };
		case 17: return s_slot{ EQI_COSTUME_GARMENT, EQP_COSTUME_GARMENT, MOONCLIENT_STYLE_LAYER_GARMENT };
		default: return std::nullopt;
	}
}

// ── Les entrées ─────────────────────────────────────────────────────────────

uint32 read_u32(const uint8* p) {
	return static_cast<uint32>(p[0]) | (static_cast<uint32>(p[1]) << 8) | (static_cast<uint32>(p[2]) << 16) | (static_cast<uint32>(p[3]) << 24);
}

void write_u32(uint8* p, uint32 value) {
	p[0] = static_cast<uint8>(value & 0xFF);
	p[1] = static_cast<uint8>((value >> 8) & 0xFF);
	p[2] = static_cast<uint8>((value >> 16) & 0xFF);
	p[3] = static_cast<uint8>((value >> 24) & 0xFF);
}

int32 popcount16(uint16 bits) {
	int32 n = 0;
	for (; bits != 0; bits >>= 1)
		n += bits & 1;
	return n;
}

// Les champs d'en-tête d'une entrée.
struct s_entry_view {
	uint8 size;
	uint8 layer;
	uint32 key;
	uint8 profile;
	uint8 flags;
	uint16 mask;
};

s_entry_view view_of(const uint8* p) {
	return s_entry_view{ p[0], p[1], read_u32(p + 2), p[6], p[7], static_cast<uint16>(p[8] | (p[9] << 8)) };
}

// Les rangs qu'un calque admet sous un profil ; zéro s'il n'en admet aucun.
int32 ranks_for(uint8 layer, uint8 profile) {
	if (layer == MOONCLIENT_STYLE_LAYER_HEAD)
		return profile == PROFILE_RAMPS ? RAMP_RANKS : 0;
	if (layer == MOONCLIENT_STYLE_LAYER_SKIN)
		return profile == PROFILE_RAMPS ? 1 : 0;
	if (profile == PROFILE_RAMPS)
		return RAMP_RANKS;
	if (profile == PROFILE_HUE_CLASSES)
		return MOONCLIENT_STYLE_LAYER_ADJUSTS_MAX;
	return 0;
}

// Le calque que peint une entrée du CZ (1, 2, 3 ou 4), ou zéro.
uint8 painted_layer(uint8 wire_layer) {
	if (wire_layer == MOONCLIENT_STYLE_LAYER_HEAD || wire_layer == MOONCLIENT_STYLE_LAYER_SKIN)
		return wire_layer;
	const std::optional<s_slot> slot = slot_of(wire_layer);
	return slot ? slot->layer : 0;
}

// 🔴 LA STRUCTURE D'UNE ENTRÉE, et rien d'autre : la longueur exacte, le profil
// connu, le masque dans les rangs du profil, `absolu` à 0 ou 1, les drapeaux
// connus. `stored` : une entrée relue en base, qui ne porte que les calques du
// personnage et aucun drapeau.
e_moonclient_style_result check_entry(const uint8* p, size_t available, bool stored) {
	if (available < MOONCLIENT_STYLE_LAYER_ENTRY_HEAD)
		return MOONCLIENT_STYLE_MALFORMED;
	const s_entry_view e = view_of(p);
	const int32 adjusts = popcount16(e.mask);
	if (e.size != MOONCLIENT_STYLE_LAYER_ENTRY_HEAD + adjusts * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES || e.size > available)
		return MOONCLIENT_STYLE_MALFORMED;
	if ((e.flags & ~MOONCLIENT_STYLE_LAYER_FLAG_CLEAR) != 0 || (stored && e.flags != 0))
		return MOONCLIENT_STYLE_MALFORMED;
	const bool clear = (e.flags & MOONCLIENT_STYLE_LAYER_FLAG_CLEAR) != 0;
	if (clear && e.mask != 0)
		return MOONCLIENT_STYLE_MALFORMED;
	const uint8 layer = painted_layer(e.layer);
	if (layer == 0 || (stored && e.layer != MOONCLIENT_STYLE_LAYER_HEAD && e.layer != MOONCLIENT_STYLE_LAYER_SKIN))
		return MOONCLIENT_STYLE_KEY;
	const int32 ranks = ranks_for(layer, e.profile);
	if (ranks == 0 || (e.mask >> ranks) != 0)
		return MOONCLIENT_STYLE_MALFORMED;
	for (int32 i = 0; i < adjusts; ++i) {
		const uint8 absolute = p[MOONCLIENT_STYLE_LAYER_ENTRY_HEAD + i * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES + 4];
		if (absolute > 1)
			return MOONCLIENT_STYLE_MALFORMED;
	}
	// La clé, dans ses bornes.
	if (e.layer == MOONCLIENT_STYLE_LAYER_SKIN) {
		if (e.key != 0)
			return MOONCLIENT_STYLE_KEY;
	} else if (e.layer == MOONCLIENT_STYLE_LAYER_HEAD) {
		const int32 style = static_cast<int32>(e.key >> HEAD_KEY_SHIFT);
		const int32 color = static_cast<int32>(e.key & HEAD_KEY_LOW);
		if (style < MIN_HAIR_STYLE || style > MAX_HAIR_STYLE || color < MIN_HAIR_COLOR || color > MAX_HAIR_COLOR)
			return MOONCLIENT_STYLE_KEY;
	} else if (e.key == 0) {
		return MOONCLIENT_STYLE_KEY;
	}
	return MOONCLIENT_STYLE_ACCEPTED;
}

// Une suite d'entrées relue en base, ou rien si elle est illisible.
std::optional<std::vector<bytes>> parse_stored(const uint8* p, size_t length) {
	std::vector<bytes> entries;
	size_t at = 0;
	while (at < length) {
		if (check_entry(p + at, length - at, true) != MOONCLIENT_STYLE_ACCEPTED)
			return std::nullopt;
		const size_t size = p[at];
		entries.emplace_back(p + at, p + at + size);
		at += size;
	}
	return entries;
}

bytes join(const std::vector<bytes>& entries) {
	bytes out;
	for (const bytes& e : entries)
		out.insert(out.end(), e.begin(), e.end());
	return out;
}

// ── La base ─────────────────────────────────────────────────────────────────

s_style_layer_state& state_of(const map_session_data& sd) {
	return s_states[sd.status.char_id];
}

// La tête et la peau du personnage, lues une fois par session.
s_style_layer_state& loaded_state(const map_session_data& sd) {
	s_style_layer_state& state = state_of(sd);
	if (state.loaded)
		return state;
	state.loaded = true;
	state.load_failed = false;
	state.character.clear();
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `version`, `data` FROM `moonclient_style_layers` WHERE `char_id` = '%u'", sd.status.char_id)) {
		Sql_ShowDebug(mmysql_handle);
		// ⚠ ON RETENTERA : une base injoignable ne vaut pas « aucune teinture », et
		// une écriture sur cet état effacerait ce qui est rangé.
		state.loaded = false;
		state.load_failed = true;
		return state;
	}
	if (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		char* data = nullptr;
		size_t length = 0;
		Sql_GetData(mmysql_handle, 0, &data, nullptr);
		const uint32 version = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
		Sql_GetData(mmysql_handle, 1, &data, &length);
		std::optional<std::vector<bytes>> entries;
		if (version == STORED_VERSION && data != nullptr)
			entries = parse_stored(reinterpret_cast<const uint8*>(data), length);
		if (entries)
			state.character = std::move(*entries);
		else
			ShowWarning("style_layers: les teintures du personnage %u sont illisibles (version %u) : ignorees.\n", sd.status.char_id, version);
	}
	Sql_FreeResult(mmysql_handle);
	return state;
}

// La teinture d'un exemplaire, du cache ou de la base.
const std::optional<s_item_dye>& item_dye(uint64 unique_id) {
	auto found = s_item_dyes.find(unique_id);
	if (found != s_item_dyes.end())
		return found->second;
	std::optional<s_item_dye> dye;
	bool readable = true;
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `nameid`, `version`, `data` FROM `moonclient_item_dyes` WHERE `unique_id` = '%" PRIu64 "'", unique_id)) {
		Sql_ShowDebug(mmysql_handle);
		readable = false;
	} else {
		if (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
			char* data = nullptr;
			size_t length = 0;
			Sql_GetData(mmysql_handle, 0, &data, nullptr);
			const t_itemid nameid = data != nullptr ? static_cast<t_itemid>(strtoul(data, nullptr, 10)) : 0;
			Sql_GetData(mmysql_handle, 1, &data, nullptr);
			const uint32 version = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
			Sql_GetData(mmysql_handle, 2, &data, &length);
			if (version == STORED_VERSION && data != nullptr && length >= MOONCLIENT_STYLE_LAYER_ENTRY_HEAD && length <= MOONCLIENT_STYLE_LAYER_ENTRY_MAX) {
				const uint8* p = reinterpret_cast<const uint8*>(data);
				const s_entry_view e = view_of(p);
				// Une entrée rangée est celle d'un objet : coiffe ou cape, sans drapeau.
				if (e.size == length && e.flags == 0 && (e.layer == MOONCLIENT_STYLE_LAYER_HEADGEAR || e.layer == MOONCLIENT_STYLE_LAYER_GARMENT)
					&& ranks_for(e.layer, e.profile) != 0 && (e.mask >> ranks_for(e.layer, e.profile)) == 0
					&& e.size == MOONCLIENT_STYLE_LAYER_ENTRY_HEAD + popcount16(e.mask) * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES)
					dye = s_item_dye{ nameid, bytes(p, p + length) };
			}
			if (!dye)
				ShowWarning("style_layers: la teinture de l'exemplaire %" PRIu64 " est illisible : ignoree.\n", unique_id);
		}
		Sql_FreeResult(mmysql_handle);
	}
	// ⚠ UNE BASE INJOIGNABLE NE SE GARDE PAS : on redemandera.
	if (!readable) {
		static const std::optional<s_item_dye> none;
		return none;
	}
	return s_item_dyes[unique_id] = std::move(dye);
}

bool store_character(const map_session_data& sd, const std::vector<bytes>& entries) {
	if (entries.empty()) {
		if (SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `moonclient_style_layers` WHERE `char_id` = '%u'", sd.status.char_id)) {
			Sql_ShowDebug(mmysql_handle);
			return false;
		}
		return true;
	}
	bytes data = join(entries);
	SqlStmt stmt{ *mmysql_handle };
	uint32 char_id = sd.status.char_id;
	uint8 version = STORED_VERSION;
	if (SQL_SUCCESS != stmt.Prepare("REPLACE INTO `moonclient_style_layers` (`char_id`, `version`, `data`, `updated_at`) VALUES (?, ?, ?, NOW())")
	||  SQL_SUCCESS != stmt.BindParam(0, SQLDT_UINT32, &char_id, sizeof(char_id))
	||  SQL_SUCCESS != stmt.BindParam(1, SQLDT_UINT8, &version, sizeof(version))
	||  SQL_SUCCESS != stmt.BindParam(2, SQLDT_BLOB, data.data(), data.size())
	||  SQL_SUCCESS != stmt.Execute()) {
		SqlStmt_ShowDebug(stmt);
		return false;
	}
	return true;
}

// Une teinture d'exemplaire écrite ou retirée, et son journal. `author_char_id`
// et `source` disent qui.
bool store_item(uint64 unique_id, t_itemid nameid, const bytes* entry, const bytes* before, uint32 account_id, uint32 author_char_id, const char* source) {
	{
		SqlStmt stmt{ *mmysql_handle };
		uint64 uid = unique_id;
		uint32 item = nameid;
		uint8 version = STORED_VERSION;
		uint32 author = author_char_id;
		bool ok;
		if (entry != nullptr) {
			ok = SQL_SUCCESS == stmt.Prepare("REPLACE INTO `moonclient_item_dyes` (`unique_id`, `nameid`, `version`, `data`, `char_id`, `updated_at`) VALUES (?, ?, ?, ?, ?, NOW())")
			  && SQL_SUCCESS == stmt.BindParam(0, SQLDT_UINT64, &uid, sizeof(uid))
			  && SQL_SUCCESS == stmt.BindParam(1, SQLDT_UINT32, &item, sizeof(item))
			  && SQL_SUCCESS == stmt.BindParam(2, SQLDT_UINT8, &version, sizeof(version))
			  && SQL_SUCCESS == stmt.BindParam(3, SQLDT_BLOB, const_cast<uint8*>(entry->data()), entry->size())
			  && SQL_SUCCESS == stmt.BindParam(4, SQLDT_UINT32, &author, sizeof(author))
			  && SQL_SUCCESS == stmt.Execute();
		} else {
			ok = SQL_SUCCESS == stmt.Prepare("DELETE FROM `moonclient_item_dyes` WHERE `unique_id` = ?")
			  && SQL_SUCCESS == stmt.BindParam(0, SQLDT_UINT64, &uid, sizeof(uid))
			  && SQL_SUCCESS == stmt.Execute();
		}
		if (!ok) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}
	}
	// 🔴 LE JOURNAL : qui a teint quel exemplaire, avant et après.
	SqlStmt log{ *mmysql_handle };
	uint64 uid = unique_id;
	uint32 item = nameid;
	uint32 account = account_id;
	uint32 author = author_char_id;
	bytes empty;
	const bytes& was = before != nullptr ? *before : empty;
	const bytes& now = entry != nullptr ? *entry : empty;
	if (SQL_SUCCESS != log.Prepare("INSERT INTO `moonclient_item_dye_log` (`logged_at`, `account_id`, `char_id`, `unique_id`, `nameid`, `source`, `before_data`, `after_data`) VALUES (NOW(), ?, ?, ?, ?, ?, ?, ?)")
	||  SQL_SUCCESS != log.BindParam(0, SQLDT_UINT32, &account, sizeof(account))
	||  SQL_SUCCESS != log.BindParam(1, SQLDT_UINT32, &author, sizeof(author))
	||  SQL_SUCCESS != log.BindParam(2, SQLDT_UINT64, &uid, sizeof(uid))
	||  SQL_SUCCESS != log.BindParam(3, SQLDT_UINT32, &item, sizeof(item))
	||  SQL_SUCCESS != log.BindParam(4, SQLDT_STRING, const_cast<char*>(source), strlen(source))
	||  SQL_SUCCESS != log.BindParam(5, SQLDT_BLOB, const_cast<uint8*>(was.data()), was.size())
	||  SQL_SUCCESS != log.BindParam(6, SQLDT_BLOB, const_cast<uint8*>(now.data()), now.size())
	||  SQL_SUCCESS != log.Execute())
		SqlStmt_ShowDebug(log);
	ShowInfo("style_layers: exemplaire %" PRIu64 " (objet %u) %s par le personnage %u (%s).\n", unique_id, nameid, entry != nullptr ? "teint" : "deteint", author_char_id, source);
	return true;
}

// ── Ce qui s'affiche ────────────────────────────────────────────────────────

// L'exemplaire qu'un joueur AFFICHE à un emplacement, ou -1. Le costume l'emporte,
// sauf sur une carte sans costume ou s'il masque les siens ; et l'objet doit
// porter l'apparence montrée, qu'un script peut avoir remplacée.
int32 shown_index(const map_session_data& sd, equip_index normal, equip_index costume, int32 shown_view) {
	if (shown_view == 0)
		return -1;
	const bool costumes = !map_getmapflag(sd.m, MF_NOCOSTUME) && !sd.status.disable_showcostumes;
	for (const equip_index eqi : { costume, normal }) {
		if (eqi == costume && !costumes)
			continue;
		const int16 idx = sd.equip_index[eqi];
		if (idx < 0 || sd.inventory_data[idx] == nullptr)
			continue;
		if (static_cast<int32>(sd.inventory_data[idx]->look) == shown_view)
			return idx;
	}
	return -1;
}

// Le bloc d'un joueur : sa tête, sa peau, et ses exemplaires affichés.
bytes compose_block(map_session_data& owner) {
	bytes block(BLOCK_HEAD, 0);
	write_u32(block.data(), static_cast<uint32>(owner.id));
	uint8 count = 0;
	const s_style_layer_state& state = loaded_state(owner);
	const uint32 head_key = (static_cast<uint32>(owner.status.hair) << HEAD_KEY_SHIFT) | (static_cast<uint32>(owner.status.hair_color) & HEAD_KEY_LOW);
	for (const bytes& e : state.character) {
		const s_entry_view v = view_of(e.data());
		const bool shown = (v.layer == MOONCLIENT_STYLE_LAYER_HEAD && v.key == head_key) || v.layer == MOONCLIENT_STYLE_LAYER_SKIN;
		if (!shown)
			continue;
		block.insert(block.end(), e.begin(), e.end());
		++count;
	}
	struct s_shown { equip_index normal; equip_index costume; int32 view; };
	const s_shown shown[SHOWN_ITEMS_MAX] = {
		{ EQI_HEAD_TOP, EQI_COSTUME_HEAD_TOP, owner.status.head_top },
		{ EQI_HEAD_MID, EQI_COSTUME_HEAD_MID, owner.status.head_mid },
		{ EQI_HEAD_LOW, EQI_COSTUME_HEAD_LOW, owner.status.head_bottom },
		{ EQI_GARMENT, EQI_COSTUME_GARMENT, owner.status.robe },
	};
	std::unordered_set<int32> seen;
	for (const s_shown& s : shown) {
		const int32 idx = shown_index(owner, s.normal, s.costume, s.view);
		// Un objet qui couvre deux emplacements ne s'affiche qu'une fois.
		if (idx < 0 || !seen.insert(idx).second)
			continue;
		const item& it = owner.inventory.u.items_inventory[idx];
		if (it.unique_id == 0)
			continue;
		const std::optional<s_item_dye>& dye = item_dye(it.unique_id);
		// 🔴 L'IDENTITÉ SE RECOUPE : un `unique_id` réemployé ne repeint pas un autre objet.
		if (!dye || dye->nameid != it.nameid)
			continue;
		bytes e = dye->entry;
		// La teinture voyage sous l'apparence AFFICHÉE.
		write_u32(e.data() + 2, static_cast<uint32>(s.view));
		block.insert(block.end(), e.begin(), e.end());
		++count;
	}
	block[4] = count;
	return block;
}

// Une trame de ZC autour d'une charge, compressée au-delà du seuil.
bytes frame_of(const bytes& raw) {
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
	bytes frame(ZC_HEAD + body.size());
	PACKET_ZC_MOONCLIENT_STYLE_LAYERS* p = reinterpret_cast<PACKET_ZC_MOONCLIENT_STYLE_LAYERS*>(frame.data());
	p->packetType = HEADER_ZC_MOONCLIENT_STYLE_LAYERS;
	p->packetLength = static_cast<int16>(frame.size());
	p->flags = flags;
	p->raw_size = static_cast<uint16>(raw.size());
	memcpy(frame.data() + ZC_HEAD, body.data(), body.size());
	return frame;
}

void send_frame(const map_session_data& sd, const bytes& frame) {
	const int32 fd = sd.fd;
	if (!session_isActive(fd))
		return;
	WFIFOHEAD(fd, frame.size());
	memcpy(WFIFOP(fd, 0), frame.data(), frame.size());
	WFIFOSET(fd, frame.size());
}

// Le bloc de `owner`, à un joueur en vue (callback de map_foreachinallrange et
// map_foreachinviewring). va_arg : le propriétaire, la trame, 1 si le
// propriétaire la reçoit aussi.
int32 send_area_sub(block_list* bl, va_list ap) {
	map_session_data* tsd = BL_CAST(BL_PC, bl);
	const map_session_data* owner = va_arg(ap, const map_session_data*);
	const bytes* frame = va_arg(ap, const bytes*);
	const int32 with_self = va_arg(ap, int32);
	if (tsd == nullptr || owner == nullptr)
		return 0;
	if (tsd == owner && with_self == 0)
		return 0;
	// 🔴 DESTINATAIRE PAR DESTINATAIRE, jamais clif_send(AREA) : un client qui
	// n'a pas le bit ne connaît pas l'opcode.
	if (!style_layers_receiver(*tsd))
		return 0;
	if (tsd != owner && clif_hidden_unit_withheld(*owner, *tsd))
		return 0;
	send_frame(*tsd, *frame);
	return 1;
}

void send_area(map_session_data& owner, const bytes& block, bool with_self) {
	const bytes frame = frame_of(block);
	map_foreachinallrange(send_area_sub, &owner, AREA_SIZE, BL_PC, static_cast<const map_session_data*>(&owner), &frame, static_cast<int32>(with_self));
	map_foreachinviewring(send_area_sub, &owner, 0, static_cast<const map_session_data*>(&owner), &frame, static_cast<int32>(with_self));
	s_style_layer_state& state = state_of(owner);
	state.last_block = block;
	state.broadcast = true;
}

// Rassemble les joueurs en vue de `self` qui ne lui sont pas cachés.
int32 collect_sub(block_list* bl, va_list ap) {
	map_session_data* owner = BL_CAST(BL_PC, bl);
	std::vector<map_session_data*>* out = va_arg(ap, std::vector<map_session_data*>*);
	const map_session_data* self = va_arg(ap, const map_session_data*);
	if (owner == nullptr || owner == self || owner->state.spectator)
		return 0;
	if (clif_hidden_unit_withheld(*owner, *self))
		return 0;
	out->push_back(owner);
	return 1;
}

// ── L'atelier ───────────────────────────────────────────────────────────────

bool atelier_open(const map_session_data& sd, const s_style_layer_state& state) {
	if (!state.atelier_open || DIFF_TICK(state.atelier_until, gettick()) <= 0)
		return false;
	const npc_data* nd = map_id2nd(state.atelier_npc);
	if (nd == nullptr || nd->m != state.atelier_m || sd.m != state.atelier_m)
		return false;
	return distance_xy(sd.x, sd.y, state.atelier_x, state.atelier_y) <= MOONCLIENT_STYLE_ATELIER_CELLS;
}

void close_atelier(map_session_data& sd, s_style_layer_state& state) {
	if (!state.atelier_open)
		return;
	state.atelier_open = false;
	style_layers_verdict(sd, MOONCLIENT_STYLE_SUBJECT_ATELIER, MOONCLIENT_STYLE_ATELIER_CLOSED, 0, 0);
}

// Un état où rien ne se teint : la mort, un échange, une boutique ou un storage
// ouverts, un dialogue avec un autre PNJ que le styliste.
bool busy(const map_session_data& sd, const s_style_layer_state& state) {
	if (pc_isdead(&sd) || sd.trade_partner.id != 0)
		return true;
	if (pc_cant_act2(const_cast<map_session_data*>(&sd)))
		return true;
	return sd.npc_id != 0 && !(state.atelier_open && sd.npc_id == state.atelier_npc);
}

// Une entrée du CZ, lue et jugée.
struct s_request_entry {
	const uint8* p;
	s_entry_view view;
};

} // namespace

bool style_layers_receiver(const map_session_data& sd) {
	return sd.state.has_bourgeon && (sd.bourgeon_ui_caps & MOONLIGHT_UI_STYLE_LAYERS) != 0;
}

void style_layers_verdict(map_session_data& sd, uint8 subject, uint8 result, uint16 wait_ms, uint32 detail) {
	if (!style_layers_receiver(sd))
		return;
	PACKET_ZC_MOONCLIENT_STYLE_VERDICT p{};
	p.packetType = HEADER_ZC_MOONCLIENT_STYLE_VERDICT;
	p.packetLength = sizeof(p);
	p.subject = subject;
	p.result = result;
	p.wait_ms = wait_ms;
	p.detail = detail;
	const int32 fd = sd.fd;
	if (!session_isActive(fd))
		return;
	WFIFOHEAD(fd, sizeof(p));
	memcpy(WFIFOP(fd, 0), &p, sizeof(p));
	WFIFOSET(fd, sizeof(p));
}

void style_layers_handshake(map_session_data& sd) {
	if (!style_layers_receiver(sd))
		return;
	s_style_layer_state& state = state_of(sd);
	// L'atelier ne survit pas à un changement de carte.
	if (state.atelier_open && sd.m != state.atelier_m)
		close_atelier(sd, state);
	else if (state.atelier_open)
		style_layers_verdict(sd, MOONCLIENT_STYLE_SUBJECT_ATELIER, MOONCLIENT_STYLE_ATELIER_OPEN, 0,
			static_cast<uint32>(std::max<t_tick>(0, DIFF_TICK(state.atelier_until, gettick()))));

	// Le sien, à lui et aux autres : ils ne l'ont peut-être pas encore.
	const bytes own = compose_block(sd);
	send_area(sd, own, true);

	// Ceux des autres, à lui, par trames de MOONCLIENT_STYLE_LAYERS_RAW_MAX au plus.
	std::vector<map_session_data*> seen;
	map_foreachinallrange(collect_sub, &sd, pc_view_range(sd), BL_PC, &seen, static_cast<const map_session_data*>(&sd));
	bytes raw;
	for (map_session_data* owner : seen) {
		const bytes block = compose_block(*owner);
		if (block[4] == 0)
			continue; // rien à dire de celui-là
		// ⚠ UN JOUEUR N'EST JAMAIS COUPÉ ENTRE DEUX TRAMES.
		if (raw.size() + block.size() > MOONCLIENT_STYLE_LAYERS_RAW_MAX) {
			send_frame(sd, frame_of(raw));
			raw.clear();
		}
		raw.insert(raw.end(), block.begin(), block.end());
	}
	if (!raw.empty())
		send_frame(sd, frame_of(raw));
}

void style_layers_spawn(map_session_data& owner) {
	const bytes block = compose_block(owner);
	// Rien à dire, et rien n'a été dit : personne n'a de teinture à oublier.
	if (block[4] == 0 && !state_of(owner).broadcast)
		return;
	send_area(owner, block, false);
}

void style_layers_single(map_session_data& sd, map_session_data& owner) {
	if (!style_layers_receiver(sd) || &sd == &owner)
		return;
	const bytes block = compose_block(owner);
	if (block[4] == 0)
		return;
	send_frame(sd, frame_of(block));
}

void style_layers_look_changed(map_session_data& owner) {
	if (owner.prev == nullptr)
		return; // pas encore sur la carte : son apparition le dira
	const bytes block = compose_block(owner);
	const s_style_layer_state& state = state_of(owner);
	if (state.broadcast ? state.last_block == block : block[4] == 0)
		return;
	send_area(owner, block, true);
}

void style_layers_atelier(map_session_data& sd, const npc_data& nd, uint32 duration_s) {
	s_style_layer_state& state = state_of(sd);
	if (duration_s == 0) {
		close_atelier(sd, state);
		return;
	}
	const uint32 seconds = std::min<uint32>(duration_s, MOONCLIENT_STYLE_ATELIER_MAX_S);
	state.atelier_open = true;
	state.atelier_npc = nd.id;
	state.atelier_m = nd.m;
	state.atelier_x = nd.x;
	state.atelier_y = nd.y;
	state.atelier_until = gettick() + static_cast<t_tick>(seconds) * 1000;
	style_layers_verdict(sd, MOONCLIENT_STYLE_SUBJECT_ATELIER, MOONCLIENT_STYLE_ATELIER_OPEN, 0, seconds * 1000);
}

void style_layers_on_logout(const map_session_data& sd) {
	s_states.erase(sd.status.char_id);
}

void style_layers_final() {
	s_states.clear();
	s_item_dyes.clear();
}

// ── Le CZ ───────────────────────────────────────────────────────────────────

namespace {

// Le jugement complet d'une trame. Rend le résultat ; n'écrit rien qu'accepté.
e_moonclient_style_result apply_request(map_session_data& sd, const uint8* frame, size_t length, uint16& wait_ms) {
	s_style_layer_state& state = state_of(sd);
	const t_tick now = gettick();

	// ── Le plafond d'octets : chaque trame compte, refusée ou non ──────────
	if (state.window_start == 0 || DIFF_TICK(now, state.window_start) >= BYTES_WINDOW_MS) {
		state.window_start = now;
		state.window_bytes = 0;
	}
	state.window_bytes += static_cast<uint32>(length);
	if (state.window_bytes > MOONCLIENT_STYLE_LAYERS_BYTES_PER_MINUTE) {
		wait_ms = static_cast<uint16>(std::min<t_tick>(UINT16_MAX, std::max<t_tick>(0, BYTES_WINDOW_MS - DIFF_TICK(now, state.window_start))));
		return MOONCLIENT_STYLE_CAP;
	}

	// ── La structure ───────────────────────────────────────────────────────
	if (length < sizeof(PACKET_CZ_MOONCLIENT_STYLE_LAYERS) || length > MOONCLIENT_STYLE_LAYERS_CZ_MAX)
		return MOONCLIENT_STYLE_MALFORMED;
	const PACKET_CZ_MOONCLIENT_STYLE_LAYERS* head = reinterpret_cast<const PACKET_CZ_MOONCLIENT_STYLE_LAYERS*>(frame);
	if ((head->flags & ~MOONCLIENT_STYLE_LAYERS_CLEAR_CHARACTER) != 0)
		return MOONCLIENT_STYLE_MALFORMED;
	const bool clear_character = (head->flags & MOONCLIENT_STYLE_LAYERS_CLEAR_CHARACTER) != 0;
	if (head->count > REQUEST_ENTRIES_MAX)
		return MOONCLIENT_STYLE_CAP;
	if (head->count == 0 && !clear_character)
		return MOONCLIENT_STYLE_MALFORMED;
	std::vector<s_request_entry> entries;
	size_t at = sizeof(PACKET_CZ_MOONCLIENT_STYLE_LAYERS);
	for (int32 i = 0; i < head->count; ++i) {
		const e_moonclient_style_result checked = check_entry(frame + at, length - at, false);
		if (checked != MOONCLIENT_STYLE_ACCEPTED)
			return checked;
		const s_entry_view view = view_of(frame + at);
		// Un calque, une entrée : la même tête, la peau ou le même emplacement deux fois.
		for (const s_request_entry& other : entries) {
			const bool same_target = other.view.layer == view.layer
				&& (view.layer != MOONCLIENT_STYLE_LAYER_HEAD || other.view.key == view.key);
			if (same_target)
				return MOONCLIENT_STYLE_KEY;
		}
		entries.push_back(s_request_entry{ frame + at, view });
		at += view.size;
	}
	if (at != length)
		return MOONCLIENT_STYLE_MALFORMED;

	// ── L'état du joueur ───────────────────────────────────────────────────
	if (busy(sd, state))
		return MOONCLIENT_STYLE_BUSY;
	const s_style_layer_state& loaded = loaded_state(sd);
	if (loaded.load_failed)
		return MOONCLIENT_STYLE_UNAVAILABLE;

	// ── Les objets : portés ici, maintenant, et à l'atelier ────────────────
	struct s_item_change {
		int16 idx;
		uint64 unique_id;
		t_itemid nameid;
		std::optional<bytes> entry; // rien : retirer
	};
	std::vector<s_item_change> items;
	for (const s_request_entry& e : entries) {
		const std::optional<s_slot> slot = slot_of(e.view.layer);
		if (!slot)
			continue;
		// D19 : une teinture d'objet ne se valide qu'auprès du styliste.
		if (!atelier_open(sd, state))
			return MOONCLIENT_STYLE_NO_ATELIER;
		// 🔴 L'OBJET SE TROUVE DANS L'ÉQUIPEMENT DE LA SESSION ÉMETTRICE, et nulle
		// part ailleurs : jamais d'identifiant global lu du client.
		const int16 idx = sd.equip_index[slot->eqi];
		if (idx < 0 || idx >= MAX_INVENTORY || sd.inventory_data[idx] == nullptr)
			return MOONCLIENT_STYLE_NOT_WORN;
		const item& it = sd.inventory.u.items_inventory[idx];
		if (it.nameid != e.view.key || (it.equip & slot->eqp) == 0 || it.unique_id == 0 || sd.inventory_data[idx]->type != IT_ARMOR)
			return MOONCLIENT_STYLE_NOT_WORN;
		// Un objet qui couvre deux emplacements ne se teint qu'une fois par trame.
		for (const s_item_change& other : items)
			if (other.idx == idx)
				return MOONCLIENT_STYLE_KEY;
		s_item_change change{ idx, it.unique_id, it.nameid, std::nullopt };
		if ((e.view.flags & MOONCLIENT_STYLE_LAYER_FLAG_CLEAR) == 0 && e.view.mask != 0) {
			bytes entry(e.p, e.p + e.view.size);
			entry[1] = slot->layer;
			entry[7] = 0;
			change.entry = std::move(entry);
		}
		items.push_back(std::move(change));
	}

	// ── La tête et la peau ─────────────────────────────────────────────────
	std::vector<bytes> character = clear_character ? std::vector<bytes>{} : loaded.character;
	const uint32 worn_head = (static_cast<uint32>(sd.status.hair) << HEAD_KEY_SHIFT) | (static_cast<uint32>(sd.status.hair_color) & HEAD_KEY_LOW);
	for (const s_request_entry& e : entries) {
		if (slot_of(e.view.layer))
			continue;
		// La place qu'elle prend : la même tête, ou la peau.
		for (size_t i = 0; i < character.size(); ++i) {
			const s_entry_view v = view_of(character[i].data());
			if (v.layer == e.view.layer && v.key == e.view.key) {
				character.erase(character.begin() + i);
				break;
			}
		}
		if ((e.view.flags & MOONCLIENT_STYLE_LAYER_FLAG_CLEAR) != 0 || e.view.mask == 0)
			continue;
		bytes entry(e.p, e.p + e.view.size);
		entry[7] = 0;
		character.push_back(std::move(entry));
		// Au plafond, la plus ancienne teinture de cheveux qui dort part ; jamais
		// celle qu'on porte, ni la peau.
		while (character.size() > MOONCLIENT_STYLE_LAYERS_MAX) {
			auto sleeping = std::find_if(character.begin(), character.end(), [worn_head](const bytes& b) {
				const s_entry_view v = view_of(b.data());
				return v.layer == MOONCLIENT_STYLE_LAYER_HEAD && v.key != worn_head;
			});
			if (sleeping == character.end())
				return MOONCLIENT_STYLE_CAP;
			character.erase(sleeping);
		}
	}

	// ── Un renvoi identique ne fait rien ───────────────────────────────────
	// ⚠ OCTET À OCTET : comparer ne collisionne jamais.
	bool same = character == loaded.character;
	for (const s_item_change& change : items) {
		const std::optional<s_item_dye>& now_dye = item_dye(change.unique_id);
		const bool had = now_dye.has_value() && now_dye->nameid == change.nameid;
		if (change.entry ? !(had && now_dye->entry == *change.entry) : had)
			same = false;
	}
	if (same)
		return MOONCLIENT_STYLE_SAME;

	// ── La cadence ─────────────────────────────────────────────────────────
	if (state.next_allowed != 0 && DIFF_TICK(state.next_allowed, now) > 0) {
		wait_ms = static_cast<uint16>(std::min<t_tick>(UINT16_MAX, DIFF_TICK(state.next_allowed, now)));
		return MOONCLIENT_STYLE_TOO_SOON;
	}
	state.next_allowed = now + MOONCLIENT_STYLE_LAYERS_CADENCE_MS;

	// ── L'écriture, d'un seul tenant ───────────────────────────────────────
	// Les teintures d'avant, copiées : le cache peut bouger pendant l'écriture.
	std::vector<std::optional<bytes>> befores;
	for (const s_item_change& change : items) {
		const std::optional<s_item_dye>& before = item_dye(change.unique_id);
		befores.push_back(before ? std::optional<bytes>(before->entry) : std::nullopt);
	}
	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, "START TRANSACTION")) {
		Sql_ShowDebug(mmysql_handle);
		return MOONCLIENT_STYLE_UNAVAILABLE;
	}
	bool written = character == loaded.character || store_character(sd, character);
	for (size_t i = 0; written && i < items.size(); ++i) {
		const s_item_change& change = items[i];
		written = store_item(change.unique_id, change.nameid, change.entry ? &*change.entry : nullptr,
			befores[i] ? &*befores[i] : nullptr, sd.status.account_id, sd.status.char_id, "joueur");
	}
	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, written ? "COMMIT" : "ROLLBACK")) {
		Sql_ShowDebug(mmysql_handle);
		written = false;
	}
	if (!written)
		return MOONCLIENT_STYLE_UNAVAILABLE;
	state.character = std::move(character);
	for (s_item_change& change : items) {
		if (change.entry)
			s_item_dyes[change.unique_id] = s_item_dye{ change.nameid, std::move(*change.entry) };
		else
			s_item_dyes[change.unique_id] = std::nullopt;
	}
	return MOONCLIENT_STYLE_ACCEPTED;
}

} // namespace

void clif_parse_moonclient_style_layers(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!style_layers_receiver(*sd))
		return;
	const size_t length = RFIFOW(fd, 2);
	uint16 wait_ms = 0;
	const e_moonclient_style_result result = apply_request(*sd, RFIFOP(fd, 0), length, wait_ms);
	style_layers_verdict(*sd, MOONCLIENT_STYLE_SUBJECT_LAYERS, result, wait_ms, 0);
	// L'émetteur reçoit son bloc avec la zone : c'est son écho.
	if (result == MOONCLIENT_STYLE_ACCEPTED && sd->prev != nullptr)
		send_area(*sd, compose_block(*sd), true);
}

// ── Staff ───────────────────────────────────────────────────────────────────

std::vector<std::string> style_layers_describe(map_session_data& sd) {
	std::vector<std::string> lines;
	const s_style_layer_state& state = loaded_state(sd);
	if (state.load_failed) {
		lines.emplace_back("teintures du personnage : base injoignable");
		return lines;
	}
	char line[CHAT_SIZE_MAX];
	for (const bytes& e : state.character) {
		const s_entry_view v = view_of(e.data());
		if (v.layer == MOONCLIENT_STYLE_LAYER_SKIN)
			safesnprintf(line, sizeof(line), "peau : %d reglage(s)", popcount16(v.mask));
		else
			safesnprintf(line, sizeof(line), "cheveux, coupe %u couleur %u : %d reglage(s)", v.key >> HEAD_KEY_SHIFT, v.key & HEAD_KEY_LOW, popcount16(v.mask));
		lines.emplace_back(line);
	}
	for (uint8 slot = MOONCLIENT_STYLE_SLOT_FIRST; slot <= MOONCLIENT_STYLE_SLOT_LAST; ++slot) {
		const std::optional<s_slot> s = slot_of(slot);
		const int16 idx = sd.equip_index[s->eqi];
		if (idx < 0 || sd.inventory_data[idx] == nullptr)
			continue;
		const item& it = sd.inventory.u.items_inventory[idx];
		if (it.unique_id == 0)
			continue;
		const std::optional<s_item_dye>& dye = item_dye(it.unique_id);
		if (!dye)
			continue;
		safesnprintf(line, sizeof(line), "emplacement %u : objet %u, exemplaire %" PRIu64 "%s, %d reglage(s)", slot, it.nameid, it.unique_id,
			dye->nameid == it.nameid ? "" : " (AUTRE OBJET : ignoree)", popcount16(view_of(dye->entry.data()).mask));
		lines.emplace_back(line);
	}
	if (lines.empty())
		lines.emplace_back("aucune teinture de calque");
	return lines;
}

bool style_layers_staff_clear_character(map_session_data& sd, const map_session_data& staff) {
	s_style_layer_state& state = loaded_state(sd);
	if (state.load_failed || !store_character(sd, {}))
		return false;
	state.character.clear();
	ShowInfo("style_layers: teintures du personnage %u retirees par %s (%u).\n", sd.status.char_id, staff.status.name, staff.status.char_id);
	style_layers_look_changed(sd);
	return true;
}

std::string style_layers_describe_item(uint64 unique_id) {
	const std::optional<s_item_dye>& dye = item_dye(unique_id);
	if (!dye)
		return std::string();
	char line[CHAT_SIZE_MAX];
	safesnprintf(line, sizeof(line), "exemplaire %" PRIu64 " : objet %u, %d reglage(s), profil %u", unique_id, dye->nameid,
		popcount16(view_of(dye->entry.data()).mask), view_of(dye->entry.data()).profile);
	return line;
}

bool style_layers_staff_clear_item(uint64 unique_id, const map_session_data& staff) {
	const std::optional<s_item_dye>& dye = item_dye(unique_id);
	if (!dye)
		return false;
	const s_item_dye before = *dye;
	if (!store_item(unique_id, before.nameid, nullptr, &before.entry, staff.status.account_id, staff.status.char_id, "staff"))
		return false;
	s_item_dyes[unique_id] = std::nullopt;
	return true;
}
