// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les tenues de moonclient. Voir style_outfits.hpp.

#include "style_outfits.hpp"

#include <algorithm>
#include <cstring>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/timer.hpp>

#include "battle.hpp" // les bornes de coupe, de couleur et de teinte
#include "clif.hpp"
#include "map.hpp" // mmysql_handle
#include "packets.hpp"
#include "pc.hpp"
#include "style_layers.hpp"

namespace {

using bytes = std::vector<uint8>;

// La fenêtre du plafond d'octets, en millisecondes.
constexpr t_tick BYTES_WINDOW_MS = 60000;
// L'alphabet des codes : ni 0 ni O, ni 1 ni I ni L, pour qu'un code recopié à la
// main ne se lise pas de deux façons. 31 signes, huit places : 8,5 × 10^11 codes.
constexpr char CODE_ALPHABET[] = "23456789ABCDEFGHJKMNPQRSTUVWXYZ";
constexpr int32 CODE_ALPHABET_SIZE = sizeof(CODE_ALPHABET) - 1;
// Les tirages d'un code neuf avant d'abandonner : une collision est rarissime,
// une base qui refuse tout ne doit pas faire tourner la boucle.
constexpr int32 CODE_ATTEMPTS = 8;
// Les octets d'un corps dans une tenue : clé, palette, palette de cheveux, coupe.
constexpr size_t BODY_KEY_BYTES = 4;
constexpr size_t BODY_NUMBER_BYTES = 2;
// Un réglage de la v7 : teinte (2), saturation, luminosité, absolu.
constexpr size_t ADJUST_BYTES = 5;
constexpr size_t ADJUST_ABSOLUTE = 4;
// Le premier octet imprimable d'un nom, et le DEL.
constexpr uint8 NAME_FIRST_PRINTABLE = 0x20;
constexpr uint8 NAME_DELETE = 0x7F;
// La plus grosse demande : l'en-tête, un nom, une tenue.
constexpr size_t REQUEST_MAX = sizeof(PACKET_CZ_MOONCLIENT_OUTFIT) + MOONCLIENT_OUTFIT_NAME_MAX + MOONCLIENT_OUTFIT_BYTES_MAX;
// Un rangement dans la liste : rangement, longueur du nom, longueur de la tenue.
constexpr size_t LISTED_HEAD = 1 + 1 + 2;

static_assert(MOONCLIENT_OUTFIT_BODY_BYTES == BODY_KEY_BYTES + 3 * BODY_NUMBER_BYTES + BOURGEON_STYLE_ADJUST_BYTES,
	"un corps de tenue ne vaut plus une entrée de la v7 sans gid");
static_assert(sizeof(PACKET_ZC_MOONCLIENT_OUTFIT) + 1 + MOONCLIENT_WARDROBE_SLOTS * (LISTED_HEAD + MOONCLIENT_OUTFIT_NAME_MAX + MOONCLIENT_OUTFIT_BYTES_MAX) < INT16_MAX,
	"la garde-robe pleine doit tenir dans une trame");

// La cadence et le plafond d'octets d'une session, par char_id.
struct s_outfit_state {
	t_tick next_allowed = 0;
	t_tick window_start = 0;
	uint32 window_bytes = 0;
};
std::unordered_map<uint32, s_outfit_state> s_states;

uint16 read_u16(const uint8* p) {
	return static_cast<uint16>(p[0] | (p[1] << 8));
}

int16 read_i16(const uint8* p) {
	return static_cast<int16>(read_u16(p));
}

uint32 read_u32(const uint8* p) {
	return static_cast<uint32>(p[0]) | (static_cast<uint32>(p[1]) << 8) | (static_cast<uint32>(p[2]) << 16) | (static_cast<uint32>(p[3]) << 24);
}

// Moins un (« d'origine ») ou un numéro dans ses bornes.
bool number_ok(int16 value, int32 low, int32 high) {
	return value == -1 || (value >= low && value <= high);
}

// 🔴 LA STRUCTURE D'UNE TENUE, et rien d'autre : la version, les corps (clés
// distinctes et non nulles, numéros dans les bornes du serveur, `absolu` à 0
// ou 1), le repli parmi les clés, les entrées de calques à la forme du ZC, sans
// doublon, et la longueur EXACTE.
bool check_outfit(const uint8* p, size_t length) {
	if (length < MOONCLIENT_OUTFIT_HEAD || length > MOONCLIENT_OUTFIT_BYTES_MAX)
		return false;
	if (p[0] != MOONCLIENT_OUTFIT_VERSION)
		return false;
	const size_t bodies = p[1];
	const uint32 fallback = read_u32(p + 2);
	if (bodies > BOURGEON_STYLE_MAX_VARIANTS)
		return false;
	size_t at = MOONCLIENT_OUTFIT_HEAD;
	if (length < at + bodies * MOONCLIENT_OUTFIT_BODY_BYTES + 1)
		return false;
	std::vector<uint32> keys;
	for (size_t i = 0; i < bodies; ++i, at += MOONCLIENT_OUTFIT_BODY_BYTES) {
		const uint8* body = p + at;
		const uint32 key = read_u32(body);
		if (key == 0 || std::find(keys.begin(), keys.end(), key) != keys.end())
			return false;
		keys.push_back(key);
		const uint8* numbers = body + BODY_KEY_BYTES;
		if (!number_ok(read_i16(numbers), MIN_CLOTH_COLOR, MAX_CLOTH_COLOR)
			|| !number_ok(read_i16(numbers + BODY_NUMBER_BYTES), MIN_HAIR_COLOR, MAX_HAIR_COLOR)
			|| !number_ok(read_i16(numbers + 2 * BODY_NUMBER_BYTES), MIN_HAIR_STYLE, MAX_HAIR_STYLE))
			return false;
		const uint8* adjusts = numbers + 3 * BODY_NUMBER_BYTES;
		for (size_t a = 0; a < BOURGEON_STYLE_ADJUST_BYTES; a += ADJUST_BYTES)
			if (adjusts[a + ADJUST_ABSOLUTE] > 1)
				return false;
	}
	if (fallback != 0 && std::find(keys.begin(), keys.end(), fallback) == keys.end())
		return false;
	const size_t count = p[at++];
	if (count > MOONCLIENT_OUTFIT_ENTRIES_MAX || bodies + count == 0)
		return false;
	std::vector<std::pair<uint8, uint32>> seen;
	for (size_t i = 0; i < count; ++i) {
		const size_t size = style_layers_check_shown_entry(p + at, length - at);
		if (size == 0)
			return false;
		const std::pair<uint8, uint32> target{ p[at + 1], read_u32(p + at + 2) };
		if (std::find(seen.begin(), seen.end(), target) != seen.end())
			return false;
		seen.push_back(target);
		at += size;
	}
	return at == length;
}

// Un code tel qu'il voyage : huit signes de l'alphabet, en majuscules.
bool check_code(const char* code) {
	for (int32 i = 0; i < MOONCLIENT_OUTFIT_CODE_LENGTH; ++i)
		if (code[i] == '\0' || std::strchr(CODE_ALPHABET, code[i]) == nullptr)
			return false;
	return true;
}

bool check_name(const uint8* name, size_t length) {
	if (length == 0 || length > MOONCLIENT_OUTFIT_NAME_MAX)
		return false;
	for (size_t i = 0; i < length; ++i)
		if (name[i] < NAME_FIRST_PRINTABLE || name[i] == NAME_DELETE)
			return false;
	return true;
}

// ── La réponse ──────────────────────────────────────────────────────────────

void reply(map_session_data& sd, uint8 action, uint8 result, uint16 wait_ms, uint8 slot, const char* code, const bytes& payload) {
	const int32 fd = sd.fd;
	if (!session_isActive(fd))
		return;
	const size_t length = sizeof(PACKET_ZC_MOONCLIENT_OUTFIT) + payload.size();
	WFIFOHEAD(fd, length);
	PACKET_ZC_MOONCLIENT_OUTFIT* p = reinterpret_cast<PACKET_ZC_MOONCLIENT_OUTFIT*>(WFIFOP(fd, 0));
	p->packetType = HEADER_ZC_MOONCLIENT_OUTFIT;
	p->packetLength = static_cast<int16>(length);
	p->action = action;
	p->result = result;
	p->wait_ms = wait_ms;
	p->slot = slot;
	std::memset(p->code, 0, sizeof(p->code));
	if (code != nullptr)
		std::memcpy(p->code, code, sizeof(p->code));
	if (!payload.empty())
		std::memcpy(WFIFOP(fd, sizeof(PACKET_ZC_MOONCLIENT_OUTFIT)), payload.data(), payload.size());
	WFIFOSET(fd, length);
}

// ── Les codes ───────────────────────────────────────────────────────────────

// Le code que ce compte a déjà donné à cette tenue, s'il en a un.
// ⚠ OCTET À OCTET, par la base : comparer ne collisionne jamais.
std::optional<std::string> existing_code(uint32 account_id, const bytes& outfit, bool& failed) {
	SqlStmt stmt{ *mmysql_handle };
	uint32 account = account_id;
	char code[MOONCLIENT_OUTFIT_CODE_LENGTH + 1] = {};
	if (SQL_SUCCESS != stmt.Prepare("SELECT `code` FROM `moonclient_outfit_codes` WHERE `account_id` = ? AND `data` = ? LIMIT 1")
	||  SQL_SUCCESS != stmt.BindParam(0, SQLDT_UINT32, &account, sizeof(account))
	||  SQL_SUCCESS != stmt.BindParam(1, SQLDT_BLOB, const_cast<uint8*>(outfit.data()), outfit.size())
	||  SQL_SUCCESS != stmt.Execute()
	||  SQL_SUCCESS != stmt.BindColumn(0, SQLDT_STRING, code, sizeof(code))) {
		SqlStmt_ShowDebug(stmt);
		failed = true;
		return std::nullopt;
	}
	if (stmt.NextRow() != SQL_SUCCESS)
		return std::nullopt;
	return std::string(code);
}

// Combien de codes ce compte a publiés, ou -1 si la base ne répond pas.
int64 codes_of(uint32 account_id) {
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT COUNT(*) FROM `moonclient_outfit_codes` WHERE `account_id` = '%u'", account_id)) {
		Sql_ShowDebug(mmysql_handle);
		return -1;
	}
	int64 count = -1;
	if (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		char* data = nullptr;
		Sql_GetData(mmysql_handle, 0, &data, nullptr);
		count = data != nullptr ? strtoll(data, nullptr, 10) : 0;
	}
	Sql_FreeResult(mmysql_handle);
	return count;
}

