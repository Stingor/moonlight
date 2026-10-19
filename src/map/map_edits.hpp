// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les ÉDITIONS DE CARTE de moonclient (CZ 0x0F45 et 0x0F49, ZC 0x0F46
// à 0x0F48, voir packets_struct.hpp ; l'ADR 0124 de moonclient).
//
// Le serveur est la source de vérité : il tient au plus un document par carte,
// sous une révision. Le staff l'enregistre en l'envoyant, fondé sur la révision
// qu'il a ouverte (verrou optimiste) ; le serveur le range en base
// (sql-files/moonclient_map_edits.sql), puis le diffuse aux joueurs de moonclient
// de la carte. À l'entrée sur une carte, il annonce la révision qu'il détient, et
// un client qui en a une autre la demande.
//
// Le serveur ne lit pas le YAML. Il borne la taille, vérifie le droit, la carte
// et la révision, refuse un texte qui porte un octet nul, et rien de plus : le
// client valide chaque document reçu par son lecteur strict.

#ifndef MAP_MAP_EDITS_HPP
#define MAP_MAP_EDITS_HPP

#include <common/cbasetypes.hpp>

class map_session_data;

// Le verdict d'un envoi (ZC 0x0F46). ⚠ Valeurs sur le fil : on AJOUTE en fin.
enum e_moonclient_map_edit_result : uint8 {
	MOONCLIENT_MAP_EDIT_ACCEPTED  = 0, ///< rangé ; `revision` est la nouvelle
	MOONCLIENT_MAP_EDIT_STALE     = 1, ///< fondé sur une révision périmée ; `revision` est la courante
	MOONCLIENT_MAP_EDIT_FORBIDDEN = 2, ///< niveau de groupe sous moonclient_map_edit_level
	MOONCLIENT_MAP_EDIT_MALFORMED = 3, ///< morceau hors d'ordre, trop gros, flux illisible, autre carte
	MOONCLIENT_MAP_EDIT_STORAGE   = 4, ///< la base a refusé l'écriture
};

// Le délai minimal entre deux demandes d'une session (CZ 0x0F49), en millisecondes.
#define MOONCLIENT_MAP_EDIT_GET_DELAY 5000
// Un transfert entrant sans morceau depuis ce délai est abandonné au suivant.
#define MOONCLIENT_MAP_EDIT_TRANSFER_TIMEOUT 30000

// Lit les documents en base, une fois, au démarrage du serveur de carte.
void map_edits_load();
// Oublie les documents et les transferts, à l'arrêt.
void map_edits_final();
// Jette le transfert et l'état de la session qui part.
void map_edits_on_logout(const map_session_data& sd);

// La révision que le serveur détient pour la carte de `sd` (ZC 0x0F48), si `sd`
// a annoncé MOONLIGHT_UI_MAP_EDITS. À l'entrée sur une carte, et à l'annonce du bit.
void map_edits_announce(map_session_data& sd);

void clif_parse_moonclient_map_edit_put(int32 fd, map_session_data* sd);
void clif_parse_moonclient_map_edit_get(int32 fd, map_session_data* sd);

#endif /* MAP_MAP_EDITS_HPP */
