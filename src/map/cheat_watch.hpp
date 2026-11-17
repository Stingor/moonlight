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

// LE RAPPEL DE FAIR-PLAY EN GUERRE DE GUILDE (ZC 0x0F6A). Seule réponse du
// serveur à la garde du client, et seulement sur un château pendant une guerre
// ouverte : ailleurs, le rapport ne fait qu'aller au journal.
//
// Deux cadences, parce qu'un outil se signale de deux façons :
//   - un rapport FRAIS (des touches postées, minute après minute) relance le
//     rappel au plus une fois par `cheat_fair_play_interval` ms ;
//   - un outil signalé plus tôt dans la session (un processus, signalé une
//     seule fois) ne le relance qu'une fois par guerre, à l'entrée d'un château
//     ou à l'ouverture de la guerre : le joueur l'a peut-être fermé depuis.
// `cheat_fair_play_interval` à 0 éteint le rappel.

// Un outil signalé par la garde du client (CZ 0x0F0A), nom déjà assaini.
void cheat_watch_note_client_tool(map_session_data& sd, const char* tool);

// Le joueur vient d'arriver sur une carte.
void cheat_watch_fair_play_on_arrival(map_session_data& sd);

// Une guerre de guilde vient de s'ouvrir : chaque joueur déjà sur un château.
void cheat_watch_fair_play_war_started();

#endif /* MAP_CHEAT_WATCH_HPP */