uint8 publish(map_session_data& sd, const bytes& outfit, std::string& code) {
	bool failed = false;
	if (std::optional<std::string> known = existing_code(sd.status.account_id, outfit, failed)) {
		code = *known;
		return MOONCLIENT_STYLE_ACCEPTED;
	}
	if (failed)
		return MOONCLIENT_STYLE_UNAVAILABLE;
	const int64 count = codes_of(sd.status.account_id);
	if (count < 0)
		return MOONCLIENT_STYLE_UNAVAILABLE;
	if (count >= MOONCLIENT_OUTFIT_CODES_PER_ACCOUNT)
		return MOONCLIENT_STYLE_CAP;
	for (int32 attempt = 0; attempt < CODE_ATTEMPTS; ++attempt) {
		char drawn[MOONCLIENT_OUTFIT_CODE_LENGTH + 1] = {};
		for (int32 i = 0; i < MOONCLIENT_OUTFIT_CODE_LENGTH; ++i)
			drawn[i] = CODE_ALPHABET[rnd_value<int32>(0, CODE_ALPHABET_SIZE - 1)];
		SqlStmt stmt{ *mmysql_handle };
		uint32 account = sd.status.account_id;
		uint32 author = sd.status.char_id;
		// ⚠ LA CLÉ PRIMAIRE TRANCHE LA COLLISION : un code déjà pris fait échouer
		// l'insertion, et l'on tire à nouveau.
		if (SQL_SUCCESS == stmt.Prepare("INSERT INTO `moonclient_outfit_codes` (`code`, `account_id`, `char_id`, `data`, `created_at`) VALUES (?, ?, ?, ?, NOW())")
		&&  SQL_SUCCESS == stmt.BindParam(0, SQLDT_STRING, drawn, MOONCLIENT_OUTFIT_CODE_LENGTH)
		&&  SQL_SUCCESS == stmt.BindParam(1, SQLDT_UINT32, &account, sizeof(account))
		&&  SQL_SUCCESS == stmt.BindParam(2, SQLDT_UINT32, &author, sizeof(author))
		&&  SQL_SUCCESS == stmt.BindParam(3, SQLDT_BLOB, const_cast<uint8*>(outfit.data()), outfit.size())
		&&  SQL_SUCCESS == stmt.Execute()) {
			code = drawn;
			ShowInfo("style_outfits: le compte %u publie la tenue %s (%u octets).\n", sd.status.account_id, drawn, static_cast<uint32>(outfit.size()));
			return MOONCLIENT_STYLE_ACCEPTED;
		}
	}
	ShowWarning("style_outfits: aucun code neuf pour le compte %u apres %d tirages.\n", sd.status.account_id, CODE_ATTEMPTS);
	return MOONCLIENT_STYLE_UNAVAILABLE;
}

