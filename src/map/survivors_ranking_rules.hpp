// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LES RÈGLES DU CLASSEMENT SURVIVORS que le serveur refait : la semaine
// ISO et la graine du défi, le score, et les bornes d'une partie par version de
// règles. Des fonctions pures, sans session, sans base ni horloge : l'appelant
// passe l'heure qu'il a lue. survivors_ranking.cpp les emploie.
//
// 🔴 CHAQUE CALCUL REFAIT CELUI DU CLIENT À L'ENTIER PRÈS. La semaine, la graine
// et le score de moonclient (ro::sim::IsoWeekOfDay, SeedOfWeek, SurvivorsScore)
// sont recalculés ici avec la même arithmétique : un écart d'un seul entier fait
// refuser au client un jeton honnête, ou dire faux un score accepté. Toute
// modification de leur côté monte leur version de règles, et se reporte ici sous
// une version neuve, sans toucher aux anciennes.

#ifndef MAP_SURVIVORS_RANKING_RULES_HPP
#define MAP_SURVIVORS_RANKING_RULES_HPP

#include <cstddef>

#include <common/cbasetypes.hpp>

// Les tableaux du fil (champ `board`).
enum e_survivors_board : uint8 {
	SURVIVORS_BOARD_RANKED    = 0, ///< partie classée : graine tirée par le serveur
	SURVIVORS_BOARD_CHALLENGE = 1, ///< défi de la semaine : graine de la semaine ISO
};

// Les issues qu'END porte : la valeur numérique de SurvivorsOutcome du client.
enum e_survivors_outcome : uint8 {
	SURVIVORS_OUTCOME_FELL     = 1,
	SURVIVORS_OUTCOME_SURVIVED = 2,
	SURVIVORS_OUTCOME_VICTORY  = 3,
};

// Un pas de simulation du client : 20 ms, cinquante par seconde.
static constexpr uint32 SURVIVORS_MS_PER_STEP = 20;
static constexpr uint32 SURVIVORS_STEPS_PER_SECOND = 50;

// Une semaine ISO 8601 : le lundi la commence, et la semaine 1 d'une année est
// celle qui contient son premier jeudi.
struct s_survivors_iso_week {
	int32 year = 0;
	uint8 week = 0; ///< 1 à 53
};

// La semaine ISO de l'instant `utc_s` (secondes UTC depuis 1970) : elle change
// le lundi à 00:00 UTC.
s_survivors_iso_week survivors_iso_week_of_utc(int64 utc_s);
// La clé de semaine d'un tableau : année ISO × 100 + semaine.
uint32 survivors_week_key(const s_survivors_iso_week& week);
// LA GRAINE DU DÉFI : le finaliseur SplitMix64 appliqué à `année ISO × 100 +
// semaine` plus un sel, modulo 2^64. L'ADR 0133 de moonclient l'écrit.
uint64 survivors_seed_of_week(const s_survivors_iso_week& week);

// Les composantes d'une partie finie, telles qu'END les porte.
struct s_survivors_components {
	uint32 survived_steps = 0;
	uint32 kill_points = 0;
	uint32 kills = 0;
	uint16 boss_kills = 0;
	uint16 level = 0;
	uint8 outcome = 0;
};

// LE SCORE : kill_points + 10 × secondes ENTIÈRES survécues (survived_steps / 50,
// la division d'abord) + 100 × level + 5 000 si survie, 25 000 si victoire.
uint64 survivors_score(const s_survivors_components& run);

// Un boss programmé d'une version de règles : l'instant où il paraît, les points
// et l'XP de sa chute (avant toute majoration).
struct s_survivors_boss_bound {
	uint32 at_ms;
	uint32 kill_points;
	uint32 exp;
	bool final; ///< sa chute est la victoire
};

// Un segment de la courbe d'XP : du niveau L au suivant, base + per_level ×
// (L − from_level), pour le dernier segment dont from_level <= L.
struct s_survivors_exp_segment {
	uint16 from_level;
	uint32 base;
	uint32 per_level;
};

// CE QUE LE SERVEUR CLASSE D'UNE VERSION DE RÈGLES, et les bornes qu'il impose à
// une partie : tirées du catalogue de cette version côté client.
struct s_survivors_ruleset {
	uint16 ruleset;
	const uint16* arena_ids;
	size_t arena_count;
	const uint16* class_ids;
	size_t class_count;
	uint32 run_ms;      ///< la durée de la partie
	uint32 overtime_ms; ///< au-delà de run_ms, tant que le boss final vit
	const s_survivors_boss_bound* bosses; ///< triés par at_ms
	size_t boss_count;
	const s_survivors_exp_segment* exp_curve; ///< triée, la première au niveau 1
	size_t exp_segment_count;
	uint32 most_kills_per_second;   ///< les tués au plus, par seconde survécue entamée
	uint32 most_common_kill_points; ///< les points d'un tué ordinaire au plus, élite comprise
	uint32 most_common_kill_exp;    ///< l'XP d'un tué ordinaire au plus, élite comprise
	uint32 most_exp_gain_pct;       ///< la majoration d'XP au plus, en % (100 : aucune)
};

// Les versions classées, bornes comprises : celles que ZC 0x0F53 annonce.
uint16 survivors_ruleset_low();
uint16 survivors_ruleset_high();
// La version `ruleset`, ou nullptr si le serveur ne la classe pas.
const s_survivors_ruleset* survivors_find_ruleset(uint16 ruleset);
// Vrai si la version classe ce tableau : arène, classe et genre connus.
bool survivors_board_known(const s_survivors_ruleset& rules, uint16 arena_id, uint16 class_id, uint8 board);

// Une marge de pas au-delà de la plus longue partie, et de secondes entre la
// durée survécue et le temps écoulé depuis le jeton.
static constexpr uint32 SURVIVORS_STEPS_SLACK = SURVIVORS_STEPS_PER_SECOND;
static constexpr int64 SURVIVORS_DURATION_SLACK_S = 5;

// VRAI SI LA PARTIE EST VRAISEMBLABLE sous `rules`, `elapsed_s` secondes après
// l'heure de son jeton : une issue portable ; survived_steps au plus la partie
// et son dépassement, marge comprise ; survived_steps / 50 au plus elapsed_s +
// 5 ; au plus most_kills_per_second tués par seconde entamée, et au moins
// autant de tués que de boss ; boss_kills au plus les boss parus avant ce pas ;
// kill_points au plus les tués ordinaires au plus fort, plus les boss_kills
// plus riches des boss parus ; level au plus celui que donne l'XP de ces mêmes
// tués, majorée au plus ; une
// survie qui a tenu run_ms ; une victoire dont le boss final a paru et compte
// parmi les boss tués.
bool survivors_plausible(const s_survivors_ruleset& rules, const s_survivors_components& run, int64 elapsed_s);

// VRAI SI UNE PARTIE CLASSÉE PEUT PARTIR DE LA CARTE `map_name`, au-delà de
// MF_TOWN : une carte qui a une fiche de ville chez moonclient, ou qu'il tient
// pour un intérieur (instance comprise). C'est la garde d'entrée du client :
// le serveur ne refuse pas un lieu que le client a laissé partir.
bool survivors_sheltered_place(const char* map_name);

#endif /* MAP_SURVIVORS_RANKING_RULES_HPP */
