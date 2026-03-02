// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

BUILDIN_FUNC(viewequip)
{
	TBL_PC *sd, *tsd;

	if (script_isstring(st, 2)) {
		const char *name = script_getstr(st, 2);

		sd = map_nick2sd(name,false);
		if (sd == NULL) {
			ShowError("buildin_viewequip: Non-existant character %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		uint32 char_id = script_getnum(st, 2);

		sd = map_charid2sd(char_id);
		if (sd == NULL) {
			ShowError("buildin_viewequip: Non-existant character %d requested.\n", char_id);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (script_isstring(st, 3)) {
		const char *name = script_getstr(st, 3);

		tsd = map_nick2sd(name,false);
		if (tsd == NULL) {
			ShowError("buildin_viewequip: Non-existant character %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		uint32 char_id = script_getnum(st, 3);

		tsd = map_charid2sd(char_id);
		if (tsd == NULL) {
			ShowError("buildin_viewequip: Non-existant character %d requested.\n", char_id);
			return SCRIPT_CMD_FAILURE;
		}
	}

	clif_viewequip_ack(sd, tsd);
	script_pushint(st,1);
	
	return SCRIPT_CMD_SUCCESS;
}

/// setwaitingroomtitle "<title>","<npc_name>"; // [Stingor]
BUILDIN_FUNC(setwaitingroomtitle)
{
	struct npc_data* nd;
	const char* title = script_getstr(st, 2);
    
	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st, 3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
    
	if( nd != NULL )
		chat_changetitlenpcchat(nd, title);

	return 0;
}

BUILDIN_FUNC(npcsit)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	
	if( nd && mobavaildb_checkid(nd->class_) ) {
		nd->sitted = 1;
		clif_sitting(&nd->bl);
	}
	
	return 0;
}

BUILDIN_FUNC(npcstand)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	
	if( nd && mobavaildb_checkid(nd->class_) ) {
		nd->sitted = 0;
		clif_standing(&nd->bl);
	}

	return 0;
}

BUILDIN_FUNC(addcloneskill)
{
    int skill_id,skill_lv, lv;
    struct map_session_data* sd;
	uint16 idx;
    
	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	
    skill_id = script_getnum(st, 2);
    skill_lv = script_getnum(st, 3);
	
	if (!(idx = skill_get_index(skill_id)))
		return SCRIPT_CMD_FAILURE;
	
	if (pc_checkskill(sd,RG_PLAGIARISM)) {
	
		map_freeblock_lock();
		
		if (sd->cloneskill_idx > 0 && sd->status.skill[sd->cloneskill_idx].flag == SKILL_FLAG_PLAGIARIZED) {
			clif_deleteskill(sd,sd->status.skill[sd->cloneskill_idx].id);
			sd->status.skill[sd->cloneskill_idx].id = 0;
			sd->status.skill[sd->cloneskill_idx].lv = 0;
			sd->status.skill[sd->cloneskill_idx].flag = SKILL_FLAG_PERMANENT;
		}
		
		lv = min(skill_lv,pc_checkskill(sd,RG_PLAGIARISM)); //Copied level never be > player's RG_PLAGIARISM level
		sd->cloneskill_idx = idx;
		pc_setglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM), skill_id);
		pc_setglobalreg(sd, add_str(SKILL_VAR_PLAGIARISM_LV), lv);
		
		sd->status.skill[idx].id = skill_id;
		sd->status.skill[idx].lv = lv;
		sd->status.skill[idx].flag = SKILL_FLAG_PLAGIARIZED;
		clif_addskill(sd, skill_id);
		clif_skillinfoblock(sd);
		
		map_freeblock_unlock();
	}
	
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getalliance)
{
	struct guild* g;
	int i;
	int j=0;
	int gid = script_getnum(st,2);

	g = guild_search(gid);
	if( g == NULL || guild_get_alliance_count(g, 0) == 0 )
		return 0;
		
	for( i = 0; i < MAX_GUILDALLIANCE; i++) {
		if(	g->alliance[i].guild_id > 0 && g->alliance[i].opposition == 0 )
			mapreg_setreg(add_str("$@alliance_id")+(j << 24), g->alliance[i].guild_id);
		j++;
	}
	mapreg_setreg(add_str("$@alliance_count"), j);
	return 0;
}

BUILDIN_FUNC(getpartnername)
{
    struct map_session_data* sd;
	TBL_PC *p_sd=NULL;
	
	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	if ( sd==NULL || (p_sd=map_charid2sd(sd->status.partner_id)) == NULL ) {
		script_pushconststr(st, "");
		return 0;
	}

	script_pushstrcopy(st,p_sd->status.name);
	return 0;
}

/*==========================================
 * Works like /WHO - displays all online users
 *------------------------------------------*/
BUILDIN_FUNC(getusersnbr)
{
	TBL_PC *sd;
	int count = 0;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		count++;
	
	mapit_free(iter);
	
	script_pushint( st, count );
	
	return 0;
}

BUILDIN_FUNC(starteventexp)
{
	event_exp = true;

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(stopeventexp)
{
	event_exp = false;

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(eventexp)
{
	script_pushint(st, event_exp);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(starteventdrop)
{
	event_drop = true;

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(stopeventdrop)
{
	event_drop = false;

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(eventdrop)
{
	script_pushint(st, event_drop);
	return SCRIPT_CMD_SUCCESS;
}
