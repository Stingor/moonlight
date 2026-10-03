// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// [Stingor] Flag Graffiti : l'emblème de la guilde du lanceur, peint au sol.
// Gravity ne l'a jamais implémentée ; c'est une création Moonlight.
class SkillFlagGraffiti : public SkillImpl {
public:
	SkillFlagGraffiti();

	void castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const override;
};
