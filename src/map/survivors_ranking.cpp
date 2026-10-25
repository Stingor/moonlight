// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LE CLASSEMENT MONDIAL DU MODE SURVIVORS de moonclient (0x0F4C à
// 0x0F53, voir packets_struct.hpp ; le cahier du lot 8 de moonclient).
//
// Le mode se joue entièrement dans le client. Une partie classée part d'un
// JETON que ce fichier délivre (BEGIN -> TICKET) et finit par ses COMPOSANTES
// (END -> RESULT) : le serveur les borne et refait le score
// (survivors_ranking_rules.cpp), range la partie acceptée, puis rend le verdict
// et le rang. Le tableau se demande à part (BOARD -> BOARD).
//
// Les jetons vivent EN MÉMOIRE, un par personnage, et se perdent au
// redémarrage ; une demande neuve remplace le précédent, et un jeton vaut 24 h
// après son heure. Le dernier jeton consommé et son RESULT sont gardés par
// personnage, déconnexion comprise : un END qui le redit (le RESULT s'est perdu
// en route) reçoit la même réponse, sans rien ranger deux fois.
//
// Les parties acceptées vont dans `survivors_run`, le meilleur de chaque compte
// par tableau dans `survivors_best` (sql-files/moonclient_survivors.sql). Un
// tableau se désigne par la version de règles, l'arène, la classe, le genre et
// la SEMAINE : année ISO × 100 + semaine de l'heure du jeton en défi, 0 en
// partie classée. L'identité est le compte Moonlight (`user_id`) ; le nom est
// celui du personnage, figé à la soumission.
//
// 🔴 RIEN NE PART, RIEN N'EST ENTENDU, sans une session moonclient vérifiée,
// chargée, et qui a annoncé MOONLIGHT_UI_SURVIVORS.

#include <algorithm>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "clif.hpp"
#include "map.hpp" // mmysql_handle, map_getmapflag
#include "packets.hpp"
#include "pc.hpp"
#include "survivors_ranking_rules.hpp"

namespace {

// Les tailles du contrat, que le client épingle à l'octet.
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_BEGIN) == 11);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_TICKET) == 25);
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_END) == 29);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_RESULT) == 27);
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_BOARD) == 11);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD) == 22);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD_entry) == 41);
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_JOURNAL) == 12);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_SERVICE) == 16);

// Les verdicts et résultats du fil. ⚠ Valeurs sur le fil : on AJOUTE en fin.
enum e_survivors_ticket_result : uint8 {
	SURVIVORS_TICKET_GRANTED          = 0,
	SURVIVORS_TICKET_PLACE_REFUSED    = 1, ///< ni ville ni intérieur (survivors_sheltered_place)
	SURVIVORS_TICKET_UNLINKED_ACCOUNT = 2, ///< user_id 0
	SURVIVORS_TICKET_UNKNOWN_RULES    = 3, ///< version de règles ou tableau inconnu
	SURVIVORS_TICKET_TOO_SOON         = 4, ///< une demande par 10 s
};
enum e_survivors_verdict : uint8 {
	SURVIVORS_VERDICT_ACCEPTED       = 0,
	SURVIVORS_VERDICT_PERSONAL_BEST  = 1,
	SURVIVORS_VERDICT_UNKNOWN_TICKET = 2, ///< inconnu, consommé ou expiré
	SURVIVORS_VERDICT_IMPLAUSIBLE    = 3,
};

// Un jeton vaut 24 h après son heure.
constexpr int64 SURVIVORS_TICKET_LIFETIME_S = 86400;
// Les lignes d'une page du tableau.
constexpr size_t SURVIVORS_BOARD_LINES = 20;

// Ce qui désigne un tableau.
struct s_survivors_board_key {
	uint16 ruleset = 0;
	uint16 arena = 0;
	uint16 class_id = 0;
	uint8 board = 0;
	uint32 week = 0;
};

// L'état d'un personnage : le jeton ouvert, et le dernier consommé avec sa
// réponse. `ticket` et `consumed_ticket` à 0 : aucun (un jeton n'est jamais 0).
struct s_survivors_character {
	uint32 ticket = 0;
	int64 issued_at = 0;
	uint64 seed = 0;
	s_survivors_board_key key;
	uint32 consumed_ticket = 0;
	PACKET_ZC_MOONLIGHT_SURVIVORS_RESULT consumed_reply{};
};

// Par char_id.
std::unordered_map<uint32, s_survivors_character> survivors_characters;

// Le meilleur d'un compte sur un tableau, tel que rangé.
struct s_survivors_best {
	bool found = false;
	uint64 score = 0;
	uint32 survived_steps = 0;
	uint64 run_id = 0;
};

