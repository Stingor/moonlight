// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mvp_tracker.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include <common/showmsg.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "clif.hpp"
#include "map.hpp"
#include "mob.hpp"
#include "pc.hpp"

/* ------------------------------------------------------------------------- *
 * Slot registry
 * ------------------------------------------------------------------------- */

static std::vector<s_mvp_slot> mvp_slot_registry;

/// (mob_id, mapid) -> slot_id. Scripted slots are stored with mob_id 0.
static std::map<std::pair<uint16, int16>, uint16> mvp_slot_index;

/**
 * The four scripted slots.
 *
 * Nothing in the spawn data describes them: they are NPC timers, so they are the
 * only constants this file copies. Each one is `OnTimer7200000` followed by
 * `sleep rand(1,10)*60000`, and rand() is inclusive on both bounds
 * (buildin_rand -> rnd_value -> std::uniform_int_distribution), hence a window of
 * 7200000+60000 .. 7200000+600000 ms, i.e. 121 to 130 minutes.
 *
 * IF ONE OF THESE SCRIPTS CHANGES, THIS TABLE LIES. The four labels are:
 *   moon/mobs/mvps.npc           mvp_lhz_dun03::OnMyMVPDead
 *   moon/mobs/mvps.npc           mvp_lhz_dun04::OnMyMVPDead
 *   moon/mobs/mvps.npc           mvp_niflheim::OnLoDDead
 *   moon/quests/thana_quest.npc  #summon_thanatos::OnMyMobDead
 *
 * mob_id is 0 on all four: the mob varies (Bio Lab draws it every cycle), and the
 * map alone is unambiguous because none of these four maps carries a
 * `boss_monster` line (the first three are explicitly excluded in
 * moon/mobs/mvps.npc, thana_boss does not appear there at all).
 */
static const struct {
	const char* map_name;
	uint32 delay1;
	uint32 delay2;
	e_mvp_slot_kind kind;
} mvp_scripted_slots[] = {
	{ "lhz_dun03", 7260000, 540000, MVP_SLOT_SCRIPT_TIMER },
	{ "lhz_dun04", 7260000, 540000, MVP_SLOT_SCRIPT_TIMER },
	{ "niflheim",  7260000, 540000, MVP_SLOT_SCRIPT_INVASION },
	{ "thana_boss", 7260000, 540000, MVP_SLOT_SUMMON_LOCK },
};

/// Maps carrying a scripted slot, resolved at boot. Kept apart from the index so
/// that mob_dead() can rule out the common case with one linear scan of four
/// int16 instead of a hash lookup per monster death.
static std::vector<int16> mvp_scripted_maps;

/// Appends a slot unless (mob_id, mapid) is already registered.
static void mvp_registry_add( uint16 mob_id, int16 mapid, uint32 delay1, uint32 delay2, e_mvp_slot_kind kind ){
	std::pair<uint16, int16> key( mob_id, mapid );

	if( mvp_slot_index.find( key ) != mvp_slot_index.end() )
		return;

	s_mvp_slot slot;

	slot.slot_id = static_cast<uint16>( mvp_slot_registry.size() );
	slot.mob_id  = mob_id;
	slot.mapid   = mapid;
	slot.delay1  = delay1;
	slot.delay2  = delay2;
	slot.kind    = kind;

	mvp_slot_index[key] = slot.slot_id;
	mvp_slot_registry.push_back( slot );
}

/// map_foreachmob() callback: catches bosses that are alive right now.
static int32 mvp_registry_collect( mob_data* md, va_list ap ){
	if( md->spawn != nullptr && md->spawn->state.boss )
		mvp_registry_add( static_cast<uint16>( md->spawn->id ), md->m, md->spawn->delay1, md->spawn->delay2, MVP_SLOT_BOSS_SPAWN );

	return 0;
}

void mvp_tracker_build_registry( void ){
	mvp_slot_registry.clear();
	mvp_slot_index.clear();
	mvp_scripted_maps.clear();

	// `dynamic_mobs` is on: a boss whose map holds no player has NO mob_data at
	// boot, only its spawn_data parked in the map's moblist (npc.cpp,
	// map_addmobtolist). Sweeping live mobs alone would therefore miss nearly
	// every MVP - so read the moblists first...
	for( int32 m = 0; m < map_num; m++ ){
		struct map_data* mapdata = map_getmapdata( static_cast<int16>( m ) );

		if( mapdata == nullptr )
			continue;

		for( int16 j = 0; j < MAX_MOB_LIST_PER_MAP; j++ ){
			const struct spawn_data* spawn = mapdata->moblist[j];

			if( spawn != nullptr && spawn->state.boss )
				mvp_registry_add( static_cast<uint16>( spawn->id ), static_cast<int16>( m ), spawn->delay1, spawn->delay2, MVP_SLOT_BOSS_SPAWN );
		}
	}

	// ...and only then the live ones, which covers `dynamic_mobs: no` and the maps
	// whose 128 moblist slots were already full when the boss was parsed.
	map_foreachmob( mvp_registry_collect );

	size_t boss_count = mvp_slot_registry.size();

	for( const auto& scripted : mvp_scripted_slots ){
		int16 mapid = map_mapname2mapid( scripted.map_name );

		if( mapid < 0 ){
			ShowWarning( "mvp_tracker: scripted slot on unknown map '%s', skipped.\n", scripted.map_name );
			continue;
		}

		mvp_registry_add( 0, mapid, scripted.delay1, scripted.delay2, scripted.kind );
		mvp_scripted_maps.push_back( mapid );
	}

	ShowStatus( "MVP tracker: '" CL_WHITE "%u" CL_RESET "' slots (%u boss spawns, %u scripted).\n",
		static_cast<uint32>( mvp_slot_registry.size() ), static_cast<uint32>( boss_count ),
		static_cast<uint32>( mvp_slot_registry.size() - boss_count ) );
}

