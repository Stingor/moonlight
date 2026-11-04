// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LA PRÉSENCE EN ARÈNE DU MODE SURVIVORS de moonclient (CZ 0x0F56 ->
// ZC 0x0F57, voir packets_struct.hpp ; le cahier du lot 13 de moonclient).
//
// Pendant une partie, le personnage reste en ligne et immobile ; le client dit
// au serveur qu'il entre en partie, s'y met en pause ou en sort. Le serveur
// relaie l'état aux sessions moonclient qui le voient, à ses amis et à sa
// guilde, et le redit à qui entre dans sa vue ou se connecte ensuite.
//
// Ce que les autres voient sans moonclient :
//   - une bulle de salon fermée, « En arène survivors », ouverte à l'entrée et
//     refermée à la sortie, si le joueur n'en tient pas déjà une et pourrait
//     en ouvrir une à la main au même endroit (loin des PNJ) ;
//   - un message de guilde quand il tient quinze minutes en partie, une fois
//     par entrée, et quand il bat son record en partie classée.
//
// Le site lit la table `survivors_presence` (sql-files/moonclient_survivors.sql),
// écrite à l'entrée, vidée à la sortie et au démarrage du serveur de carte.
//
// Les présences vivent EN MÉMOIRE, par char_id. Une déconnexion ou un changement
// de carte font sortir de l'arène sans rien attendre du client : un client qui
// plante en partie n'enverrait jamais sa sortie.
//
// 🔴 RIEN NE PART À UNE SESSION QUI N'A PAS ANNONCÉ MOONLIGHT_UI_SURVIVORS_PRESENCE,
// et rien n'est entendu d'une session moonclient non vérifiée ou non chargée.

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <unordered_map>
#include <unordered_set>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/nullpo.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "battle.hpp" // battle_config, qu'AREA_SIZE lit
#include "chat.hpp"
#include "clif.hpp"
#include "guild.hpp"
#include "intif.hpp"
#include "map.hpp" // mmysql_handle, map_getmapflag
#include "npc.hpp" // npc_isnear
#include "packets.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "survivors_ranking_rules.hpp"

