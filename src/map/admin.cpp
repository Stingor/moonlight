// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <cstdio>
#include <cstring>
#include <new>
#include <string>

#include "../common/admin_channel.hpp"
#include "../common/malloc.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"

#include "../common/socket.hpp"

#include "atcommand.hpp"
#include "guild.hpp"
#include "map.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"

/**
 * ============================================================================
 *  Map-server administration channel: map-specific commands
 * ============================================================================
 *
 *  The listener, the configuration (conf/admin.conf), AUTH, LOG, NAME, SERVER
 *  and QUIT live in src/common/admin_channel.cpp, shared with the login and
 *  char servers. The map-server adds:
 *
 *    RELOAD <target>    shortcut for "@reload<target>" (itemdb, mobdb, skilldb,
 *                       script, battleconf, statusdb, atcommand, ...)
 *    ATCMD <command>    run any atcommand (a leading '@' is optional)
 *    PLAYERS            one ": " line per online character, fields separated
 *                       by a tab, then "OK PLAYERS <count>" (see admin_players)
 * ============================================================================
 */

#define ADMIN_DEFAULT_NAME "Admin Console"

static map_session_data* admin_dummy = nullptr; // virtual GM used to run atcommands

// --- atcommand output capture ---------------------------------------------
// While an admin-channel atcommand runs, clif_displaymessage() routes its GM
// feedback here instead of building RO packets for the dummy session.
static bool admin_capturing = false;
static std::string admin_capture_buf;

bool admin_capture_line( const char* mes ){
	if( !admin_capturing || mes == nullptr ){
		return false; // normal path: not capturing
	}
	admin_capture_buf += mes;
	admin_capture_buf += '\n';
	return true;
}

/// A free-text field of a PLAYERS line: a tab inside a name would shift
/// every column after it.
static std::string admin_field( const char* text ){
	std::string field = text != nullptr ? text : "";
	for( char& c : field ){
		if( c == '\t' ){
			c = ' ';
		}
	}
	return field;
}

/// PLAYERS: every online character, hidden GMs included, one line each.
/// Fields, in order: char_id, account_id, name, base_level, job_level, job
/// name, group_id, map, x, y, party name, guild name, vending (0/1),
/// autotrade (0/1), idle seconds. A field may be empty, never absent.
static void admin_players( int32 fd ){
	int32 count = 0;
	s_mapiterator* iter = mapit_getallusers();
	for( map_session_data* pl_sd = (TBL_PC*)mapit_first( iter ); mapit_exists( iter ); pl_sd = (TBL_PC*)mapit_next( iter ) ){
		struct party_data* party = party_search( pl_sd->status.party_id );
		const auto& guild = pl_sd->guild;
		const std::string fields[] = {
			std::to_string( pl_sd->status.char_id ),
			std::to_string( pl_sd->status.account_id ),
			admin_field( pl_sd->status.name ),
			std::to_string( pl_sd->status.base_level ),
			std::to_string( pl_sd->status.job_level ),
			admin_field( job_name( pl_sd->status.class_ ) ),
			std::to_string( pc_get_group_id( pl_sd ) ),
			admin_field( mapindex_id2name( pl_sd->mapindex ) ),
			std::to_string( pl_sd->x ),
			std::to_string( pl_sd->y ),
			admin_field( party != nullptr ? party->party.name : "" ),
			admin_field( guild != nullptr ? guild->guild.name : "" ),
			pl_sd->state.vending ? "1" : "0",
			pl_sd->state.autotrade ? "1" : "0",
			std::to_string( static_cast<int64>( last_tick - pl_sd->idletime ) ),
		};
		std::string line = ": ";
		for( size_t i = 0; i < ARRAYLENGTH( fields ); ++i ){
			if( i > 0 ){
				line += '\t';
			}
			line += fields[i];
		}
		admin_reply( fd, line );
		count++;
	}
	mapit_free( iter );
	admin_reply( fd, "OK PLAYERS " + std::to_string( count ) );
}

/// RELOAD, ATCMD and PLAYERS, called by the shared channel once the session
/// is authenticated. Returns false for an unknown command.
static bool admin_map_exec( int32 fd, s_admin_session& asd, const std::string& cmd, const std::string& arg ){
	if( cmd == "PLAYERS" ){
		admin_players( fd );
		return true;
	}

	// Everything here is turned into an atcommand run by the virtual GM.
	std::string atcmd;
	if( cmd == "RELOAD" ){
		if( arg.empty() ){
			admin_reply( fd, "ERR usage: RELOAD <target>" );
			return true;
		}
		atcmd = std::string( 1, atcommand_symbol ) + "reload" + arg;
	}else if( cmd == "ATCMD" ){
		if( arg.empty() ){
			admin_reply( fd, "ERR usage: ATCMD <command>" );
			return true;
		}
		atcmd = arg;
		// Keep an explicit '@' (atcommand, self) or '#' (charcommand, targets
		// another player by name) prefix; only default to '@' when neither.
		if( atcmd[0] != atcommand_symbol && atcmd[0] != charcommand_symbol ){
			atcmd = std::string( 1, atcommand_symbol ) + atcmd;
		}
	}else{
		return false; // the shared channel answers "ERR unknown command"
	}

	// Apply this session's display name so commands like @broadcast have a nick.
	safestrncpy( admin_dummy->status.name, asd.display_name, sizeof( admin_dummy->status.name ) );

	// Capture the atcommand's GM output (sent via clif_displaymessage) so we can
	// stream it back to the admin client instead of dropping it.
	// Use type=2 (console invocation): bypasses ATCMD_NOSCRIPT (type 0/3 only) which
	// would silently skip commands like @reloadscript without executing them. [Stingor]
	admin_capturing = true;
	admin_capture_buf.clear();
	bool ok = is_atcommand( admin_dummy->fd, admin_dummy, atcmd.c_str(), 2 );
	admin_capturing = false;

	// Captured output: one line per "OUT" record, prefixed with ": ".
	size_t start = 0;
	while( start < admin_capture_buf.size() ){
		size_t nl = admin_capture_buf.find( '\n', start );
		if( nl == std::string::npos ){
			nl = admin_capture_buf.size();
		}
		admin_reply( fd, ": " + admin_capture_buf.substr( start, nl - start ) );
		start = nl + 1;
	}

	// Terminal status line (never prefixed with ": ").
	if( ok ){
		admin_reply( fd, "OK " + atcmd );
	}else{
		admin_reply( fd, "ERR not an atcommand: " + atcmd );
	}

	return true;
}

void do_init_admin( void ){
	// Virtual GM character: group 99 = full privileges, fd 0 = no client output.
	CREATE( admin_dummy, map_session_data, 1 );
	new( admin_dummy ) map_session_data();
	admin_dummy->group_id = 99;
	admin_dummy->fd = 0;
	safestrncpy( admin_dummy->status.name, ADMIN_DEFAULT_NAME, sizeof( admin_dummy->status.name ) );
	pc_group_pc_load( admin_dummy );

	do_init_admin_channel( ADMIN_SERVER_MAP, admin_map_exec );
}

void do_final_admin( void ){
	do_final_admin_channel();
	if( admin_dummy != nullptr ){
		admin_dummy->~map_session_data();
		aFree( admin_dummy );
		admin_dummy = nullptr;
	}
}
