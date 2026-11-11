// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] The snapshot of what a character SHOWS, for the character select of
// moonclient: written by the map-server at logout (map/style_absent.cpp), served
// by the char-server (char/char_style_shown.cpp, CH 0x0F65 / HC 0x0F66). Both
// sides read these limits from here.

#ifndef COMMON_MOONCLIENT_STYLE_SHOWN_HPP
#define COMMON_MOONCLIENT_STYLE_SHOWN_HPP

// The CH capability: this client reads HC 0x0F66.
#define MOONCLIENT_CHAR_STYLES_CAP_READS 0x00000001
// The snapshot: [bodies:1] then bodies x 52 bytes (a 0x0F27 entry without its
// gid), [entries:1] then entries x a 0x0F5F layer entry.
#define MOONCLIENT_CHAR_STYLES_STORED_VERSION 1
#define MOONCLIENT_CHAR_STYLES_BODY_BYTES 52
#define MOONCLIENT_CHAR_STYLES_BODIES_MAX 4
#define MOONCLIENT_CHAR_STYLES_ENTRY_HEAD 10
#define MOONCLIENT_CHAR_STYLES_ENTRY_MAX 246
#define MOONCLIENT_CHAR_STYLES_ENTRIES_MAX 14
#define MOONCLIENT_CHAR_STYLES_RAW_MAX 16384
// HC flags: the payload is zlib compressed (the char-server never sends it
// compressed: it does not link zlib); another frame of the same answer follows.
#define MOONCLIENT_CHAR_STYLES_ZIPPED 0x01
#define MOONCLIENT_CHAR_STYLES_MORE 0x02

#endif /* COMMON_MOONCLIENT_STYLE_SHOWN_HPP */
