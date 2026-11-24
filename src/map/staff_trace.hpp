// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LE TRACEUR DU STAFF (ZC 0x0F6E, voir packets_struct.hpp) : à chaque
// variable ou quête qu'un script, une commande @ ou le code du serveur change
// chez un membre du staff, sa valeur avant et après part à son client, qui la
// range dans une catégorie à part de la conversation. De quoi suivre un PNJ pas
// à pas : quelle variable il pose, dans quel ordre, et où l'on en est.
//
// 🔴 TROIS PORTES, toutes au serveur : session vérifiée, niveau de groupe au
// moins MOONLIGHT_STAFF_TRACE_MIN_LEVEL, et la famille allumée dans le réglage
// MOONCLIENT_SETTING_STAFF_TRACE (masque e_moonlight_staff_trace), éteint
// d'office. Le destinataire est TOUJOURS le joueur dont la variable change (ou,
// pour une variable de PNJ ou d'instance, celui à qui le script est attaché) :
// jamais le joueur d'un autre.
//
// Une écriture qui ne change pas la valeur ne se dit pas. Au-delà de
// MOONLIGHT_STAFF_TRACE_BURST annonces par seconde, les suivantes se taisent, et
// la première de la seconde d'après dit combien se sont tues.

#ifndef MAP_STAFF_TRACE_HPP
#define MAP_STAFF_TRACE_HPP

#include <common/cbasetypes.hpp>

class map_session_data;
struct script_state;

// Le niveau de groupe qui ouvre le traceur : celui des outils du staff.
#define MOONLIGHT_STAFF_TRACE_MIN_LEVEL 80
// Les annonces qu'une session reçoit au plus par seconde.
#define MOONLIGHT_STAFF_TRACE_BURST 200
// Les octets d'un texte du paquet au plus, zéro compris.
#define MOONLIGHT_STAFF_TRACE_TEXT_MAX 256

// Les familles que le réglage allume, une par bit. ⚠ CONTRAT avec le client
// (staff_trace.h) : on AJOUTE à la suite.
enum e_moonlight_staff_trace : uint8 {
	MOONLIGHT_STAFF_TRACE_QUESTS    = 0x01, ///< setquest, completequest, erasequest, changequest…
	MOONLIGHT_STAFF_TRACE_PERMANENT = 0x02, ///< `var`, `#var`, `##var`
	MOONLIGHT_STAFF_TRACE_TEMPORARY = 0x04, ///< `@var`
	MOONLIGHT_STAFF_TRACE_NPC       = 0x08, ///< `.var` du PNJ et `'var` de l'instance
	MOONLIGHT_STAFF_TRACE_ALL       = 0x0F,
};

// Ce qu'annonce un paquet. ⚠ Valeurs sur le fil : on AJOUTE en fin.
enum e_moonlight_staff_trace_kind : uint8 {
	MOONLIGHT_STAFF_TRACE_KIND_VARIABLE = 1, ///< une variable ; avant et après
	MOONLIGHT_STAFF_TRACE_KIND_QUEST    = 2, ///< une quête ; états avant et après
	MOONLIGHT_STAFF_TRACE_KIND_DROPPED  = 3, ///< `before_num` annonces tues par le plafond
};

// La portée d'une variable, lue à son préfixe. ⚠ Valeurs sur le fil.
enum e_moonlight_staff_trace_scope : uint8 {
	MOONLIGHT_STAFF_TRACE_SCOPE_NONE      = 0,
	MOONLIGHT_STAFF_TRACE_SCOPE_CHARACTER = 1, ///< `var`
	MOONLIGHT_STAFF_TRACE_SCOPE_ACCOUNT   = 2, ///< `#var`
	MOONLIGHT_STAFF_TRACE_SCOPE_GLOBAL    = 3, ///< `##var`
	MOONLIGHT_STAFF_TRACE_SCOPE_TEMPORARY = 4, ///< `@var`
	MOONLIGHT_STAFF_TRACE_SCOPE_NPC       = 5, ///< `.var`
	MOONLIGHT_STAFF_TRACE_SCOPE_INSTANCE  = 6, ///< `'var`
};

// D'où vient le changement. ⚠ Valeurs sur le fil.
enum e_moonlight_staff_trace_source : uint8 {
	MOONLIGHT_STAFF_TRACE_SOURCE_SERVER  = 0, ///< le code du serveur, hors script et commande
	MOONLIGHT_STAFF_TRACE_SOURCE_SCRIPT  = 1, ///< un script ; `npc_id` dit lequel
	MOONLIGHT_STAFF_TRACE_SOURCE_COMMAND = 2, ///< une commande @ (ou #) en cours
};

// L'état d'une quête dans l'annonce : celui du journal, ou absente.
#define MOONLIGHT_STAFF_TRACE_QUEST_ABSENT (-1)

// Une variable de joueur (`var`, `#var`, `##var`, `@var`) a changé. `before` et
// `after` valent pour un entier ; les deux textes pour une chaîne (nullptr = vide).
void staff_trace_player_num( map_session_data* sd, int64 uid, int64 before, int64 after );
void staff_trace_player_str( map_session_data* sd, int64 uid, const char* before, const char* after );

// Une variable de PNJ (`.var`) ou d'instance (`'var`) a changé sous le script `st`.
void staff_trace_script_num( script_state* st, int64 uid, const char* name, int64 before, int64 after );
void staff_trace_script_str( script_state* st, int64 uid, const char* name, const char* before, const char* after );

// Une quête du journal de `sd` passe de l'état `before` à `after`
// (e_quest_state, ou MOONLIGHT_STAFF_TRACE_QUEST_ABSENT).
void staff_trace_quest( map_session_data* sd, int32 quest_id, int32 before, int32 after );

// Une commande @ s'exécute pour `sd` : ce qu'elle change se dit d'elle.
void staff_trace_command_begin( map_session_data* sd );
void staff_trace_command_end( map_session_data* sd );

#endif /* MAP_STAFF_TRACE_HPP */
