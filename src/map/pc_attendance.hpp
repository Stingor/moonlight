// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PC_ATTENDANCE_HPP
#define PC_ATTENDANCE_HPP

#include <memory>
#include <string>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/mmo.hpp> // t_itemid

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
 * Le calendrier se lit dans db/import/moonlight_attendance.yml, se publie dans
 * la table moonlight_attendance_calendar pour le site, et part au client dans
 * ZC_MOONLIGHT_ATTENDANCE (0x0F6F).
 */

/// Longueur d'un cycle, en jours de calendrier, et nombre de récompenses.
#define MOONLIGHT_ATTENDANCE_CYCLE_DAYS 28
/// Réclamations par le site permises dans un cycle.
#define MOONLIGHT_ATTENDANCE_WEB_CHARGES 3
/// Items d'un jour au plus : un courrier RODEX en porte autant.
#define MOONLIGHT_ATTENDANCE_DAY_ITEMS_MAX MAIL_MAX_ITEM

struct s_moonlight_attendance_reward {
	t_itemid item_id;
	uint16 amount;
};

struct s_moonlight_attendance_day {
	uint16 day;
	bool milestone;
	/// Points du Vote Shop crédités au compte Moonlight ; 0 sans.
	uint16 vote_points;
	std::vector<s_moonlight_attendance_reward> rewards;
};

class MoonlightAttendanceDatabase : public TypesafeYamlDatabase<uint16, s_moonlight_attendance_day> {
public:
	MoonlightAttendanceDatabase() : TypesafeYamlDatabase( "MOONLIGHT_ATTENDANCE_DB", 1 ){
	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;
	/// Refuse un calendrier incomplet (il reste vide : présence fermée), puis
	/// le publie dans moonlight_attendance_calendar.
	void loadingFinished() override;
};

extern MoonlightAttendanceDatabase moonlight_attendance_db;

/// Vrai si `feature.attendance` est allumé ET le calendrier chargé.
bool moonlight_attendance_open();

/// Envoie ZC_MOONLIGHT_ATTENDANCE : l'état du cycle du compte, `result`
/// (e_moonlight_attendance_result) et le calendrier. `open` demande au client
/// d'ouvrir la fenêtre.
void moonlight_attendance_send( map_session_data& sd, bool open, uint8 result );

/// La fenêtre est demandée (CZ_UI_OPEN, IN_UI_ATTENDANCE).
void moonlight_attendance_request_state( map_session_data& sd );

/// Le joueur réclame le jour (0x0AEF) : réclamation, livraison, état.
void moonlight_attendance_request_claim( map_session_data& sd );

/// À l'entrée en jeu : livre les récompenses réclamées sur le site, puis ouvre
/// la fenêtre si le jour n'est pas encore réclamé.
void moonlight_attendance_login( map_session_data& sd );

void do_init_moonlight_attendance( void );
void do_final_moonlight_attendance( void );

#endif /* PC_ATTENDANCE_HPP */
