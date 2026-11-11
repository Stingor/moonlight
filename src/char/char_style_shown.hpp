// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les recettes de la tablée de moonclient (CH 0x0F65, HC 0x0F66, voir
// common/packets.hpp ; dans moonclient, l'ADR 0147 et
// docs/couleurs-du-personnage/conception.md, lot L14).
//
// Le serveur de personnages n'a ni table d'objets ni équipement en mémoire. Il
// rend donc l'INSTANTANÉ que le serveur de carte range à la déconnexion
// (`moonclient_style_shown`) : ce que chaque personnage montrait, v7 et calques,
// sous les clés affichées.
//
// 🔴 SEULS LES PERSONNAGES DU COMPTE : les char_id viennent de `found_char`, la
// liste que ce serveur a dressée pour la session, jamais du client. Ce qui sort
// de la base se rejuge avant de partir.

#ifndef CHAR_STYLE_SHOWN_HPP
#define CHAR_STYLE_SHOWN_HPP

#include <common/cbasetypes.hpp>

struct char_session_data;

// L'écart minimal entre deux CH 0x0F65 d'une session, en millisecondes.
#define MOONCLIENT_CHAR_STYLES_CADENCE_MS 1000

bool chclif_parse_moonclient_char_styles(int32 fd, char_session_data& sd);

#endif /* CHAR_STYLE_SHOWN_HPP */