namespace {

// Les tailles du contrat, que le client épingle à l'octet.
static_assert(sizeof(PACKET_CZ_MOONLIGHT_SURVIVORS_PRESENCE) == 10);
static_assert(sizeof(PACKET_ZC_MOONLIGHT_SURVIVORS_PRESENCE) == 21);

// Les états du fil. ⚠ Valeurs sur le fil : on AJOUTE en fin.
enum e_survivors_presence_state : uint8 {
	SURVIVORS_PRESENCE_OUT     = 0,
	SURVIVORS_PRESENCE_PLAYING = 1,
	SURVIVORS_PRESENCE_PAUSED  = 2,
};

// Le temps de jeu, pauses exclues, que la guilde apprend une fois par entrée.
constexpr t_tick SURVIVORS_PRESENCE_HOLD_MS = 15 * 60 * 1000;
constexpr t_tick SURVIVORS_PRESENCE_MS_PER_S = 1000;
// Un salon à une place : le titre se lit, personne n'y entre.
constexpr int32 SURVIVORS_PRESENCE_ROOM_LIMIT = 1;
// En latin-1, comme tout texte que le client natif affiche.
const char SURVIVORS_PRESENCE_ROOM_TITLE[] = "En ar\xE8" "ne survivors";

// La partie en cours d'un personnage.
struct s_survivors_presence {
	uint32 account_id = 0;
	uint8 state = SURVIVORS_PRESENCE_OUT;
	uint16 arena = 0;
	uint16 class_id = 0;
	uint8 flags = 0;
	t_tick entered_at = 0;
	// Le temps passé en jeu, pauses exclues, jusqu'à `counted_at`.
	t_tick played = 0;
	t_tick counted_at = 0;
	// La guilde a appris les quinze minutes de cette entrée.
	bool hold_told = false;
	// Le salon que la présence a ouvert, 0 sans.
	int32 room_id = 0;
};

// Par char_id.
std::unordered_map<uint32, s_survivors_presence> survivors_presences;

// Une session moonclient vérifiée, chargée, qui a reçu l'annonce.
bool presence_heard(const map_session_data& sd) {
	return sd.state.moonclient && sd.state.pc_loaded && (sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS_PRESENCE) != 0;
}

// Une session qui sait lire 0x0F57.
bool presence_listens(const map_session_data& sd) {
	return sd.state.moonclient && (sd.bourgeon_ui_caps & MOONLIGHT_UI_SURVIVORS_PRESENCE) != 0;
}

PACKET_ZC_MOONLIGHT_SURVIVORS_PRESENCE presence_packet(uint32 account_id, uint32 char_id, const s_survivors_presence* presence) {
	PACKET_ZC_MOONLIGHT_SURVIVORS_PRESENCE packet{};
	packet.packetType   = HEADER_ZC_MOONLIGHT_SURVIVORS_PRESENCE;
	packet.packetLength = static_cast<int16>(sizeof(packet));
	packet.account_id   = account_id;
	packet.char_id      = char_id;
	if (presence != nullptr) {
		packet.state     = presence->state;
		packet.arena     = presence->arena;
		packet.class_id  = presence->class_id;
		const t_tick elapsed = DIFF_TICK(gettick(), presence->entered_at);
		packet.elapsed_s = elapsed > 0 ? static_cast<uint32>(elapsed / SURVIVORS_PRESENCE_MS_PER_S) : 0;
	}
	return packet;
}

void presence_send(const map_session_data& sd, const PACKET_ZC_MOONLIGHT_SURVIVORS_PRESENCE& packet) {
	const int32 fd = sd.fd;
	if (!session_isActive(fd))
		return;
	WFIFOHEAD(fd, sizeof(packet));
	memcpy(WFIFOP(fd, 0), &packet, sizeof(packet));
	WFIFOSET(fd, sizeof(packet));
}

// Les destinataires d'un relais, chacun une fois.
struct s_presence_relay {
	const map_session_data* owner = nullptr;
	const PACKET_ZC_MOONLIGHT_SURVIVORS_PRESENCE* packet = nullptr;
	std::unordered_set<int32> told;
};

void presence_tell(s_presence_relay& relay, const map_session_data& tsd) {
	if (&tsd == relay.owner || !presence_listens(tsd))
		return;
	if (!relay.told.insert(tsd.fd).second)
		return;
	presence_send(tsd, *relay.packet);
}

// Aux joueurs autour (callback BL_PC) : va_arg = s_presence_relay*. Ceux à qui
// le joueur caché est retenu n'apprennent rien : le paquet trahirait sa présence.
int32 presence_area_sub(block_list* bl, va_list ap) {
	map_session_data* tsd = BL_CAST(BL_PC, bl);
	s_presence_relay* relay = va_arg(ap, s_presence_relay*);
	if (tsd == nullptr || relay == nullptr || clif_hidden_unit_withheld(*relay->owner, *tsd))
		return 0;
	presence_tell(*relay, *tsd);
	return 0;
}

// L'état de `sd` à sa zone, à ses amis connectés et à sa guilde. `presence`
// nul : il sort de l'arène.
void presence_relay(map_session_data& sd, const s_survivors_presence* presence) {
	const PACKET_ZC_MOONLIGHT_SURVIVORS_PRESENCE packet =
		presence_packet(sd.status.account_id, sd.status.char_id, presence);
	s_presence_relay relay;
	relay.owner  = &sd;
	relay.packet = &packet;
	if (sd.prev != nullptr) {
		map_foreachinallrange(presence_area_sub, &sd, AREA_SIZE, BL_PC, &relay);
		map_foreachinviewring(presence_area_sub, &sd, 0, &relay);
	}
	for (const s_friend& chum : sd.status.friends) {
		if (chum.char_id == 0)
			continue;
		if (const map_session_data* tsd = map_charid2sd(static_cast<int32>(chum.char_id)); tsd != nullptr)
			presence_tell(relay, *tsd);
	}
	if (sd.guild != nullptr) {
		const mmo_guild& g = sd.guild->guild;
		for (int32 i = 0; i < g.max_member; ++i) {
			if (const map_session_data* tsd = g.member[i].sd; tsd != nullptr)
				presence_tell(relay, *tsd);
		}
	}
}

// Un message de guilde au nom de `sd`, à tous les serveurs de carte.
void presence_guild_say(map_session_data& sd, const char* message) {
	if (sd.status.guild_id == 0)
		return;
	const size_t length = strlen(message);
	intif_guild_message(sd.status.guild_id, sd.status.account_id, message, length);
	guild_recv_message(sd.status.guild_id, sd.status.account_id, message, length);
}

// Compte le temps de jeu jusqu'à maintenant, et dit les quinze minutes à la
// guilde la première fois qu'elles sont atteintes.
void presence_count(map_session_data& sd, s_survivors_presence& presence) {
	const t_tick now = gettick();
	if (presence.state == SURVIVORS_PRESENCE_PLAYING)
		presence.played += DIFF_TICK(now, presence.counted_at);
	presence.counted_at = now;
	if (presence.hold_told || presence.played < SURVIVORS_PRESENCE_HOLD_MS)
		return;
	presence.hold_told = true;
	char message[CHAT_SIZE_MAX];
	safesnprintf(message, sizeof(message), "[Survivors] %s a tenu 15 minutes dans l'ar\xE8" "ne.", sd.status.name);
	presence_guild_say(sd, message);
}

// La table du site : la partie en cours, ou plus rien.
void presence_store(const map_session_data& sd, const s_survivors_presence* presence) {
	if (presence == nullptr) {
		if (SQL_SUCCESS != Sql_Query(mmysql_handle, "DELETE FROM `survivors_presence` WHERE `char_id` = '%u'", sd.status.char_id))
			Sql_ShowDebug(mmysql_handle);
		return;
	}
	char name[NAME_LENGTH * 2 + 1];
	Sql_EscapeStringLen(mmysql_handle, name, sd.status.name, strnlen(sd.status.name, NAME_LENGTH));
	const int64 entered_utc = static_cast<int64>(std::time(nullptr)) -
		DIFF_TICK(gettick(), presence->entered_at) / SURVIVORS_PRESENCE_MS_PER_S;
	if (SQL_SUCCESS != Sql_Query(mmysql_handle,
		"REPLACE INTO `survivors_presence` (`char_id`, `account_id`, `name`, `arena`, `class_id`, `flags`, `state`, `entered_at`)"
		" VALUES ('%u', '%u', '%s', '%u', '%u', '%u', '%u', %" PRId64 ")",
		sd.status.char_id, sd.status.account_id, name, static_cast<uint32>(presence->arena),
		static_cast<uint32>(presence->class_id), static_cast<uint32>(presence->flags),
		static_cast<uint32>(presence->state), entered_utc))
		Sql_ShowDebug(mmysql_handle);
}

// `viewer` connaît `owner` : il le voit, ou c'est un ami, ou un membre de sa guilde.
bool presence_known_to(const map_session_data& owner, const map_session_data& viewer) {
	if (owner.prev != nullptr && owner.m == viewer.m && check_distance_bl(&owner, &viewer, AREA_SIZE) &&
		!clif_hidden_unit_withheld(owner, viewer))
		return true;
	for (const s_friend& chum : viewer.status.friends) {
		if (chum.char_id != 0 && chum.char_id == owner.status.char_id)
			return true;
	}
	return owner.status.guild_id != 0 && owner.status.guild_id == viewer.status.guild_id;
}

} // namespace

