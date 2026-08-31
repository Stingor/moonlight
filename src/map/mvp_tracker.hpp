// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MVP_TRACKER_HPP
#define MVP_TRACKER_HPP

#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp> // NAME_LENGTH

class map_session_data;
struct mob_data;

/**
 * MVP tracker - shared hunting log.
 *
 * Three ideas, and they are not interchangeable:
 *
 *  - a SLOT is a recurring MVP opportunity (a place where an MVP comes back on a
 *    schedule). The registry of slots is derived from the spawn data at boot, so
 *    no respawn constant is ever copied into C++;
 *  - a GROUP is a set of Moonlight accounts that share what they see. One account
 *    belongs to at most one group, and that rule is enforced by the SQL schema
 *    (PRIMARY KEY on mvp_group_member.user_id), not by application code;
 *  - an OBSERVATION is what a group knows about a slot. It is never "the state of
 *    the MVP": it carries who said it, when, and how precisely.
 *
 * The line that must stay true everywhere: the tracker may know THE LAW
 * (delay1, delay2, both public - they are written in the spawn script) but never
 * THE DRAW (respawn_tick), except where a player has paid for it with a Convex
 * Mirror. See mvp_tracker_earn_exact().
 *
 * See docs/mvp_tracker.md and docs/mvp_tracker_blueprint.md (Bourgeon repository).
 */

#define MVP_GROUP_NAME_LEN 32     ///< bytes. The SQL column is varchar(32) CHARACTERS, so it never truncates.
#define MVP_GROUP_MAX_MEMBERS 24  ///< bounds the packet, the broadcast and the appetite (blueprint 10.3)
#define MVP_INVITE_DAYS 7         ///< an invitation older than this is gone (blueprint 10.4)

/* ------------------------------------------------------------------------- *
 * Slots
 * ------------------------------------------------------------------------- */

/// Nature of a slot. Decides how the client words the countdown, and how the
/// server learns that the MVP died.
enum e_mvp_slot_kind : uint8 {
	MVP_SLOT_BOSS_SPAWN = 0,  ///< `boss_monster`: the map-server's own spawn_timer
	MVP_SLOT_SCRIPT_TIMER,    ///< Bio Lab: NPC timer, the mob_id is drawn every cycle
	MVP_SLOT_SCRIPT_INVASION, ///< Lord of Death: NPC timer, the position is drawn
	MVP_SLOT_SUMMON_LOCK,     ///< Thanatos: not a respawn, an availability lock
};

struct s_mvp_slot {
	uint16 slot_id;          ///< rank in the registry: IDENTITY ON THE WIRE, never persisted
	uint16 mob_id;           ///< 0 when the mob is drawn every cycle (Bio Lab)
	int16  mapid;            ///< internal map index
	uint32 delay1;           ///< ms, THE LAW: earliest respawn after the kill. Publishable.
	uint32 delay2;           ///< ms, amplitude of the draw on top of delay1. Publishable.
	e_mvp_slot_kind kind;
};

/// Builds (or rebuilds) the registry. Must run once every `boss_monster` line has
/// been parsed, i.e. after npc_event_do_oninit().
void mvp_tracker_build_registry( void );

/// The registry, indexed by slot_id.
const std::vector<s_mvp_slot>& mvp_tracker_slots( void );

/// Slot for a kill, or nullptr. Falls back to the map's scripted slot (mob_id 0)
/// when no exact (mob_id, mapid) pair matches: that is how Bio Lab, whose mob_id
/// changes at every cycle, resolves to a stable slot.
const s_mvp_slot* mvp_tracker_find_slot( uint16 mob_id, int16 mapid );

/* ------------------------------------------------------------------------- *
 * Observations
 * ------------------------------------------------------------------------- */

/// Where an observation comes from. THE ORDER IS THE PRECISION ORDER and the
/// overwrite rule reads it as such - do not reorder.
enum e_mvp_source : uint8 {
	MVP_SRC_MANUAL = 0,  ///< what a player types in
	MVP_SRC_TOMB,        ///< a tomb read on the spot
	MVP_SRC_KILL,        ///< a member killed it: the server saw it happen
	MVP_SRC_MIRROR,      ///< a member's Convex Mirror: the real instant, paid for
};