int64 survivors_now_utc() {
	return static_cast<int64>(std::time(nullptr));
}

bool survivors_heard(const map_session_data& sd) {
	return sd.state.moonclient && sd.state.pc_loaded && (sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS) != 0;
}

template <typename T>
void survivors_send(const map_session_data& sd, const T& packet) {
	const int32 fd = sd.fd;
	if (!session_isActive(fd))
		return;
	WFIFOHEAD(fd, sizeof(packet));
	memcpy(WFIFOP(fd, 0), &packet, sizeof(packet));
	WFIFOSET(fd, sizeof(packet));
}

void survivors_send_ticket(const map_session_data& sd, e_survivors_ticket_result result, uint32 ticket, uint64 seed, int64 issued_at) {
	PACKET_ZC_MOONLIGHT_SURVIVORS_TICKET packet{};
	packet.packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_TICKET;
	packet.packetLength = static_cast<int16>(sizeof(packet));
	packet.result       = result;
	packet.ticket       = ticket;
	packet.seed         = seed;
	packet.issued_at    = issued_at;
	survivors_send(sd, packet);
}

PACKET_ZC_MOONLIGHT_SURVIVORS_RESULT survivors_result(uint32 ticket, e_survivors_verdict verdict, uint64 score, uint16 rank, uint64 best) {
	PACKET_ZC_MOONLIGHT_SURVIVORS_RESULT packet{};
	packet.packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_RESULT;
	packet.packetLength = static_cast<int16>(sizeof(packet));
	packet.ticket       = ticket;
	packet.verdict      = verdict;
	packet.score        = score;
	packet.rank         = rank;
	packet.best         = best;
	return packet;
}

// La clause WHERE d'un tableau, sans le compte.
#define SURVIVORS_BOARD_WHERE "`ruleset` = '%u' AND `arena` = '%u' AND `class_id` = '%u' AND `board` = '%u' AND `week` = '%u'"
#define SURVIVORS_BOARD_ARGS(key) static_cast<uint32>((key).ruleset), static_cast<uint32>((key).arena), static_cast<uint32>((key).class_id), static_cast<uint32>((key).board), (key).week

// Le meilleur du compte `user_id` sur le tableau `key`. Faux si la base ne
// répond pas.
bool survivors_read_best(uint32 user_id, const s_survivors_board_key& key, s_survivors_best& best) {
	best = s_survivors_best{};
	if (SQL_SUCCESS != Sql_Query(mmysql_handle,
		"SELECT `score`, `survived_steps`, `run_id` FROM `survivors_best` WHERE `user_id` = '%u' AND " SURVIVORS_BOARD_WHERE,
		user_id, SURVIVORS_BOARD_ARGS(key))) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}
	if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
		char* data = nullptr;
		best.found = true;
		Sql_GetData(mmysql_handle, 0, &data, nullptr); best.score = strtoull(data, nullptr, 10);
		Sql_GetData(mmysql_handle, 1, &data, nullptr); best.survived_steps = static_cast<uint32>(strtoul(data, nullptr, 10));
		Sql_GetData(mmysql_handle, 2, &data, nullptr); best.run_id = strtoull(data, nullptr, 10);
	}
	Sql_FreeResult(mmysql_handle);
	return true;
}

// Le rang de `best` sur son tableau : un de plus que les lignes devant, dans
// l'ordre du tableau (score, puis temps survécu, décroissants, puis run_id). 0
// sans meilleur ou si la base ne répond pas.
uint16 survivors_rank_of(const s_survivors_board_key& key, const s_survivors_best& best) {
	if (!best.found)
		return 0;
	if (SQL_SUCCESS != Sql_Query(mmysql_handle,
		"SELECT COUNT(*) FROM `survivors_best` WHERE " SURVIVORS_BOARD_WHERE
		" AND (`score` > %" PRIu64 " OR (`score` = %" PRIu64 " AND (`survived_steps` > '%u' OR (`survived_steps` = '%u' AND `run_id` < %" PRIu64 "))))",
		SURVIVORS_BOARD_ARGS(key), best.score, best.score, best.survived_steps, best.survived_steps, best.run_id)) {
		Sql_ShowDebug(mmysql_handle);
		return 0;
	}
	uint64 ahead = 0;
	if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
		char* data = nullptr;
		Sql_GetData(mmysql_handle, 0, &data, nullptr);
		ahead = strtoull(data, nullptr, 10);
	}
	Sql_FreeResult(mmysql_handle);
	return static_cast<uint16>(std::min<uint64>(ahead + 1, UINT16_MAX));
}

