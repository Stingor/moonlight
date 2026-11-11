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
#include "homunculus.hpp" // homun_data, hom_is_active, homdb_checkid
#include "itemdb.hpp"
#include "map.hpp" // mmysql_handle, map_id2nd, map_foreachinallrange
#include "npc.hpp"
#include "packets.hpp"
#include "path.hpp" // distance_xy
#include "pc.hpp"
#include "pet.hpp" // pet_data
#include "status.hpp" // status_get_viewdata

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
constexpr uint8 PROFILE_BODY_MORE = 3;
constexpr int32 RAMP_RANKS = 8;
// La suite du corps : les rampes 9 à 12.
constexpr int32 BODY_MORE_RANKS = 4;
// Le magenta pur, interdit à une retouche : certains lecteurs le prennent pour
// un transparent.
constexpr uint8 CHANNEL_FULL = 0xFF;
// Une coupe dans le mot haut d'une clé de tête, une couleur dans le mot bas.
constexpr uint32 HEAD_KEY_SHIFT = 16;
constexpr uint32 HEAD_KEY_LOW = 0xFFFF;
// Les compagnons qu'un joueur peut teindre d'un coup : son homoncule et son
// familier.
constexpr int32 COMPANION_KINDS = 2;
// Un CZ ne décrit pas plus que ce qu'un personnage range, plus ses huit
// emplacements et ses deux compagnons.
constexpr int32 REQUEST_ENTRIES_MAX = MOONCLIENT_STYLE_LAYERS_MAX + (MOONCLIENT_STYLE_SLOT_LAST - MOONCLIENT_STYLE_SLOT_FIRST + 1) + COMPANION_KINDS;
// Le plus d'exemplaires qu'un joueur affiche : trois coiffes et une cape.
constexpr int32 SHOWN_ITEMS_MAX = 4;
// Les octets d'en-tête d'un bloc : gid, nombre.
constexpr size_t BLOCK_HEAD = 5;
// L'en-tête d'un ZC.
constexpr size_t ZC_HEAD = sizeof(PACKET_ZC_MOONCLIENT_STYLE_LAYERS);
// Une entrée tient sa taille sur un octet, queue du profil mixte comprise.
static_assert(MOONCLIENT_STYLE_LAYER_ENTRY_MAX <= UINT8_MAX, "une entrée doit tenir sa taille sur un octet");

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

// ── Les compagnons ──────────────────────────────────────────────────────────
//
// Une teinture de compagnon appartient au compagnon, par son identifiant DURABLE
// (`hom_id`, `pet_id`), et à la classe pour laquelle elle a été faite : sous une
// autre classe (une évolution), elle dort et ne sort pas.

// Le genre, tel que la table le range.
enum class e_companion_kind : uint8 { homunculus = 1, pet = 2 };

struct s_companion_dye {
	uint32 class_ = 0;
	bytes entry;
};

// Le compagnon d'une unité sur la carte : qui il est, et à qui.
struct s_companion {
	e_companion_kind kind;
	uint32 companion_id;
	uint32 class_;
	map_session_data* master;
};

uint64 companion_key(e_companion_kind kind, uint32 companion_id) {
	return (static_cast<uint64>(kind) << 32) | companion_id;
}

// Le cache des teintures de compagnons, par genre et identifiant ; une absence
// se garde aussi.
std::unordered_map<uint64, std::optional<s_companion_dye>> s_companion_dyes;

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

// Une peau, du visage ou d'un corps : un seul réglage, ni reste ni retouche.
bool skin_layer(uint8 layer) {
	return layer == MOONCLIENT_STYLE_LAYER_SKIN || layer == MOONCLIENT_STYLE_LAYER_BODY_SKIN;
}

// Une teinture rangée par corps, sous la clé du corps : la suite et la peau.
bool body_layer(uint8 layer) {
	return layer == MOONCLIENT_STYLE_LAYER_BODY_MORE || layer == MOONCLIENT_STYLE_LAYER_BODY_SKIN;
}

// Les rangs qu'un calque admet sous un profil ; zéro s'il n'en admet aucun.
int32 ranks_for(uint8 layer, uint8 profile) {
	if (layer == MOONCLIENT_STYLE_LAYER_HEAD)
		return profile == PROFILE_RAMPS ? RAMP_RANKS : 0;
	if (skin_layer(layer))
		return profile == PROFILE_RAMPS ? 1 : 0;
	if (layer == MOONCLIENT_STYLE_LAYER_BODY_MORE)
		return profile == PROFILE_BODY_MORE ? BODY_MORE_RANKS : 0;
	if (profile == PROFILE_RAMPS)
		return RAMP_RANKS;
	if (profile == PROFILE_HUE_CLASSES)
		return MOONCLIENT_STYLE_LAYER_ADJUSTS_MAX;
	return 0;
}

// Un calque qui appartient au PERSONNAGE, et se range avec lui : la tête, les
// deux peaux, la suite du corps.
bool character_layer(uint8 layer) {
	return layer == MOONCLIENT_STYLE_LAYER_HEAD || skin_layer(layer) || layer == MOONCLIENT_STYLE_LAYER_BODY_MORE;
}