uint8 fetch(const char* code, bytes& outfit) {
	// Le code a passé check_code : huit signes de l'alphabet, rien à échapper.
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `data` FROM `moonclient_outfit_codes` WHERE `code` = '%.*s'", MOONCLIENT_OUTFIT_CODE_LENGTH, code)) {
		Sql_ShowDebug(mmysql_handle);
		return MOONCLIENT_STYLE_UNAVAILABLE;
	}
	uint8 result = MOONCLIENT_OUTFIT_UNKNOWN_CODE;
	if (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		char* data = nullptr;
		size_t length = 0;
		Sql_GetData(mmysql_handle, 0, &data, &length);
		// 🔴 CE QUI SORT DE LA BASE SE REJUGE : une ligne modifiée à la main ne
		// part pas telle quelle.
		if (data != nullptr && check_outfit(reinterpret_cast<const uint8*>(data), length)) {
			outfit.assign(data, data + length);
			result = MOONCLIENT_STYLE_ACCEPTED;
		} else {
			ShowWarning("style_outfits: la tenue %.*s est illisible : ignoree.\n", MOONCLIENT_OUTFIT_CODE_LENGTH, code);
		}
	}
	Sql_FreeResult(mmysql_handle);
	return result;
}

// ── La garde-robe ───────────────────────────────────────────────────────────

