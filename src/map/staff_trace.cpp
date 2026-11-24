// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "staff_trace.hpp"

#include <cstring>
#include <string>

#include <common/socket.hpp>
#include <common/timer.hpp>

#include "clif.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "packets.hpp"
#include "pc.hpp"
#include "script.hpp"

// La fenêtre du plafond d'annonces, en millisecondes.
static constexpr t_tick kStaffTraceWindowMs = 1000;

// Le contexte d'une annonce : d'où elle vient, quel PNJ, quelle fonction.
struct s_staff_trace_context {
	uint8 source = MOONLIGHT_STAFF_TRACE_SOURCE_SERVER;
	uint32 npc_id = 0;
	std::string npc_name;
	std::string function_name;
};

// La famille du réglage qu'une portée réclame.
static uint8 staff_trace_family_of( uint8 scope ){
	switch( scope ){
		case MOONLIGHT_STAFF_TRACE_SCOPE_CHARACTER:
		case MOONLIGHT_STAFF_TRACE_SCOPE_ACCOUNT:
		case MOONLIGHT_STAFF_TRACE_SCOPE_GLOBAL:
			return MOONLIGHT_STAFF_TRACE_PERMANENT;
		case MOONLIGHT_STAFF_TRACE_SCOPE_TEMPORARY:
			return MOONLIGHT_STAFF_TRACE_TEMPORARY;
		case MOONLIGHT_STAFF_TRACE_SCOPE_NPC:
		case MOONLIGHT_STAFF_TRACE_SCOPE_INSTANCE:
			return MOONLIGHT_STAFF_TRACE_NPC;
		default:
			return 0;
	}
}

// La portée que dit le préfixe d'un nom de variable. Les variables du serveur
// (`$`) et de portée de fonction (`.@`) n'en ont pas : elles ne se tracent pas.
static uint8 staff_trace_scope_of( const char* name ){
	if( name == nullptr || name[0] == '\0' )
		return MOONLIGHT_STAFF_TRACE_SCOPE_NONE;

	switch( name[0] ){
		case '$':
			return MOONLIGHT_STAFF_TRACE_SCOPE_NONE;
		case '@':
			return MOONLIGHT_STAFF_TRACE_SCOPE_TEMPORARY;
		case '#':
			return name[1] == '#' ? MOONLIGHT_STAFF_TRACE_SCOPE_GLOBAL : MOONLIGHT_STAFF_TRACE_SCOPE_ACCOUNT;
		case '.':
			return name[1] == '@' ? MOONLIGHT_STAFF_TRACE_SCOPE_NONE : MOONLIGHT_STAFF_TRACE_SCOPE_NPC;
		case '\'':
			return MOONLIGHT_STAFF_TRACE_SCOPE_INSTANCE;
		default:
			return MOONLIGHT_STAFF_TRACE_SCOPE_CHARACTER;
	}
}

// Vrai si `sd` doit recevoir les annonces de `family`.
static bool staff_trace_wanted( const map_session_data* sd, uint8 family ){
	if( sd == nullptr || family == 0 || !sd->state.client_verified )
		return false;
	if( ( sd->moonlight_staff_trace & family ) == 0 )
		return false;
	if( pc_get_group_level( sd ) < MOONLIGHT_STAFF_TRACE_MIN_LEVEL )
		return false;

	return session_isActive( sd->fd );
}

// Le contexte de `st` : le PNJ qui l'exécute et la fonction en cours.
static s_staff_trace_context staff_trace_context_of( const map_session_data* sd, const script_state* st ){
	s_staff_trace_context context;

	if( st != nullptr ){
		context.source = MOONLIGHT_STAFF_TRACE_SOURCE_SCRIPT;
		context.npc_id = static_cast<uint32>( st->oid );
		if( st->funcname != nullptr )
			context.function_name = st->funcname;

		if( const npc_data* nd = map_id2nd( st->oid ); nd != nullptr )
			context.npc_name = nd->exname;
	}

	if( sd != nullptr && sd->moonlight_trace_command_depth > 0 )
		context.source = MOONLIGHT_STAFF_TRACE_SOURCE_COMMAND;

	return context;
}

// Ajoute `text` et son zéro à `tail`, borné à MOONLIGHT_STAFF_TRACE_TEXT_MAX.
static void staff_trace_append( std::string& tail, const char* text ){
	if( text != nullptr )
		tail.append( text, strnlen( text, MOONLIGHT_STAFF_TRACE_TEXT_MAX - 1 ) );
	tail.push_back( '\0' );
}

// Écrit un paquet : l'en-tête `fixed` (type et longueur posés ici), puis `tail`.
static void staff_trace_write( map_session_data* sd, PACKET_ZC_MOONLIGHT_STAFF_TRACE& fixed, const std::string& tail ){
	const size_t length = sizeof( fixed ) + tail.size();

	fixed.packetType = HEADER_ZC_MOONLIGHT_STAFF_TRACE;
	fixed.packetLength = static_cast<int16>( length );

	const int32 fd = sd->fd;

	WFIFOHEAD( fd, length );
	memcpy( WFIFOP( fd, 0 ), &fixed, sizeof( fixed ) );
	memcpy( WFIFOP( fd, sizeof( fixed ) ), tail.data(), tail.size() );
	WFIFOSET( fd, length );
}