/// What a group knows about one slot. An observation, never a state.
struct s_mvp_obs {
	e_mvp_source source;
	int64  kill_time;      ///< UNIX, 0 if unknown
	int64  exact_respawn;  ///< UNIX. 0 = NOT EARNED, and the client then draws a WINDOW
	uint16 mob_id;         ///< what actually fell (Bio Lab: varies from cycle to cycle)
	int16  tomb_x;         ///< -1 = position unknown. NEVER 0,0: that is a valid cell,
	int16  tomb_y;         ///<   and confusing the two is the native Convex Mirror's bug.
	uint32 by_user_id;     ///< who supplied it - shown, the group runs on trust
	int64  reported_at;    ///< UNIX. The AGE is part of the information.
	char   killer_name[NAME_LENGTH];
};

/* ------------------------------------------------------------------------- *
 * Groups
 * ------------------------------------------------------------------------- */

struct s_mvp_group {
	uint32 group_id;
	uint32 owner_user_id;
	char   name[MVP_GROUP_NAME_LEN];
	std::vector<uint32> members;                 ///< user_id, mirror of the SQL rows
	std::unordered_map<uint16, s_mvp_obs> obs;   ///< slot_id -> observation. RAM ONLY: dies with the server.
	/// Broadcast index. A VECTOR, not a pointer: one Moonlight account may have
	/// several game accounts online at once - that is the whole point of keying on
	/// user_id. There is no user_id -> sessions index in the map-server.
	std::vector<map_session_data*> online;
};

enum e_mvp_group_result : uint8 {
	MVP_GROUP_OK = 0,
	MVP_GROUP_ERR_NO_ACCOUNT,     ///< user_id 0: game account not tied to a Moonlight one
	MVP_GROUP_ERR_ALREADY_MEMBER,
	MVP_GROUP_ERR_NOT_MEMBER,
	MVP_GROUP_ERR_NOT_OWNER,
	MVP_GROUP_ERR_NO_SUCH_USER,
	MVP_GROUP_ERR_SELF,
	MVP_GROUP_ERR_FULL,
	MVP_GROUP_ERR_BAD_NAME,
	MVP_GROUP_ERR_NO_INVITE,
	MVP_GROUP_ERR_TARGET_IN_GROUP,
	MVP_GROUP_ERR_SQL,
	// ⚠ Les valeurs partent sur le fil (champ `result` de ZC_BOURGEON_MVP_GROUP) :
	// toute nouvelle entrée s'AJOUTE EN FIN, jamais au milieu.
	//
	// La cible est déjà dans CE groupe-ci - typiquement une autre tête du même
	// compte Moonlight. Distinct de TARGET_IN_GROUP, qui parle d'un groupe rival.
	MVP_GROUP_ERR_TARGET_SAME_GROUP,
	// Le personnage EXISTE mais n'est ni dans votre guilde ni dans vos amis.
	// Distinct de NO_SUCH_USER, qui les confondait : « je ne le trouve pas » et
	// « je le trouve mais vous n'avez pas le droit » appellent deux gestes
	// différents du joueur - vérifier l'orthographe, ou l'ajouter en ami.
	MVP_GROUP_ERR_NOT_INVITABLE,
};

/// Loads every group and its members from SQL. Observations are NOT loaded:
/// they die with the server, by design.
void mvp_tracker_load_groups( void );

s_mvp_group* mvp_tracker_group_of_user( uint32 user_id );
s_mvp_group* mvp_tracker_group_of( const map_session_data& sd );

/// Broadcast index upkeep.
void mvp_tracker_on_login( map_session_data& sd );
void mvp_tracker_on_logout( map_session_data& sd );

e_mvp_group_result mvp_group_create( map_session_data& sd, const char* name );
e_mvp_group_result mvp_group_dissolve( map_session_data& sd );
e_mvp_group_result mvp_group_invite( map_session_data& sd, const char* char_name );
e_mvp_group_result mvp_group_accept( map_session_data& sd );
e_mvp_group_result mvp_group_decline( map_session_data& sd );
e_mvp_group_result mvp_group_leave( map_session_data& sd );
e_mvp_group_result mvp_group_kick( map_session_data& sd, const char* char_name );

