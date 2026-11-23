// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "admin_channel.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "md5calc.hpp"
#include "showmsg.hpp"
#include "socket.hpp"
#include "strlib.hpp"

/**
 * ============================================================================
 *  Administration channel (login, char and map servers)
 * ============================================================================
 *
 *  A standalone TCP listener that speaks a line-based ASCII protocol, so an
 *  external admin program can watch and drive the servers WITHOUT emulating a
 *  Ragnarok client (no PACKETVER, no obfuscation, no login/char/map handshake).
 *
 *  Each request is one line terminated by '\n' (a trailing '\r' is ignored).
 *  The server answers each line with one line: "OK ..." or "ERR ...".
 *
 *    AUTH <secret>      authenticate; required before any other command
 *    LOG on|off         stream this server's console ("* <flag> <text>" lines)
 *    NAME <name>        display name used by e.g. @broadcast (map-server)
 *    SERVER             answers "OK login", "OK char" or "OK map"
 *    QUIT               close the connection
 *
 *  The map-server adds RELOAD and ATCMD (src/map/admin.cpp). Each server has
 *  its own port: admin_login_port, admin_char_port and admin_port (map), all
 *  in conf/admin.conf, which the three servers read with the same secret and
 *  the same allow-list. A port set to 0 disables that server's channel.
 *
 *  Commands run inside the single-threaded main loop, exactly like the server
 *  console, so no locking is required.
 *
 *  The per-connection state lives in a table of this file, NOT in
 *  session[fd]->session_data: the char-server walks every session and casts
 *  session_data to char_session_data (char.cpp, ARR_FIND on account_id), so a
 *  foreign struct there would be read as a player.
 *
 *  SECURITY: this is an administrative back door. Every command except AUTH
 *  requires the shared secret. Change admin_secret in conf/admin.conf before
 *  use and keep the ports firewalled to trusted hosts.
 * ============================================================================
 */

#define ADMIN_CONF_PATH "conf/admin.conf"
#define ADMIN_DEFAULT_SECRET "CHANGE-ME-please"
#define ADMIN_DEFAULT_NAME "Admin Console" // display name used by e.g. @broadcast
#define ADMIN_LINE_MAX 4096               // anti-flood: drop a line longer than this
#define ADMIN_MAX_AUTH_FAILS 5            // close the session after this many bad AUTH

static bool   admin_enable      = true;
static uint32 admin_bind_ip     = 0;      // 0 = INADDR_ANY = every interface, public ones included
static uint16 admin_ports[3]    = { 7797, 7798, 7799 }; // indexed by e_admin_server
static char   admin_secret[256] = ADMIN_DEFAULT_SECRET;
static bool   admin_secret_md5  = false;  // if true, admin_secret holds an MD5 hash

static const char* const admin_server_names[3] = { "login", "char", "map" };

/// One admin_allow entry: a network the channel accepts connections from.
/// Both fields are in host byte order, like session[fd]->client_addr and the
/// value str2ip() returns, so a match is a plain mask-and-compare.
struct s_admin_allow {
	uint32 net;   // network address, already masked
	uint32 mask;  // netmask
};

/// Networks allowed to reach the channel. EMPTY MEANS ACCEPT EVERYONE: an
/// existing conf without admin_allow keeps working exactly as before, and
/// do_init_admin_channel() warns when that combines with a wide-open bind.
static std::vector<s_admin_allow> admin_allow;

static e_admin_server admin_server = ADMIN_SERVER_MAP;
static t_admin_exec admin_exec_cb = nullptr;
static int32 admin_listen_fd = -1;
static std::unordered_map<int32, s_admin_session> admin_sessions; // keyed by fd

// connect_client() is the generic accept handler in common/socket.cpp. It is
// not declared in socket.hpp, so we forward-declare it here to reuse it.
extern int32 connect_client( int32 listen_fd );

/// True when ip (host byte order) matches at least one allow-list entry.
static bool admin_ip_allowed( uint32 ip ){
	for( const s_admin_allow& a : admin_allow ){
		if( ( ip & a.mask ) == a.net ){
			return true;
		}
	}

	return false;
}

/// Parse one admin_allow value and append it. Accepted forms:
///   192.168.1.20            single host
///   192.168.1.0/24          prefix length
///   192.168.1.0/255.255.255.0   dotted netmask (same spelling packet_athena.conf uses)
static void admin_allow_add( const char* spec ){
	char ip_s[64] = {}, mask_s[64] = {};
	uint32 mask;

	if( sscanf( spec, "%63[^/]/%63s", ip_s, mask_s ) == 2 ){
		if( strchr( mask_s, '.' ) != nullptr ){
			mask = str2ip( mask_s );
		}else{
			const int32 bits = atoi( mask_s );

			if( bits <= 0 ){
				mask = 0;
			}else if( bits >= 32 ){
				mask = 0xFFFFFFFFu;
			}else{
				mask = 0xFFFFFFFFu << ( 32 - bits );
			}
		}
	}else{
		mask = 0xFFFFFFFFu; // bare address = one host
	}

	const uint32 ip = str2ip( ip_s );

	if( ip == 0xFFFFFFFFu ){ // what str2ip returns on a malformed address
		ShowWarning( "Admin channel: ignoring invalid admin_allow entry '%s'.\n", spec );
		return;
	}

	admin_allow.push_back( { ip & mask, mask } );
}

