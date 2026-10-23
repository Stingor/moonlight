// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "map_edits.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ryml.hpp>
#include <ryml_std.hpp>

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
	// L'instant de la dernière demande, par document : celui de la carte et
	// celui des lumières de modèle se demandent ensemble à l'entrée.
	std::unordered_map<std::string, t_tick> last_gets;
};

// [Stingor] Une case que la section `blocked_cells` bloque (l'ADR 0131 de
// moonclient), dans le repère de la GAT.
struct s_blocked_cell {
	int16 x = 0;
	int16 y = 0;
	bool wall = false; ///< `wall` : ni la marche ni les tirs ; `fence` : la marche seulement
};

// Une case que le serveur a vraiment changée. On ne fait que passer un bit de
// vrai à faux : un bit retenu valait vrai à l'origine, et c'est ce qu'on lui
// rend. Un bit déjà faux (la GAT, ou un script, bloquait) n'est pas retenu.
struct s_changed_cell {
	int16 x = 0;
	int16 y = 0;
	bool walkable_was_set = false;
	bool shootable_was_set = false;
};

// Par nom de carte.
std::unordered_map<std::string, s_map_edit_document> s_documents;
// Les cases changées par la dernière application, par nom de carte.
std::unordered_map<std::string, std::vector<s_changed_cell>> s_changed_cells;
// Par identifiant de bloc de la session.
std::unordered_map<int32, s_map_edit_session> s_sessions;
// Le dossier du miroir ; vide : rien ne s'exporte.
std::string s_mirror_folder;

// Un nom de document qui ne peut pas sortir du dossier du miroir : ni
// séparateur, ni point en tête — les noms en `.` ne sont pas publiés, et `..`
// remonterait.
bool is_exportable_name(const std::string& name) {
	return !name.empty() && name[0] != '.' && name.find_first_of("/\\") == std::string::npos;
}

// Le texte du document, décompressé. Faux si le flux ne donne pas exactement la
// taille annoncée.
bool unzip_document(const s_map_edit_document& document, std::vector<char>& text) {
	text.assign(document.text_size, '\0');
	unsigned long text_size = document.text_size;
	return document.text_size != 0
	    && decode_zip(text.data(), &text_size, document.zipped.data(), static_cast<unsigned long>(document.zipped.size())) == 0
	    && text_size == document.text_size;
}

// [Stingor] LES CASES BLOQUÉES (l'ADR 0131 de moonclient). Le serveur ne lit du
// document que la section `blocked_cells` :
//
//   blocked_cells:
//     why: "les colonnes de la place"
//     cells:
//       - [150, 180, wall]
//       - [152, 181, fence]
//
// Le reste du document est visuel, et c'est le client qui le valide.

// Une case : sa colonne, sa rangée, sa sorte.
constexpr size_t BLOCKED_CELL_FIELDS = 3;
// Une coordonnée tient dans un int16 : cinq chiffres au plus.
constexpr size_t BLOCKED_CELL_COORDINATE_DIGITS = 5;

// UNE ERREUR DE RAPIDYAML LÈVE UNE EXCEPTION, que la lecture rattrape : le texte
// vient d'un client, et un YAML malformé ne doit jamais abattre le serveur. Le
// gestionnaire est passé au lecteur lui-même, sans dépendre de celui que
// do_init_database pose pour tout le processus.
void on_blocked_cells_yaml_error(const char* message, size_t length, ryml::Location, void*) {
	throw std::runtime_error(std::string(message, length));
}

// Une coordonnée de case : des chiffres seulement, sans signe, dans un int16.
bool read_cell_coordinate(const ryml::Tree& tree, size_t node, int16& coordinate) {
	if (!tree.is_val(node))
		return false;
	const ryml::csubstr scalar = tree.val(node);
	if (scalar.len == 0 || scalar.len > BLOCKED_CELL_COORDINATE_DIGITS)
		return false;
	int32 value = 0;
	for (size_t i = 0; i < scalar.len; ++i) {
		const char digit = scalar.str[i];
		if (digit < '0' || digit > '9')
			return false;
		value = value * 10 + (digit - '0');
	}
	if (value > INT16_MAX)
		return false;
	coordinate = static_cast<int16>(value);
	return true;
}

