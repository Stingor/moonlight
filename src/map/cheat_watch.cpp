// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Le guet des heuristiques de triche : voir l'en-tête.

#include "cheat_watch.hpp"

#include <cstdlib>

#include <common/showmsg.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "pc.hpp"

namespace {

const char* const CADENCE_KIND_NAMES[CHEAT_CADENCE_KIND_COUNT] = {
	"attaques",
	"competences",
	"objets",
};

// Une fois par intervalle et par indice : un indice qui dure se lit en une ligne
// toutes les quelques minutes, pas en une ligne par paquet.
bool cheat_watch_may_report(t_tick& reported_tick, t_tick now) {
	const t_tick interval_ms = static_cast<t_tick>(battle_config.cheat_report_interval);
	if (reported_tick != 0 && DIFF_TICK(now, reported_tick) < interval_ms)
		return false;
	reported_tick = now;
	return true;
}

void cheat_watch_count(uint16& reports) {
	if (reports < UINT16_MAX)
		++reports;
}

}  // namespace

void cheat_watch_note_client_tick(map_session_data& sd, uint32 client_tick) {
	s_clock_drift_params params;
	params.window_ms = static_cast<uint32>(battle_config.cheat_clock_window);
	params.threshold_permille = static_cast<uint32>(battle_config.cheat_clock_drift);
	params.strikes = static_cast<uint32>(battle_config.cheat_clock_strikes);

	const t_tick now = gettick();
	const auto verdict = clock_drift_note(sd.cheat_watch.clock, params, now, client_tick);
	if (!verdict || !cheat_watch_may_report(sd.cheat_watch.clock_reported_tick, now))
		return;
	cheat_watch_count(sd.cheat_watch.clock_reports);

	const int32 drift = verdict->drift_permille;
	ShowWarning("[Heuristique] %s (AID:%d CID:%d) -- horloge du client %s de %d.%d %% sur %u s\n",
		sd.status.name, sd.status.account_id, sd.status.char_id,
		drift > 0 ? "en avance" : "en retard",
		std::abs(drift) / 10, std::abs(drift) % 10,
		verdict->over_ms / 1000);
}

void cheat_watch_note_action(map_session_data& sd, e_cheat_cadence_kind kind) {
	if (kind >= CHEAT_CADENCE_KIND_COUNT)
		return;

	s_cadence_params params;
	params.samples = static_cast<uint32>(battle_config.cheat_cadence_samples);
	params.floor_ms = static_cast<uint32>(battle_config.cheat_cadence_floor);
	params.ceiling_ms = static_cast<uint32>(battle_config.cheat_cadence_ceiling);
	params.threshold_permille = static_cast<uint32>(battle_config.cheat_cadence_variation);

	const t_tick now = gettick();
	const auto verdict = cadence_note(sd.cheat_watch.cadence[kind], params, now);
	if (!verdict || !cheat_watch_may_report(sd.cheat_watch.cadence_reported_tick[kind], now))
		return;
	cheat_watch_count(sd.cheat_watch.cadence_reports);

	ShowWarning("[Heuristique] %s (AID:%d CID:%d) -- %u %s a %u ms d'intervalle, variation %u.%u %%\n",
		sd.status.name, sd.status.account_id, sd.status.char_id,
		verdict->samples, CADENCE_KIND_NAMES[kind], verdict->mean_ms,
		verdict->variation_permille / 10, verdict->variation_permille % 10);
}