/// Read conf/admin.conf (same "w1: w2" format as the other server confs).
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
		else if( strcmpi( w1, "admin_allow" ) == 0 )
			admin_allow_add( w2 ); // cumulative: repeat the key for several networks
		else if( strcmpi( w1, "admin_port" ) == 0 )
			admin_ports[ADMIN_SERVER_MAP] = (uint16)atoi( w2 );
		else if( strcmpi( w1, "admin_char_port" ) == 0 )
			admin_ports[ADMIN_SERVER_CHAR] = (uint16)atoi( w2 );
		else if( strcmpi( w1, "admin_login_port" ) == 0 )
			admin_ports[ADMIN_SERVER_LOGIN] = (uint16)atoi( w2 );
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

void admin_reply( int32 fd, const std::string& msg ){
	std::string out = msg;
	out += "\n";

	WFIFOHEAD( fd, out.size() );
	memcpy( WFIFOP( fd, 0 ), out.c_str(), out.size() );
	WFIFOSET( fd, static_cast<int32>( out.size() ) );
}

/// Execute a single command line.
static void admin_exec( int32 fd, s_admin_session& asd, const std::string& line ){
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
			asd.authed = true;
			asd.auth_fails = 0;
			admin_reply( fd, "OK authenticated" );
			return;
		}

		// The secret is the only thing between the network and ATCMD, so a
		// connection does not get to sit there trying candidates: drop it after
		// a handful of misses and make the attempt visible in the console.
		asd.auth_fails++;
		ShowWarning( "Admin channel: failed AUTH on session #%d from %s (%d/%d).\n",
			fd, ip2str( session[fd]->client_addr, nullptr ),
			(int32)asd.auth_fails, ADMIN_MAX_AUTH_FAILS );
		admin_reply( fd, "ERR bad secret" );

		if( asd.auth_fails >= ADMIN_MAX_AUTH_FAILS ){
			admin_reply( fd, "ERR too many failed attempts" );
			set_eof( fd );
		}
		return;
	}

	if( !asd.authed ){
		admin_reply( fd, "ERR not authenticated" );
		return;
	}

	if( cmd == "QUIT" ){
		admin_reply( fd, "OK bye" );
		set_eof( fd );
		return;
	}

	if( cmd == "SERVER" ){
		admin_reply( fd, std::string( "OK " ) + admin_server_names[admin_server] );
		return;
	}

	if( cmd == "NAME" ){
		safestrncpy( asd.display_name, arg.empty() ? ADMIN_DEFAULT_NAME : arg.c_str(), sizeof( asd.display_name ) );
		admin_reply( fd, std::string( "OK name set: " ) + asd.display_name );
		return;
	}

	if( cmd == "LOG" ){
		asd.log_subscribed = ( strcmpi( arg.c_str(), "on" ) == 0 || strcmpi( arg.c_str(), "yes" ) == 0 || arg == "1" );
		admin_reply( fd, asd.log_subscribed ? "OK log on" : "OK log off" );
		return;
	}

	if( admin_exec_cb == nullptr || !admin_exec_cb( fd, asd, cmd, arg ) ){
		admin_reply( fd, "ERR unknown command" );
	}
}

/// Per-session parse handler: split the incoming byte stream into lines.
static int32 admin_parse( int32 fd ){
	if( session[fd]->flag.eof ){
		admin_sessions.erase( fd );
		do_close( fd );
		return 0;
	}

	while( RFIFOREST( fd ) > 0 && !session[fd]->flag.eof ){
		char* buf = reinterpret_cast<char*>( RFIFOP( fd, 0 ) );
		size_t rest = RFIFOREST( fd );
		size_t i = 0;

		while( i < rest && buf[i] != '\n' ){
			i++;
		}

		if( i == rest ){
			// no complete line in the buffer yet
			if( rest > ADMIN_LINE_MAX ){
				ShowWarning( "Admin channel: oversized line on session #%d, dropping.\n", fd );
				set_eof( fd );
			}
			break;
		}

		std::string line( buf, i );
		RFIFOSKIP( fd, i + 1 );

		if( !line.empty() && line.back() == '\r' ){
			line.pop_back();
		}
		if( line.empty() ){
			continue;
		}

		auto it = admin_sessions.find( fd );
		if( it == admin_sessions.end() ){ // cannot happen: created on accept
			set_eof( fd );
			break;
		}
		admin_exec( fd, it->second, line );
	}

	return 0;
}