// Le reste du profil mixte se règle sur les pièces palettisées : tête, coiffe et
// cape aux rampes, suite du corps au profil 3.
bool takes_rest(uint8 layer, uint8 profile) {
	if (layer == MOONCLIENT_STYLE_LAYER_BODY_MORE)
		return profile == PROFILE_BODY_MORE;
	return !skin_layer(layer) && profile == PROFILE_RAMPS;
}

// Une entrée porte-t-elle quelque chose à ranger : un réglage, ou une queue ?
bool has_content(const s_entry_view& e) {
	return (e.flags & MOONCLIENT_STYLE_LAYER_FLAG_CLEAR) == 0 && (e.mask != 0 || (e.flags & MOONCLIENT_STYLE_LAYER_FLAG_TAIL) != 0);
}

// Les octets `absolu` de `count` réglages à partir de `p` : 0 ou 1.
bool absolutes_valid(const uint8* p, int32 count) {
	for (int32 i = 0; i < count; ++i)
		if (p[i * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES + 4] > 1)
			return false;
	return true;
}

// 🔴 LA QUEUE DU PROFIL MIXTE, de `at` à `size` exclu : le masque du reste dans
// les treize classes et permis à ce calque, des retouches au plus
// MOONCLIENT_STYLE_LAYER_RETOUCHES_MAX, d'index strictement croissants, ni à 0 ni
// en magenta pur, et une longueur exacte. Une queue vide est refusée : elle ne
// part pas sans contenu.
e_moonclient_style_result check_tail(const uint8* p, size_t at, size_t size, uint8 layer, uint8 profile) {
	if (skin_layer(layer) || at + MOONCLIENT_STYLE_LAYER_TAIL_HEAD > size)
		return MOONCLIENT_STYLE_MALFORMED;
	const uint16 rest_mask = static_cast<uint16>(p[at] | (p[at + 1] << 8));
	at += 2;
	if ((rest_mask >> MOONCLIENT_STYLE_LAYER_ADJUSTS_MAX) != 0 || (rest_mask != 0 && !takes_rest(layer, profile)))
		return MOONCLIENT_STYLE_MALFORMED;
	const int32 rest = popcount16(rest_mask);
	if (at + static_cast<size_t>(rest) * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES + 1 > size || !absolutes_valid(p + at, rest))
		return MOONCLIENT_STYLE_MALFORMED;
	at += static_cast<size_t>(rest) * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES;
	const uint8 count = p[at++];
	if (count > MOONCLIENT_STYLE_LAYER_RETOUCHES_MAX)
		return MOONCLIENT_STYLE_CAP;
	if (at + static_cast<size_t>(count) * MOONCLIENT_STYLE_LAYER_RETOUCH_BYTES != size || (rest_mask == 0 && count == 0))
		return MOONCLIENT_STYLE_MALFORMED;
	for (uint8 i = 0; i < count; ++i, at += MOONCLIENT_STYLE_LAYER_RETOUCH_BYTES) {
		const uint8 index = p[at];
		const bool magenta = p[at + 1] == CHANNEL_FULL && p[at + 2] == 0 && p[at + 3] == CHANNEL_FULL;
		if (index == 0 || magenta || (i > 0 && p[at - MOONCLIENT_STYLE_LAYER_RETOUCH_BYTES] >= index))
			return MOONCLIENT_STYLE_MALFORMED;
	}
	return MOONCLIENT_STYLE_ACCEPTED;
}

// Le calque que peint une entrée du CZ (1 à 7), ou zéro.
uint8 painted_layer(uint8 wire_layer) {
	if (character_layer(wire_layer) || wire_layer == MOONCLIENT_STYLE_LAYER_COMPANION)
		return wire_layer;
	const std::optional<s_slot> slot = slot_of(wire_layer);
	return slot ? slot->layer : 0;
}

// La forme d'une entrée qu'on juge : celle d'un CZ, celle du personnage rangée en
// base (calques du personnage, aucun drapeau d'effacement), celle d'un objet
// rangée en base ou dans une tenue (coiffe ou cape sous son numéro de calque),
// celle d'un compagnon rangée en base (calque 7 sous sa classe).
enum class e_entry_form : uint8 { request, character, item, companion };

