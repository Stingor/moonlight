// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <cstdio>
#include <cstring>
#include <new>
#include <string>

#include "../common/malloc.hpp"
#include "../common/md5calc.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"

#include "atcommand.hpp"
#include "map.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"

/**
 * ============================================================================
 *  Map-server administration channel
 * ============================================================================
 *
 *  A standalone TCP listener that speaks a line-based ASCII protocol, so an
 *  external admin program can drive the server WITHOUT emulating a Ragnarok
 *  client (no PACKETVER, no obfuscation, no login/char/map handshake).
 *
 *  Each request is one line terminated by '\n' (a trailing '\r' is ignored).
 *  The server answers each line with one line: "OK ..." or "ERR ...".
 *
 *    AUTH <secret>      authenticate; required before any other command
 *    RELOAD <target>    shortcut for "@reload<target>" (itemdb, mobdb, skilldb,
 *                       script, battleconf, statusdb, atcommand, ...)
 *    ATCMD <command>    run any atcommand (a leading '@' is optional)
 *    QUIT               close the connection
 *
 *  Commands run inside the single-threaded main loop, exactly like the server
 *  console, so no locking is required.
 *
 *  Configuration lives in conf/admin.conf (admin_enable, admin_bind_ip,
 *  admin_port, admin_secret).
 *
 *  SECURITY: this is an administrative back door. Every command except AUTH
 *  requires the shared secret. Change admin_secret in conf/admin.conf before
 *  use and keep the port firewalled to trusted hosts.
 * ============================================================================
 */

// ---------------------------------------------------------------------------
// Configuration: read from conf/admin.conf (see admin_config_read).
// ---------------------------------------------------------------------------
#define ADMIN_CONF_PATH "conf/admin.conf"
#define ADMIN_DEFAULT_SECRET "CHANGE-ME-please"
#define ADMIN_DEFAULT_NAME "Admin Console" // display name used by e.g. @broadcast
#define ADMIN_LINE_MAX 4096               // anti-flood: drop a line longer than this

static bool   admin_enable     = true;
static uint32 admin_bind_ip    = 0;       // 0 = INADDR_ANY = all interfaces (LAN)
static uint16 admin_port       = 7799;
static char   admin_secret[256] = ADMIN_DEFAULT_SECRET;
static bool   admin_secret_md5 = false;   // if true, admin_secret holds an MD5 hash

// connect_client() is the generic accept handler in common/socket.cpp. It is
// not declared in socket.hpp, so we forward-declare it here to reuse it.
extern int32 connect_client( int32 listen_fd );

static int32 admin_listen_fd = -1;
static map_session_data* admin_dummy = nullptr; // virtual GM used to run atcommands

struct s_admin_session {
	bool authed;
	bool log_subscribed;            // receives the server console feed (LOG ON)
	char display_name[NAME_LENGTH];
};

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

/// Send one reply line back to the admin client.
static void admin_reply( int32 fd, const std::string& msg ){
	std::string out = msg;
	out += "\n";

	WFIFOHEAD( fd, out.size() );
	memcpy( WFIFOP( fd, 0 ), out.c_str(), out.size() );
	WFIFOSET( fd, static_cast<int32>( out.size() ) );
}