// Range la partie acceptée, puis le meilleur du compte s'il est battu, d'un
// seul tenant. `personal_best` dit si le meilleur a changé. Faux si la base a
// refusé : rien n'est rangé.
bool survivors_store(const map_session_data& sd, const s_survivors_character& state, const s_survivors_components& run,
	uint32 journal_bytes, uint64 score, int64 submitted_at, bool& personal_best) {
	personal_best = false;
	char name[NAME_LENGTH * 2 + 1];
	Sql_EscapeStringLen(mmysql_handle, name, sd.status.name, strnlen(sd.status.name, NAME_LENGTH));
	const uint32 user_id = sd.status.user_id;
	const s_survivors_board_key& key = state.key;

	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, "START TRANSACTION")) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}
	bool written = SQL_SUCCESS == Sql_Query(mmysql_handle,
		"INSERT INTO `survivors_run` (`user_id`, `account_id`, `char_id`, `name`, `ruleset`, `arena`, `class_id`, `board`, `week`,"
		" `seed`, `ticket`, `issued_at`, `submitted_at`, `survived_steps`, `kill_points`, `kills`, `boss_kills`, `level`, `outcome`,"
		" `journal_bytes`, `score`) VALUES ('%u', '%u', '%u', '%s', '%u', '%u', '%u', '%u', '%u', "
		"%" PRIu64 ", '%u', %" PRId64 ", %" PRId64 ", '%u', '%u', '%u', '%u', '%u', '%u', '%u', %" PRIu64 ")",
		user_id, sd.status.account_id, sd.status.char_id, name, SURVIVORS_BOARD_ARGS(key),
		state.seed, state.ticket, state.issued_at, submitted_at, run.survived_steps, run.kill_points, run.kills,
		static_cast<uint32>(run.boss_kills), static_cast<uint32>(run.level), static_cast<uint32>(run.outcome), journal_bytes, score);
	if (!written)
		Sql_ShowDebug(mmysql_handle);
	const uint64 run_id = written ? Sql_LastInsertId(mmysql_handle) : 0;

	s_survivors_best best;
	if (written)
		written = survivors_read_best(user_id, key, best);
	if (written && (!best.found || score > best.score)) {
		written = SQL_SUCCESS == Sql_Query(mmysql_handle,
			"REPLACE INTO `survivors_best` (`user_id`, `ruleset`, `arena`, `class_id`, `board`, `week`, `run_id`, `char_id`, `name`,"
			" `score`, `survived_steps`, `level`, `verified`) VALUES ('%u', '%u', '%u', '%u', '%u', '%u', %" PRIu64 ", '%u', '%s',"
			" %" PRIu64 ", '%u', '%u', '0')",
			user_id, SURVIVORS_BOARD_ARGS(key), run_id, sd.status.char_id, name, score, run.survived_steps, static_cast<uint32>(run.level));
		if (!written)
			Sql_ShowDebug(mmysql_handle);
		personal_best = written;
	}
	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, written ? "COMMIT" : "ROLLBACK")) {
		Sql_ShowDebug(mmysql_handle);
		return false;
	}
	return written;
}

// Lit la clé d'un tableau dans une demande (BEGIN ou BOARD : même forme).
template <typename T>
s_survivors_board_key survivors_key_of(const T& packet) {
	s_survivors_board_key key;
	key.ruleset  = packet.ruleset;
	key.arena    = packet.arena;
	key.class_id = packet.class_id;
	key.board    = packet.board;
	return key;
}

// La clé de semaine d'un tableau à l'instant `utc_s` : celle de la semaine ISO
// en défi, 0 en partie classée.
uint32 survivors_week_of(uint8 board, int64 utc_s) {
	if (board != SURVIVORS_BOARD_CHALLENGE)
		return 0;
	return survivors_week_key(survivors_iso_week_of_utc(utc_s));
}

} // namespace

void clif_moonlight_survivors_service(map_session_data& sd) {
	if (!sd.state.moonclient || !(sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS))
		return;
	PACKET_ZC_MOONLIGHT_SURVIVORS_SERVICE packet{};
	packet.packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_SERVICE;
	packet.packetLength = static_cast<int16>(sizeof(packet));
	packet.ruleset_low  = survivors_ruleset_low();
	packet.ruleset_high = survivors_ruleset_high();
	packet.server_utc   = survivors_now_utc();
	survivors_send(sd, packet);
}