const std::vector<s_mvp_slot>& mvp_tracker_slots( void ){
	return mvp_slot_registry;
}

const s_mvp_slot* mvp_tracker_find_slot( uint16 mob_id, int16 mapid ){
	auto it = mvp_slot_index.find( std::pair<uint16, int16>( mob_id, mapid ) );

	if( it != mvp_slot_index.end() )
		return &mvp_slot_registry[it->second];

	// Scripted slot: the mob_id varies from cycle to cycle, the map does not.
	it = mvp_slot_index.find( std::pair<uint16, int16>( 0, mapid ) );

	if( it != mvp_slot_index.end() )
		return &mvp_slot_registry[it->second];

	return nullptr;
}

/* ------------------------------------------------------------------------- *
 * Groups
 * ------------------------------------------------------------------------- */

static std::unordered_map<uint32, s_mvp_group> mvp_group_table;  ///< group_id -> group
static std::unordered_map<uint32, uint32> mvp_member_index;      ///< user_id -> group_id

s_mvp_group* mvp_tracker_group_of_user( uint32 user_id ){
	if( user_id == 0 )
		return nullptr;

	auto member = mvp_member_index.find( user_id );

	if( member == mvp_member_index.end() )
		return nullptr;

	auto group = mvp_group_table.find( member->second );

	return group != mvp_group_table.end() ? &group->second : nullptr;
}

s_mvp_group* mvp_tracker_group_of( const map_session_data& sd ){
	return mvp_tracker_group_of_user( sd.status.user_id );
}

void mvp_tracker_load_groups( void ){
	mvp_group_table.clear();
	mvp_member_index.clear();

	// An invitation nobody answered is not a decision to preserve.
	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group_invite` WHERE `expires_at` < NOW()" ) != SQL_SUCCESS )
		Sql_ShowDebug( mmysql_handle );

	if( Sql_Query( mmysql_handle, "SELECT `group_id`, `owner_user_id`, `name` FROM `mvp_group`" ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return;
	}

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* data;
		s_mvp_group group;

		if( Sql_GetData( mmysql_handle, 0, &data, nullptr ) != SQL_SUCCESS || data == nullptr )
			continue;

		group.group_id = static_cast<uint32>( strtoul( data, nullptr, 10 ) );

		Sql_GetData( mmysql_handle, 1, &data, nullptr );
		group.owner_user_id = data != nullptr ? static_cast<uint32>( strtoul( data, nullptr, 10 ) ) : 0;

		Sql_GetData( mmysql_handle, 2, &data, nullptr );
		safestrncpy( group.name, data != nullptr ? data : "", MVP_GROUP_NAME_LEN );

		mvp_group_table[group.group_id] = group;
	}

	Sql_FreeResult( mmysql_handle );

	// joined_at ascending: members[0] is the oldest, which is also the successor
	// when the owner leaves. No vote, no extra column.
	if( Sql_Query( mmysql_handle, "SELECT `user_id`, `group_id` FROM `mvp_group_member` ORDER BY `group_id`, `joined_at`" ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return;
	}

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* data;
		uint32 user_id;
		uint32 group_id;

		if( Sql_GetData( mmysql_handle, 0, &data, nullptr ) != SQL_SUCCESS || data == nullptr )
			continue;

		user_id = static_cast<uint32>( strtoul( data, nullptr, 10 ) );

		Sql_GetData( mmysql_handle, 1, &data, nullptr );
		group_id = data != nullptr ? static_cast<uint32>( strtoul( data, nullptr, 10 ) ) : 0;

		auto group = mvp_group_table.find( group_id );

		if( user_id == 0 || group == mvp_group_table.end() )
			continue;

		group->second.members.push_back( user_id );
		mvp_member_index[user_id] = group_id;
	}

	Sql_FreeResult( mmysql_handle );

	ShowStatus( "MVP tracker: '" CL_WHITE "%u" CL_RESET "' groups, %u members.\n",
		static_cast<uint32>( mvp_group_table.size() ), static_cast<uint32>( mvp_member_index.size() ) );
}

void mvp_tracker_on_login( map_session_data& sd ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return;

	if( std::find( group->online.begin(), group->online.end(), &sd ) == group->online.end() )
		group->online.push_back( &sd );
}

void mvp_tracker_on_logout( map_session_data& sd ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return;

	auto it = std::find( group->online.begin(), group->online.end(), &sd );

	if( it != group->online.end() )
		group->online.erase( it );
}

/// Drops a member from RAM and from SQL. Does NOT handle succession or the last
/// member leaving: mvp_group_leave() owns those decisions.
static void mvp_group_remove_member( s_mvp_group& group, uint32 user_id ){
	auto member = std::find( group.members.begin(), group.members.end(), user_id );

	if( member != group.members.end() )
		group.members.erase( member );

	mvp_member_index.erase( user_id );

	// Retirées de l'index de diffusion, ces sessions ne recevraient plus rien -
	// pas même la nouvelle qu'elles sont sorties. On les met de côté pour les
	// prévenir une fois l'index à jour : le paquet portera group_id 0.
	std::vector<map_session_data*> dropped;

	for( auto it = group.online.begin(); it != group.online.end(); ){
		if( ( *it )->status.user_id == user_id ){
			dropped.push_back( *it );
			it = group.online.erase( it );
		}else{
			++it;
		}
	}

	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group_member` WHERE `user_id` = '%u'", user_id ) != SQL_SUCCESS )
		Sql_ShowDebug( mmysql_handle );

	for( map_session_data* sd : dropped )
		clif_bourgeon_mvp_group( *sd );
}