// La charge d'une liste, ou rien si la base ne répond pas.
std::optional<bytes> wardrobe_payload(uint32 account_id) {
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `slot`, `name`, `data` FROM `moonclient_wardrobe` WHERE `account_id` = '%u' ORDER BY `slot`", account_id)) {
		Sql_ShowDebug(mmysql_handle);
		return std::nullopt;
	}
	bytes payload(1, 0);
	uint8 count = 0;
	while (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		char* data = nullptr;
		size_t name_length = 0;
		size_t outfit_length = 0;
		Sql_GetData(mmysql_handle, 0, &data, nullptr);
		const uint32 slot = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : MOONCLIENT_WARDROBE_SLOTS;
		char* name = nullptr;
		char* outfit = nullptr;
		Sql_GetData(mmysql_handle, 1, &name, &name_length);
		Sql_GetData(mmysql_handle, 2, &outfit, &outfit_length);
		// Une ligne hors bornes ou illisible ne part pas.
		if (slot >= MOONCLIENT_WARDROBE_SLOTS || name == nullptr || outfit == nullptr
			|| !check_name(reinterpret_cast<const uint8*>(name), name_length)
			|| !check_outfit(reinterpret_cast<const uint8*>(outfit), outfit_length)) {
			ShowWarning("style_outfits: un rangement du compte %u est illisible : ignore.\n", account_id);
			continue;
		}
		payload.push_back(static_cast<uint8>(slot));
		payload.push_back(static_cast<uint8>(name_length));
		payload.insert(payload.end(), name, name + name_length);
		payload.push_back(static_cast<uint8>(outfit_length & 0xFF));
		payload.push_back(static_cast<uint8>((outfit_length >> 8) & 0xFF));
		payload.insert(payload.end(), outfit, outfit + outfit_length);
		++count;
	}
	Sql_FreeResult(mmysql_handle);
	payload[0] = count;
	return payload;
}

