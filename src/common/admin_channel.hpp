// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ADMIN_CHANNEL_HPP
#define ADMIN_CHANNEL_HPP

#include <string>

#include "cbasetypes.hpp"
#include "mmo.hpp"

/**
 * [Stingor] Administration channel shared by the three servers.
 *
 * A TCP listener speaking a line-based ASCII protocol (see admin_channel.cpp).
 * Every server answers AUTH, QUIT, NAME, LOG and SERVER; the map-server adds
 * its own commands (RELOAD, ATCMD) through the exec callback. The login and
 * char servers only stream their console, so the admin tool sees all three.
 *
 * Configuration: conf/admin.conf (admin_port, admin_char_port, admin_login_port).
 */

enum e_admin_server : uint8 {
	ADMIN_SERVER_LOGIN = 0,
	ADMIN_SERVER_CHAR,
	ADMIN_SERVER_MAP,
};

struct s_admin_session {
	bool  authed;
	bool  log_subscribed;           // receives the server console feed (LOG ON)
	uint8 auth_fails;               // bad AUTH attempts so far on this connection
	char  display_name[NAME_LENGTH];
};

/// Server-specific commands, called once the session is authenticated, for
/// anything the shared channel does not handle itself. Return false when the
/// command is unknown, the channel then answers "ERR unknown command".
typedef bool (*t_admin_exec)( int32 fd, s_admin_session& asd, const std::string& cmd, const std::string& arg );

/// Send one reply line ("\n" appended) to an admin client.
void admin_reply( int32 fd, const std::string& msg );

void do_init_admin_channel( e_admin_server server, t_admin_exec exec );
void do_final_admin_channel( void );

#endif /* ADMIN_CHANNEL_HPP */