// 🔴 LA STRUCTURE D'UNE ENTRÉE, et rien d'autre : la longueur exacte, le profil
// connu, le masque dans les rangs du profil, `absolu` à 0 ou 1, les drapeaux
// connus, et la queue du profil mixte si elle s'annonce.
e_moonclient_style_result check_entry(const uint8* p, size_t available, e_entry_form form) {
	if (available < MOONCLIENT_STYLE_LAYER_ENTRY_HEAD)
		return MOONCLIENT_STYLE_MALFORMED;
	const s_entry_view e = view_of(p);
	const int32 adjusts = popcount16(e.mask);
	const size_t pieces_end = MOONCLIENT_STYLE_LAYER_ENTRY_HEAD + static_cast<size_t>(adjusts) * MOONCLIENT_STYLE_LAYER_ADJUST_BYTES;
	if (e.size < pieces_end || e.size > available)
		return MOONCLIENT_STYLE_MALFORMED;
	const uint8 known_flags = MOONCLIENT_STYLE_LAYER_FLAG_CLEAR | MOONCLIENT_STYLE_LAYER_FLAG_TAIL;
	const bool clear = (e.flags & MOONCLIENT_STYLE_LAYER_FLAG_CLEAR) != 0;
	const bool tail = (e.flags & MOONCLIENT_STYLE_LAYER_FLAG_TAIL) != 0;
	if ((e.flags & ~known_flags) != 0 || (form != e_entry_form::request && clear))
		return MOONCLIENT_STYLE_MALFORMED;
	if (clear && (e.mask != 0 || tail))
		return MOONCLIENT_STYLE_MALFORMED;
	uint8 layer = 0;
	switch (form) {
		case e_entry_form::request:
			layer = painted_layer(e.layer);
			break;
		case e_entry_form::character:
			layer = character_layer(e.layer) ? e.layer : 0;
			break;
		case e_entry_form::item:
			layer = (e.layer == MOONCLIENT_STYLE_LAYER_HEADGEAR || e.layer == MOONCLIENT_STYLE_LAYER_GARMENT) ? e.layer : 0;
			break;
		case e_entry_form::companion:
			layer = e.layer == MOONCLIENT_STYLE_LAYER_COMPANION ? e.layer : 0;
			break;
	}
	if (layer == 0)
		return MOONCLIENT_STYLE_KEY;
	const int32 ranks = ranks_for(layer, e.profile);
	if (ranks == 0 || (e.mask >> ranks) != 0)
		return MOONCLIENT_STYLE_MALFORMED;
	if (!absolutes_valid(p + MOONCLIENT_STYLE_LAYER_ENTRY_HEAD, adjusts))
		return MOONCLIENT_STYLE_MALFORMED;
	if (tail) {
		const e_moonclient_style_result checked = check_tail(p, pieces_end, e.size, layer, e.profile);
		if (checked != MOONCLIENT_STYLE_ACCEPTED)
			return checked;
	} else if (e.size != pieces_end) {
		return MOONCLIENT_STYLE_MALFORMED;
	}
	// La clé, dans ses bornes. La peau du visage sous zéro ; la peau d'un corps,
	// comme sa suite, sous une clé non nulle.
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
		if (check_entry(p + at, length - at, e_entry_form::character) != MOONCLIENT_STYLE_ACCEPTED)
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

// Les teintures du personnage (têtes, peaux, suites), lues une fois par session.
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
				// 🔴 CE QUI SORT DE LA BASE SE REJUGE : une entrée d'objet, coiffe ou
				// cape, sans effacement, de la longueur exacte.
				if (view_of(p).size == length && check_entry(p, length, e_entry_form::item) == MOONCLIENT_STYLE_ACCEPTED)
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
	// 🔴 UN BLOB VIDE A BESOIN D'UNE ADRESSE : MySQL lit un tampon nul comme NULL,
	// que les colonnes refusent. La première teinture n'a pas d'« avant », un
	// retrait n'a pas d'« après » : ils s'écrivent vides, pas NULL.
	static uint8 no_byte = 0;
	uint8* was_at = was.empty() ? &no_byte : const_cast<uint8*>(was.data());
	uint8* now_at = now.empty() ? &no_byte : const_cast<uint8*>(now.data());
	if (SQL_SUCCESS != log.Prepare("INSERT INTO `moonclient_item_dye_log` (`logged_at`, `account_id`, `char_id`, `unique_id`, `nameid`, `source`, `before_data`, `after_data`) VALUES (NOW(), ?, ?, ?, ?, ?, ?, ?)")
	||  SQL_SUCCESS != log.BindParam(0, SQLDT_UINT32, &account, sizeof(account))
	||  SQL_SUCCESS != log.BindParam(1, SQLDT_UINT32, &author, sizeof(author))
	||  SQL_SUCCESS != log.BindParam(2, SQLDT_UINT64, &uid, sizeof(uid))
	||  SQL_SUCCESS != log.BindParam(3, SQLDT_UINT32, &item, sizeof(item))
	||  SQL_SUCCESS != log.BindParam(4, SQLDT_STRING, const_cast<char*>(source), strlen(source))
	||  SQL_SUCCESS != log.BindParam(5, SQLDT_BLOB, was_at, was.size())
	||  SQL_SUCCESS != log.BindParam(6, SQLDT_BLOB, now_at, now.size())
	||  SQL_SUCCESS != log.Execute())
		SqlStmt_ShowDebug(log);
	ShowInfo("style_layers: exemplaire %" PRIu64 " (objet %u) %s par le personnage %u (%s).\n", unique_id, nameid, entry != nullptr ? "teint" : "deteint", author_char_id, source);
	return true;
}

