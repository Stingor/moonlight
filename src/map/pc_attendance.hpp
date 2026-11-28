// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PC_ATTENDANCE_HPP
#define PC_ATTENDANCE_HPP

#include <common/cbasetypes.hpp>

class map_session_data;

/**
 * [Stingor] La présence Moonlight (ADR 0156 de moonclient).
 *
 * Un cycle GLISSANT de MOONLIGHT_ATTENDANCE_CYCLE_DAYS jours par compte
 * Moonlight (`sd->status.user_id`), tous comptes de jeu et personnages
 * confondus. Il s'ouvre à la première réclamation ; la Ne réclamation du cycle
 * donne le jour N du calendrier ; un jour manqué est perdu.
 *
 * L'état vit dans SQL (sql-files/moonlight_attendance.sql), lu et écrit en
 * transaction par le map-server comme par le site : deux sessions de la même
 * personne ne se doublent pas une réclamation. Le JOUR est CURDATE() du serveur
 * de base, jamais l'horloge du map-server.
 *
 * Le calendrier est une ÉDITION mensuelle que le site tire et écrit en base
 * (moonlight_attendance_edition, moonlight_attendance_calendar) ; le
 * map-server ne fait que la lire, à chaque envoi d'état et à chaque livraison.
 * Un cycle suit l'édition du jour où il s'ouvre jusqu'à sa fin. Le calendrier
 * part au client dans ZC_MOONLIGHT_ATTENDANCE (0x0F6F).
 */

/// Longueur d'un cycle, en jours de calendrier, et nombre de récompenses.
#define MOONLIGHT_ATTENDANCE_CYCLE_DAYS 28
/// Réclamations par le site permises dans un cycle.
#define MOONLIGHT_ATTENDANCE_WEB_CHARGES 3

/// Vrai si `feature.attendance` est allumé ET une édition couvre aujourd'hui.
bool moonlight_attendance_open();

/// Envoie ZC_MOONLIGHT_ATTENDANCE : l'état du cycle du compte, `result`
/// (e_moonlight_attendance_result) et le calendrier de son édition. `open`
/// demande au client d'ouvrir la fenêtre.
void moonlight_attendance_send( map_session_data& sd, bool open, uint8 result );

/// La fenêtre est demandée (CZ_UI_OPEN, IN_UI_ATTENDANCE).
void moonlight_attendance_request_state( map_session_data& sd );

/// Le joueur réclame le jour (0x0AEF) : réclamation, livraison, état.
void moonlight_attendance_request_claim( map_session_data& sd );

/// À l'entrée en jeu : livre les récompenses réclamées sur le site, puis ouvre
/// la fenêtre si le jour n'est pas encore réclamé.
void moonlight_attendance_login( map_session_data& sd );

#endif /* PC_ATTENDANCE_HPP */
