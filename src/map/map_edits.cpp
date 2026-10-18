// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "map_edits.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include <common/grfio.hpp> // decode_zip
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "map.hpp" // mmysql_handle, map_getmapdata, map_foreachinmap
#include "packets.hpp"
#include "pc.hpp"

namespace {

// Le document qu'une carte porte, tel que rangé : le flux compressé reçu, jamais
// recompressé.
struct s_map_edit_document {
	uint32 revision = 0;
	uint32 text_size = 0;
	std::string author;
	std::vector<uint8> zipped;
};

// Le transfert entrant d'une session, et l'instant de sa dernière demande.
struct s_map_edit_session {
	bool receiving = false;
	std::string map;
	uint32 base_revision = 0;
	uint32 zipped_size = 0;
	uint32 text_size = 0;
	std::vector<uint8> zipped;
	t_tick last_chunk = 0;
	t_tick last_get = 0;
	bool asked = false;
};

// Par nom de carte.
std::unordered_map<std::string, s_map_edit_document> s_documents;
// Par identifiant de bloc de la session.
std::unordered_map<int32, s_map_edit_session> s_sessions;

bool announced(const map_session_data& sd) {
	return sd.state.has_bourgeon && (sd.bourgeon_ui_caps & BOURGEON_UI_MAP_EDITS) != 0;
}

bool may_publish(const map_session_data& sd) {
	return pc_get_group_level(&sd) >= battle_config.moonclient_map_edit_level;
}

// Le nom de la carte de `sd`.
const char* map_name_of(const map_session_data& sd) {
	const map_data* mapdata = map_getmapdata(sd.m);
	return mapdata != nullptr ? mapdata->name : "";
}

// Le nom du paquet, borné et terminé, s'il est celui de la carte de `sd`.
bool names_own_map(const map_session_data& sd, const char (&wire_name)[MAP_NAME_LENGTH_EXT], std::string& name) {
	char bounded[MAP_NAME_LENGTH_EXT];
	safestrncpy(bounded, wire_name, sizeof(bounded));
	name = bounded;
	return !name.empty() && name == map_name_of(sd);
}

uint32 revision_of(const std::string& map) {
	const auto found = s_documents.find(map);
	return found != s_documents.end() ? found->second.revision : 0;
}

void send_result(map_session_data& sd, const std::string& map, e_moonclient_map_edit_result result, uint32 revision) {
	const int32 fd = sd.fd;
	if (!announced(sd) || !session_isActive(fd))
		return;
	PACKET_ZC_MOONCLIENT_MAP_EDIT_RESULT packet{};
	packet.packetType   = HEADER_ZC_MOONCLIENT_MAP_EDIT_RESULT;
	packet.packetLength = static_cast<int16>(sizeof(packet));
	safestrncpy(packet.map, map.c_str(), sizeof(packet.map));
	packet.result   = result;
	packet.revision = revision;
	WFIFOHEAD(fd, sizeof(packet));
	memcpy(WFIFOP(fd, 0), &packet, sizeof(packet));
	WFIFOSET(fd, sizeof(packet));
}

// Le document de `map`, en morceaux, à UNE session.
void send_document(map_session_data& sd, const std::string& map, const s_map_edit_document& document) {
	const int32 fd = sd.fd;
	if (!announced(sd) || !session_isActive(fd))
		return;
	const size_t total = document.zipped.size();
	size_t offset = 0;
	do {
		const size_t chunk = std::min<size_t>(MOONCLIENT_MAP_EDIT_DOC_CHUNK, total - offset);
		PACKET_ZC_MOONCLIENT_MAP_EDIT_DOC head{};
		head.packetType   = HEADER_ZC_MOONCLIENT_MAP_EDIT_DOC;
		head.packetLength = static_cast<int16>(sizeof(head) + chunk);
		safestrncpy(head.map, map.c_str(), sizeof(head.map));
		head.revision    = document.revision;
		head.zipped_size = static_cast<uint32>(total);
		head.text_size   = document.text_size;
		head.offset      = static_cast<uint32>(offset);
		safestrncpy(head.author, document.author.c_str(), sizeof(head.author));
		WFIFOHEAD(fd, sizeof(head) + chunk);
		memcpy(WFIFOP(fd, 0), &head, sizeof(head));
		if (chunk > 0)
			memcpy(WFIFOP(fd, sizeof(head)), document.zipped.data() + offset, chunk);
		WFIFOSET(fd, sizeof(head) + chunk);
		offset += chunk;
	} while (offset < total);
}

// Aux joueurs de la carte (callback BL_PC) : va_arg = const std::string* carte,
// puis const s_map_edit_document*.
int32 send_document_sub(block_list* bl, va_list ap) {
	map_session_data* sd = BL_CAST(BL_PC, bl);
	const std::string* map = va_arg(ap, const std::string*);
	const s_map_edit_document* document = va_arg(ap, const s_map_edit_document*);
	if (sd != nullptr && map != nullptr && document != nullptr)
		send_document(*sd, *map, *document);
	return 0;
}

// Range le document en base, sur les deux tables d'un seul tenant.
bool store(const std::string& map, const s_map_edit_document& document, const map_session_data& author) {
	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, "START TRANSACTION")) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}
	const char* const tables[] = { "moonclient_map_edit", "moonclient_map_edit_history" };
	bool written = true;
	for (const char* table : tables) {
		SqlStmt stmt{ *mmysql_handle };
		uint32 revision   = document.revision;
		uint32 text_size  = document.text_size;
		uint32 account_id = author.status.account_id;
		uint32 char_id    = author.status.char_id;
		if (SQL_SUCCESS != stmt.Prepare("REPLACE INTO `%s` (`map`, `revision`, `text_size`, `account_id`, `char_id`, `author_name`, `document`, `updated_at`)"
		                                " VALUES (?, ?, ?, ?, ?, ?, ?, NOW())", table)
		||  SQL_SUCCESS != stmt.BindParam(0, SQLDT_STRING, const_cast<char*>(map.c_str()), map.size())
		||  SQL_SUCCESS != stmt.BindParam(1, SQLDT_UINT32, &revision, sizeof(revision))
		||  SQL_SUCCESS != stmt.BindParam(2, SQLDT_UINT32, &text_size, sizeof(text_size))
		||  SQL_SUCCESS != stmt.BindParam(3, SQLDT_UINT32, &account_id, sizeof(account_id))
		||  SQL_SUCCESS != stmt.BindParam(4, SQLDT_UINT32, &char_id, sizeof(char_id))
		||  SQL_SUCCESS != stmt.BindParam(5, SQLDT_STRING, const_cast<char*>(document.author.c_str()), document.author.size())
		||  SQL_SUCCESS != stmt.BindParam(6, SQLDT_BLOB, const_cast<uint8*>(document.zipped.data()), document.zipped.size())
		||  SQL_SUCCESS != stmt.Execute()) {
			SqlStmt_ShowDebug(stmt);
			written = false;
			break;
		}
	}
	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, written ? "COMMIT" : "ROLLBACK")) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}
	return written;
}

