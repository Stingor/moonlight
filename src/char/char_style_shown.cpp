// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les recettes de la tablée de moonclient. Voir char_style_shown.hpp.

#include "char_style_shown.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include <common/mmo.hpp>
#include <common/packets.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/timer.hpp>

#include "char.hpp"
#include "inter.hpp" // sql_handle

namespace {

using bytes = std::vector<uint8>;

constexpr size_t HC_HEAD = sizeof(PACKET_HC_MOONCLIENT_CHAR_STYLES);
constexpr size_t CHAR_ID_BYTES = sizeof(uint32);

static_assert(HC_HEAD == 7, "l'en-tête du HC 0x0F66 ne vaut plus ce que le client attend");
static_assert(sizeof(PACKET_CH_MOONCLIENT_CHAR_STYLES) == 6, "le CH 0x0F65 ne vaut plus ce que le client envoie");
// Même incompressible, une trame garde une longueur positive.
static_assert(HC_HEAD + MOONCLIENT_CHAR_STYLES_RAW_MAX < INT16_MAX, "une trame doit tenir sa longueur");

// 🔴 CE QUI SORT DE LA BASE SE REJUGE : 4 corps de 52 octets au plus, puis 14
// entrées au plus dont chaque taille tient dans la ligne, et rien après.
bool shown_ok(const uint8* p, size_t length) {
	if (length < 2)
		return false;
	const size_t bodies = p[0];
	size_t at = 1 + bodies * MOONCLIENT_CHAR_STYLES_BODY_BYTES;
	if (bodies > MOONCLIENT_CHAR_STYLES_BODIES_MAX || at + 1 > length)
		return false;
	const size_t entries = p[at++];
	if (entries > MOONCLIENT_CHAR_STYLES_ENTRIES_MAX)
		return false;
	for (size_t i = 0; i < entries; ++i) {
		if (at >= length)
			return false;
		const size_t size = p[at];
		if (size < MOONCLIENT_CHAR_STYLES_ENTRY_HEAD || size > MOONCLIENT_CHAR_STYLES_ENTRY_MAX || at + size > length)
			return false;
		at += size;
	}
	return at == length;
}

// `more` : une autre trame de la même réponse suit.
void send_frame(int32 fd, const bytes& raw, bool more) {
	// ⚠ JAMAIS COMPRESSÉE : le serveur de personnages ne lie pas zlib. Seize Kio de
	// charge brute par trame, sur la liaison de la sélection, une fois par liste.
	const size_t length = HC_HEAD + raw.size();
	WFIFOHEAD(fd, length);
	PACKET_HC_MOONCLIENT_CHAR_STYLES* p = reinterpret_cast<PACKET_HC_MOONCLIENT_CHAR_STYLES*>(WFIFOP(fd, 0));
	p->packetType = HEADER_HC_MOONCLIENT_CHAR_STYLES;
	p->packetLength = static_cast<int16>(length);
	p->flags = more ? MOONCLIENT_CHAR_STYLES_MORE : 0;
	p->raw_size = static_cast<uint16>(raw.size());
	if (!raw.empty())
		memcpy(WFIFOP(fd, HC_HEAD), raw.data(), raw.size());
	WFIFOSET(fd, length);
}

} // namespace

bool chclif_parse_moonclient_char_styles(int32 fd, char_session_data& sd) {
	const PACKET_CH_MOONCLIENT_CHAR_STYLES* p = reinterpret_cast<const PACKET_CH_MOONCLIENT_CHAR_STYLES*>(RFIFOP(fd, 0));
	// Une session qui n'a pas de compte, ou qui annonce un bit inconnu : rien.
	if (!sd.auth || p->caps != MOONCLIENT_CHAR_STYLES_CAP_READS)
		return true;
	const t_tick now = gettick();
	if (sd.moonclient_styles_next != 0 && DIFF_TICK(sd.moonclient_styles_next, now) > 0)
		return true;
	sd.moonclient_styles_next = now + MOONCLIENT_CHAR_STYLES_CADENCE_MS;

	// 🔴 LES PERSONNAGES DU COMPTE, tels que ce serveur les a listés.
	std::string ids;
	for (int32 i = 0; i < MAX_CHARS; ++i) {
		if (sd.found_char[i] <= 0)
			continue;
		if (!ids.empty())
			ids += ',';
		ids += std::to_string(sd.found_char[i]);
	}
	bytes raw;
	if (!ids.empty()) {
		if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`, `version`, `data` FROM `moonclient_style_shown` WHERE `char_id` IN (%s)", ids.c_str())) {
			Sql_ShowDebug(sql_handle);
			return true;
		}
		while (SQL_SUCCESS == Sql_NextRow(sql_handle)) {
			char* data = nullptr;
			size_t length = 0;
			Sql_GetData(sql_handle, 0, &data, nullptr);
			const uint32 char_id = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
			Sql_GetData(sql_handle, 1, &data, nullptr);
			const uint32 version = data != nullptr ? static_cast<uint32>(strtoul(data, nullptr, 10)) : 0;
			Sql_GetData(sql_handle, 2, &data, &length);
			const uint8* shown = reinterpret_cast<const uint8*>(data);
			if (char_id == 0 || version != MOONCLIENT_CHAR_STYLES_STORED_VERSION || shown == nullptr || !shown_ok(shown, length)) {
				ShowWarning("char_style_shown: l'instantane du personnage %u est illisible (version %u) : omis.\n", char_id, version);
				continue;
			}
			// ⚠ UN PERSONNAGE N'EST JAMAIS COUPÉ ENTRE DEUX TRAMES.
			if (raw.size() + CHAR_ID_BYTES + length > MOONCLIENT_CHAR_STYLES_RAW_MAX) {
				send_frame(fd, raw, true);
				raw.clear();
			}
			for (int32 shift = 0; shift < 32; shift += 8)
				raw.push_back(static_cast<uint8>((char_id >> shift) & 0xFF));
			raw.insert(raw.end(), shown, shown + length);
		}
		Sql_FreeResult(sql_handle);
	}
	// Une trame part toujours, vide comprise : elle dit « rien à montrer ».
	send_frame(fd, raw, false);
	return true;
}