// La teinture d'un compagnon, du cache ou de la base.
const std::optional<s_companion_dye>& companion_dye(e_companion_kind kind, uint32 companion_id) {
	const uint64 key = companion_key(kind, companion_id);
	auto found = s_companion_dyes.find(key);
	if (found != s_companion_dyes.end())
		return found->second;
	std::optional<s_companion_dye> dye;
	bool readable = true;
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `class`, `version`, `data` FROM `moonclient_companion_dyes` WHERE `kind` = '%u' AND `companion_id` = '%u'",
		static_cast<uint32>(kind), companion_id)) {
		Sql_ShowDebug(mmysql_handle);
		readable = false;
	} else {
		if (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
			char* data = nullptr;
			size_t length = 0;
			Sql_GetData(mmysql_handle, 0, &data, nullptr);
			const uint32 class_ = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
			Sql_GetData(mmysql_handle, 1, &data, nullptr);
			const uint32 version = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
			Sql_GetData(mmysql_handle, 2, &data, &length);
			if (version == STORED_VERSION && data != nullptr && length >= MOONCLIENT_STYLE_LAYER_ENTRY_HEAD && length <= MOONCLIENT_STYLE_LAYER_ENTRY_MAX) {
				const uint8* p = reinterpret_cast<const uint8*>(data);
				// 🔴 CE QUI SORT DE LA BASE SE REJUGE, et sa clé doit être sa classe.
				if (view_of(p).size == length && view_of(p).key == class_ && check_entry(p, length, e_entry_form::companion) == MOONCLIENT_STYLE_ACCEPTED)
					dye = s_companion_dye{ class_, bytes(p, p + length) };
			}
			if (!dye)
				ShowWarning("style_layers: la teinture du compagnon %u/%u est illisible : ignoree.\n", static_cast<uint32>(kind), companion_id);
		}
		Sql_FreeResult(mmysql_handle);
	}
	if (!readable) {
		static const std::optional<s_companion_dye> none;
		return none;
	}
	return s_companion_dyes[key] = std::move(dye);
}

// Une teinture de compagnon écrite ou retirée, et son journal.
bool store_companion(e_companion_kind kind, uint32 companion_id, uint32 class_, const bytes* entry, const bytes* before,
	uint32 account_id, uint32 author_char_id, const char* source) {
	uint8 kind_number = static_cast<uint8>(kind);
	uint32 id = companion_id;
	uint32 cls = class_;
	{
		SqlStmt stmt{ *mmysql_handle };
		uint8 version = STORED_VERSION;
		uint32 author = author_char_id;
		bool ok;
		if (entry != nullptr) {
			ok = SQL_SUCCESS == stmt.Prepare("REPLACE INTO `moonclient_companion_dyes` (`kind`, `companion_id`, `class`, `version`, `data`, `char_id`, `updated_at`) VALUES (?, ?, ?, ?, ?, ?, NOW())")
			  && SQL_SUCCESS == stmt.BindParam(0, SQLDT_UINT8, &kind_number, sizeof(kind_number))
			  && SQL_SUCCESS == stmt.BindParam(1, SQLDT_UINT32, &id, sizeof(id))
			  && SQL_SUCCESS == stmt.BindParam(2, SQLDT_UINT32, &cls, sizeof(cls))
			  && SQL_SUCCESS == stmt.BindParam(3, SQLDT_UINT8, &version, sizeof(version))
			  && SQL_SUCCESS == stmt.BindParam(4, SQLDT_BLOB, const_cast<uint8*>(entry->data()), entry->size())
			  && SQL_SUCCESS == stmt.BindParam(5, SQLDT_UINT32, &author, sizeof(author))
			  && SQL_SUCCESS == stmt.Execute();
		} else {
			ok = SQL_SUCCESS == stmt.Prepare("DELETE FROM `moonclient_companion_dyes` WHERE `kind` = ? AND `companion_id` = ?")
			  && SQL_SUCCESS == stmt.BindParam(0, SQLDT_UINT8, &kind_number, sizeof(kind_number))
			  && SQL_SUCCESS == stmt.BindParam(1, SQLDT_UINT32, &id, sizeof(id))
			  && SQL_SUCCESS == stmt.Execute();
		}
		if (!ok) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}
	}
	// 🔴 LE JOURNAL : qui a teint quel compagnon, avant et après.
	SqlStmt log{ *mmysql_handle };
	uint32 account = account_id;
	uint32 author = author_char_id;
	static uint8 no_byte = 0;
	bytes empty;
	const bytes& was = before != nullptr ? *before : empty;
	const bytes& now = entry != nullptr ? *entry : empty;
	uint8* was_at = was.empty() ? &no_byte : const_cast<uint8*>(was.data());
	uint8* now_at = now.empty() ? &no_byte : const_cast<uint8*>(now.data());
	if (SQL_SUCCESS != log.Prepare("INSERT INTO `moonclient_companion_dye_log` (`logged_at`, `account_id`, `char_id`, `kind`, `companion_id`, `class`, `source`, `before_data`, `after_data`) VALUES (NOW(), ?, ?, ?, ?, ?, ?, ?, ?)")
	||  SQL_SUCCESS != log.BindParam(0, SQLDT_UINT32, &account, sizeof(account))
	||  SQL_SUCCESS != log.BindParam(1, SQLDT_UINT32, &author, sizeof(author))
	||  SQL_SUCCESS != log.BindParam(2, SQLDT_UINT8, &kind_number, sizeof(kind_number))
	||  SQL_SUCCESS != log.BindParam(3, SQLDT_UINT32, &id, sizeof(id))
	||  SQL_SUCCESS != log.BindParam(4, SQLDT_UINT32, &cls, sizeof(cls))
	||  SQL_SUCCESS != log.BindParam(5, SQLDT_STRING, const_cast<char*>(source), strlen(source))
	||  SQL_SUCCESS != log.BindParam(6, SQLDT_BLOB, was_at, was.size())
	||  SQL_SUCCESS != log.BindParam(7, SQLDT_BLOB, now_at, now.size())
	||  SQL_SUCCESS != log.Execute())
		SqlStmt_ShowDebug(log);
	ShowInfo("style_layers: compagnon %u/%u (classe %u) %s par le personnage %u (%s).\n", static_cast<uint32>(kind), companion_id, class_,
		entry != nullptr ? "teint" : "deteint", author_char_id, source);
	return true;
}

