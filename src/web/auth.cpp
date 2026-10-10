// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "auth.hpp"

#include <array>
#include <charconv>
#include <cstring>
#include <string>

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include <common/showmsg.hpp>
#include <common/sql.hpp>

#include "http.hpp"
#include "sqllock.hpp"
#include "web.hpp"


bool isAuthorized(const Request &request, bool checkGuildLeader) {
	if (!request.has_file("AuthToken") || !request.has_file("AID"))
		return false;

	if (checkGuildLeader && !request.has_file("GDID"))
		return false;
	
	auto token_str = request.get_file_value("AuthToken").content;
	auto token = token_str.c_str();
	auto account_id = std::stoi(request.get_file_value("AID").content);

	SQLLock loginlock(LOGIN_SQL_LOCK);

	loginlock.lock();

	auto handle = loginlock.getHandle();

	SqlStmt stmt{ *handle };

	if (SQL_SUCCESS != stmt.Prepare(
			"SELECT `account_id` FROM `%s` WHERE (`account_id` = ? AND `web_auth_token` = ? AND `web_auth_token_enabled` = '1')",
			login_table)
		|| SQL_SUCCESS != stmt.BindParam(0, SQLDT_INT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != stmt.BindParam(1, SQLDT_STRING, (void *)token, strlen(token))
		|| SQL_SUCCESS != stmt.Execute()
	) {
		SqlStmt_ShowDebug(stmt);
		loginlock.unlock();
		return false;
	}

	if (stmt.NumRows() <= 0) {
		ShowWarning("Request with AID %d and token %s unverified\n", account_id, token);
		loginlock.unlock();
		return false;
	}

	loginlock.unlock();
	if (!checkGuildLeader) {
		// we're done, auth ok
		return true;
	}

	auto guild_id = std::stoi(request.get_file_value("GDID").content);

	SQLLock charlock(CHAR_SQL_LOCK);
	charlock.lock();
	handle = charlock.getHandle();
	SqlStmt stmt2{ *handle };

	if (SQL_SUCCESS != stmt2.Prepare(
		"SELECT `account_id` FROM `%s` LEFT JOIN `%s` using (`char_id`) WHERE (`%s`.`account_id` = ? AND `%s`.`guild_id` = ?) LIMIT 1",
		guild_db_table, char_db_table, char_db_table, guild_db_table)
		|| SQL_SUCCESS != stmt2.BindParam(0, SQLDT_INT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != stmt2.BindParam(1, SQLDT_INT32, &guild_id, sizeof(guild_id))
		|| SQL_SUCCESS != stmt2.Execute()
	) {
		SqlStmt_ShowDebug(stmt2);
		charlock.unlock();
		return false;
	}

	if (stmt2.NumRows() <= 0) {
		ShowDebug("Request with AID %d GDID %d and token %s unverified\n", account_id, guild_id, token);
		charlock.unlock();
		return false;
	}
	charlock.unlock();
	return true;
}

bool parseAccountId(const Request &request, int32 &account_id) {
	if (!request.has_file("AID"))
		return false;

	const auto &text = request.get_file_value("AID").content;
	const char *first = text.data();
	const char *last = first + text.size();
	int32 value = 0;
	auto [end, error] = std::from_chars(first, last, value);

	if (error != std::errc() || end != last || value <= 0)
		return false;

	account_id = value;
	return true;
}

// [Stingor] Une adresse textuelle (IPv4 ou IPv6) sous sa forme IPv6 binaire :
// une IPv4 devient ::ffff:a.b.c.d, ce qui rend egales ses deux ecritures.
static bool canonicalAddress(const std::string &text, std::array<uint8, 16> &address) {
	in6_addr ipv6{};
	if (inet_pton(AF_INET6, text.c_str(), &ipv6) == 1) {
		std::memcpy(address.data(), &ipv6, address.size());
		return true;
	}

	in_addr ipv4{};
	if (inet_pton(AF_INET, text.c_str(), &ipv4) == 1) {
		const size_t ipv4_size = sizeof(ipv4);
		const size_t mapped_prefix_size = address.size() - ipv4_size;
		address.fill(0);
		// Le prefixe ::ffff: des adresses IPv4 mappees (RFC 4291).
		address[mapped_prefix_size - 2] = 0xff;
		address[mapped_prefix_size - 1] = 0xff;
		std::memcpy(address.data() + mapped_prefix_size, &ipv4, ipv4_size);
		return true;
	}

	return false;
}

bool isFromActiveSession(const Request &request, int32 account_id) {
	std::array<uint8, 16> request_address;
	if (!canonicalAddress(request.remote_addr, request_address)) {
		ShowWarning("Request with AID %d from unreadable address '%s' refused\n", account_id, request.remote_addr.c_str());
		return false;
	}

	SQLLock loginlock(LOGIN_SQL_LOCK);
	loginlock.lock();
	auto handle = loginlock.getHandle();
	SqlStmt stmt{ *handle };

	// [Stingor] web_auth_token_enabled reflete la liste des comptes en ligne du
	// login-server : passe a 1 a l'authentification, remis a 0 quelques secondes
	// apres la deconnexion, et remis a 0 pour tous au demarrage du login-server.
	char last_ip[101];
	if (SQL_SUCCESS != stmt.Prepare(
			"SELECT `last_ip` FROM `%s` WHERE (`account_id` = ? AND `web_auth_token_enabled` = '1') LIMIT 1",
			login_table)
		|| SQL_SUCCESS != stmt.BindParam(0, SQLDT_INT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != stmt.Execute()
	) {
		SqlStmt_ShowDebug(stmt);
		loginlock.unlock();
		return false;
	}

	if (stmt.NumRows() <= 0) {
		ShowWarning("Request with AID %d from %s refused: no active session\n", account_id, request.remote_addr.c_str());
		loginlock.unlock();
		return false;
	}

	if (SQL_SUCCESS != stmt.BindColumn(0, SQLDT_STRING, last_ip, sizeof(last_ip))
		|| SQL_SUCCESS != stmt.NextRow()
	) {
		SqlStmt_ShowDebug(stmt);
		loginlock.unlock();
		return false;
	}

	loginlock.unlock();
	last_ip[sizeof(last_ip) - 1] = '\0';

	std::array<uint8, 16> session_address;
	if (!canonicalAddress(last_ip, session_address) || session_address != request_address) {
		ShowWarning("Request with AID %d from %s refused: session address is %s\n", account_id, request.remote_addr.c_str(), last_ip);
		return false;
	}

	return true;
}
