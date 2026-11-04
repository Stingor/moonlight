// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] Les heuristiques de triche : voir l'en-tête.

#include "cheat_heuristics.hpp"

#include <cmath>
#include <cstdlib>

namespace {

// Un saut d'horloge client de plus de la moitié du compteur 32 bits ne se lit
// pas comme une avance : c'est un retour en arrière, donc une horloge qui a
// repris de zéro. La mesure recommence avec elle.
constexpr uint32 CLIENT_TICK_BACKWARDS = 0x80000000u;

// Une fenêtre compte si elle a reçu au moins ce nombre d'horloges : à une
// horloge toutes les dix secondes, une fenêtre d'une minute en reçoit six.
constexpr uint32 CLOCK_MIN_SAMPLES = 3;

constexpr int64 PERMILLE = 1000;

void clock_open_window(s_clock_drift_state& state, t_tick server_tick, int64 offset_ms) {
	state.window_start_tick = server_tick;
	state.window_samples = 1;
	state.window_min_offset_ms = offset_ms;
	state.window_min_tick = server_tick;
}

void clock_restart(s_clock_drift_state& state, t_tick server_tick, uint32 client_tick) {
	state = s_clock_drift_state();
	state.started = true;
	state.last_client_tick = client_tick;
	state.client_unwrapped_ms = client_tick;
	clock_open_window(state, server_tick, server_tick - state.client_unwrapped_ms);
}

}  // namespace

std::optional<s_clock_drift_verdict> clock_drift_note(s_clock_drift_state& state, const s_clock_drift_params& params, t_tick server_tick, uint32 client_tick) {
	if (params.threshold_permille == 0)
		return std::nullopt;

	if (!state.started) {
		clock_restart(state, server_tick, client_tick);
		return std::nullopt;
	}

	const uint32 advance_ms = client_tick - state.last_client_tick;
	if (advance_ms >= CLIENT_TICK_BACKWARDS) {
		clock_restart(state, server_tick, client_tick);
		return std::nullopt;
	}
	state.last_client_tick = client_tick;
	state.client_unwrapped_ms += advance_ms;

	const int64 offset_ms = server_tick - state.client_unwrapped_ms;
	++state.window_samples;
	if (offset_ms < state.window_min_offset_ms) {
		state.window_min_offset_ms = offset_ms;
		state.window_min_tick = server_tick;
	}

	if (DIFF_TICK(server_tick, state.window_start_tick) < static_cast<t_tick>(params.window_ms))
		return std::nullopt;

	std::optional<s_clock_drift_verdict> verdict;

	if (state.window_samples >= CLOCK_MIN_SAMPLES) {
		// Les deux minimums peuvent tomber n'importe où dans leur fenêtre : la
		// dérive se rapporte au temps qui les sépare vraiment, et une paire trop
		// proche pour dire quoi que ce soit ne compte pas.
		const t_tick between_ms = DIFF_TICK(state.window_min_tick, state.reference_tick);
		if (state.has_reference && between_ms >= static_cast<t_tick>(params.window_ms / 2)) {
			// Une horloge client qui avance plus vite réduit l'écart : la dérive
			// positive est la baisse du minimum.
			const int64 drift_ms = state.reference_offset_ms - state.window_min_offset_ms;
			const int32 drift_permille = static_cast<int32>(drift_ms * PERMILLE / between_ms);

			if (static_cast<uint32>(std::abs(drift_permille)) >= params.threshold_permille)
				++state.strikes;
			else
				state.strikes = 0;

			if (state.strikes >= params.strikes)
				verdict = s_clock_drift_verdict{ drift_permille, static_cast<uint32>(between_ms) };
		}
		state.has_reference = true;
		state.reference_offset_ms = state.window_min_offset_ms;
		state.reference_tick = state.window_min_tick;
	}

	clock_open_window(state, server_tick, offset_ms);
	return verdict;
}

std::optional<s_cadence_verdict> cadence_note(s_cadence_state& state, const s_cadence_params& params, t_tick server_tick) {
	if (params.threshold_permille == 0 || params.samples == 0)
		return std::nullopt;

	if (!state.has_last) {
		state.has_last = true;
		state.last_tick = server_tick;
		return std::nullopt;
	}

	const t_tick interval_ms = DIFF_TICK(server_tick, state.last_tick);
	state.last_tick = server_tick;
	if (interval_ms < static_cast<t_tick>(params.floor_ms) || interval_ms > static_cast<t_tick>(params.ceiling_ms))
		return std::nullopt;

	const uint64 interval = static_cast<uint64>(interval_ms);
	++state.count;
	state.sum_ms += interval;
	state.sum_sq_ms += interval * interval;
	if (state.count < params.samples)
		return std::nullopt;

	const double count = static_cast<double>(state.count);
	const double mean = static_cast<double>(state.sum_ms) / count;
	const double variance = std::fmax(0.0, static_cast<double>(state.sum_sq_ms) / count - mean * mean);
	const uint32 variation_permille = static_cast<uint32>(std::sqrt(variance) / mean * PERMILLE);
	const s_cadence_verdict measured{ variation_permille, static_cast<uint32>(mean), state.count };

	state.count = 0;
	state.sum_ms = 0;
	state.sum_sq_ms = 0;

	if (variation_permille >= params.threshold_permille)
		return std::nullopt;
	return measured;
}