void do_init_survivors_presence() {
	if (SQL_SUCCESS != Sql_QueryStr(mmysql_handle, "DELETE FROM `survivors_presence`"))
		Sql_ShowDebug(mmysql_handle);
}

void clif_moonlight_survivors_presence_announce(map_session_data& sd) {
	if (!presence_listens(sd))
		return;
	presence_send(sd, presence_packet(0, 0, nullptr));
	for (const auto& [char_id, presence] : survivors_presences) {
		const map_session_data* owner = map_charid2sd(static_cast<int32>(char_id));
		if (owner == nullptr || owner == &sd || !presence_known_to(*owner, sd))
			continue;
		presence_send(sd, presence_packet(presence.account_id, char_id, &presence));
	}
}

void clif_moonlight_survivors_presence_seen(const map_session_data& owner, map_session_data& viewer) {
	if (&owner == &viewer || !presence_listens(viewer))
		return;
	const auto found = survivors_presences.find(owner.status.char_id);
	if (found == survivors_presences.end())
		return;
	presence_send(viewer, presence_packet(owner.status.account_id, owner.status.char_id, &found->second));
}

void survivors_presence_leave(map_session_data& sd) {
	const auto found = survivors_presences.find(sd.status.char_id);
	if (found == survivors_presences.end())
		return;
	presence_count(sd, found->second);
	// Le salon qu'elle a ouvert se referme ; celui qu'il tient lui-même reste.
	if (found->second.room_id != 0 && sd.chatID == found->second.room_id)
		chat_leavechat(&sd, false);
	survivors_presences.erase(found);
	presence_store(sd, nullptr);
	presence_relay(sd, nullptr);
}