// Le transfert entier : vérifie le flux, la révision, range et diffuse.
void complete(map_session_data& sd, s_map_edit_session& state) {
	const std::string map = state.map;
	state.receiving = false;
	std::vector<uint8> zipped = std::move(state.zipped);
	state.zipped.clear();

	// LE FLUX DOIT DONNER EXACTEMENT LA TAILLE ANNONCÉE : une bombe zlib s'arrête
	// au plafond du tampon, et ne passe pas.
	std::vector<char> text(state.text_size);
	unsigned long text_size = state.text_size;
	if (state.text_size == 0
	||  decode_zip(text.data(), &text_size, zipped.data(), static_cast<unsigned long>(zipped.size())) != 0
	||  text_size != state.text_size
	||  memchr(text.data(), '\0', text.size()) != nullptr) {
		ShowWarning("map_edits: %s (compte %u) envoie un document illisible pour %s.\n",
			sd.status.name, sd.status.account_id, map.c_str());
		send_result(sd, map, MOONCLIENT_MAP_EDIT_MALFORMED, revision_of(map));
		return;
	}
	const uint32 current = revision_of(map);
	if (state.base_revision != current) {
		send_result(sd, map, MOONCLIENT_MAP_EDIT_STALE, current);
		return;
	}
	s_map_edit_document document;
	document.revision  = current + 1;
	document.text_size = state.text_size;
	document.author    = sd.status.name;
	document.zipped    = std::move(zipped);
	if (!store(map, document, sd)) {
		send_result(sd, map, MOONCLIENT_MAP_EDIT_STORAGE, current);
		return;
	}
	ShowInfo("map_edits: %s, revision %u, enregistree par %s (compte %u, personnage %u), %u octets de texte, %u compresses.\n",
		map.c_str(), document.revision, sd.status.name, sd.status.account_id, sd.status.char_id,
		document.text_size, static_cast<uint32>(document.zipped.size()));
	s_map_edit_document& kept = s_documents[map];
	kept = std::move(document);
	send_result(sd, map, MOONCLIENT_MAP_EDIT_ACCEPTED, kept.revision);
	map_foreachinmap(send_document_sub, sd.m, BL_PC, &map, static_cast<const s_map_edit_document*>(&kept));
}

} // namespace

