// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vulcanarrow.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillVulcanArrow::SkillVulcanArrow() : WeaponSkillImpl(CG_ARROWVULCAN) {
}

void SkillVulcanArrow::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const map_session_data* sd = BL_CAST(BL_PC, &src);
	if( battle_config.vulcanpvphit != 0 ) {
		if( sd && map_flag_vs(sd->m) )
			dmg.div_= battle_config.vulcanpvphit;
		else
			dmg.div_= skill_get_num( getSkillId(), skill_lv );
	}
}

void SkillVulcanArrow::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += 400 + 100 * skill_lv;
	RE_LVL_DMOD(100);
#else
	skillratio += 100 + 100 * skill_lv;
#endif
}