void survivors_presence_ranked_best(map_session_data& sd, uint32 survived_steps) {
	const uint32 seconds = survived_steps / SURVIVORS_STEPS_PER_SECOND;
	char message[CHAT_SIZE_MAX];
	safesnprintf(message, sizeof(message), "[Survivors] %s bat son record en partie class\xE9" "e : %u:%02u.",
		sd.status.name, seconds / 60, seconds % 60);
	presence_guild_say(sd, message);
}

// Dans l'ordre : la cadence, l'état, le lieu. Une sortie passe partout.
void clif_parse_moonlight_survivors_presence(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!presence_heard(*sd))
		return;
	if (!clif_moonlight_throttle_pass(*sd, MOONLIGHT_THROTTLE_SURVIVORS_PRESENCE))
		return;
	const PACKET_CZ_MOONLIGHT_SURVIVORS_PRESENCE* p =
		reinterpret_cast<const PACKET_CZ_MOONLIGHT_SURVIVORS_PRESENCE*>(RFIFOP(fd, 0));
	const uint8 state = p->state == SURVIVORS_PRESENCE_PLAYING || p->state == SURVIVORS_PRESENCE_PAUSED
		? p->state : static_cast<uint8>(SURVIVORS_PRESENCE_OUT);
	if (state == SURVIVORS_PRESENCE_OUT) {
		survivors_presence_leave(*sd);
		return;
	}
	// Le lieu que la garde du client accepte, comme pour un jeton du classement.
	if (!map_getmapflag(sd->m, MF_TOWN) && !survivors_sheltered_place(map_mapid2mapname(sd->m)))
		return;

	const auto [found, entered] = survivors_presences.try_emplace(sd->status.char_id);
	s_survivors_presence& presence = found->second;
	if (entered) {
		presence.account_id = sd->status.account_id;
		presence.entered_at = gettick();
		presence.counted_at = presence.entered_at;
		presence.state      = state;
		// Le salon suit les gardes d'un salon ouvert à la main : pas à moins de
		// min_npc_vendchat_distance cases d'un PNJ, dont la bulle couvrirait le
		// PNJ et empêcherait de lui parler, ni pour un joueur privé de salons.
		// chat_createpcchat rend 0 dans tous les cas : le salon se juge à chatID.
		const status_change_entry* no_chat = sd->sc.getSCE(SC_NOCHAT);
		const bool room_barred = npc_isnear(sd) || (no_chat != nullptr && (no_chat->val1 & MANNER_NOROOM));
		if (sd->chatID == 0 && !room_barred) {
			chat_createpcchat(sd, SURVIVORS_PRESENCE_ROOM_TITLE, "", SURVIVORS_PRESENCE_ROOM_LIMIT, true);
			presence.room_id = sd->chatID;
		}
	} else {
		presence_count(*sd, presence);
		presence.state = state;
	}
	presence.arena    = p->arena;
	presence.class_id = p->class_id;
	presence.flags    = p->flags;
	presence_store(*sd, &presence);
	presence_relay(*sd, &presence);
}