void map_edits_load() {
	s_documents.clear();
	if (SQL_SUCCESS != Sql_Query(mmysql_handle, "SELECT `map`, `revision`, `text_size`, `author_name`, `document` FROM `moonclient_map_edit`")) {
		Sql_ShowDebug(mmysql_handle);
		ShowWarning("map_edits: la table moonclient_map_edit ne se lit pas (sql-files/moonclient_map_edits.sql est-il installe ?).\n");
		return;
	}
	while (Sql_NextRow(mmysql_handle) == SQL_SUCCESS) {
		char* data = nullptr;
		size_t length = 0;
		Sql_GetData(mmysql_handle, 0, &data, &length);
		if (data == nullptr || length == 0)
			continue;
		const std::string map(data, length);
		s_map_edit_document document;
		Sql_GetData(mmysql_handle, 1, &data, nullptr);
		document.revision = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
		Sql_GetData(mmysql_handle, 2, &data, nullptr);
		document.text_size = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
		Sql_GetData(mmysql_handle, 3, &data, &length);
		if (data != nullptr)
			document.author.assign(data, length);
		Sql_GetData(mmysql_handle, 4, &data, &length);
		if (data == nullptr || length == 0 || length > MOONCLIENT_MAP_EDIT_MAX_ZIPPED || document.revision == 0) {
			ShowWarning("map_edits: le document de %s est ecarte (vide, trop gros, ou sans revision).\n", map.c_str());
			continue;
		}
		document.zipped.assign(reinterpret_cast<const uint8*>(data), reinterpret_cast<const uint8*>(data) + length);
		s_documents[map] = std::move(document);
	}
	Sql_FreeResult(mmysql_handle);
	ShowStatus("map_edits: " CL_WHITE "%u" CL_RESET " document(s) d'edition de carte.\n", static_cast<uint32>(s_documents.size()));
}

void map_edits_final() {
	s_documents.clear();
	s_sessions.clear();
}

void map_edits_on_logout(const map_session_data& sd) {
	s_sessions.erase(sd.id);
}

void map_edits_announce(map_session_data& sd) {
	const int32 fd = sd.fd;
	if (!announced(sd) || !session_isActive(fd))
		return;
	const std::string map = map_name_of(sd);
	PACKET_ZC_MOONCLIENT_MAP_EDIT_REVISION packet{};
	packet.packetType   = HEADER_ZC_MOONCLIENT_MAP_EDIT_REVISION;
	packet.packetLength = static_cast<int16>(sizeof(packet));
	safestrncpy(packet.map, map.c_str(), sizeof(packet.map));
	packet.revision    = revision_of(map);
	packet.can_publish = may_publish(sd) ? 1 : 0;
	WFIFOHEAD(fd, sizeof(packet));
	memcpy(WFIFOP(fd, 0), &packet, sizeof(packet));
	WFIFOSET(fd, sizeof(packet));
}

