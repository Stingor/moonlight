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

/* getskillname (skill_id)
 * Pushes the name of the given skill id as a string, or empty string if unknown.
 */
BUILDIN_FUNC(getskillname)
{
	int32 skill_id = script_getnum(st, 2);
	const char* name = skill_get_desc((uint16)skill_id);
	if (name && *name)
		script_pushstrcopy(st, name);
	else
		script_pushconststr(st, "");
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(viewequip)
{
	TBL_PC *sd, *tsd;

	if (script_isstring(st, 2)) {
		const char *name = script_getstr(st, 2);

		sd = map_nick2sd(name,false);
		if (sd == nullptr) {
			ShowError("buildin_viewequip: Non-existant character %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		uint32 char_id = script_getnum(st, 2);

		sd = map_charid2sd(char_id);
		if (sd == nullptr) {
			ShowError("buildin_viewequip: Non-existant character %d requested.\n", char_id);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (script_isstring(st, 3)) {
		const char *name = script_getstr(st, 3);

		tsd = map_nick2sd(name,false);
		if (tsd == nullptr) {
			ShowError("buildin_viewequip: Non-existant character %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		uint32 char_id = script_getnum(st, 3);

		tsd = map_charid2sd(char_id);
		if (tsd == nullptr) {
			ShowError("buildin_viewequip: Non-existant character %d requested.\n", char_id);
			return SCRIPT_CMD_FAILURE;
		}
	}

	clif_viewequip_ack( *sd, *tsd );
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
    
	if( nd != nullptr )
		chat_changetitlenpcchat(nd, title);

	return 0;
}

BUILDIN_FUNC(npcsit)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	
	if( nd && mobavaildb_checkid(nd->class_) ) {
		nd->sitted = 1;
		clif_sitting(*nd);
	}
	
	return 0;
}

BUILDIN_FUNC(npcstand)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	
	if( nd && mobavaildb_checkid(nd->class_) ) {
		nd->sitted = 0;
		clif_standing(*nd);
	}

	return 0;
}

/*BUILDIN_FUNC(addcloneskill)
{
    int skill_id,skill_lv, lv;
    map_session_data* sd;
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
}*/

BUILDIN_FUNC(getalliance)
{
	int32 j = 0;
	auto g = guild_search(script_getnum(st, 2));

	if( !g || g && guild_get_alliance_count(g->guild, 0) == 0 )
		return 0;
		
	for(int i = 0; i < MAX_GUILDALLIANCE; i++) {
		if(	g->guild.alliance[i].guild_id > 0 && g->guild.alliance[i].opposition == 0 )
			mapreg_setreg(add_str("$@alliance_id")+(j << 24), g->guild.alliance[i].guild_id);
		j++;
	}
	mapreg_setreg(add_str("$@alliance_count"), j);
	return 0;
}

BUILDIN_FUNC(getpartnername)
{
    TBL_PC *sd = nullptr;
	
	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	TBL_PC *p_sd = map_charid2sd(sd->status.partner_id);

	if (p_sd == nullptr) {
		script_pushconststr(st, "null");
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushstrcopy(st, p_sd->status.name);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Works like /WHO - displays all online users
 *------------------------------------------*/
BUILDIN_FUNC(getusersnbr)
{
	TBL_PC *sd;
	int32 count = 0;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		count++;
	
	mapit_free(iter);
	
	script_pushint( st, count );
	
	return SCRIPT_CMD_SUCCESS;
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
/*==========================================
 * getcartlist ({<char_id>});
 * @cartlist_id, @cartlist_amount, @cartlist_equip,
 * @cartlist_refine, @cartlist_identify, @cartlist_attribute,
 * @cartlist_card(0..3), @cartlist_expire
 * @cartlist_count = scalar
 *------------------------------------------*/
BUILDIN_FUNC(getcartlist)
{
	TBL_PC* sd;
	char card_var[NAME_LENGTH], randopt_var[50];
	int32 i, j = 0, k;

	if (!script_charid2sd(2, sd))
		return SCRIPT_CMD_FAILURE;
	for (i = 0; i < MAX_CART; i++) {
		if (sd->cart.u.items_cart[i].nameid > 0 && sd->cart.u.items_cart[i].amount > 0) {
			pc_setreg(sd, reference_uid(add_str("@cartlist_id"), j), sd->cart.u.items_cart[i].nameid);
			pc_setreg(sd, reference_uid(add_str("@cartlist_idx"), j), i);
			pc_setreg(sd, reference_uid(add_str("@cartlist_amount"), j), sd->cart.u.items_cart[i].amount);
			pc_setreg(sd, reference_uid(add_str("@cartlist_equip"), j), sd->cart.u.items_cart[i].equip);
			pc_setreg(sd, reference_uid(add_str("@cartlist_refine"), j), sd->cart.u.items_cart[i].refine);
			pc_setreg(sd, reference_uid(add_str("@cartlist_identify"), j), sd->cart.u.items_cart[i].identify);
			pc_setreg(sd, reference_uid(add_str("@cartlist_attribute"), j), sd->cart.u.items_cart[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++)
			{
				sprintf(card_var, "@cartlist_card%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(card_var), j), sd->cart.u.items_cart[i].card[k]);
			}
			for (k = 0; k < MAX_ITEM_RDM_OPT; k++)
			{
				sprintf(randopt_var, "@cartlist_option_id%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), sd->cart.u.items_cart[i].option[k].id);
				sprintf(randopt_var, "@cartlist_option_value%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), sd->cart.u.items_cart[i].option[k].value);
				sprintf(randopt_var, "@cartlist_option_parameter%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), sd->cart.u.items_cart[i].option[k].param);
			}
			j++;
		}
	}
	pc_setreg(sd, add_str("@cartlist_count"), j);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getstoragelist(storageid)
 * @storagelist_id, @storagelist_amount, @storagelist_equip,
 * @storagelist_refine, @storagelist_identify, @storagelist_attribute,
 * @storagelist_card(0..3), @storagelist_expire
 * @storagelist_count = scalar
 *------------------------------------------*/
BUILDIN_FUNC(getstoragelist)
{
	TBL_PC* sd;
	char card_var[NAME_LENGTH], randopt_var[50];
	int32 i, j = 0, k, stor_id = 0;
	struct item* items;

	if (!script_rid2sd(sd)) {
		ShowError("buildin_getstoragelist: Player not attached. \n");
		return SCRIPT_CMD_FAILURE;
	}

	if (!script_hasdata(st, 2) || !script_isint(st, 2))
		return SCRIPT_CMD_FAILURE;

	stor_id = cap_value(script_getnum(st, 2), 0, 6);

	if (!storage_exists(stor_id)) {
		ShowError("buildin_getstoragelist: Invalid storage_id '%d'!\n", stor_id);
		return SCRIPT_CMD_FAILURE;
	}

	if (stor_id == 0)
		items = sd->storage.u.items_storage;
	else
		items = sd->premiumStorage.u.items_storage;

	for (i = 0; i < MAX_STORAGE; i++) {
		if (items[i].nameid > 0 && items[i].amount > 0) {
			pc_setreg(sd, reference_uid(add_str("@storagelist_id"), j), items[i].nameid);
			pc_setreg(sd, reference_uid(add_str("@storagelist_idx"), j), i);
			pc_setreg(sd, reference_uid(add_str("@storagelist_amount"), j), items[i].amount);
			pc_setreg(sd, reference_uid(add_str("@storagelist_equip"), j), items[i].equip);
			pc_setreg(sd, reference_uid(add_str("@storagelist_refine"), j), items[i].refine);
			pc_setreg(sd, reference_uid(add_str("@storagelist_attribute"), j), items[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++)
			{
				sprintf(card_var, "@storagelist_card%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(card_var), j), items[i].card[k]);
			}
			for (k = 0; k < MAX_ITEM_RDM_OPT; k++)
			{
				sprintf(randopt_var, "@storagelist_option_id%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), items[i].option[k].id);
				sprintf(randopt_var, "@storagelist_option_value%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), items[i].option[k].value);
				sprintf(randopt_var, "@storagelist_option_parameter%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), items[i].option[k].param);
			}
			j++;
		}
	}
	pc_setreg(sd, add_str("@storagelist_count"), j);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getguildstoragelist()
 * @guildstoragelist_id, @guildstoragelist_amount, @guildstoragelist_equip,
 * @guildstoragelist_refine, @guildstoragelist_identify, @guildstoragelist_attribute,
 * @guildstoragelist_card(0..3), @guildstoragelist_expire
 * @guildstoragelist_count = scalar
 *------------------------------------------*/
BUILDIN_FUNC(getguildstoragelist)
{
	TBL_PC* sd;
	char card_var[NAME_LENGTH], randopt_var[50];
	int32 i, j = 0, k;
	struct s_storage* gstor;
	if (!script_rid2sd(sd)) {
		ShowError("buildin_getguildstoragelist: Player not attached. \n");
		return SCRIPT_CMD_FAILURE;
	}

	if (!(gstor = guild2storage(sd->status.guild_id))) {
		ShowError("buildin_getguildstoragelist: No Guild !\n");
		return SCRIPT_CMD_FAILURE;
	}

	for (i = 0; i < MAX_STORAGE; i++) {
		if (gstor->u.items_guild[i].nameid > 0 && gstor->u.items_guild[i].amount > 0) {
			pc_setreg(sd, reference_uid(add_str("@guildstoragelist_id"), j), gstor->u.items_guild[i].nameid);
			pc_setreg(sd, reference_uid(add_str("@guildstoragelist_idx"), j), i);
			pc_setreg(sd, reference_uid(add_str("@guildstoragelist_amount"), j), gstor->u.items_guild[i].amount);
			pc_setreg(sd, reference_uid(add_str("@guildstoragelist_equip"), j), gstor->u.items_guild[i].equip);
			pc_setreg(sd, reference_uid(add_str("@guildstoragelist_refine"), j), gstor->u.items_guild[i].refine);
			pc_setreg(sd, reference_uid(add_str("@guildstoragelist_attribute"), j), gstor->u.items_guild[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++)
			{
				sprintf(card_var, "@guildstoragelist_card%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(card_var), j), gstor->u.items_guild[i].card[k]);
			}
			for (k = 0; k < MAX_ITEM_RDM_OPT; k++)
			{
				sprintf(randopt_var, "@guildstoragelist_option_id%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), gstor->u.items_guild[i].option[k].id);
				sprintf(randopt_var, "@guildstoragelist_option_value%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), gstor->u.items_guild[i].option[k].value);
				sprintf(randopt_var, "@guildstoragelist_option_parameter%d", k + 1);
				pc_setreg(sd, reference_uid(add_str(randopt_var), j), gstor->u.items_guild[i].option[k].param);
			}
			j++;
		}
	}
	pc_setreg(sd, add_str("@guildstoragelist_count"), j);
	return SCRIPT_CMD_SUCCESS;
}

/**
* Load specified storage, use it before using storagecountitem with storage id > 0
 */
BUILDIN_FUNC(loadstorage)
{
	TBL_PC* sd;
	int32 stor_id;
	char* command = (char*)script_getfuncname(st);

	if (!script_rid2sd(sd)) {
		ShowError("buildin_loadpremiumstorage: Player not attached. \n");
		return SCRIPT_CMD_FAILURE;
	}

	if (!script_hasdata(st, 2) || !script_isint(st, 2))
		return SCRIPT_CMD_FAILURE;

	stor_id = cap_value((script_getnum(st, 2) - 1), 0, 6);

	if (sd->premiumStorage.stor_id == stor_id) // Storage already loaded
		return SCRIPT_CMD_SUCCESS;

	if (!storage_exists(stor_id)) {
		ShowError("buildin_loadpremiumstorage: Invalid storage_id '%d'!\n", stor_id);
		return SCRIPT_CMD_FAILURE;
	}

	intif_storage_request(sd, TABLE_STORAGE, stor_id, STOR_MODE_NONE);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Returns number of items in storage
 * storagecountitem(<nameID>{,<accountID>{,<storageID>}})
 * storagecountitem2(<nameID>,<Identified>,<Refine>,<Attribute>,<Card0>,<Card1>,<Card2>,<Card3>{,<accountID>{,<storageID>}})
 */
BUILDIN_FUNC(storagecountitem)
{
	TBL_PC *sd;
	char *command = (char *)script_getfuncname(st);
	int32 aid = 3, stor_id = 4;
	int32 expanded = 0, count = 0;

	if (command[strlen(command) - 1] == '2') {
		expanded = 1;
		aid = 10;
		stor_id = 11;
	}

	if (!script_accid2sd(aid, sd))
		return SCRIPT_CMD_FAILURE;

	std::shared_ptr<item_data> id;

	if (script_isstring(st, 2)) // item name
		id = item_db.searchname( script_getstr( st, 2 ) );
	else // item id
		id = item_db.find( script_getnum( st, 2 ) );

	if (script_hasdata(st, stor_id) && script_isint(st, stor_id))
		stor_id = script_getnum(st, stor_id);
	else
		stor_id = 0;

	if (!storage_exists(stor_id)) {
		ShowError("buildin_storagecountitem: Invalid storage_id '%d'!\n", stor_id);
		return SCRIPT_CMD_FAILURE;
	}

	if (!id) {
		ShowError("buildin_%s: Invalid item '%s'.\n", command, script_getstr(st, 2)); // returns string, regardless of what it was
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if (sd->state.storage_flag != 0) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	if (stor_id == 0)
		count = script_countitem_sub(sd->storage.u.items_storage, id, MAX_STORAGE, expanded, st);
	else
		count = script_countitem_sub(sd->premiumStorage.u.items_storage, id, MAX_STORAGE, expanded, st);

	if (count < 0) {
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}
	script_pushint(st, count);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Creates the instance
 * Returns Instance ID if created successfully
 *------------------------------------------*/
BUILDIN_FUNC(instance_create)
{
	e_instance_mode mode = IM_PARTY;
	int32 owner_id = 0;
	int32 hardmode = 1;
	int32 turbo = 0;

	if (script_hasdata(st, 3)) {
		mode = static_cast<e_instance_mode>(script_getnum(st, 3));

		if (mode < IM_NONE || mode >= IM_MAX) {
			ShowError("buildin_instance_create: Unknown instance mode %d for '%s'\n", mode, script_getstr(st, 2));
			return SCRIPT_CMD_FAILURE;
		}
	}
	if (script_hasdata(st, 4))
		owner_id = script_getnum(st, 4);
	else {
		// If sd is NULL, instance_create will return -2.
		map_session_data *sd = nullptr;

		switch(mode) {
			case IM_NONE:
				owner_id = st->oid;
				break;
			case IM_CHAR:
				if (script_rid2sd(sd))
					owner_id = sd->status.char_id;
				break;
			case IM_PARTY:
				if (script_rid2sd(sd))
					owner_id = sd->status.party_id;
				break;
			case IM_GUILD:
				if (script_rid2sd(sd))
					owner_id = sd->status.guild_id;
				break;
			case IM_CLAN:
				if (script_rid2sd(sd))
					owner_id = sd->status.clan_id;
				break;
			default:
				ShowError("buildin_instance_create: Invalid instance mode (instance name: %s)\n", script_getstr(st, 2));
				return SCRIPT_CMD_FAILURE;
		}
	}
	if (script_hasdata(st, 5)) {
		hardmode = script_getnum(st, 5);
		if (script_hasdata(st, 6))
			turbo = script_getnum(st, 6);
	}

	script_pushint(st, instance_create(owner_id, script_getstr(st, 2), mode, hardmode, turbo));
	return SCRIPT_CMD_SUCCESS;
}
