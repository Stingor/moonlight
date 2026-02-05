// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "heal.hpp"

#include "../../mob.hpp"
#include "../../pc.hpp"

SkillHeal::SkillHeal() : SkillImpl(AL_HEAL)
{
}

void SkillHeal::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	status_change *tsc = status_get_sc(bl);
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *dstsd = nullptr;
	status_data* sstatus = status_get_status_data(*src);
	mob_data *dstmd = BL_CAST(BL_MOB, bl);

	int32 heal = skill_calc_heal(src, bl, getSkillId(), skill_lv, true);

	// if (status_isimmune(bl) || (dstmd && (status_get_class(bl) == MOBID_EMPERIUM || status_get_class_(bl) == CLASS_BATTLEFIELD)))
		// heal = 0;

	// [Stingor] -->
	if (status_get_class(bl) == MOBID_EMPERIUM || status_get_class(bl) == MOBID_GUARDIAN_STONE1 || status_get_class(bl)== MOBID_GUARDIAN_STONE2) {
		status_heal(bl,battle_config.emp_heal_hp,0,0);
		clif_skill_nodamage (src, *bl, getSkillId(), battle_config.emp_heal_hp);
		return;
	}

	if (status_get_class_(bl) == CLASS_BATTLEFIELD)
		heal = 0;
	// [Stingor] <--

	if (tsc != nullptr && !tsc->empty())
	{
		if (tsc->getSCE(SC_KAITE) && !status_has_mode(sstatus, MD_STATUSIMMUNE))
		{ // Bounce back heal
			if (--tsc->getSCE(SC_KAITE)->val2 <= 0)
				status_change_end(bl, SC_KAITE);
			if (src == bl)
				heal = 0; // When you try to heal yourself under Kaite, the heal is voided.
			else
			{
				bl = src;
				dstsd = sd;
			}
		}
		else if (tsc->getSCE(SC_BERSERK) || tsc->getSCE(SC_SATURDAYNIGHTFEVER))
		{
			heal = 0; // Needed so that it actually displays 0 when healing.
		}
	}

	status_change_end(bl, SC_BITESCAR);
	if (tsc && tsc->getSCE(SC_AKAITSUKI) && heal)
		heal = ~heal + 1;

	// [Stingor] -->
	if (status_isimmune(bl) && battle_config.healgtb)
		heal /= 100 / status_isimmune(bl);
	heal = (heal * battle_config.heal_rate)/100;
	// [Stingor] <--

	clif_skill_nodamage(src, *bl, getSkillId(), heal);
	t_exp heal_get_jobexp = status_heal(bl, heal, 0, 0);

	if (sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0)
	{
		heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
		if (heal_get_jobexp <= 0)
			heal_get_jobexp = 1;
		pc_gainexp(sd, bl, 0, heal_get_jobexp, 0);
	}
}

void SkillHeal::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const
{
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