void send_wardrobe(map_session_data& sd, uint8 action) {
	const std::optional<bytes> payload = wardrobe_payload(sd.status.account_id);
	if (!payload) {
		reply(sd, action, MOONCLIENT_STYLE_UNAVAILABLE, 0, 0, nullptr, bytes());
		return;
	}
	reply(sd, MOONCLIENT_OUTFIT_WARDROBE_LIST, MOONCLIENT_STYLE_ACCEPTED, 0, 0, nullptr, *payload);
}

uint8 store(map_session_data& sd, uint8 slot, const uint8* name, size_t name_length, const bytes& outfit) {
	SqlStmt stmt{ *mmysql_handle };
	uint32 account = sd.status.account_id;
	uint8 rank = slot;
	if (SQL_SUCCESS != stmt.Prepare("REPLACE INTO `moonclient_wardrobe` (`account_id`, `slot`, `name`, `data`, `updated_at`) VALUES (?, ?, ?, ?, NOW())")
	||  SQL_SUCCESS != stmt.BindParam(0, SQLDT_UINT32, &account, sizeof(account))
	||  SQL_SUCCESS != stmt.BindParam(1, SQLDT_UINT8, &rank, sizeof(rank))
	||  SQL_SUCCESS != stmt.BindParam(2, SQLDT_BLOB, const_cast<uint8*>(name), name_length)
	||  SQL_SUCCESS != stmt.BindParam(3, SQLDT_BLOB, const_cast<uint8*>(outfit.data()), outfit.size())
	||  SQL_SUCCESS != stmt.Execute()) {
		SqlStmt_ShowDebug(stmt);
		return MOONCLIENT_STYLE_UNAVAILABLE;
	}
	return MOONCLIENT_STYLE_ACCEPTED;
}

uint8 clear(map_session_data& sd, uint8 slot) {
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `moonclient_wardrobe` WHERE `account_id` = '%u' AND `slot` = '%u'", sd.status.account_id, static_cast<uint32>(slot))) {
		Sql_ShowDebug(mmysql_handle);
		return MOONCLIENT_STYLE_UNAVAILABLE;
	}
	return Sql_NumRowsAffected(mmysql_handle) == 0 ? MOONCLIENT_OUTFIT_EMPTY_SLOT : MOONCLIENT_STYLE_ACCEPTED;
}

} // namespace

void style_outfits_on_logout(const map_session_data& sd) {
	s_states.erase(sd.status.char_id);
}

