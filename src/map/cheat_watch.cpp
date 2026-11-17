// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Le guet des heuristiques de triche : voir l'en-tête.

#include "cheat_watch.hpp"

#include <cstdarg>
#include <cstdlib>

#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "map.hpp"
#include "pc.hpp"

namespace {

// Le numéro de la guerre de guilde en cours, ou de la dernière : il avance à
// chaque ouverture. Zéro avant la première, que s_cheat_watch::fair_play_war
// tient pour « aucun rappel encore ».
uint32 fair_play_war = 0;

// Un château pendant la guerre qui le concerne : FE et SE pour les châteaux
// classiques, TE pour les siens. Les cartes GvG permanentes n'en sont pas.
bool fair_play_applies(int16 m) {
	const map_data* mapdata = map_getmapdata(m);
	if (mapdata == nullptr)
		return false;
	return ((agit_flag || agit2_flag) && mapdata->getMapFlag(MF_GVG_CASTLE)) ||
		(agit3_flag && mapdata->getMapFlag(MF_GVG_TE_CASTLE));
}

// `fresh` : un rapport vient d'arriver. Sinon, le rappel d'un outil signalé
// plus tôt, une fois par guerre.
void fair_play_remind(map_session_data& sd, bool fresh) {
	const t_tick interval_ms = static_cast<t_tick>(battle_config.cheat_fair_play_interval);
	s_cheat_watch& watch = sd.cheat_watch;
	if (interval_ms == 0 || watch.client_tool[0] == '\0' || !fair_play_applies(sd.m))
		return;

	const t_tick now = gettick();
	if (watch.fair_play_war == fair_play_war) {
		if (!fresh || (watch.fair_play_tick != 0 && DIFF_TICK(now, watch.fair_play_tick) < interval_ms))
			return;
	}
	if (!clif_moonlight_fair_play(sd, watch.client_tool))
		return;
	watch.fair_play_tick = now;
	watch.fair_play_war = fair_play_war;

	ShowInfo("[CheatDetector] %s (AID:%d CID:%d) -- rappel de fair-play en guerre de guilde (%s)\n",
		sd.status.name, sd.status.account_id, sd.status.char_id, watch.client_tool);
}

int32 fair_play_remind_sub(map_session_data* sd, va_list /*args*/) {
	if (sd != nullptr)
		fair_play_remind(*sd, false);
	return 0;
}

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

void cheat_watch_note_client_tool(map_session_data& sd, const char* tool) {
	if (tool == nullptr || tool[0] == '\0')
		return;
	safestrncpy(sd.cheat_watch.client_tool, tool, sizeof(sd.cheat_watch.client_tool));
	fair_play_remind(sd, true);
}

void cheat_watch_fair_play_on_arrival(map_session_data& sd) {
	fair_play_remind(sd, false);
}

void cheat_watch_fair_play_war_started() {
	++fair_play_war;
	map_foreachpc(&fair_play_remind_sub);
}