/// Wipes a group and everything hanging off it. The favourites go too: they are
/// per account, not per group, so only the invitations and the membership rows
/// are group-scoped.
static void mvp_group_destroy( s_mvp_group& group ){
	uint32 group_id = group.group_id;
	std::vector<map_session_data*> dropped = group.online;

	for( uint32 user_id : group.members )
		mvp_member_index.erase( user_id );

	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group_member` WHERE `group_id` = '%u'", group_id ) != SQL_SUCCESS )
		Sql_ShowDebug( mmysql_handle );

	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group_invite` WHERE `group_id` = '%u'", group_id ) != SQL_SUCCESS )
		Sql_ShowDebug( mmysql_handle );

	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group` WHERE `group_id` = '%u'", group_id ) != SQL_SUCCESS )
		Sql_ShowDebug( mmysql_handle );

	mvp_group_table.erase( group_id );

	// Après l'effacement, sinon le paquet décrirait encore le groupe détruit.
	for( map_session_data* sd : dropped )
		clif_bourgeon_mvp_group( *sd );
}

e_mvp_group_result mvp_group_create( map_session_data& sd, const char* name ){
	if( sd.status.user_id == 0 )
		return MVP_GROUP_ERR_NO_ACCOUNT;

	if( mvp_tracker_group_of( sd ) != nullptr )
		return MVP_GROUP_ERR_ALREADY_MEMBER;

	if( name == nullptr || name[0] == '\0' || strlen( name ) >= MVP_GROUP_NAME_LEN )
		return MVP_GROUP_ERR_BAD_NAME;

	char esc_name[MVP_GROUP_NAME_LEN * 2 + 1];

	Sql_EscapeStringLen( mmysql_handle, esc_name, name, strnlen( name, MVP_GROUP_NAME_LEN - 1 ) );

	if( Sql_Query( mmysql_handle, "INSERT INTO `mvp_group` (`owner_user_id`, `name`) VALUES ('%u', '%s')",
			sd.status.user_id, esc_name ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return MVP_GROUP_ERR_SQL;
	}

	uint32 group_id = static_cast<uint32>( Sql_LastInsertId( mmysql_handle ) );

	if( Sql_Query( mmysql_handle, "INSERT INTO `mvp_group_member` (`user_id`, `group_id`) VALUES ('%u', '%u')",
			sd.status.user_id, group_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );

		if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group` WHERE `group_id` = '%u'", group_id ) != SQL_SUCCESS )
			Sql_ShowDebug( mmysql_handle );

		return MVP_GROUP_ERR_SQL;
	}

	s_mvp_group group;

	group.group_id      = group_id;
	group.owner_user_id = sd.status.user_id;
	safestrncpy( group.name, name, MVP_GROUP_NAME_LEN );
	group.members.push_back( sd.status.user_id );

	mvp_group_table[group_id] = group;
	mvp_member_index[sd.status.user_id] = group_id;
	mvp_tracker_on_login( sd );

	return MVP_GROUP_OK;
}

e_mvp_group_result mvp_group_dissolve( map_session_data& sd ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return MVP_GROUP_ERR_NOT_MEMBER;

	if( group->owner_user_id != sd.status.user_id )
		return MVP_GROUP_ERR_NOT_OWNER;

	mvp_group_destroy( *group );

	return MVP_GROUP_OK;
}

/**
 * Is `target` someone `sd` is allowed to invite?
 *
 * Guild mates and friends only - no strangers, which removes moderation,
 * blocking and spam in one go (docs/mvp_tracker.md 2.1). Both checks are made at
 * CHARACTER level, which is right: the gate is on who you may invite, while the
 * invitation itself lands on their whole Moonlight account.
 */