// La section `blocked_cells` de l'arbre. Section absente : vrai, `present` faux,
// aucune case. Faux si elle est illisible, avec la raison dans `error`.
bool read_blocked_cells_tree(const ryml::Tree& tree, std::vector<s_blocked_cell>& cells, bool& present, std::string& error) {
	if (tree.empty())
		return true;
	size_t document = tree.root_id();
	if (tree.is_stream(document)) {
		const size_t documents = tree.num_children(document);
		if (documents == 0)
			return true;
		if (documents > 1) {
			error = "plusieurs documents YAML";
			return false;
		}
		document = tree.first_child(document);
	}
	if (!tree.is_map(document))
		return true;
	const size_t section = tree.find_child(document, c4::to_csubstr("blocked_cells"));
	if (section == ryml::NONE)
		return true;
	present = true;
	if (!tree.is_map(section)) {
		error = "blocked_cells n'est pas une table";
		return false;
	}
	const size_t why = tree.find_child(section, c4::to_csubstr("why"));
	const size_t list = tree.find_child(section, c4::to_csubstr("cells"));
	if (why == ryml::NONE || list == ryml::NONE) {
		error = "blocked_cells : why et cells sont dus";
		return false;
	}
	if (tree.is_container(why) || !tree.has_val(why)) {
		error = "blocked_cells : why n'est pas un texte";
		return false;
	}
	const bool why_empty = tree.val_is_null(why) || tree.val(why).empty();
	if (!tree.is_seq(list)) {
		error = "blocked_cells : cells n'est pas une liste";
		return false;
	}
	std::set<std::pair<int16, int16>> seen;
	size_t rank = 0;
	for (size_t node = tree.first_child(list); node != ryml::NONE; node = tree.next_sibling(node), ++rank) {
		const std::string where = "blocked_cells, case " + std::to_string(rank) + " : ";
		if (!tree.is_seq(node) || tree.num_children(node) != BLOCKED_CELL_FIELDS) {
			error = where + "[colonne, rangee, sorte] attendu";
			return false;
		}
		const size_t x_node = tree.first_child(node);
		const size_t y_node = tree.next_sibling(x_node);
		const size_t kind_node = tree.next_sibling(y_node);
		s_blocked_cell cell;
		if (!read_cell_coordinate(tree, x_node, cell.x) || !read_cell_coordinate(tree, y_node, cell.y)) {
			error = where + "coordonnee illisible ou negative";
			return false;
		}
		if (!tree.is_val(kind_node)) {
			error = where + "sorte illisible";
			return false;
		}
		const ryml::csubstr kind = tree.val(kind_node);
		if (kind == "wall")
			cell.wall = true;
		else if (kind == "fence")
			cell.wall = false;
		else {
			error = where + "ni wall ni fence";
			return false;
		}
		if (!seen.emplace(cell.x, cell.y).second) {
			error = where + "(" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + ") revient";
			return false;
		}
		cells.push_back(cell);
	}
	if (!cells.empty() && why_empty) {
		error = "blocked_cells : why est vide";
		return false;
	}
	return true;
}

// LIT LA SECTION `blocked_cells` DE `text`, et elle seule. Section absente :
// vrai, `present` faux, aucune case. Faux si la section, ou le YAML entier, est
// illisible, avec la raison dans `error` ; `cells` est alors vide.
bool read_blocked_cells(const std::vector<char>& text, std::vector<s_blocked_cell>& cells, bool& present, std::string& error) {
	cells.clear();
	present = false;
	bool readable = false;
	try {
		ryml::Parser parser(ryml::Callbacks(nullptr, nullptr, nullptr, on_blocked_cells_yaml_error));
		const ryml::Tree tree = parser.parse_in_arena(c4::to_csubstr("map_edit"), ryml::csubstr(text.data(), text.size()));
		readable = read_blocked_cells_tree(tree, cells, present, error);
	} catch (const std::exception& failure) {
		error = std::string("YAML illisible (") + failure.what() + ")";
	} catch (...) {
		error = "YAML illisible";
	}
	if (!readable)
		cells.clear();
	return readable;
}

// La carte `map`, si ce serveur de carte la tient ; sinon nullptr.
map_data* held_map(const std::string& map) {
	const int16 m = map_mapname2mapid(map.c_str());
	if (m < 0)
		return nullptr;
	map_data* mapdata = map_getmapdata(m);
	return mapdata != nullptr && mapdata->cell != nullptr ? mapdata : nullptr;
}