/// Pending invitation for this account, or 0. Fills `out_name` (MVP_GROUP_NAME_LEN)
/// with the inviting group's name when non-null.
uint32 mvp_group_pending_invite( const map_session_data& sd, char* out_name );

/// One member as the client needs to see them. Names are NOT stored - a frozen
/// label lies from the first rename - so they are recomputed here, exactly the
/// way pc_ignorechat_load does it: the account's highest-level character.
struct s_mvp_member_view {
	uint32 user_id;
	int16  level;
	bool   online;
	char   name[NAME_LENGTH];
};

void mvp_group_member_views( const s_mvp_group& group, std::vector<s_mvp_member_view>& out );

/* ------------------------------------------------------------------------- *
 * Favourites
 * ------------------------------------------------------------------------- */

/// Slot ids this account has starred. Loaded from SQL on first use and kept in
/// memory afterwards; the persisted key is (mob_id, map_name), never slot_id,
/// which is only a rank in a registry rebuilt at every boot.
const std::vector<uint16>& mvp_favorites_of( uint32 user_id );
void mvp_favorite_set( map_session_data& sd, uint16 slot_id, bool on );

/* ------------------------------------------------------------------------- *
 * Attribution
 * ------------------------------------------------------------------------- */

/// Should mob_dead() bother calling the tracker for this corpse? Cheap on
/// purpose: it runs on EVERY monster death.
bool mvp_tracker_is_tracked( const mob_data& md );

/// A `boss_monster` just died. Called from mob_dead(), the ONLY place that still
/// knows players: further down, the killer is nothing but a name copied from the
/// tomb, and turning a name back into a user_id would cost an SQL query per kill.
void mvp_tracker_on_mvp_dead( mob_data& md, map_session_data* mvp_sd, map_session_data* first_sd );

/// A player just read an MVP tomb. Called from run_tomb() (npc.cpp).
///
/// This is what finally PRODUCES the MVP_SRC_TOMB source, which the model has
/// carried since day one with nothing to fill it: the tomb is public - anyone
/// walking past may click it - so the information is earned by being there, and
/// it is exactly the fallback for the members that mob_dead() does not credit.
///
/// Weaker than a kill and than a mirror, so the overwrite rule leaves those
/// alone; stronger than a typed-in time, which it therefore replaces.
void mvp_tracker_on_tomb_read( map_session_data& sd, mob_data& md, time_t kill_time,
	const char* killer_name );

/// Same thing for the four scripted slots, which never reach mob_dead()'s MVP
/// branch. Called by the mvptracker_report() script command from their labels.
void mvp_tracker_report_scripted( map_session_data* sd, uint16 mob_id, int16 mapid, int16 x, int16 y );

/// What a player types in. The weakest source, so it never overwrites a kill or
/// a mirror - that is the overwrite rule doing its job, not a special case.
/// Records what a player ASSERTS: the least precise source there is, and the
/// right one for both ways a claim reaches us -- typed into the log by hand,
/// or imported from an `<MVPL>` chat link somebody shared.
///
/// `tomb_x`/`tomb_y` at -1 when the spot is unknown, which is always the case
/// for a typed entry. A shared link carries them when its author had them, and
/// -1 is deliberate: (0,0) is a perfectly valid cell.
/// `shared_by` is who the claim came FROM when it was imported off a chat
/// link -- not who typed it in. It is recorded as the observation's name, the
/// same slot a kill fills with the killer: both answer "who says so".
e_mvp_group_result mvp_tracker_report_manual( map_session_data& sd, uint16 slot_id, int64 kill_time, int16 tomb_x = -1, int16 tomb_y = -1, const char* shared_by = nullptr );

/// THE ONLY function allowed to read the draw. Two callers, not three: the two
/// sites where the server has already decided that this player paid for the
/// information with a Convex Mirror (status.cpp, SC_BOSSMAPINFO).
void mvp_tracker_earn_exact( map_session_data& sd, mob_data& boss_md );

#endif /* MVP_TRACKER_HPP */