static bool mvp_group_may_invite( const map_session_data& sd, const char* target_name ){
	for( int32 i = 0; i < MAX_FRIENDS; i++ ){
		if( sd.status.friends[i].char_id == 0 )
			break;

		if( strncmp( sd.status.friends[i].name, target_name, NAME_LENGTH ) == 0 )
			return true;
	}

	if( sd.status.guild_id == 0 )
		return false;

	map_session_data* tsd = map_nick2sd( target_name, false );

	if( tsd != nullptr )
		return tsd->status.guild_id == sd.status.guild_id;

	// Offline guild mate: the friend list held nothing, so the guild is the only
	// remaining reason, and only the `char` table knows it.
	char esc_name[NAME_LENGTH * 2 + 1];

	Sql_EscapeStringLen( mmysql_handle, esc_name, target_name, strnlen( target_name, NAME_LENGTH - 1 ) );

	if( Sql_Query( mmysql_handle, "SELECT `guild_id` FROM `char` WHERE `name` = '%s' LIMIT 1", esc_name ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return false;
	}

	bool same_guild = false;

	if( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* data;

		if( Sql_GetData( mmysql_handle, 0, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
			same_guild = static_cast<int32>( strtol( data, nullptr, 10 ) ) == sd.status.guild_id;
	}

	Sql_FreeResult( mmysql_handle );

	return same_guild;
}

/// map_foreachpc() callback : pousse l'invitation en attente à chaque session du
/// compte visé.
static int32 mvp_invite_notify( map_session_data* sd, va_list ap ){
	const uint32 target_id = va_arg( ap, uint32 );

	if( sd != nullptr && sd->status.user_id == target_id )
		clif_bourgeon_mvp_invite( *sd );

	return 0;
}

e_mvp_group_result mvp_group_invite( map_session_data& sd, const char* char_name ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return MVP_GROUP_ERR_NOT_MEMBER;

	if( group->members.size() >= MVP_GROUP_MAX_MEMBERS )
		return MVP_GROUP_ERR_FULL;

	uint32 target_id = pc_ignorechat_name2userid( char_name );

	if( target_id == 0 )
		return MVP_GROUP_ERR_NO_SUCH_USER;

	if( target_id == sd.status.user_id )
		return MVP_GROUP_ERR_SELF;

	s_mvp_group* target_group = mvp_tracker_group_of_user( target_id );

	if( target_group != nullptr ){
		// Inviter une AUTRE TÊTE d'un compte déjà membre du même groupe n'est pas
		// une erreur du joueur, c'est un malentendu sur l'identité : le compte est
		// déjà dedans, avec tous ses personnages. On le dit tel quel plutôt que
		// « appartient déjà à un groupe », qui laisse croire à un groupe rival.
		if( target_group->group_id == group->group_id )
			return MVP_GROUP_ERR_TARGET_SAME_GROUP;

		return MVP_GROUP_ERR_TARGET_IN_GROUP;
	}

	if( !mvp_group_may_invite( sd, char_name ) )
		return MVP_GROUP_ERR_NOT_INVITABLE;

	if( Sql_Query( mmysql_handle,
			"REPLACE INTO `mvp_group_invite` (`group_id`, `user_id`, `from_user_id`, `expires_at`) "
			"VALUES ('%u', '%u', '%u', DATE_ADD(NOW(), INTERVAL %d DAY))",
			group->group_id, target_id, sd.status.user_id, MVP_INVITE_DAYS ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return MVP_GROUP_ERR_SQL;
	}

	// 🔴 POUSSER L'INVITATION, comme le fait une demande d'ami.
	//
	// Elle n'était qu'écrite en base, et le destinataire ne l'apprenait qu'en
	// demandant un instantané — c'est-à-dire en ouvrant le carnet de lui-même.
	// Une invitation qu'il faut aller chercher n'en est pas une.
	//
	// Le balayage des sessions est assumé : il n'existe aucun index
	// `user_id -> sessions` dans le map-server, et une invitation est un geste
	// rare. Un même compte Moonlight peut avoir plusieurs comptes de jeu
	// connectés : on les prévient TOUS, sinon elle tomberait sur la tête que le
	// joueur ne regarde pas.
	map_foreachpc( mvp_invite_notify, target_id );

	return MVP_GROUP_OK;
}

uint32 mvp_group_pending_invite( const map_session_data& sd, char* out_name ){
	if( sd.status.user_id == 0 )
		return 0;

	if( Sql_Query( mmysql_handle,
			"SELECT `i`.`group_id`, `g`.`name` FROM `mvp_group_invite` AS `i` "
			"JOIN `mvp_group` AS `g` ON `g`.`group_id` = `i`.`group_id` "
			"WHERE `i`.`user_id` = '%u' AND `i`.`expires_at` >= NOW() "
			"ORDER BY `i`.`expires_at` DESC LIMIT 1",
			sd.status.user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return 0;
	}

	uint32 group_id = 0;

	if( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* data;

		if( Sql_GetData( mmysql_handle, 0, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
			group_id = static_cast<uint32>( strtoul( data, nullptr, 10 ) );

		if( out_name != nullptr && Sql_GetData( mmysql_handle, 1, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
			safestrncpy( out_name, data, MVP_GROUP_NAME_LEN );
	}

	Sql_FreeResult( mmysql_handle );

	return group_id;
}

e_mvp_group_result mvp_group_accept( map_session_data& sd ){
	if( sd.status.user_id == 0 )
		return MVP_GROUP_ERR_NO_ACCOUNT;

	// Explicitly refused, never a silent departure (docs/mvp_tracker.md 2.1).
	if( mvp_tracker_group_of( sd ) != nullptr )
		return MVP_GROUP_ERR_ALREADY_MEMBER;

	uint32 group_id = mvp_group_pending_invite( sd, nullptr );

	if( group_id == 0 )
		return MVP_GROUP_ERR_NO_INVITE;

	auto entry = mvp_group_table.find( group_id );

	if( entry == mvp_group_table.end() )
		return MVP_GROUP_ERR_NO_INVITE;

	s_mvp_group& group = entry->second;

	if( group.members.size() >= MVP_GROUP_MAX_MEMBERS )
		return MVP_GROUP_ERR_FULL;

	// The schema, not this code, is what makes "one account = at most one group"
	// true: PRIMARY KEY (user_id) makes a second membership impossible.
	if( Sql_Query( mmysql_handle, "INSERT INTO `mvp_group_member` (`user_id`, `group_id`) VALUES ('%u', '%u')",
			sd.status.user_id, group_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return MVP_GROUP_ERR_SQL;
	}

	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group_invite` WHERE `user_id` = '%u'", sd.status.user_id ) != SQL_SUCCESS )
		Sql_ShowDebug( mmysql_handle );

	group.members.push_back( sd.status.user_id );
	mvp_member_index[sd.status.user_id] = group_id;
	mvp_tracker_on_login( sd );

	return MVP_GROUP_OK;
}

e_mvp_group_result mvp_group_decline( map_session_data& sd ){
	if( sd.status.user_id == 0 )
		return MVP_GROUP_ERR_NO_ACCOUNT;

	if( mvp_group_pending_invite( sd, nullptr ) == 0 )
		return MVP_GROUP_ERR_NO_INVITE;

	if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_group_invite` WHERE `user_id` = '%u'", sd.status.user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return MVP_GROUP_ERR_SQL;
	}

	return MVP_GROUP_OK;
}

e_mvp_group_result mvp_group_leave( map_session_data& sd ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return MVP_GROUP_ERR_NOT_MEMBER;

	bool was_owner = group->owner_user_id == sd.status.user_id;

	mvp_group_remove_member( *group, sd.status.user_id );

	// The last one out takes the group with them: an empty group would keep
	// answering invitations nobody owns.
	if( group->members.empty() ){
		mvp_group_destroy( *group );
		return MVP_GROUP_OK;
	}

	// members is loaded and maintained in joined_at order, so members[0] is the
	// oldest. Succession without a vote.
	if( was_owner ){
		group->owner_user_id = group->members.front();

		if( Sql_Query( mmysql_handle, "UPDATE `mvp_group` SET `owner_user_id` = '%u' WHERE `group_id` = '%u'",
				group->owner_user_id, group->group_id ) != SQL_SUCCESS )
			Sql_ShowDebug( mmysql_handle );
	}

	return MVP_GROUP_OK;
}

e_mvp_group_result mvp_group_kick( map_session_data& sd, const char* char_name ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return MVP_GROUP_ERR_NOT_MEMBER;

	if( group->owner_user_id != sd.status.user_id )
		return MVP_GROUP_ERR_NOT_OWNER;

	uint32 target_id = pc_ignorechat_name2userid( char_name );

	if( target_id == 0 )
		return MVP_GROUP_ERR_NO_SUCH_USER;

	if( target_id == sd.status.user_id )
		return MVP_GROUP_ERR_SELF;

	if( std::find( group->members.begin(), group->members.end(), target_id ) == group->members.end() )
		return MVP_GROUP_ERR_NOT_MEMBER;

	mvp_group_remove_member( *group, target_id );

	return MVP_GROUP_OK;
}

/* ------------------------------------------------------------------------- *
 * Attribution
 * ------------------------------------------------------------------------- */

/**
 * The overwrite rule, in one place.
 *
 * An observation replaces the previous one if its source is strictly more
 * precise, or if the source is equal and it is more recent. No human arbitration,
 * no conflict to display.
 */
static void mvp_tracker_record( s_mvp_group& group, uint16 slot_id, const s_mvp_obs& obs ){
	auto it = group.obs.find( slot_id );

	if( it != group.obs.end() ){
		const s_mvp_obs& current = it->second;

		if( obs.source < current.source )
			return;

		if( obs.source == current.source && obs.reported_at <= current.reported_at )
			return;
	}

	group.obs[slot_id] = obs;

	// Something actually changed, so - and only so - the group hears about it.
	// Members without Bourgeon, or with the window switched off, receive nothing:
	// they still FEED the group with their kills, which is the right behaviour and
	// not a degraded case.
	clif_bourgeon_mvp_delta( group, slot_id, obs );
}

/// Is this map one of the four whose MVP is spawned by an NPC timer?
static bool mvp_tracker_map_is_scripted( int16 mapid ){
	return std::find( mvp_scripted_maps.begin(), mvp_scripted_maps.end(), mapid ) != mvp_scripted_maps.end();
}

void mvp_tracker_on_mvp_dead( mob_data& md, map_session_data* mvp_sd, map_session_data* first_sd ){
	// A scripted MVP has no spawn data, so the slot is found by map alone; a
	// `boss_monster` is found by (spawn->id, map). md.mob_id is what FELL, which
	// is not the same thing on Bio Lab, where the class is drawn every cycle.
	uint16 spawn_id = md.spawn != nullptr ? static_cast<uint16>( md.spawn->id ) : 0;
	const s_mvp_slot* slot = mvp_tracker_find_slot( spawn_id, md.m );

	if( slot == nullptr )
		return;

	map_session_data* credited_by = mvp_sd != nullptr ? mvp_sd : first_sd;

	s_mvp_obs obs;

	obs.source = MVP_SRC_KILL;
	obs.kill_time = static_cast<int64>( time( nullptr ) );
	// NOT earned: the draw belongs to the server until a Convex Mirror pays for
	// it. The client draws a window from delay1/delay2 instead.
	obs.exact_respawn = 0;
	obs.mob_id = static_cast<uint16>( md.mob_id );
	obs.tomb_x = md.x;
	obs.tomb_y = md.y;
	obs.by_user_id = credited_by != nullptr ? credited_by->status.user_id : 0;
	obs.reported_at = obs.kill_time;
	safestrncpy( obs.killer_name, credited_by != nullptr ? credited_by->status.name : "", NAME_LENGTH );

	// ONLY mvp_sd and first_sd are credited - the two players rAthena itself
	// considers to have earned the MVP: the top damager and the one with loot
	// priority. Crediting the whole dmglog was rejected: a single bolt for one
	// point of damage puts a passer-by in that log, and their entire hunting
	// group would inherit the kill.
	//
	// The cost is accepted and known: in a party of twelve, the ten who are
	// neither of those two do not feed their group automatically. They can still
	// read the tomb on the spot, or type the time in - both are legitimate
	// sources, just weaker ones.
	map_session_data* earners[2] = { mvp_sd, first_sd };
	uint32 credited_group = 0;

	for( map_session_data* earner : earners ){
		if( earner == nullptr )
			continue;

		s_mvp_group* group = mvp_tracker_group_of( *earner );

		if( group == nullptr )
			continue;

		// mvp_sd and first_sd are often the same person, and even when they are
		// not they may share a group: the observation is written once.
		if( group->group_id == credited_group )
			continue;

		credited_group = group->group_id;
		mvp_tracker_record( *group, slot->slot_id, obs );
	}
}

/// Should mob_dead() bother calling the tracker for this corpse?
///
/// Cheap on purpose: this runs on EVERY monster death. A `boss_monster` is
/// settled by its spawn flag; on the four scripted maps the MVP is a plain
/// `monster`, so it is told from the level-99 decoys spawned alongside it by its
/// boss type - B_SEYREN carries MvpExp, G_SEYREN does not.
bool mvp_tracker_is_tracked( const mob_data& md ){
	if( md.spawn != nullptr && md.spawn->state.boss )
		return true;

	return mvp_tracker_map_is_scripted( md.m ) && md.get_bosstype() == BOSSTYPE_MVP;
}

void mvp_tracker_on_tomb_read( map_session_data& sd, mob_data& md, time_t kill_time,
	const char* killer_name ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return;

	// La tombe est posée sur la carte du MVP, à l'endroit de sa mort : le créneau
	// se retrouve donc par (classe de spawn, carte) comme partout ailleurs.
	uint16 spawn_id = md.spawn != nullptr ? static_cast<uint16>( md.spawn->id ) : 0;
	const s_mvp_slot* slot = mvp_tracker_find_slot( spawn_id, md.m );

	if( slot == nullptr )
		return;

	s_mvp_obs obs;

	obs.source = MVP_SRC_TOMB;
	obs.kill_time = static_cast<int64>( kill_time );
	// Toujours pas mérité : la tombe dit QUAND il est mort, jamais quand il
	// revient. Le client en tire une fenêtre depuis delay1/delay2, comme pour un
	// kill.
	obs.exact_respawn = 0;
	obs.mob_id = static_cast<uint16>( md.mob_id );
	// La tombe EST à l'endroit de la mort - c'est tout son intérêt ici, et cette
	// position-là ne s'obtient autrement qu'en ayant tué le MVP soi-même.
	obs.tomb_x = md.x;
	obs.tomb_y = md.y;
	obs.by_user_id = sd.status.user_id;
	obs.reported_at = static_cast<int64>( time( nullptr ) );
	safestrncpy( obs.killer_name, killer_name != nullptr ? killer_name : "", NAME_LENGTH );

	mvp_tracker_record( *group, slot->slot_id, obs );
}

void mvp_tracker_report_scripted( map_session_data* sd, uint16 mob_id, int16 mapid, int16 x, int16 y ){
	const s_mvp_slot* slot = mvp_tracker_find_slot( mob_id, mapid );

	if( slot == nullptr || sd == nullptr )
		return;

	s_mvp_group* group = mvp_tracker_group_of( *sd );

	if( group == nullptr )
		return;

	s_mvp_obs obs;

	obs.source = MVP_SRC_KILL;
	obs.kill_time = static_cast<int64>( time( nullptr ) );
	obs.exact_respawn = 0;
	obs.mob_id = mob_id;
	obs.tomb_x = x;
	obs.tomb_y = y;
	obs.by_user_id = sd->status.user_id;
	obs.reported_at = obs.kill_time;
	safestrncpy( obs.killer_name, sd->status.name, NAME_LENGTH );

	mvp_tracker_record( *group, slot->slot_id, obs );
}

void mvp_tracker_earn_exact( map_session_data& sd, mob_data& boss_md ){
	if( boss_md.spawn == nullptr )
		return;

	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return;

	uint16 spawn_id = static_cast<uint16>( boss_md.spawn->id );
	const s_mvp_slot* slot = mvp_tracker_find_slot( spawn_id, boss_md.m );

	if( slot == nullptr )
		return;

	auto it = mvp_respawn_cache.find( s_mvp_respawn_key( spawn_id, boss_md.m ) );

	if( it == mvp_respawn_cache.end() )
		return;

	const s_mvp_respawn_info& info = it->second;

	s_mvp_obs obs;

	obs.source = MVP_SRC_MIRROR;
	obs.kill_time = info.kill_time;
	// To the second. The native mirror adds +60 s then truncates to hours and
	// minutes, which is where its 1-2 minutes of lag come from; published as an
	// absolute UNIX instant, the reason for that margin does not exist.
	obs.exact_respawn = static_cast<int64>( time( nullptr ) ) + DIFF_TICK( info.respawn_tick, gettick() ) / 1000;
	obs.mob_id = spawn_id;
	// The cache writes 0,0 when the mob left no tomb NPC. Here that has to become
	// -1: 0,0 is a VALID cell, and confusing the two is exactly the native
	// mirror's bug (icon parked in the bottom-left corner).
	obs.tomb_x = ( info.tomb_x != 0 || info.tomb_y != 0 ) ? info.tomb_x : -1;
	obs.tomb_y = ( info.tomb_x != 0 || info.tomb_y != 0 ) ? info.tomb_y : -1;
	obs.by_user_id = sd.status.user_id;
	obs.reported_at = static_cast<int64>( time( nullptr ) );
	safestrncpy( obs.killer_name, info.killer_name, NAME_LENGTH );

	mvp_tracker_record( *group, slot->slot_id, obs );
}

/* ------------------------------------------------------------------------- *
 * Member views
 * ------------------------------------------------------------------------- */

void mvp_group_member_views( const s_mvp_group& group, std::vector<s_mvp_member_view>& out ){
	out.clear();

	if( group.members.empty() )
		return;

	// 🔴 UNE LIGNE PAR SESSION EN LIGNE, pas par compte. Un compte Moonlight peut
	// avoir plusieurs comptes de jeu connectés en même temps - c'est même le
	// propos de l'identité par user_id - et le groupe veut voir QUI est là, pas
	// un représentant. N'en montrer qu'un (le plus haut niveau) cachait la moitié
	// des présents à qui joue en multi-client.
	//
	// L'appartenance, elle, ne bouge pas : elle reste au COMPTE, un seul groupe
	// par compte, garanti par le schéma. Plusieurs lignes, un seul membre.
	//
	// Ordre : celui de `group.members`, donc l'ordre d'arrivée dans le groupe -
	// les sessions d'un même compte se suivent.
	std::string id_list;

	for( uint32 user_id : group.members ){
		bool any_online = false;

		for( map_session_data* sd : group.online ){
			if( sd->status.user_id != user_id )
				continue;

			// Garde-fou : le compte est un uint8 sur le fil. Vingt-quatre comptes
			// en multi-client n'y arriveront jamais, mais un dépassement passerait
			// inaperçu et tronquerait la liste sans le dire.
			if( out.size() >= 200 )
				break;

			s_mvp_member_view view;

			view.user_id = user_id;
			view.level   = static_cast<int16>( sd->status.base_level );
			view.online  = true;
			safestrncpy( view.name, sd->status.name, NAME_LENGTH );
			out.push_back( view );
			any_online = true;
		}

		if( any_online )
			continue;

		// Personne en ligne pour ce compte : une ligne, remplie par la requête
		// ci-dessous avec son personnage de plus haut niveau.
		if( !id_list.empty() )
			id_list += ",";

		id_list += std::to_string( user_id );

		s_mvp_member_view view;

		view.user_id = user_id;
		view.level   = 0;
		view.online  = false;
		view.name[0] = '\0';
		out.push_back( view );
	}

	// Rien à résoudre si tout le monde est connecté : la RAM a déjà tout dit.
	if( id_list.empty() )
		return;

	// One query for the whole group, not one per member. The rows come sorted by
	// level so the first one seen for an account is the one to keep.
	if( Sql_Query( mmysql_handle,
			"SELECT `l`.`user_id`, `c`.`name`, `c`.`base_level` FROM `login` AS `l` "
			"JOIN `char` AS `c` ON `c`.`account_id` = `l`.`account_id` "
			"WHERE `l`.`user_id` IN (%s) "
			"ORDER BY `l`.`user_id`, `c`.`base_level` DESC, `c`.`char_id` ASC",
			id_list.c_str() ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return;
	}

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* data;

		if( Sql_GetData( mmysql_handle, 0, &data, nullptr ) != SQL_SUCCESS || data == nullptr )
			continue;

		uint32 user_id = static_cast<uint32>( strtoul( data, nullptr, 10 ) );

		for( s_mvp_member_view& view : out ){
			// Already filled means either an online session (authoritative) or a
			// higher-level character seen earlier in this same result set.
			if( view.user_id != user_id || view.name[0] != '\0' )
				continue;

			if( Sql_GetData( mmysql_handle, 1, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
				safestrncpy( view.name, data, NAME_LENGTH );

			if( Sql_GetData( mmysql_handle, 2, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
				view.level = static_cast<int16>( strtol( data, nullptr, 10 ) );

			break;
		}
	}

	Sql_FreeResult( mmysql_handle );
}

/* ------------------------------------------------------------------------- *
 * Favourites
 * ------------------------------------------------------------------------- */

static std::unordered_map<uint32, std::vector<uint16>> mvp_favorite_cache;

const std::vector<uint16>& mvp_favorites_of( uint32 user_id ){
	static const std::vector<uint16> empty;

	if( user_id == 0 )
		return empty;

	auto cached = mvp_favorite_cache.find( user_id );

	if( cached != mvp_favorite_cache.end() )
		return cached->second;

	// Loaded on first use rather than at login: opening the window is rare, and a
	// query per connection for a panel most people never open is not worth it.
	std::vector<uint16>& slots = mvp_favorite_cache[user_id];

	if( Sql_Query( mmysql_handle, "SELECT `mob_id`, `map_name` FROM `mvp_favorite` WHERE `user_id` = '%u'",
			user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return slots;
	}

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* data;
		uint16 mob_id = 0;
		char map_name[MAP_NAME_LENGTH_EXT];

		map_name[0] = '\0';

		if( Sql_GetData( mmysql_handle, 0, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
			mob_id = static_cast<uint16>( strtoul( data, nullptr, 10 ) );

		if( Sql_GetData( mmysql_handle, 1, &data, nullptr ) == SQL_SUCCESS && data != nullptr )
			safestrncpy( map_name, data, MAP_NAME_LENGTH_EXT );

		int16 mapid = map_mapname2mapid( map_name );

		if( mapid < 0 )
			continue;

		// The persisted key survives a registry rebuild; the slot_id does not, so
		// it is resolved here and nowhere else.
		const s_mvp_slot* slot = mvp_tracker_find_slot( mob_id, mapid );

		if( slot != nullptr && slot->mob_id == mob_id )
			slots.push_back( slot->slot_id );
	}

	Sql_FreeResult( mmysql_handle );

	return slots;
}

void mvp_favorite_set( map_session_data& sd, uint16 slot_id, bool on ){
	if( sd.status.user_id == 0 || slot_id >= mvp_slot_registry.size() )
		return;

	const s_mvp_slot& slot = mvp_slot_registry[slot_id];
	const char* map_name = slot.mapid >= 0 ? map_getmapdata( slot.mapid )->name : "";

	char esc_map[MAP_NAME_LENGTH_EXT * 2 + 1];

	Sql_EscapeStringLen( mmysql_handle, esc_map, map_name, strnlen( map_name, MAP_NAME_LENGTH_EXT - 1 ) );

	if( on ){
		if( Sql_Query( mmysql_handle, "REPLACE INTO `mvp_favorite` (`user_id`, `mob_id`, `map_name`) VALUES ('%u', '%u', '%s')",
				sd.status.user_id, slot.mob_id, esc_map ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
			return;
		}
	}else{
		if( Sql_Query( mmysql_handle, "DELETE FROM `mvp_favorite` WHERE `user_id` = '%u' AND `mob_id` = '%u' AND `map_name` = '%s'",
				sd.status.user_id, slot.mob_id, esc_map ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
			return;
		}
	}

	std::vector<uint16>& slots = mvp_favorite_cache[sd.status.user_id];
	auto it = std::find( slots.begin(), slots.end(), slot_id );

	if( on && it == slots.end() )
		slots.push_back( slot_id );
	else if( !on && it != slots.end() )
		slots.erase( it );
}

/* ------------------------------------------------------------------------- *
 * Manual entry
 * ------------------------------------------------------------------------- */

e_mvp_group_result mvp_tracker_report_manual( map_session_data& sd, uint16 slot_id, int64 kill_time, int16 tomb_x, int16 tomb_y ){
	s_mvp_group* group = mvp_tracker_group_of( sd );

	if( group == nullptr )
		return MVP_GROUP_ERR_NOT_MEMBER;

	if( slot_id >= mvp_slot_registry.size() )
		return MVP_GROUP_ERR_NO_SUCH_USER;

	int64 now = static_cast<int64>( time( nullptr ) );

	if( kill_time <= 0 )
		kill_time = now;

	// A death cannot be in the future, and a window that already expired is not
	// worth publishing: both would only produce a countdown nobody can act on.
	if( kill_time > now )
		kill_time = now;

	s_mvp_obs obs;

	obs.source = MVP_SRC_MANUAL;
	obs.kill_time = kill_time;
	obs.exact_respawn = 0;
	obs.mob_id = mvp_slot_registry[slot_id].mob_id;
	// A shared link may carry the tomb its author read; a typed entry never can.
	// Clamped to the map, because nothing here is trusted: the numbers came off
	// a chat line, which anybody can write by hand.
	// mapid < 0 is possible on a scripted slot whose map never loaded, and
	// map_getmapdata() does not guard: it would index the array with -1.
	const int16 slot_mapid = mvp_slot_registry[slot_id].mapid;
	struct map_data* mapdata = slot_mapid >= 0 ? map_getmapdata( slot_mapid ) : nullptr;

	if( mapdata != nullptr && tomb_x >= 0 && tomb_y >= 0 &&
		tomb_x < mapdata->xs && tomb_y < mapdata->ys ){
		obs.tomb_x = tomb_x;
		obs.tomb_y = tomb_y;
	}else{
		obs.tomb_x = -1;
		obs.tomb_y = -1;
	}
	obs.by_user_id = sd.status.user_id;
	obs.reported_at = now;
	safestrncpy( obs.killer_name, "", NAME_LENGTH );

	mvp_tracker_record( *group, slot_id, obs );

	return MVP_GROUP_OK;
}