// Faux si une case sort de la carte `map`. Une carte que ce serveur ne tient
// pas ne se juge pas : c'est le serveur qui la tient qui applique.
bool cells_fit_map(const std::string& map, const std::vector<s_blocked_cell>& cells, std::string& error) {
	if (cells.empty())
		return true;
	const map_data* mapdata = held_map(map);
	if (mapdata == nullptr)
		return true;
	for (const s_blocked_cell& cell : cells) {
		if (cell.x >= mapdata->xs || cell.y >= mapdata->ys) {
			error = "blocked_cells : (" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + ") sort de la carte ("
			      + std::to_string(mapdata->xs) + " x " + std::to_string(mapdata->ys) + ")";
			return false;
		}
	}
	return true;
}

// Rend à leur état d'origine les cases que la dernière application a changées
// sur `map`, et les oublie.
void restore_blocked_cells(const std::string& map) {
	const auto found = s_changed_cells.find(map);
	if (found == s_changed_cells.end())
		return;
	const std::vector<s_changed_cell> changed = std::move(found->second);
	s_changed_cells.erase(found);
	const map_data* mapdata = held_map(map);
	if (mapdata == nullptr)
		return;
	for (const s_changed_cell& cell : changed) {
		if (cell.walkable_was_set)
			map_setcell(mapdata->m, cell.x, cell.y, CELL_WALKABLE, true);
		if (cell.shootable_was_set)
			map_setcell(mapdata->m, cell.x, cell.y, CELL_SHOOTABLE, true);
	}
}

// RESTAURE LES CASES DE L'APPLICATION PRÉCÉDENTE, PUIS BLOQUE `cells` sur `map` :
// `wall` ôte la marche et les tirs, `fence` la marche seulement. On bloque, on ne
// libère jamais. Rend le nombre de cases réellement changées ; une carte que ce
// serveur ne tient pas n'en change aucune.
uint32 apply_blocked_cells(const std::string& map, const std::vector<s_blocked_cell>& cells) {
	restore_blocked_cells(map);
	if (cells.empty())
		return 0;
	map_data* mapdata = held_map(map);
	if (mapdata == nullptr)
		return 0;
	std::vector<s_changed_cell> changed;
	for (const s_blocked_cell& cell : cells) {
		if (cell.x >= mapdata->xs || cell.y >= mapdata->ys)
			continue;
		const mapcell& current = mapdata->cell[static_cast<int32>(cell.x) + static_cast<int32>(cell.y) * mapdata->xs];
		s_changed_cell change;
		change.x = cell.x;
		change.y = cell.y;
		change.walkable_was_set  = current.walkable != 0;
		change.shootable_was_set = cell.wall && current.shootable != 0;
		if (!change.walkable_was_set && !change.shootable_was_set)
			continue;
		if (change.walkable_was_set)
			map_setcell(mapdata->m, cell.x, cell.y, CELL_WALKABLE, false);
		if (change.shootable_was_set)
			map_setcell(mapdata->m, cell.x, cell.y, CELL_SHOOTABLE, false);
		changed.push_back(change);
	}
	const uint32 count = static_cast<uint32>(changed.size());
	if (!changed.empty())
		s_changed_cells[map] = std::move(changed);
	return count;
}

// RÉÉCRIT LE DOCUMENT DE `map` DANS LE MIROIR : `revision: N`, puis le texte
// publié. Écrit à côté sous un nom en `.`, que le manifeste ne publie pas, puis
// renommé : le miroir ne sert jamais un fichier à moitié écrit, et le nouvel
// inode invalide le cache d'empreintes du manifeste.
void export_document(const std::string& map, const s_map_edit_document& document) {
	if (s_mirror_folder.empty())
		return;
	if (!is_exportable_name(map)) {
		ShowWarning("map_edits: %s ne s'exporte pas dans le miroir (nom de fichier refuse).\n", map.c_str());
		return;
	}
	std::vector<char> text;
	if (!unzip_document(document, text)) {
		ShowWarning("map_edits: %s, revision %u, ne se decompresse pas : non exporte.\n", map.c_str(), document.revision);
		return;
	}
	const std::filesystem::path folder(s_mirror_folder);
	const std::filesystem::path target = folder / (map + ".yml");
	const std::filesystem::path partial = folder / ("." + map + ".yml.part");
	{
		std::ofstream stream(partial, std::ios::binary | std::ios::trunc);
		stream << "revision: " << document.revision << '\n';
		stream.write(text.data(), static_cast<std::streamsize>(text.size()));
		if (!stream) {
			ShowWarning("map_edits: %s ne s'ecrit pas dans le miroir (%s).\n", map.c_str(), partial.string().c_str());
			return;
		}
	}
	std::error_code failure;
	std::filesystem::rename(partial, target, failure);
	if (failure) {
		ShowWarning("map_edits: %s ne prend pas sa place dans le miroir (%s).\n", map.c_str(), failure.message().c_str());
		std::filesystem::remove(partial, failure);
	}
}

