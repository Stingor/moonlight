// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flaggraffiti.hpp"

#include "map/unit.hpp"

SkillFlagGraffiti::SkillFlagGraffiti() : SkillImpl(RG_FLAGGRAFFITI) {
}

// [Stingor] UN SEUL EMBLÈME À LA FOIS par lanceur : le précédent s'efface avant
// que le nouveau ne se pose. La guilde et la version de son emblème sont prises
// par skill_unitsetting, au moment même de la pose.
void SkillFlagGraffiti::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (unit_data* ud = unit_bl2ud(src); ud != nullptr) {
		for (auto it = ud->skillunits.begin(); it != ud->skillunits.end();) {
			if ((*it)->skill_id == RG_FLAGGRAFFITI) {
				skill_delunitgroup(*it);
				it = ud->skillunits.begin();
			} else {
				++it;
			}
		}
	}
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
	flag|=1;
}
