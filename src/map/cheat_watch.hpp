// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LE GUET DES HEURISTIQUES DE TRICHE : relie cheat_heuristics.hpp à
// la session, lit les réglages (`cheat_*` de battle_conf) et écrit au journal.
//
// Il ne sanctionne pas : il journalise, une fois par réglage
// `cheat_report_interval` et par indice, pour qu'un humain tranche.

#ifndef MAP_CHEAT_WATCH_HPP
#define MAP_CHEAT_WATCH_HPP

#include <common/cbasetypes.hpp>

#include "cheat_heuristics.hpp"

class map_session_data;

// Une horloge reçue du client (CZ_REQUEST_TIME et ses variantes).
void cheat_watch_note_client_tick(map_session_data& sd, uint32 client_tick);

// Une action demandée par le client : une attaque, une compétence, un objet.
void cheat_watch_note_action(map_session_data& sd, e_cheat_cadence_kind kind);

#endif /* MAP_CHEAT_WATCH_HPP */