bool announced(const map_session_data& sd) {
	return sd.state.has_bourgeon && (sd.bourgeon_ui_caps & MOONLIGHT_UI_MAP_EDITS) != 0;
}

bool may_publish(const map_session_data& sd) {
	return pc_get_group_level(&sd) >= battle_config.moonclient_map_edit_level;
}

// Le nom de la carte de `sd`.
const char* map_name_of(const map_session_data& sd) {
	const map_data* mapdata = map_getmapdata(sd.m);
	return mapdata != nullptr ? mapdata->name : "";
}

bool is_shared_document(const std::string& name) {
	return name == MOONCLIENT_MODEL_LIGHTS_DOCUMENT;
}

// Le nom du paquet, borné et terminé, s'il est celui de la carte de `sd` ou
// celui du document des lumières de modèle.
bool names_own_document(const map_session_data& sd, const char (&wire_name)[MAP_NAME_LENGTH_EXT], std::string& name) {
	char bounded[MAP_NAME_LENGTH_EXT];
	safestrncpy(bounded, wire_name, sizeof(bounded));
	name = bounded;
	return !name.empty() && (name == map_name_of(sd) || is_shared_document(name));
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

// À tous les joueurs du serveur (map_foreachpc), mêmes arguments.
int32 send_document_everywhere_sub(map_session_data* sd, va_list ap) {
	const std::string* map = va_arg(ap, const std::string*);
	const s_map_edit_document* document = va_arg(ap, const s_map_edit_document*);
	if (sd != nullptr && map != nullptr && document != nullptr)
		send_document(*sd, *map, *document);
	return 0;
}

// L'annonce de la révision de `map` (ZC 0x0F48) à `sd`.
void send_revision(map_session_data& sd, const std::string& map) {
	const int32 fd = sd.fd;
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
	// [Stingor] LA SECTION `blocked_cells` SE JUGE AVANT LE RANGEMENT : illisible,
	// hors de la carte, ou portée par le document des lumières de modèle, l'envoi
	// entier est refusé et rien n'est rangé.
	std::vector<s_blocked_cell> blocked_cells;
	bool has_blocked_cells = false;
	std::string blocked_cells_error;
	bool blocked_cells_valid = read_blocked_cells(text, blocked_cells, has_blocked_cells, blocked_cells_error);
	if (blocked_cells_valid && has_blocked_cells && is_shared_document(map)) {
		blocked_cells_error = "blocked_cells n'appartient qu'au document d'une carte";
		blocked_cells_valid = false;
	}
	if (blocked_cells_valid)
		blocked_cells_valid = cells_fit_map(map, blocked_cells, blocked_cells_error);
	if (!blocked_cells_valid) {
		ShowWarning("map_edits: %s (compte %u) envoie pour %s une section refusee : %s.\n",
			sd.status.name, sd.status.account_id, map.c_str(), blocked_cells_error.c_str());
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
	// [Stingor] Rangé : les cases s'appliquent avant la diffusion.
	if (!is_shared_document(map)) {
		const uint32 changed = apply_blocked_cells(map, blocked_cells);
		ShowInfo("map_edits: %s, revision %u : %u case(s) bloquee(s) declaree(s), %u changee(s) sur la carte.\n",
			map.c_str(), document.revision, static_cast<uint32>(blocked_cells.size()), changed);
	}
	s_map_edit_document& kept = s_documents[map];
	kept = std::move(document);
	export_document(map, kept);
	send_result(sd, map, MOONCLIENT_MAP_EDIT_ACCEPTED, kept.revision);
	// LE DOCUMENT DES LUMIÈRES DE MODÈLE vaut pour toutes les cartes : il va à
	// tous les joueurs ; celui d'une carte, aux joueurs de cette carte.
	if (is_shared_document(map))
		map_foreachpc(send_document_everywhere_sub, &map, static_cast<const s_map_edit_document*>(&kept));
	else
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
	// [Stingor] LES CASES BLOQUÉES DE CHAQUE DOCUMENT S'APPLIQUENT : les cartes sont
	// déjà lues. Une section illisible en base laisse sa carte sans cases bloquées.
	{
		std::vector<std::string> previous;
		for (const auto& [map, changed] : s_changed_cells)
			previous.push_back(map);
		for (const std::string& map : previous)
			restore_blocked_cells(map);
	}
	uint32 blocked_total = 0;
	uint32 blocked_maps = 0;
	for (const auto& [map, document] : s_documents) {
		std::vector<char> text;
		if (!unzip_document(document, text))
			continue;
		std::vector<s_blocked_cell> blocked_cells;
		bool has_blocked_cells = false;
		std::string blocked_cells_error;
		if (!read_blocked_cells(text, blocked_cells, has_blocked_cells, blocked_cells_error)) {
			ShowWarning("map_edits: %s, revision %u : section blocked_cells illisible (%s), carte sans cases bloquees.\n",
				map.c_str(), document.revision, blocked_cells_error.c_str());
			continue;
		}
		if (!has_blocked_cells)
			continue;
		if (is_shared_document(map)) {
			ShowWarning("map_edits: %s porte une section blocked_cells, ignoree.\n", map.c_str());
			continue;
		}
		if (!cells_fit_map(map, blocked_cells, blocked_cells_error)) {
			ShowWarning("map_edits: %s, revision %u : %s, carte sans cases bloquees.\n",
				map.c_str(), document.revision, blocked_cells_error.c_str());
			continue;
		}
		const uint32 changed = apply_blocked_cells(map, blocked_cells);
		if (changed > 0) {
			blocked_total += changed;
			++blocked_maps;
		}
	}
	ShowStatus("map_edits: " CL_WHITE "%u" CL_RESET " case(s) bloquee(s) sur " CL_WHITE "%u" CL_RESET " carte(s).\n", blocked_total, blocked_maps);
	// LE MIROIR SE RECONSTRUIT DEPUIS LA BASE : un dossier effacé ou en retard se
	// remet d'aplomb à chaque démarrage.
	if (!s_mirror_folder.empty()) {
		for (const auto& [map, document] : s_documents)
			export_document(map, document);
		ShowStatus("map_edits: documents exportes dans le miroir " CL_WHITE "%s" CL_RESET ".\n", s_mirror_folder.c_str());
	}
}

void map_edits_set_mirror(const char* folder) {
	s_mirror_folder = folder != nullptr ? folder : "";
}

void map_edits_final() {
	s_documents.clear();
	// Les cartes s'en vont avec le serveur : rien à leur rendre.
	s_changed_cells.clear();
	s_sessions.clear();
}

void map_edits_on_logout(const map_session_data& sd) {
	s_sessions.erase(sd.id);
}

void map_edits_announce(map_session_data& sd) {
	const int32 fd = sd.fd;
	if (!announced(sd) || !session_isActive(fd))
		return;
	send_revision(sd, map_name_of(sd));
	send_revision(sd, MOONCLIENT_MODEL_LIGHTS_DOCUMENT);
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
	const bool own_map = names_own_document(*sd, p->map, map);
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
// tient le joueur, ou celui des lumières de modèle. Rien pour une autre carte,
// un document absent, ou une demande trop proche de la précédente du même
// document.
void clif_parse_moonclient_map_edit_get(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!announced(*sd))
		return;
	const PACKET_CZ_MOONCLIENT_MAP_EDIT_GET* p =
		reinterpret_cast<const PACKET_CZ_MOONCLIENT_MAP_EDIT_GET*>(RFIFOP(fd, 0));
	std::string map;
	if (!names_own_document(*sd, p->map, map))
		return;
	s_map_edit_session& state = s_sessions[sd->id];
	const t_tick now = gettick();
	const auto asked = state.last_gets.find(map);
	if (asked != state.last_gets.end() && DIFF_TICK(now, asked->second) < MOONCLIENT_MAP_EDIT_GET_DELAY)
		return;
	state.last_gets[map] = now;
	const auto found = s_documents.find(map);
	if (found == s_documents.end())
		return;
	send_document(*sd, map, found->second);
}