// Vrai si une annonce peut encore partir dans la fenêtre courante. À l'ouverture
// d'une fenêtre neuve, dit d'abord combien se sont tues dans la précédente.
static bool staff_trace_budget( map_session_data* sd ){
	const t_tick now = gettick();

	if( DIFF_TICK( now, sd->moonlight_trace_window ) >= kStaffTraceWindowMs ){
		sd->moonlight_trace_window = now;
		sd->moonlight_trace_sent = 0;

		if( sd->moonlight_trace_dropped > 0 ){
			PACKET_ZC_MOONLIGHT_STAFF_TRACE dropped{};

			dropped.kind = MOONLIGHT_STAFF_TRACE_KIND_DROPPED;
			dropped.before_num = sd->moonlight_trace_dropped;
			sd->moonlight_trace_dropped = 0;
			staff_trace_write( sd, dropped, std::string( 5, '\0' ) );
		}
	}

	if( sd->moonlight_trace_sent >= MOONLIGHT_STAFF_TRACE_BURST ){
		sd->moonlight_trace_dropped++;
		return false;
	}

	sd->moonlight_trace_sent++;
	return true;
}

// Annonce une variable à `sd`. Les textes valent pour une chaîne, les nombres
// pour un entier.
static void staff_trace_variable( map_session_data* sd, const script_state* st, int64 uid, const char* name, uint8 scope, bool is_string, int64 before_num, int64 after_num, const char* before_str, const char* after_str ){
	if( !staff_trace_budget( sd ) )
		return;

	const s_staff_trace_context context = staff_trace_context_of( sd, st );
	PACKET_ZC_MOONLIGHT_STAFF_TRACE fixed{};

	fixed.kind = MOONLIGHT_STAFF_TRACE_KIND_VARIABLE;
	fixed.scope = scope;
	fixed.source = context.source;
	fixed.is_string = is_string ? 1 : 0;
	fixed.npc_id = context.npc_id;
	fixed.array_index = script_getvaridx( uid );
	fixed.before_num = before_num;
	fixed.after_num = after_num;

	std::string tail;

	staff_trace_append( tail, name );
	staff_trace_append( tail, before_str );
	staff_trace_append( tail, after_str );
	staff_trace_append( tail, context.npc_name.c_str() );
	staff_trace_append( tail, context.function_name.c_str() );
	staff_trace_write( sd, fixed, tail );
}

// Deux textes de variable sont égaux, nullptr valant le texte vide.
static bool staff_trace_same_text( const char* left, const char* right ){
	return strcmp( left != nullptr ? left : "", right != nullptr ? right : "" ) == 0;
}

void staff_trace_player_num( map_session_data* sd, int64 uid, int64 before, int64 after ){
	if( before == after )
		return;

	const char* name = get_str( script_getvarid( uid ) );
	const uint8 scope = staff_trace_scope_of( name );

	if( !staff_trace_wanted( sd, staff_trace_family_of( scope ) ) )
		return;

	staff_trace_variable( sd, sd->st, uid, name, scope, false, before, after, nullptr, nullptr );
}

void staff_trace_player_str( map_session_data* sd, int64 uid, const char* before, const char* after ){
	if( staff_trace_same_text( before, after ) )
		return;

	const char* name = get_str( script_getvarid( uid ) );
	const uint8 scope = staff_trace_scope_of( name );

	if( !staff_trace_wanted( sd, staff_trace_family_of( scope ) ) )
		return;

	staff_trace_variable( sd, sd->st, uid, name, scope, true, 0, 0, before, after );
}

// Le joueur auquel le script `st` est attaché, s'il en a un.
static map_session_data* staff_trace_attached( const script_state* st ){
	return st != nullptr && st->rid != 0 ? map_id2sd( st->rid ) : nullptr;
}

void staff_trace_script_num( script_state* st, int64 uid, const char* name, int64 before, int64 after ){
	if( before == after )
		return;

	const uint8 scope = staff_trace_scope_of( name );
	map_session_data* sd = staff_trace_attached( st );

	if( !staff_trace_wanted( sd, staff_trace_family_of( scope ) ) )
		return;

	staff_trace_variable( sd, st, uid, name, scope, false, before, after, nullptr, nullptr );
}

void staff_trace_script_str( script_state* st, int64 uid, const char* name, const char* before, const char* after ){
	if( staff_trace_same_text( before, after ) )
		return;

	const uint8 scope = staff_trace_scope_of( name );
	map_session_data* sd = staff_trace_attached( st );

	if( !staff_trace_wanted( sd, staff_trace_family_of( scope ) ) )
		return;

	staff_trace_variable( sd, st, uid, name, scope, true, 0, 0, before, after );
}

void staff_trace_quest( map_session_data* sd, int32 quest_id, int32 before, int32 after ){
	if( before == after || !staff_trace_wanted( sd, MOONLIGHT_STAFF_TRACE_QUESTS ) )
		return;
	if( !staff_trace_budget( sd ) )
		return;

	const s_staff_trace_context context = staff_trace_context_of( sd, sd->st );
	PACKET_ZC_MOONLIGHT_STAFF_TRACE fixed{};

	fixed.kind = MOONLIGHT_STAFF_TRACE_KIND_QUEST;
	fixed.source = context.source;
	fixed.npc_id = context.npc_id;
	fixed.quest_id = quest_id;
	fixed.before_num = before;
	fixed.after_num = after;

	std::string tail;

	staff_trace_append( tail, nullptr );
	staff_trace_append( tail, nullptr );
	staff_trace_append( tail, nullptr );
	staff_trace_append( tail, context.npc_name.c_str() );
	staff_trace_append( tail, context.function_name.c_str() );
	staff_trace_write( sd, fixed, tail );
}

void staff_trace_command_begin( map_session_data* sd ){
	if( sd != nullptr )
		sd->moonlight_trace_command_depth++;
}

void staff_trace_command_end( map_session_data* sd ){
	if( sd != nullptr && sd->moonlight_trace_command_depth > 0 )
		sd->moonlight_trace_command_depth--;
}