// Dans l'ordre : la cadence, la version et le tableau, le compte, le lieu.
void clif_parse_moonlight_survivors_begin(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!survivors_heard(*sd))
		return;
	const PACKET_CZ_MOONLIGHT_SURVIVORS_BEGIN* p =
		reinterpret_cast<const PACKET_CZ_MOONLIGHT_SURVIVORS_BEGIN*>(RFIFOP(fd, 0));
	s_survivors_board_key key = survivors_key_of(*p);

	// Le refus « trop tôt » a sa propre cadence : au-delà, la demande se tait,
	// et un flot de BEGIN ne se change pas en flot de TICKET.
	if (!clif_moonlight_throttle_pass(*sd, MOONLIGHT_THROTTLE_SURVIVORS_TICKET)) {
		if (clif_moonlight_throttle_pass(*sd, MOONLIGHT_THROTTLE_SURVIVORS_TICKET_REFUSAL))
			survivors_send_ticket(*sd, SURVIVORS_TICKET_TOO_SOON, 0, 0, 0);
		return;
	}
	const s_survivors_ruleset* rules = survivors_find_ruleset(key.ruleset);
	if (rules == nullptr || !survivors_board_known(*rules, key.arena, key.class_id, key.board)) {
		survivors_send_ticket(*sd, SURVIVORS_TICKET_UNKNOWN_RULES, 0, 0, 0);
		return;
	}
	if (sd->status.user_id == 0) {
		survivors_send_ticket(*sd, SURVIVORS_TICKET_UNLINKED_ACCOUNT, 0, 0, 0);
		return;
	}
	// Le lieu que la garde du client accepte : ville ou intérieur. Une copie
	// d'instance se juge par le nom de sa carte d'origine.
	if (!map_getmapflag(sd->m, MF_TOWN) && !survivors_sheltered_place(map_mapid2mapname(sd->m))) {
		survivors_send_ticket(*sd, SURVIVORS_TICKET_PLACE_REFUSED, 0, 0, 0);
		return;
	}

	s_survivors_character& state = survivors_characters[sd->status.char_id];
	// Un numéro neuf, jamais 0, ni celui du jeton remplacé ni du dernier consommé :
	// un END tardif de l'un ou de l'autre ne doit pas valoir pour celui-ci.
	uint32 ticket = 0;
	do {
		ticket = rnd_value<uint32>(1, UINT32_MAX);
	} while (ticket == state.ticket || ticket == state.consumed_ticket);
	const int64 issued_at = survivors_now_utc();
	uint64 seed = rnd_value<uint64>(1, UINT64_MAX);
	if (key.board == SURVIVORS_BOARD_CHALLENGE)
		seed = survivors_seed_of_week(survivors_iso_week_of_utc(issued_at));
	key.week = survivors_week_of(key.board, issued_at);

	state.ticket    = ticket;
	state.issued_at = issued_at;
	state.seed      = seed;
	state.key       = key;
	survivors_send_ticket(*sd, SURVIVORS_TICKET_GRANTED, ticket, seed, issued_at);
}

void clif_parse_moonlight_survivors_end(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!survivors_heard(*sd))
		return;
	const PACKET_CZ_MOONLIGHT_SURVIVORS_END* p =
		reinterpret_cast<const PACKET_CZ_MOONLIGHT_SURVIVORS_END*>(RFIFOP(fd, 0));
	const uint32 ticket = p->ticket;
	const int64 now = survivors_now_utc();

	auto found = survivors_characters.find(sd->status.char_id);
	if (found != survivors_characters.end() && ticket != 0 && found->second.consumed_ticket == ticket) {
		survivors_send(*sd, found->second.consumed_reply);
		return;
	}
	if (found == survivors_characters.end() || ticket == 0 || found->second.ticket != ticket
		|| now - found->second.issued_at > SURVIVORS_TICKET_LIFETIME_S) {
		survivors_send(*sd, survivors_result(ticket, SURVIVORS_VERDICT_UNKNOWN_TICKET, 0, 0, 0));
		return;
	}
	s_survivors_character& state = found->second;

	s_survivors_components run;
	run.survived_steps = p->survived_steps;
	run.kill_points    = p->kill_points;
	run.kills          = p->kills;
	run.boss_kills     = p->boss_kills;
	run.level          = p->level;
	run.outcome        = p->outcome;
	const uint64 score = survivors_score(run);
	const s_survivors_ruleset* rules = survivors_find_ruleset(state.key.ruleset);
	const bool plausible = rules != nullptr && survivors_plausible(*rules, run, now - state.issued_at);

	e_survivors_verdict verdict = SURVIVORS_VERDICT_IMPLAUSIBLE;
	if (plausible) {
		bool personal_best = false;
		if (!survivors_store(*sd, state, run, p->journal_bytes, score, now, personal_best)) {
			// Le jeton reste ouvert et rien ne répond : le client garde la partie
			// en attente et la renverra.
			ShowError("survivors: partie du jeton %u de '%s' (char_id %u) non rangee, la base a refuse.\n",
				ticket, sd->status.name, sd->status.char_id);
			return;
		}
		verdict = personal_best ? SURVIVORS_VERDICT_PERSONAL_BEST : SURVIVORS_VERDICT_ACCEPTED;
	}
	s_survivors_best best;
	uint16 rank = 0;
	if (survivors_read_best(sd->status.user_id, state.key, best))
		rank = survivors_rank_of(state.key, best);

	state.consumed_ticket = ticket;
	state.consumed_reply  = survivors_result(ticket, verdict, score, rank, best.found ? best.score : 0);
	state.ticket          = 0;
	survivors_send(*sd, state.consumed_reply);
}

