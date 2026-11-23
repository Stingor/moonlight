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

#include "atcommand.hpp"
#include "map.hpp"
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

/// RELOAD and ATCMD, called by the shared channel once the session is
/// authenticated. Returns false for an unknown command.
static bool admin_map_exec( int32 fd, s_admin_session& asd, const std::string& cmd, const std::string& arg ){
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