// Le compagnon qu'est une unité, ou rien : un homoncule ou un familier SUR LA
// CARTE, qui a un maître joueur. Un homoncule au repos ou mort, un familier
// rentré dans son œuf, ne sont pas des compagnons qu'on montre.
std::optional<s_companion> companion_of(block_list& bl) {
	if (bl.prev == nullptr)
		return std::nullopt;
	if (bl.type == BL_HOM) {
		homun_data& hd = static_cast<homun_data&>(bl);
		if (!hom_is_active(&hd) || hd.master == nullptr || hd.homunculus.hom_id <= 0)
			return std::nullopt;
		return s_companion{ e_companion_kind::homunculus, static_cast<uint32>(hd.homunculus.hom_id), static_cast<uint32>(hd.homunculus.class_), hd.master };
	}
	if (bl.type == BL_PET) {
		pet_data& pd = static_cast<pet_data&>(bl);
		if (pd.pet.incubate != 0 || pd.master == nullptr || pd.pet.pet_id <= 0)
			return std::nullopt;
		return s_companion{ e_companion_kind::pet, static_cast<uint32>(pd.pet.pet_id), static_cast<uint32>(pd.pet.class_), pd.master };
	}
	return std::nullopt;
}

// 🔴 LE COMPAGNON ACTIF DE LA SESSION, de la classe attendue : le seul que le CZ
// puisse viser. Il est lu dans `sd`, jamais désigné par le client : homoncule
// pour une classe d'homoncule, familier sinon. Il doit être sur la carte du
// maître, et le maître doit être `sd`.
block_list* active_companion(map_session_data& sd, uint32 expected_class) {
	block_list* bl = homdb_checkid(static_cast<int32>(expected_class)) ? static_cast<block_list*>(sd.hd) : static_cast<block_list*>(sd.pd);
	if (bl == nullptr || bl->m != sd.m)
		return nullptr;
	const std::optional<s_companion> c = companion_of(*bl);
	if (!c || c->master != &sd || c->class_ != expected_class)
		return nullptr;
	return bl;
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
		// ⚠ TOUTES LES SUITES ET TOUTES LES PEAUX DES CORPS : leur clé est un
		// condensé que seul le client sait rattacher au corps qu'il charge.
		const bool shown = (v.layer == MOONCLIENT_STYLE_LAYER_HEAD && v.key == head_key) || v.layer == MOONCLIENT_STYLE_LAYER_SKIN
			|| body_layer(v.layer);
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

// Le bloc d'un compagnon : sa teinture sous son gid, si elle a été faite pour sa
// classe d'aujourd'hui ; un bloc vide sinon. Rien si l'unité n'est pas un
// compagnon qu'on montre.
std::optional<bytes> compose_companion_block(block_list& bl) {
	const std::optional<s_companion> c = companion_of(bl);
	if (!c)
		return std::nullopt;
	bytes block(BLOCK_HEAD, 0);
	write_u32(block.data(), static_cast<uint32>(bl.id));
	const std::optional<s_companion_dye>& dye = companion_dye(c->kind, c->companion_id);
	// 🔴 UNE TEINTURE QUI DORT NE SORT PAS : faite pour une autre classe, elle
	// peindrait les rampes d'un autre sprite.
	if (dye && dye->class_ == c->class_) {
		block.insert(block.end(), dye->entry.begin(), dye->entry.end());
		block[4] = 1;
	}
	return block;
}

// Le compagnon se montre-t-il à `viewer` ? Ni lui ni son maître ne doivent lui
// être cachés, et il ne porte pas l'apparence invisible.
bool companion_seen_by(block_list& bl, const map_session_data& master, const map_session_data& viewer) {
	const view_data* vd = status_get_viewdata(&bl);
	if (vd == nullptr || vd->look[LOOK_BASE] == JT_INVISIBLE)
		return false;
	if (clif_hidden_unit_withheld(bl, viewer))
		return false;
	return &viewer == &master || !clif_hidden_unit_withheld(master, viewer);
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

// Qui reçoit la diffusion : voir sa définition.
bool layers_viewer(const map_session_data& sd);

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
	if (!layers_viewer(*tsd))
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

// Le bloc d'un compagnon, à un joueur en vue (callback BL_PC). va_arg : le
// compagnon, son maître, la trame.
int32 send_companion_sub(block_list* bl, va_list ap) {
	map_session_data* tsd = BL_CAST(BL_PC, bl);
	block_list* companion = va_arg(ap, block_list*);
	const map_session_data* master = va_arg(ap, const map_session_data*);
	const bytes* frame = va_arg(ap, const bytes*);
	if (tsd == nullptr || companion == nullptr || master == nullptr)
		return 0;
	// 🔴 DESTINATAIRE PAR DESTINATAIRE, et rien de ce qu'il ne voit pas.
	if (!layers_viewer(*tsd) || !companion_seen_by(*companion, *master, *tsd))
		return 0;
	send_frame(*tsd, *frame);
	return 1;
}

// Le bloc d'un compagnon, à la zone, son maître compris. `even_empty` : un bloc
// vide part aussi, pour effacer une teinture retirée.
void send_companion_area(block_list& bl, bool even_empty) {
	const std::optional<s_companion> c = companion_of(bl);
	const std::optional<bytes> block = compose_companion_block(bl);
	if (!c || !block || (!even_empty && (*block)[4] == 0))
		return;
	const bytes frame = frame_of(*block);
	map_foreachinallrange(send_companion_sub, &bl, AREA_SIZE, BL_PC, &bl, static_cast<const map_session_data*>(c->master), &frame);
	map_foreachinviewring(send_companion_sub, &bl, 0, &bl, static_cast<const map_session_data*>(c->master), &frame);
}

// Les compagnons en vue de `self` (callback BL_HOM|BL_PET) : va_arg = la liste à
// remplir, `self`.
int32 collect_companion_sub(block_list* bl, va_list ap) {
	std::vector<block_list*>* out = va_arg(ap, std::vector<block_list*>*);
	const map_session_data* self = va_arg(ap, const map_session_data*);
	const std::optional<s_companion> c = companion_of(*bl);
	if (!c || !companion_seen_by(*bl, *c->master, *self))
		return 0;
	out->push_back(bl);
	return 1;
}

// Le joueur en vue de `self` a-t-il un bloc à lui envoyer ? Rassemble ceux qui
// ne lui sont pas cachés, en trames de MOONCLIENT_STYLE_LAYERS_RAW_MAX au plus.
void send_seen(map_session_data& sd);

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

namespace {

// 🔴 QUI REÇOIT LA DIFFUSION : un joueur vérifié qui a le bit, ou le spectateur
// qui a le droit de VOIR et l'a annoncé. Ce n'est pas `style_layers_receiver`,
// qui garde aussi les CZ : un spectateur n'émet rien.
bool layers_viewer(const map_session_data& sd) {
	const bool viewer = sd.state.has_bourgeon || (sd.state.spectator && sd.state.bourgeon_cosmetic);
	return viewer && (sd.bourgeon_ui_caps & MOONLIGHT_UI_STYLE_LAYERS) != 0;
}

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

	send_seen(sd);
}

void style_layers_spectator_join(map_session_data& sd) {
	if (!sd.state.spectator || !layers_viewer(sd))
		return;
	send_seen(sd);
}

namespace {

void send_seen(map_session_data& sd) {
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
	// Puis les compagnons en vue, chacun sous son gid.
	std::vector<block_list*> companions;
	map_foreachinallrange(collect_companion_sub, &sd, pc_view_range(sd), BL_HOM | BL_PET, &companions, static_cast<const map_session_data*>(&sd));
	for (block_list* companion : companions) {
		const std::optional<bytes> block = compose_companion_block(*companion);
		if (!block || (*block)[4] == 0)
			continue;
		if (raw.size() + block->size() > MOONCLIENT_STYLE_LAYERS_RAW_MAX) {
			send_frame(sd, frame_of(raw));
			raw.clear();
		}
		raw.insert(raw.end(), block->begin(), block->end());
	}
	if (!raw.empty())
		send_frame(sd, frame_of(raw));
}

} // namespace

void style_layers_spawn(map_session_data& owner) {
	// Un spectateur ne se montre pas : rien de lui ne se compose ni ne se lit.
	if (owner.state.spectator)
		return;
	const bytes block = compose_block(owner);
	// Rien à dire, et rien n'a été dit : personne n'a de teinture à oublier.
	if (block[4] == 0 && !state_of(owner).broadcast)
		return;
	send_area(owner, block, false);
}

void style_layers_single(map_session_data& sd, map_session_data& owner) {
	if (!layers_viewer(sd) || &sd == &owner || owner.state.spectator)
		return;
	const bytes block = compose_block(owner);
	if (block[4] == 0)
		return;
	send_frame(sd, frame_of(block));
}

void style_layers_companion_spawn(const block_list& bl) {
	if (bl.type != BL_HOM && bl.type != BL_PET)
		return;
	// L'apparition se dit sur une unité constante ; la lecture de sa vue et de son
	// état ne la modifie pas.
	send_companion_area(const_cast<block_list&>(bl), false);
}

void style_layers_companion_single(map_session_data& sd, block_list& bl) {
	if ((bl.type != BL_HOM && bl.type != BL_PET) || !layers_viewer(sd))
		return;
	const std::optional<s_companion> c = companion_of(bl);
	if (!c || !companion_seen_by(bl, *c->master, sd))
		return;
	const std::optional<bytes> block = compose_companion_block(bl);
	if (!block || (*block)[4] == 0)
		return;
	send_frame(sd, frame_of(*block));
}

void style_layers_forget_companion(uint8 kind, int32 companion_id) {
	if (companion_id <= 0)
		return;
	const uint64 key = companion_key(static_cast<e_companion_kind>(kind), static_cast<uint32>(companion_id));
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `moonclient_companion_dyes` WHERE `kind` = '%u' AND `companion_id` = '%d'", static_cast<uint32>(kind), companion_id))
		Sql_ShowDebug(mmysql_handle);
	s_companion_dyes.erase(key);
}

void style_layers_look_changed(map_session_data& owner) {
	if (owner.prev == nullptr || owner.state.spectator)
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
	s_companion_dyes.clear();
}

// ── Le CZ ───────────────────────────────────────────────────────────────────

namespace {

// Une teinture de compagnon à écrire : qui, pour quelle classe, et l'unité à
// rediffuser.
struct s_companion_change {
	e_companion_kind kind;
	uint32 companion_id;
	uint32 class_;
	int32 unit_id;
	std::optional<bytes> entry; // rien : retirer
};

// Le jugement complet d'une trame. Rend le résultat ; n'écrit rien qu'accepté.
// `changed_units` reçoit les compagnons dont la teinture a changé.
e_moonclient_style_result apply_request(map_session_data& sd, const uint8* frame, size_t length, uint16& wait_ms, std::vector<int32>& changed_units) {
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
		const e_moonclient_style_result checked = check_entry(frame + at, length - at, e_entry_form::request);
		if (checked != MOONCLIENT_STYLE_ACCEPTED)
			return checked;
		const s_entry_view view = view_of(frame + at);
		// Un calque, une entrée : la même tête, la peau du visage, le même corps ou
		// le même emplacement deux fois.
		for (const s_request_entry& other : entries) {
			const bool keyed = view.layer == MOONCLIENT_STYLE_LAYER_HEAD || body_layer(view.layer) || view.layer == MOONCLIENT_STYLE_LAYER_COMPANION;
			const bool same_target = other.view.layer == view.layer && (!keyed || other.view.key == view.key);
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
	std::vector<s_companion_change> companions;
	for (const s_request_entry& e : entries) {
		const std::optional<s_slot> slot = slot_of(e.view.layer);
		if (!slot)
			continue;
		// Auprès du styliste seulement, si l'option l'exige. Sans elle, les gardes
		// qui suivent suffisent : l'objet est celui que l'émetteur porte.
		if (battle_config.moonclient_style_atelier && !atelier_open(sd, state))
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
		if (has_content(e.view)) {
			bytes entry(e.p, e.p + e.view.size);
			entry[1] = slot->layer;
			// Seul le drapeau de queue se range : il dit comment relire l'entrée.
			entry[7] = e.view.flags & MOONCLIENT_STYLE_LAYER_FLAG_TAIL;
			change.entry = std::move(entry);
		}
		items.push_back(std::move(change));
	}

	// ── Les compagnons : celui de la session, là, de la classe attendue ────
	for (const s_request_entry& e : entries) {
		if (e.view.layer != MOONCLIENT_STYLE_LAYER_COMPANION)
			continue;
		// Auprès du styliste seulement, si l'option l'exige : comme un item (D23).
		if (battle_config.moonclient_style_atelier && !atelier_open(sd, state))
			return MOONCLIENT_STYLE_NO_ATELIER;
		// 🔴 LE COMPAGNON SE LIT DANS LA SESSION ÉMETTRICE, jamais par un
		// identifiant du client : la clé n'est que la classe attendue.
		block_list* bl = active_companion(sd, e.view.key);
		const std::optional<s_companion> c = bl != nullptr ? companion_of(*bl) : std::nullopt;
		if (!c)
			return MOONCLIENT_STYLE_NOT_WORN;
		for (const s_companion_change& other : companions)
			if (other.kind == c->kind)
				return MOONCLIENT_STYLE_KEY;
		s_companion_change change{ c->kind, c->companion_id, c->class_, bl->id, std::nullopt };
		if (has_content(e.view)) {
			bytes entry(e.p, e.p + e.view.size);
			entry[7] = e.view.flags & MOONCLIENT_STYLE_LAYER_FLAG_TAIL;
			change.entry = std::move(entry);
		}
		companions.push_back(std::move(change));
	}

	// ── Les teintures du personnage ────────────────────────────────────────
	std::vector<bytes> character = clear_character ? std::vector<bytes>{} : loaded.character;
	const uint32 worn_head = (static_cast<uint32>(sd.status.hair) << HEAD_KEY_SHIFT) | (static_cast<uint32>(sd.status.hair_color) & HEAD_KEY_LOW);
	for (const s_request_entry& e : entries) {
		if (slot_of(e.view.layer) || e.view.layer == MOONCLIENT_STYLE_LAYER_COMPANION)
			continue;
		// La place qu'elle prend : la même tête, ou la peau.
		for (size_t i = 0; i < character.size(); ++i) {
			const s_entry_view v = view_of(character[i].data());
			if (v.layer == e.view.layer && v.key == e.view.key) {
				character.erase(character.begin() + i);
				break;
			}
		}
		if (!has_content(e.view))
			continue;
		bytes entry(e.p, e.p + e.view.size);
		entry[7] = e.view.flags & MOONCLIENT_STYLE_LAYER_FLAG_TAIL;
		character.push_back(std::move(entry));
		// Une suite et une peau par corps habillé : au-delà des variantes de la v7,
		// la plus ancienne de ce calque part, comme le corps qu'elle habillait.
		if (body_layer(e.view.layer)) {
			const uint8 layer = e.view.layer;
			const auto same_layer = [layer](const bytes& b) { return view_of(b.data()).layer == layer; };
			if (std::count_if(character.begin(), character.end(), same_layer) > BOURGEON_STYLE_MAX_VARIANTS)
				character.erase(std::find_if(character.begin(), character.end(), same_layer));
		}
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
	for (const s_companion_change& change : companions) {
		const std::optional<s_companion_dye>& now_dye = companion_dye(change.kind, change.companion_id);
		const bool had = now_dye.has_value() && now_dye->class_ == change.class_;
		// Une teinture qui dort sous une autre classe compte comme absente : en
		// retirer une qu'on ne voit pas ne fait rien.
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
	std::vector<std::optional<bytes>> companion_befores;
	for (const s_companion_change& change : companions) {
		const std::optional<s_companion_dye>& before = companion_dye(change.kind, change.companion_id);
		companion_befores.push_back(before ? std::optional<bytes>(before->entry) : std::nullopt);
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
	for (size_t i = 0; written && i < companions.size(); ++i) {
		const s_companion_change& change = companions[i];
		written = store_companion(change.kind, change.companion_id, change.class_, change.entry ? &*change.entry : nullptr,
			companion_befores[i] ? &*companion_befores[i] : nullptr, sd.status.account_id, sd.status.char_id, "joueur");
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
	for (s_companion_change& change : companions) {
		const uint64 key = companion_key(change.kind, change.companion_id);
		if (change.entry)
			s_companion_dyes[key] = s_companion_dye{ change.class_, std::move(*change.entry) };
		else
			s_companion_dyes[key] = std::nullopt;
		changed_units.push_back(change.unit_id);
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
	std::vector<int32> changed_units;
	const e_moonclient_style_result result = apply_request(*sd, RFIFOP(fd, 0), length, wait_ms, changed_units);
	style_layers_verdict(*sd, MOONCLIENT_STYLE_SUBJECT_LAYERS, result, wait_ms, 0);
	// L'émetteur reçoit son bloc avec la zone : c'est son écho.
	if (result == MOONCLIENT_STYLE_ACCEPTED && sd->prev != nullptr)
		send_area(*sd, compose_block(*sd), true);
	// Et chaque compagnon teint ou déteint, le sien compris : un bloc vide efface.
	for (const int32 unit_id : changed_units)
		if (block_list* bl = map_id2bl(unit_id); bl != nullptr)
			send_companion_area(*bl, true);
}

bool style_layers_readable(map_session_data& owner) {
	return !loaded_state(owner).load_failed;
}

std::vector<uint8> style_layers_shown_entries(map_session_data& owner) {
	const bytes block = compose_block(owner);
	// Le bloc sans son gid : le nombre, puis les entrées.
	return bytes(block.begin() + sizeof(uint32), block.end());
}

size_t style_layers_check_shown_entry(const uint8* p, size_t available) {
	if (available < MOONCLIENT_STYLE_LAYER_ENTRY_HEAD)
		return 0;
	const s_entry_view e = view_of(p);
	// Le personnage : la forme rangée ; un objet, sous l'apparence affichée : la
	// forme d'une teinture d'exemplaire. Toutes deux avec leur queue éventuelle.
	const e_entry_form form = character_layer(e.layer) ? e_entry_form::character : e_entry_form::item;
	return check_entry(p, available, form) == MOONCLIENT_STYLE_ACCEPTED ? e.size : 0;
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
			safesnprintf(line, sizeof(line), "peau du visage : %d reglage(s)", popcount16(v.mask));
		else if (v.layer == MOONCLIENT_STYLE_LAYER_BODY_SKIN)
			safesnprintf(line, sizeof(line), "peau du corps %08X : %d reglage(s)", v.key, popcount16(v.mask));
		else if (v.layer == MOONCLIENT_STYLE_LAYER_BODY_MORE)
			safesnprintf(line, sizeof(line), "suite du corps %08X : %d reglage(s)", v.key, popcount16(v.mask));
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
	for (block_list* bl : { static_cast<block_list*>(sd.hd), static_cast<block_list*>(sd.pd) }) {
		if (bl == nullptr)
			continue;
		const std::optional<s_companion> c = companion_of(*bl);
		if (!c)
			continue;
		const std::optional<s_companion_dye>& dye = companion_dye(c->kind, c->companion_id);
		if (!dye)
			continue;
		safesnprintf(line, sizeof(line), "%s %u (classe %u) : teinture de la classe %u%s, %d reglage(s)",
			c->kind == e_companion_kind::homunculus ? "homoncule" : "familier", c->companion_id, c->class_, dye->class_,
			dye->class_ == c->class_ ? "" : " (EN SOMMEIL)", popcount16(view_of(dye->entry.data()).mask));
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