// Une demande hors cadence est tue. Un tableau que le serveur ne classe pas
// rend une page vide.
void clif_parse_moonlight_survivors_board(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!survivors_heard(*sd))
		return;
	if (!clif_moonlight_throttle_pass(*sd, MOONLIGHT_THROTTLE_SURVIVORS_BOARD))
		return;
	const PACKET_CZ_MOONLIGHT_SURVIVORS_BOARD* p =
		reinterpret_cast<const PACKET_CZ_MOONLIGHT_SURVIVORS_BOARD*>(RFIFOP(fd, 0));
	s_survivors_board_key key = survivors_key_of(*p);
	key.week = survivors_week_of(key.board, survivors_now_utc());

	PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD_entry entries[SURVIVORS_BOARD_LINES]{};
	size_t count = 0;
	uint16 own_rank = 0;
	uint64 own_score = 0;
	const s_survivors_ruleset* rules = survivors_find_ruleset(key.ruleset);
	if (rules != nullptr && survivors_board_known(*rules, key.arena, key.class_id, key.board)) {
		if (SQL_SUCCESS != Sql_Query(mmysql_handle,
			"SELECT `score`, `survived_steps`, `level`, `verified`, `name` FROM `survivors_best` WHERE " SURVIVORS_BOARD_WHERE
			" ORDER BY `score` DESC, `survived_steps` DESC, `run_id` ASC LIMIT %u",
			SURVIVORS_BOARD_ARGS(key), static_cast<uint32>(SURVIVORS_BOARD_LINES))) {
			Sql_ShowDebug(mmysql_handle);
		} else {
			while (count < SURVIVORS_BOARD_LINES && SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
				PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD_entry& entry = entries[count];
				char* data = nullptr;
				size_t length = 0;
				entry.rank = static_cast<uint16>(count + 1);
				Sql_GetData(mmysql_handle, 0, &data, nullptr); entry.score = strtoull(data, nullptr, 10);
				Sql_GetData(mmysql_handle, 1, &data, nullptr); entry.survived_steps = static_cast<uint32>(strtoul(data, nullptr, 10));
				Sql_GetData(mmysql_handle, 2, &data, nullptr); entry.level = static_cast<uint16>(strtoul(data, nullptr, 10));
				Sql_GetData(mmysql_handle, 3, &data, nullptr); entry.verified = static_cast<uint8>(strtoul(data, nullptr, 10));
				Sql_GetData(mmysql_handle, 4, &data, &length);
				safestrncpy(entry.name, data, std::min(length + 1, sizeof(entry.name)));
				++count;
			}
			Sql_FreeResult(mmysql_handle);
		}
		s_survivors_best best;
		if (survivors_read_best(sd->status.user_id, key, best) && best.found) {
			own_rank = survivors_rank_of(key, best);
			own_score = best.score;
		}
	}

	const size_t len = sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD) + count * sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD_entry);
	const int32 out = sd->fd;
	if (!session_isActive(out))
		return;
	WFIFOHEAD(out, len);
	PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD* packet = reinterpret_cast<PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD*>(WFIFOP(out, 0));
	packet->packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_BOARD;
	packet->packetLength = static_cast<int16>(len);
	packet->ruleset      = p->ruleset;
	packet->arena        = p->arena;
	packet->class_id     = p->class_id;
	packet->board        = p->board;
	packet->own_rank     = own_rank;
	packet->own_score    = own_score;
	packet->count        = static_cast<uint8>(count);
	memcpy(packet->entries, entries, count * sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_BOARD_entry));
	WFIFOSET(out, len);
}
