// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les STYLES HORS DE VUE de moonclient (CZ 0x0F63, ZC 0x0F64, voir
// packets_struct.hpp ; dans moonclient, l'ADR 0147 et
// docs/couleurs-du-personnage/conception.md, lot L13).
//
// La guilde, le groupe et la liste d'amis dessinent la tête d'un membre connecté
// mais absent de la carte. Le client demande ses styles par char_id ; le serveur
// rend, pour chacun, ce qu'un joueur en vue recevrait : la v7 du 0x0F27, le bloc
// du 0x0F5F, et l'apparence de la tête, que les listes du groupe et des amis ne
// portent pas.
//
// 🔴 LE CLIENT EST RÉPUTÉ HOSTILE. Le serveur relit lui-même la guilde, le groupe
// et les amis de la session ; tout autre char_id, un membre hors ligne ou caché
// reçoivent le même état 0, et rien d'autre. Seize char_id par demande, une
// demande par seconde, un plafond d'octets par minute.

#ifndef MAP_STYLE_ABSENT_HPP
#define MAP_STYLE_ABSENT_HPP

#include <common/cbasetypes.hpp>

class map_session_data;

// L'écart minimal entre deux CZ 0x0F63 d'une session, en millisecondes.
#define MOONCLIENT_STYLE_ABSENT_CADENCE_MS 1000
// Les octets de CZ 0x0F63 qu'une session envoie par minute, au plus. Toute trame
// reçue compte, refusée ou non. Une demande pleine pèse 69 octets : soixante par
// minute tiennent dessous.
#define MOONCLIENT_STYLE_ABSENT_BYTES_PER_MINUTE 8192

// Jette la cadence de la session qui part.
void style_absent_on_logout(const map_session_data& sd);

void clif_parse_moonclient_style_absent(int32 fd, map_session_data* sd);

#endif /* MAP_STYLE_ABSENT_HPP */
