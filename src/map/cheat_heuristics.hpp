// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] LES HEURISTIQUES DE TRICHE que le serveur tient seul, sans rien
// demander au client : la dérive de son horloge (accélérateur de vitesse) et la
// régularité de ses actions (macro, bot). Elles valent pour tout client, celui
// d'origine, moonclient, ou un programme qui parle le protocole sans fenêtre.
//
// Des fonctions pures, sans session ni horloge : l'appelant passe l'heure du
// serveur qu'il a lue et les réglages en vigueur. cheat_watch.cpp les relie à
// la session.
//
// 🔴 ELLES SIGNALENT, ELLES NE SANCTIONNENT PAS. Un indice seul ne prouve rien :
// un réseau capricieux, une touche tenue enfoncée, un joueur très régulier. Le
// verdict revient à un humain qui lit le journal.

#ifndef MAP_CHEAT_HEURISTICS_HPP
#define MAP_CHEAT_HEURISTICS_HPP

#include <optional>

#include <common/cbasetypes.hpp>
#include <common/timer.hpp> // t_tick

// ── LA DÉRIVE D'HORLOGE ─────────────────────────────────────────────────────
//
// Le client envoie son horloge (CZ_REQUEST_TIME) à intervalle régulier. Un
// accélérateur de vitesse fait avancer cette horloge plus vite que celle du
// serveur : l'écart `serveur - client` diminue alors sans cesse.
//
// ⚠ LA LATENCE NE FAIT QU'AUGMENTER CET ÉCART, jamais le diminuer : un paquet
// retardé arrive plus tard, son horloge client est donc plus vieille. Le
// MINIMUM de l'écart sur une fenêtre est l'estimation sans latence, et c'est
// lui qu'on compare d'une fenêtre à la suivante. Un pic de latence isolé ne
// fausse donc rien, tant qu'un paquet de la fenêtre est passé sans retard.

struct s_clock_drift_params {
	uint32 window_ms = 60000;        ///< durée d'une fenêtre de mesure
	uint32 threshold_permille = 30;  ///< dérive signalée, en pour mille ; 0 = éteint
	uint32 strikes = 2;              ///< fenêtres consécutives au-delà du seuil
};

struct s_clock_drift_state {
	bool started = false;
	uint32 last_client_tick = 0;
	// L'horloge du client déroulée : son compteur 32 bits fait le tour en 49 jours.
	int64 client_unwrapped_ms = 0;

	t_tick window_start_tick = 0;
	uint32 window_samples = 0;
	int64 window_min_offset_ms = 0;
	t_tick window_min_tick = 0;

	bool has_reference = false;
	int64 reference_offset_ms = 0;
	t_tick reference_tick = 0;

	uint32 strikes = 0;
};

// Ce que la mesure a trouvé : une horloge client qui avance (`drift_permille`
// positif) ou recule (négatif) par rapport au serveur, mesurée sur `over_ms`.
struct s_clock_drift_verdict {
	int32 drift_permille = 0;
	uint32 over_ms = 0;
};

// Note une horloge reçue. Rend un verdict quand la dérive dépasse le seuil
// pendant `strikes` fenêtres de suite, et le rend à chaque fenêtre tant qu'elle
// dure : la cadence du journal est l'affaire de l'appelant.
std::optional<s_clock_drift_verdict> clock_drift_note(s_clock_drift_state& state, const s_clock_drift_params& params, t_tick server_tick, uint32 client_tick);

// ── LA RÉGULARITÉ DES ACTIONS ───────────────────────────────────────────────
//
// Entre deux attaques, deux compétences ou deux objets, l'intervalle d'un
// humain varie beaucoup ; celui d'une macro presque pas. On mesure le
// coefficient de variation (écart type / moyenne) d'un lot d'intervalles.
//
// ⚠ DEUX BORNES ÉCARTENT CE QUI N'EST PAS UN RYTHME :
//   - sous le plancher, la répétition automatique d'une touche tenue, aussi
//     régulière qu'une macro et parfaitement légitime ;
//   - au-dessus du plafond, une pause : le joueur marche, parle, regarde.
// Les intervalles hors bornes ne comptent pas, mais ne vident pas le lot.

struct s_cadence_params {
	uint32 samples = 32;             ///< intervalles par lot
	uint32 floor_ms = 150;           ///< plus court : répétition de touche
	uint32 ceiling_ms = 5000;        ///< plus long : une pause
	uint32 threshold_permille = 30;  ///< variation signalée, en pour mille ; 0 = éteint
};

struct s_cadence_state {
	bool has_last = false;
	t_tick last_tick = 0;
	uint32 count = 0;
	uint64 sum_ms = 0;
	uint64 sum_sq_ms = 0;
};

// Ce que le lot a trouvé : son coefficient de variation et son intervalle moyen.
struct s_cadence_verdict {
	uint32 variation_permille = 0;
	uint32 mean_ms = 0;
	uint32 samples = 0;
};

// Note une action reçue. Rend un verdict quand un lot complet est plus régulier
// que le seuil ; le lot repart de zéro dans tous les cas.
std::optional<s_cadence_verdict> cadence_note(s_cadence_state& state, const s_cadence_params& params, t_tick server_tick);

// ── CE QUE LA SESSION PORTE ─────────────────────────────────────────────────

enum e_cheat_cadence_kind : uint8 {
	CHEAT_CADENCE_ATTACK = 0,
	CHEAT_CADENCE_SKILL,
	CHEAT_CADENCE_ITEM,
	CHEAT_CADENCE_KIND_COUNT,
};

struct s_cheat_watch {
	s_clock_drift_state clock;
	t_tick clock_reported_tick = 0;
	s_cadence_state cadence[CHEAT_CADENCE_KIND_COUNT];
	t_tick cadence_reported_tick[CHEAT_CADENCE_KIND_COUNT] = {};
};

#endif /* MAP_CHEAT_HEURISTICS_HPP */
