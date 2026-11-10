// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les TENUES de moonclient : le code court et la garde-robe du compte
// (CZ 0x0F61, ZC 0x0F62, voir packets_struct.hpp ; dans moonclient,
// docs/couleurs-du-personnage/conception.md, section « Tenues »).
//
// Une tenue est le corps (v7, un par corps habillé) et les calques d'un joueur,
// d'un seul tenant. Le serveur ne lit jamais une couleur : il juge la structure,
// range, et rend.
//
// - Le CODE COURT : huit caractères que le serveur attribue à une tenue publiée.
//   Le même compte qui republie la même tenue retrouve le même code. Un code est
//   IMMUABLE : personne, pas même son auteur, ne change ce qu'il désigne.
// - La GARDE-ROBE : seize rangements nommés par compte, que tous ses personnages
//   relisent. Elle se désigne par l'account_id de la SESSION, jamais par un
//   identifiant lu du client.
//
// Deux tables (sql-files/moonclient_style_outfits.sql) plutôt que des
// `acc_reg_str` : une tenue pèse jusqu'à 1 Kio de binaire, au-delà des 254
// caractères d'une variable de compte ; une variable de compte se lit à chaque
// connexion et s'écrit depuis n'importe quel script, alors que ces lignes ne se
// lisent qu'à la demande et ne s'écrivent que par ce module.
//
// 🔴 UNE TENUE N'ÉCRIT RIEN SUR UN PERSONNAGE. Le client en fait une
// proposition dans son éditeur ; le joueur la valide ensuite par 0x0F26 et
// 0x0F5E, avec toutes leurs gardes.

#ifndef MAP_STYLE_OUTFITS_HPP
#define MAP_STYLE_OUTFITS_HPP

#include <common/cbasetypes.hpp>

class map_session_data;

// L'écart minimal entre deux CZ 0x0F61 d'une session, en millisecondes : il
// borne aussi l'essai de codes au hasard, à soixante par minute.
#define MOONCLIENT_OUTFIT_CADENCE_MS 1000
// Les octets de CZ 0x0F61 qu'une session envoie par minute, au plus.
#define MOONCLIENT_OUTFIT_BYTES_PER_MINUTE 16384
// Les codes qu'un compte publie, au plus. 🔴 AU PLAFOND, ON REFUSE : retirer un
// code ancien casserait un partage que d'autres joueurs gardent.
#define MOONCLIENT_OUTFIT_CODES_PER_ACCOUNT 64

// Jette l'état de la session qui part : la cadence.
void style_outfits_on_logout(const map_session_data& sd);

void clif_parse_moonclient_outfit(int32 fd, map_session_data* sd);

#endif /* MAP_STYLE_OUTFITS_HPP */