/// Execute a single command line.
static void admin_exec( int32 fd, const std::string& line ){
	s_admin_session* asd = reinterpret_cast<s_admin_session*>( session[fd]->session_data );

	// split into "<word> <rest>"
	std::string cmd, arg;
	size_t sp = line.find( ' ' );
	if( sp == std::string::npos ){
		cmd = line;
	}else{
		cmd = line.substr( 0, sp );
		arg = line.substr( sp + 1 );
	}

	if( cmd == "AUTH" ){
		bool match;
		if( admin_secret_md5 ){
			// admin_secret holds an MD5 hash; hash the received password and
			// compare (case-insensitive hex). The plaintext is never stored.
			char hash[33];
			MD5_String( arg.c_str(), hash );
			match = ( strcmpi( hash, admin_secret ) == 0 );
		}else{
			match = ( arg == admin_secret );
		}

		if( match ){
			asd->authed = true;
			admin_reply( fd, "OK authenticated" );
		}else{
			ShowWarning( "Admin channel: failed AUTH on session #%d.\n", fd );
			admin_reply( fd, "ERR bad secret" );
		}
		return;
	}

	if( asd == nullptr || !asd->authed ){
		admin_reply( fd, "ERR not authenticated" );
		return;
	}

	if( cmd == "QUIT" ){
		admin_reply( fd, "OK bye" );
		set_eof( fd );
		return;
	}

	if( cmd == "NAME" ){
		safestrncpy( asd->display_name, arg.empty() ? ADMIN_DEFAULT_NAME : arg.c_str(), sizeof( asd->display_name ) );
		admin_reply( fd, std::string( "OK name set: " ) + asd->display_name );
		return;
	}

	if( cmd == "LOG" ){
		asd->log_subscribed = ( strcmpi( arg.c_str(), "on" ) == 0 || strcmpi( arg.c_str(), "yes" ) == 0 || arg == "1" );
		admin_reply( fd, asd->log_subscribed ? "OK log on" : "OK log off" );
		return;
	}

	// Everything else is turned into an atcommand run by the virtual GM.
	std::string atcmd;
	if( cmd == "RELOAD" ){
		if( arg.empty() ){
			admin_reply( fd, "ERR usage: RELOAD <target>" );
			return;
		}
		atcmd = std::string( 1, atcommand_symbol ) + "reload" + arg;
	}else if( cmd == "ATCMD" ){
		if( arg.empty() ){
			admin_reply( fd, "ERR usage: ATCMD <command>" );
			return;
		}
		atcmd = arg;
		// Keep an explicit '@' (atcommand, self) or '#' (charcommand, targets
		// another player by name) prefix; only default to '@' when neither.
		if( atcmd[0] != atcommand_symbol && atcmd[0] != charcommand_symbol ){
			atcmd = std::string( 1, atcommand_symbol ) + atcmd;
		}
	}else{
		admin_reply( fd, "ERR unknown command" );
		return;
	}

	// Apply this session's display name so commands like @broadcast have a nick.
	safestrncpy( admin_dummy->status.name, asd->display_name, sizeof( admin_dummy->status.name ) );

	// Capture the atcommand's GM output (sent via clif_displaymessage) so we can
	// stream it back to the admin client instead of dropping it.
	admin_capturing = true;
	admin_capture_buf.clear();
	bool ok = is_atcommand( admin_dummy->fd, admin_dummy, atcmd.c_str(), 0 );
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
}

/// Per-session parse handler: split the incoming byte stream into lines.
static int32 admin_parse( int32 fd ){
	if( session[fd]->flag.eof ){
		do_close( fd );
		return 0;
	}

	while( RFIFOREST( fd ) > 0 ){
		char* buf = reinterpret_cast<char*>( RFIFOP( fd, 0 ) );
		size_t rest = RFIFOREST( fd );
		size_t i = 0;

		while( i < rest && buf[i] != '\n' ){
			i++;
		}

		if( i == rest ){
			// no complete line in the buffer yet
			if( rest > ADMIN_LINE_MAX ){
				ShowWarning( "admin_parse: oversized line on session #%d, dropping.\n", fd );
				set_eof( fd );
			}
			break;
		}

		std::string line( buf, i );
		RFIFOSKIP( fd, i + 1 );

		if( !line.empty() && line.back() == '\r' ){
			line.pop_back();
		}
		if( !line.empty() ){
			admin_exec( fd, line );
		}
	}

	return 0;
}

/// Accept handler for the admin listener: reuse the generic accept, then swap
/// the per-session parse function and attach our auth state.
static int32 admin_connect( int32 listen_fd ){
	int32 fd = connect_client( listen_fd );

	if( fd > 0 ){
		s_admin_session* asd;
		CREATE( asd, s_admin_session, 1 );
		asd->authed = false;
		asd->log_subscribed = false;
		safestrncpy( asd->display_name, ADMIN_DEFAULT_NAME, sizeof( asd->display_name ) );

		session[fd]->func_parse = admin_parse;
		session[fd]->flag.server = 1; // no inactivity timeout, not a game client
		session[fd]->session_data = asd;

		ShowInfo( "Admin channel: new connection on session #%d.\n", fd );
	}

	return fd;
}