// [Stingor] CZ_MOONCLIENT_MAP_EDIT_PUT (0x0F45) : un morceau du document qu'enregistre
// le staff. Le droit, la carte et l'ordre se jugent à CHAQUE morceau ; une faute
// annule le transfert entier.
void clif_parse_moonclient_map_edit_put(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!announced(*sd))
		return;
	const PACKET_CZ_MOONCLIENT_MAP_EDIT_PUT* p =
		reinterpret_cast<const PACKET_CZ_MOONCLIENT_MAP_EDIT_PUT*>(RFIFOP(fd, 0));
	if (p->packetLength < static_cast<int16>(sizeof(*p)))
		return;
	std::string map;
	const bool own_map = names_own_map(*sd, p->map, map);
	s_map_edit_session& state = s_sessions[sd->id];
	const auto abandon = [&](e_moonclient_map_edit_result result) {
		state.receiving = false;
		state.zipped.clear();
		state.zipped.shrink_to_fit();
		send_result(*sd, map, result, revision_of(map));
	};
	if (!may_publish(*sd)) {
		abandon(MOONCLIENT_MAP_EDIT_FORBIDDEN);
		return;
	}
	const size_t chunk = static_cast<size_t>(p->packetLength) - sizeof(*p);
	const uint8* bytes = RFIFOP(fd, sizeof(*p));
	const t_tick now = gettick();
	if (!own_map || chunk == 0 || chunk > MOONCLIENT_MAP_EDIT_PUT_CHUNK
	||  p->zipped_size == 0 || p->zipped_size > MOONCLIENT_MAP_EDIT_MAX_ZIPPED
	||  p->text_size == 0 || p->text_size > MOONCLIENT_MAP_EDIT_MAX_TEXT
	||  p->offset > p->zipped_size || chunk > p->zipped_size - p->offset) {
		abandon(MOONCLIENT_MAP_EDIT_MALFORMED);
		return;
	}
	if (p->offset == 0) {
		// UN ENVOI PÉRIMÉ S'ARRÊTE AU PREMIER MORCEAU : rien ne sert de recevoir le
		// reste. La révision se revérifie au dernier.
		const uint32 current = revision_of(map);
		if (p->base_revision != current) {
			state.receiving = false;
			state.zipped.clear();
			send_result(*sd, map, MOONCLIENT_MAP_EDIT_STALE, current);
			return;
		}
		state.receiving     = true;
		state.map           = map;
		state.base_revision = p->base_revision;
		state.zipped_size   = p->zipped_size;
		state.text_size     = p->text_size;
		state.zipped.clear();
		state.zipped.reserve(p->zipped_size);
	} else if (!state.receiving || state.map != map
	       ||  state.base_revision != p->base_revision || state.zipped_size != p->zipped_size
	       ||  state.text_size != p->text_size || state.zipped.size() != p->offset
	       ||  DIFF_TICK(now, state.last_chunk) > MOONCLIENT_MAP_EDIT_TRANSFER_TIMEOUT) {
		abandon(MOONCLIENT_MAP_EDIT_MALFORMED);
		return;
	}
	state.last_chunk = now;
	state.zipped.insert(state.zipped.end(), bytes, bytes + chunk);
	if (state.zipped.size() == state.zipped_size)
		complete(*sd, state);
}

// [Stingor] CZ_MOONCLIENT_MAP_EDIT_GET (0x0F49) : le document de la carte où se
// tient le joueur. Rien pour une autre carte, une carte sans document, ou une
// demande trop proche de la précédente.
void clif_parse_moonclient_map_edit_get(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!announced(*sd))
		return;
	const PACKET_CZ_MOONCLIENT_MAP_EDIT_GET* p =
		reinterpret_cast<const PACKET_CZ_MOONCLIENT_MAP_EDIT_GET*>(RFIFOP(fd, 0));
	std::string map;
	if (!names_own_map(*sd, p->map, map))
		return;
	s_map_edit_session& state = s_sessions[sd->id];
	const t_tick now = gettick();
	if (state.asked && DIFF_TICK(now, state.last_get) < MOONCLIENT_MAP_EDIT_GET_DELAY)
		return;
	state.asked    = true;
	state.last_get = now;
	const auto found = s_documents.find(map);
	if (found == s_documents.end())
		return;
	send_document(*sd, map, found->second);
}
