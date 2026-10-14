// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Le MARCHÉ de moonclient (CZ 0x0F41 -> ZC 0x0F42, voir
// packets_struct.hpp) : toutes les échoppes ouvertes du serveur, lues en mémoire
// dans vending_db et buyingstore_db, et les boutiques de PNJ (shop_finder.cpp),
// jamais en SQL.

#ifndef MAP_MARKET_HPP
#define MAP_MARKET_HPP

#include <common/cbasetypes.hpp>

class map_session_data;

// La session a-t-elle annoncé le Marché (BOURGEON_UI_MARKET) ?
bool market_announced(const map_session_data& sd);

// La demande d'une session qui a annoncé le Marché. Une autre est ignorée.
void clif_parse_moonlight_market_req(int32 fd, map_session_data* sd);

// OUVRE LE MARCHÉ chez le client (MOONLIGHT_MARKET_OPEN), la saisie posée sur
// `query` (un ID, un nom, ou rien). Le Marché se consulte partout : aucune
// condition de carte. À n'appeler que si `market_announced`.
void market_open(map_session_data& sd, const char* query);

#endif /* MAP_MARKET_HPP */
