// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiritofthewizard.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpiritoftheWizard::SkillSpiritoftheWizard() : SkillImpl(SL_WIZARD) {
}

void SkillSpiritoftheWizard::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	// [Stingor] temps SPIRIT indexé sur le level du buffer pendant woe
	t_tick skilltime = skill_get_time( getSkillId(), skill_lv );
	if( sd && is_agit_start() )
		skilltime = (sd->status.base_level * 580 < skilltime / 3 ? skilltime / 3 : sd->status.base_level * 580);

	if( sc_start2( src, target, type, 100, skill_lv, getSkillId(), skilltime ) ){
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

		sc_start( src, src, SC_SMA, 100, skill_lv, skill_get_time( SL_SMA, skill_lv ) );
	}else{
		if( sd ){
			clif_skill_fail( *sd, getSkillId() );
		}
	}
}