void clif_parse_moonclient_outfit(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!style_layers_receiver(*sd))
		return;
	const size_t length = RFIFOW(fd, 2);
	const uint8* frame = RFIFOP(fd, 0);
	const uint8 action = length >= sizeof(PACKET_CZ_MOONCLIENT_OUTFIT) ? frame[4] : 0;
	s_outfit_state& state = s_states[sd->status.char_id];
	const t_tick now = gettick();

	// ── Le plafond d'octets : chaque trame compte, refusée ou non ──────────
	if (state.window_start == 0 || DIFF_TICK(now, state.window_start) >= BYTES_WINDOW_MS) {
		state.window_start = now;
		state.window_bytes = 0;
	}
	state.window_bytes += static_cast<uint32>(length);
	if (state.window_bytes > MOONCLIENT_OUTFIT_BYTES_PER_MINUTE) {
		const uint16 wait_ms = static_cast<uint16>(std::min<t_tick>(UINT16_MAX, std::max<t_tick>(0, BYTES_WINDOW_MS - DIFF_TICK(now, state.window_start))));
		reply(*sd, action, MOONCLIENT_STYLE_CAP, wait_ms, 0, nullptr, bytes());
		return;
	}

	// ── La structure ───────────────────────────────────────────────────────
	if (length < sizeof(PACKET_CZ_MOONCLIENT_OUTFIT) || length > REQUEST_MAX) {
		reply(*sd, action, MOONCLIENT_STYLE_MALFORMED, 0, 0, nullptr, bytes());
		return;
	}
	const PACKET_CZ_MOONCLIENT_OUTFIT* head = reinterpret_cast<const PACKET_CZ_MOONCLIENT_OUTFIT*>(frame);
	const size_t name_at = sizeof(PACKET_CZ_MOONCLIENT_OUTFIT);
	const size_t outfit_at = name_at + head->name_length;
	const bool with_slot = action == MOONCLIENT_OUTFIT_WARDROBE_STORE || action == MOONCLIENT_OUTFIT_WARDROBE_CLEAR;
	const bool with_code = action == MOONCLIENT_OUTFIT_FETCH;
	const bool with_name = action == MOONCLIENT_OUTFIT_WARDROBE_STORE;
	const bool with_outfit = action == MOONCLIENT_OUTFIT_PUBLISH || action == MOONCLIENT_OUTFIT_WARDROBE_STORE;
	bool code_blank = true;
	for (char c : head->code)
		code_blank = code_blank && c == '\0';
	const bool well_formed = action >= MOONCLIENT_OUTFIT_PUBLISH && action <= MOONCLIENT_OUTFIT_WARDROBE_CLEAR
		&& outfit_at <= length
		&& (with_slot ? head->slot < MOONCLIENT_WARDROBE_SLOTS : head->slot == 0)
		&& (with_code ? check_code(head->code) : code_blank)
		&& (with_name ? check_name(frame + name_at, head->name_length) : head->name_length == 0)
		&& (with_outfit ? check_outfit(frame + outfit_at, length - outfit_at) : outfit_at == length);
	if (!well_formed) {
		reply(*sd, action, MOONCLIENT_STYLE_MALFORMED, 0, head->slot, nullptr, bytes());
		return;
	}

	// ── La cadence ─────────────────────────────────────────────────────────
	if (state.next_allowed != 0 && DIFF_TICK(state.next_allowed, now) > 0) {
		const uint16 wait_ms = static_cast<uint16>(std::min<t_tick>(UINT16_MAX, DIFF_TICK(state.next_allowed, now)));
		reply(*sd, action, MOONCLIENT_STYLE_TOO_SOON, wait_ms, head->slot, with_code ? head->code : nullptr, bytes());
		return;
	}
	state.next_allowed = now + MOONCLIENT_OUTFIT_CADENCE_MS;

	const bytes outfit = with_outfit ? bytes(frame + outfit_at, frame + length) : bytes();
	switch (action) {
		case MOONCLIENT_OUTFIT_PUBLISH: {
			std::string code;
			const uint8 result = publish(*sd, outfit, code);
			reply(*sd, action, result, 0, 0, result == MOONCLIENT_STYLE_ACCEPTED ? code.c_str() : nullptr, bytes());
			return;
		}
		case MOONCLIENT_OUTFIT_FETCH: {
			bytes found;
			const uint8 result = fetch(head->code, found);
			reply(*sd, action, result, 0, 0, head->code, found);
			return;
		}
		case MOONCLIENT_OUTFIT_WARDROBE_LIST:
			send_wardrobe(*sd, action);
			return;
		case MOONCLIENT_OUTFIT_WARDROBE_STORE:
		case MOONCLIENT_OUTFIT_WARDROBE_CLEAR: {
			const uint8 result = action == MOONCLIENT_OUTFIT_WARDROBE_STORE
				? store(*sd, head->slot, frame + name_at, head->name_length, outfit)
				: clear(*sd, head->slot);
			reply(*sd, action, result, 0, head->slot, nullptr, bytes());
			if (result == MOONCLIENT_STYLE_ACCEPTED)
				send_wardrobe(*sd, action);
			return;
		}
	}
}