/// Console output hook: push each server console line to subscribed clients.
/// Called from clif/showmsg in the (single) main thread, like everything else.
static void admin_console_forward( int32 flag, const char* msg ){
	if( msg == nullptr ){
		return;
	}

	// Sanitize to a single line (Show* messages usually end with '\n').
	std::string line( msg );
	while( !line.empty() && ( line.back() == '\n' || line.back() == '\r' ) ){
		line.pop_back();
	}
	if( line.empty() ){
		return;
	}
	for( size_t i = 0; i < line.size(); i++ ){
		if( line[i] == '\n' || line[i] == '\r' ){
			line[i] = ' ';
		}
	}

	// "* <flag> <message>" — distinct from command output (": ") and status.
	std::string out = "* " + std::to_string( (int32)flag ) + " " + line;

	for( int32 i = 0; i < fd_max; i++ ){
		if( session[i] == nullptr || session[i]->func_parse != admin_parse ){
			continue;
		}
		s_admin_session* asd = reinterpret_cast<s_admin_session*>( session[i]->session_data );
		if( asd != nullptr && asd->log_subscribed ){
			admin_reply( i, out );
		}
	}
}

/// Read conf/admin.conf (same "w1: w2" format as the other map-server confs).
static void admin_config_read( const char* cfgName ){
	char line[1024], w1[64], w2[1024];

	FILE* fp = fopen( cfgName, "r" );
	if( fp == nullptr ){
		ShowError( "Admin channel: configuration file '%s' not found, using defaults.\n", cfgName );
		return;
	}

	while( fgets( line, sizeof( line ), fp ) ){
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( ( ptr = strstr( line, "//" ) ) != nullptr )
			*ptr = '\n'; // strip comments

		if( sscanf( line, "%63[^:]: %1023[^\t\r\n]", w1, w2 ) < 2 )
			continue;

		// strip trailing spaces
		ptr = w2 + strlen( w2 );
		while( --ptr >= w2 && *ptr == ' ' );
		ptr++;
		*ptr = '\0';

		if( strcmpi( w1, "admin_enable" ) == 0 )
			admin_enable = config_switch( w2 ) != 0;
		else if( strcmpi( w1, "admin_bind_ip" ) == 0 )
			admin_bind_ip = str2ip( w2 );
		else if( strcmpi( w1, "admin_port" ) == 0 )
			admin_port = (uint16)atoi( w2 );
		else if( strcmpi( w1, "admin_secret" ) == 0 )
			safestrncpy( admin_secret, w2, sizeof( admin_secret ) );
		else if( strcmpi( w1, "admin_secret_md5" ) == 0 )
			admin_secret_md5 = config_switch( w2 ) != 0;
		else if( strcmpi( w1, "import" ) == 0 )
			admin_config_read( w2 );
		else
			ShowWarning( "Admin channel: unknown setting '%s' in file %s.\n", w1, cfgName );
	}

	fclose( fp );
}

void do_init_admin( void ){
	admin_config_read( ADMIN_CONF_PATH );

	if( !admin_enable ){
		ShowStatus( "Admin channel: " CL_RED "disabled" CL_RESET " by configuration.\n" );
		return;
	}

	if( !admin_secret_md5 && strcmp( admin_secret, ADMIN_DEFAULT_SECRET ) == 0 ){
		ShowWarning( "Admin channel: admin_secret is still the default value - change it in %s!\n", ADMIN_CONF_PATH );
	}
	if( admin_secret_md5 && strlen( admin_secret ) != 32 ){
		ShowWarning( "Admin channel: admin_secret_md5 is enabled but admin_secret is not a 32-char MD5 hash.\n" );
	}

	// Virtual GM character: group 99 = full privileges, fd 0 = no client output.
	CREATE( admin_dummy, map_session_data, 1 );
	new( admin_dummy ) map_session_data();
	admin_dummy->group_id = 99;
	admin_dummy->fd = 0;
	safestrncpy( admin_dummy->status.name, ADMIN_DEFAULT_NAME, sizeof( admin_dummy->status.name ) );
	pc_group_pc_load( admin_dummy );

	admin_listen_fd = make_listen_bind( admin_bind_ip, admin_port );
	session[admin_listen_fd]->func_recv = admin_connect;

	set_console_hook( admin_console_forward ); // stream server console to LOG subscribers

	ShowStatus( "Admin channel listening on port " CL_WHITE "%d" CL_RESET ".\n", admin_port );
}

void do_final_admin( void ){
	set_console_hook( nullptr );
	if( admin_listen_fd != -1 ){
		do_close( admin_listen_fd );
		admin_listen_fd = -1;
	}
	if( admin_dummy != nullptr ){
		admin_dummy->~map_session_data();
		aFree( admin_dummy );
		admin_dummy = nullptr;
	}
}