/// Accept handler for the admin listener: reuse the generic accept, then swap
/// the per-session parse function and attach our auth state.
static int32 admin_connect( int32 listen_fd ){
	int32 fd = connect_client( listen_fd );

	// Address check first, before any session state exists: an unlisted peer
	// never gets to send a byte, so it cannot brute force AUTH nor hold a
	// connection open (these sockets carry flag.server, so they never time out).
	if( fd > 0 && !admin_allow.empty() && !admin_ip_allowed( session[fd]->client_addr ) ){
		ShowWarning( "Admin channel: rejected connection from %s (not in admin_allow).\n",
			ip2str( session[fd]->client_addr, nullptr ) );
		do_close( fd );
		return -1;
	}

	if( fd > 0 ){
		s_admin_session& asd = admin_sessions[fd];
		asd.authed = false;
		asd.log_subscribed = false;
		asd.auth_fails = 0;
		safestrncpy( asd.display_name, ADMIN_DEFAULT_NAME, sizeof( asd.display_name ) );

		session[fd]->func_parse = admin_parse;
		session[fd]->flag.server = 1; // no inactivity timeout, not a game client

		ShowInfo( "Admin channel: new connection on session #%d.\n", fd );
	}

	return fd;
}

/// Console output hook: push each server console line to subscribed clients.
/// Called from showmsg in the (single) main thread, like everything else.
/// A single Show* call may contain embedded '\n'; each sub-line is sent as a
/// separate "* <flag> <text>" record so the GUI can display them individually.
static void admin_console_forward( int32 flag, const char* msg ){
	if( msg == nullptr || admin_sessions.empty() ){
		return;
	}

	const std::string flag_prefix = "* " + std::to_string( (int32)flag ) + " ";

	// Split on '\n' and send each non-empty fragment as its own record.
	const char* p = msg;
	while( *p ){
		const char* nl = p;
		while( *nl && *nl != '\n' ) nl++;

		// Build the fragment, stripping trailing '\r'.
		std::string fragment( p, nl );
		while( !fragment.empty() && fragment.back() == '\r' ){
			fragment.pop_back();
		}

		if( !fragment.empty() ){
			std::string out = flag_prefix + fragment;
			for( const auto& [fd, asd] : admin_sessions ){
				if( asd.log_subscribed && session_isActive( fd ) && session[fd]->func_parse == admin_parse ){
					admin_reply( fd, out );
				}
			}
		}

		p = ( *nl == '\n' ) ? nl + 1 : nl;
	}
}

void do_init_admin_channel( e_admin_server server, t_admin_exec exec ){
	admin_server = server;
	admin_exec_cb = exec;
	admin_allow.clear(); // admin_config_read() only ever appends (it recurses into imports)
	admin_config_read( ADMIN_CONF_PATH );

	const uint16 port = admin_ports[server];

	if( !admin_enable || port == 0 ){
		ShowStatus( "Admin channel: " CL_RED "disabled" CL_RESET " by configuration.\n" );
		return;
	}

	if( !admin_secret_md5 && strcmp( admin_secret, ADMIN_DEFAULT_SECRET ) == 0 ){
		ShowWarning( "Admin channel: admin_secret is still the default value - change it in %s!\n", ADMIN_CONF_PATH );
	}
	if( admin_secret_md5 && strlen( admin_secret ) != 32 ){
		ShowWarning( "Admin channel: admin_secret_md5 is enabled but admin_secret is not a 32-char MD5 hash.\n" );
	}
	if( admin_allow.empty() && admin_bind_ip == 0 ){
		ShowWarning( "Admin channel: bound to every interface with no admin_allow - anyone who can reach port %d may attempt AUTH. Set admin_allow (and/or admin_bind_ip) in %s.\n",
			port, ADMIN_CONF_PATH );
	}

	admin_listen_fd = make_listen_bind( admin_bind_ip, port );
	session[admin_listen_fd]->func_recv = admin_connect;

	set_console_hook( admin_console_forward ); // stream server console to LOG subscribers

	ShowStatus( "Admin channel listening on port " CL_WHITE "%d" CL_RESET ".\n", port );
}

void do_final_admin_channel( void ){
	set_console_hook( nullptr );

	for( const auto& [fd, asd] : admin_sessions ){
		if( session_isValid( fd ) ){
			do_close( fd );
		}
	}
	admin_sessions.clear();

	if( admin_listen_fd != -1 ){
		do_close( admin_listen_fd );
		admin_listen_fd = -1;
	}
}
