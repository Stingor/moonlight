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


/**
 * Returns the percentage of EXP (base or job) a player would receive from killing a monster.
 * getmonsterexprate(<mob_id>, <type>{, "<player_name>"})
 * <type>: 0 = base exp %, 1 = job exp %
 * Return value is in 1/100th of a percent (e.g. 1234 = 12.34%)
 */
BUILDIN_FUNC(getmonsterexprate)
{
	map_session_data *sd = nullptr;

	if (script_hasdata(st, 4)) {
		if (!script_nick2sd(4, sd)) {
			script_pushint(st, -1);
			return SCRIPT_CMD_SUCCESS;
		}
	} else {
		script_rid2sd(sd);
	}

	if (sd == nullptr) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	uint16 mob_id = script_getnum(st, 2);
	int32 type	 = script_getnum(st, 3);

	if (type != 0 && type != 1) {
		script_pushstrcopy(st, "");
		return SCRIPT_CMD_SUCCESS;
	}

	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);
	if (mob == nullptr) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	t_exp base_exp = mob->base_exp;
	t_exp job_exp  = mob->job_exp;

	if (sd->status.base_level > 200 && base_exp > 0) { // [Stingor]
		base_exp = (t_exp)((float)base_exp / exp(((float)(sd->status.base_level) - 200.0f) / 250.0f));
		if (sd->status.class_ == JOB_TAEKWON)
			base_exp /= 2;
	}

	pc_calcexp(sd, &base_exp, &job_exp, sd);

	double base_pct = 0, job_pct = 0;
	t_exp base_exp2 = base_exp;
	t_exp nextbase = pc_nextbaseexp(sd);
	t_exp nextjob = pc_nextjobexp(sd);

	if (nextbase)
		base_pct = (double)base_exp2 * 100.0 / nextbase;
	if (nextjob)
		job_pct = (double)job_exp * 100.0 / nextjob;

		double chosen_pct  = (type == 1) ? job_pct  : base_pct;
		t_exp next_exp     = (type == 1) ? nextjob : nextbase;

		if ((type == 1 && next_exp == MAX_LEVEL_JOB_EXP) ||
				(type == 0 && next_exp == MAX_LEVEL_BASE_EXP)) {
				script_pushstrcopy(st, "");
				return SCRIPT_CMD_SUCCESS;
		}
		if (chosen_pct > 100.0)
				chosen_pct = 100.0;
		char expstr[10];
		snprintf(expstr, sizeof(expstr), "%.2f%%", chosen_pct);
		if( strcmp(expstr, "0.00%") == 0 )
				script_pushstrcopy(st, "");
		else
				script_pushstrcopy(st, expstr);
		return SCRIPT_CMD_SUCCESS;
}

/* getskillname (skill_id)
 * Pushes the name of the given skill id as a string, or empty string if unknown.
 */
BUILDIN_FUNC(getmapspawns)
{
	if (script_isstring(st, 2)) {
		const char *mapname = script_getstr(st, 2);
		int32 m = mapindex_name2idx(mapname, nullptr);
		int32 i = 0, total = 0;
		if( m < 0 ) {
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}
		std::vector<map_spawn_info> spawns = map_get_spawns(m);
		for (const auto& spawn : spawns) {
			setd_sub_num( st, nullptr, ".@mobids", i, spawn.mob_id, nullptr );
			setd_sub_num( st, nullptr, ".@mobnums", i, spawn.qty, nullptr );
			total += spawn.qty;
			i++;
		}
		setd_sub_num( st, nullptr, ".@mobtotal", 0, total, nullptr );
	}
	return SCRIPT_CMD_SUCCESS;
}

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

/**
 * sethomskinn(<class_id>) — change le skin visuel de l'homonculus sans affecter les stats.
 * class_id == 0 → remet le sprite d'origine.
 * Retourne 1 si succès, 0 si class_id invalide.
 * Modifie aussi le nom du hom : ajoute " (NomSkin)" ou retire le suffixe.
 */
BUILDIN_FUNC(sethomskinn)
{
	map_session_data* sd;
	if( !script_rid2sd(sd) ) return SCRIPT_CMD_FAILURE;

	int32 class_id = script_getnum(st, 2);

	// Correspondance class_id → nom affiché
	auto get_skin_name = [](int32 id) -> const char* {
		switch(id) {
			case MER_LIF:          return "Lif";
			case MER_AMISTR:       return "Amistr";
			case MER_FILIR:        return "Filir";
			case MER_VANILMIRTH:   return "Vanilmirth";
			case MER_LIF2:         return "Lif 2";
			case MER_AMISTR2:      return "Amistr 2";
			case MER_FILIR2:       return "Filir 2";
			case MER_VANILMIRTH2:  return "Vanilmirth 2";
			case MER_LIF_H:        return "Lif E";
			case MER_AMISTR_H:     return "Amistr E";
			case MER_FILIR_H:      return "Filir E";
			case MER_VANILMIRTH_H: return "Vanilmirth E";
			case MER_LIF_H2:       return "Lif 2 E";
			case MER_AMISTR_H2:    return "Amistr 2 E";
			case MER_FILIR_H2:     return "Filir 2 E";
			case MER_VANILMIRTH_H2:return "Vanilm. 2 E";
			case MER_EIRA:         return "Eira";
			case MER_BAYERI:       return "Bayeri";
			case MER_SERA:         return "Sera";
			case MER_DIETER:       return "Dieter";
			case MER_ELEANOR:      return "Eleanor";
			default:               return nullptr;
		}
	};
	const char* skin_name = get_skin_name(class_id);

	// Retire le suffixe " (xxx)" existant du nom, retourne le nom de base
	auto strip_skin_suffix = [](char* name) {
		char* p = strrchr(name, '(');
		if( p && p > name && *(p - 1) == ' ' )
			*(p - 1) = '\0';
	};

	if( class_id == 0 ) {
		pc_setglobalreg(sd, add_str("homskin"), 0);
		if( sd->hd ) {
			status_set_viewdata(sd->hd, sd->hd->homunculus.class_);
			strip_skin_suffix(sd->hd->homunculus.name);
			unit_refresh(sd->hd);
		}
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	// Valider via hom_get_viewdata : accepte toutes les formes (base, évoluées, Hom S)
	// même si elles n'ont pas d'entrée complète dans homunculus_db.
	if( !hom_get_viewdata(class_id) ) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	pc_setglobalreg(sd, add_str("homskin"), class_id);
	if( sd->hd ) {
		status_set_viewdata(sd->hd, class_id);
		if( skin_name ) {
			// Retire l'ancien suffixe puis ajoute le nouveau
			strip_skin_suffix(sd->hd->homunculus.name);
			char base[NAME_LENGTH];
			safestrncpy(base, sd->hd->homunculus.name, NAME_LENGTH);
			snprintf(sd->hd->homunculus.name, NAME_LENGTH, "%s (%s)", base, skin_name);
		}
		unit_refresh(sd->hd);
	}
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(npcsit)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

	if( nd && nd->vd.dead_sit == 0 ) {
		nd->vd.dead_sit = 2;
		// Le client RO ignore ZC_NOTIFY_ACT(DMG_SIT_DOWN) pour les NPCs.
		// unit_refresh envoie clear + clif_set_unit_idle (inclut state=2 depuis
		// le fix clif_spawn), ce qui est le seul moyen fiable de mettre à jour
		// l'apparence assise pour tous les joueurs en vue.
		unit_refresh(nd);
	}
	return 0;
}

BUILDIN_FUNC(npcstand)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

	if( nd && nd->vd.dead_sit == 2 ) {
		nd->vd.dead_sit = 0;
		unit_refresh(nd);
	}
	return 0;
}

/* npcwalktothensit <x>,<y>
 * Marche vers (x,y) et s'assoit automatiquement a l'arrivee. [Stingor]
 */
BUILDIN_FUNC(npcwalktothensit)
{
	struct npc_data *nd = (struct npc_data *)map_id2bl(st->oid);
	int x = script_getnum(st, 2);
	int y = script_getnum(st, 3);

	if (!nd) return SCRIPT_CMD_FAILURE;
	if (nd->m < 0) return SCRIPT_CMD_FAILURE;

	if (!nd->status.hp)
		status_calc_npc(nd, SCO_FIRST);
	else
		status_calc_npc(nd, SCO_NONE);

	// Se lever d'abord si assis
	if (nd->vd.dead_sit == 2) {
		nd->vd.dead_sit = 0;
		unit_refresh(nd);
	}

	nd->ud.state.sit_on_arrive = 1;
	unit_walktoxy(nd, (short)x, (short)y, 0);

	return SCRIPT_CMD_SUCCESS;
}

/* logchat "<message>";
 * Insère un message dans la chatlog avec src_charid = nom du NPC appelant.
 * Identique à logmes mais cible la table chatlog (LOG_CHAT_GLOBAL). [Stingor]
 */
BUILDIN_FUNC(logchat)
{
	const char* msg = script_getstr(st, 2);
	npc_data* nd = (npc_data*)map_id2bl(st->oid);

	if (!nd) {
		ShowError("buildin_logchat: pas de NPC source.\n");
		return SCRIPT_CMD_FAILURE;
	}

	const char* mapname = map_getmapdata(nd->m) ? map_getmapdata(nd->m)->name : "?";
	log_chat(LOG_CHAT_GLOBAL, 0, nd->name, 0, mapname, nd->x, nd->y, "", msg);

	return SCRIPT_CMD_SUCCESS;
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

/* getmapdisplayname("<map_name>")
 * Returns the display name of a map as defined in the Map Index database.
 * Falls back to the internal map name if no display name is set.
 * Returns an empty string if the map is not found.
 */
BUILDIN_FUNC(getmapdisplayname)
{
	const char* map_name = script_getstr(st, 2);
	uint16 idx = mapindex_name2idx(map_name, nullptr);
	if (!idx) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_SUCCESS;
	}
	const char* display = mapindex_idx2displayname(idx);
	script_pushconststr(st, display ? display : "");
	return SCRIPT_CMD_SUCCESS;
}

/* isinitemgroup(<item_id>, <group_id>)
 * Returns 1 if the given item ID is present in any sub-group of the item group,
 * 0 if not found, -1 if the group does not exist.
 */
BUILDIN_FUNC(isinitemgroup)
{
	t_itemid nameid  = (t_itemid)script_getnum(st, 2);
	uint16   group_id = (uint16)script_getnum(st, 3);

	std::shared_ptr<s_item_group_db> group = itemdb_group.find(group_id);
	if (!group) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	for (const auto& random_pair : group->random) {
		for (const auto& entry_pair : random_pair.second->data) {
			if (entry_pair.second->nameid == nameid) {
				script_pushint(st, entry_pair.second->rate);
				return SCRIPT_CMD_SUCCESS;
			}
		}
	}

	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}

/* itemlinkuid("<unique_id>"{, <char_id>})
 * Returns an itemlink string for the inventory item matching the given unique_id.
 * Encodes all instance data (refine, cards, enchantgrade, options) like itemlink().
 * Returns "" if not found.
 */
BUILDIN_FUNC(itemlinkuid)
{
	TBL_PC *sd;
	if (!script_accid2sd(3, sd)) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_itemlinkuid: invalid unique_id \"%s\"\n", uid_str);
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0 || it->unique_id != uid)
			continue;

		std::string link = item_db.create_item_link(*it);
		script_pushstrcopy(st, link.c_str());
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushconststr(st, "");
	return SCRIPT_CMD_SUCCESS;
}

/* getuidlist("<unique_id>"{, <char_id>})
 * Finds an item in the player's inventory by its unique_id (as string).
 * Populates @itembyuid_* scalar variables with all fields of the item.
 * Returns 1 if found, 0 if not found.
 */
BUILDIN_FUNC(getuidlist)
{
	TBL_PC *sd;
	if (!script_accid2sd(3, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_getuidlist: invalid unique_id \"%s\"\n", uid_str);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0 || it->unique_id != uid)
			continue;

		setd_sub_num(st, nullptr, ".@itembyuid_nameid", 0, it->nameid, nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_idx",    0, i,           nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_amount", 0, it->amount,  nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_equip",  0, it->equip,   nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_refine", 0, it->refine,  nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_identify",    0, it->identify,    nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_attribute",   0, it->attribute,   nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_expire",      0, it->expire_time, nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_bound",       0, it->bound,       nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_enchantgrade",0, it->enchantgrade,nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_tradable",    0, pc_can_trade_item(sd, i), nullptr);
		setd_sub_num(st, nullptr, ".@itembyuid_favorite",    0, it->favorite,    nullptr);
		for (int32 k = 0; k < MAX_SLOTS; k++) {
			char var[NAME_LENGTH];
			sprintf(var, ".@itembyuid_card%d", k + 1);
			setd_sub_num(st, nullptr, var, 0, it->card[k], nullptr);
		}
		for (int32 k = 0; k < MAX_ITEM_RDM_OPT; k++) {
			char var[50];
			sprintf(var, ".@itembyuid_option_id%d", k + 1);
			setd_sub_num(st, nullptr, var, 0, it->option[k].id, nullptr);
			sprintf(var, ".@itembyuid_option_value%d", k + 1);
			setd_sub_num(st, nullptr, var, 0, it->option[k].value, nullptr);
			sprintf(var, ".@itembyuid_option_parameter%d", k + 1);
			setd_sub_num(st, nullptr, var, 0, it->option[k].param, nullptr);
		}

		// Static DB fields
		std::shared_ptr<item_data> id = item_db.find(it->nameid);
		if (id) {
			setd_sub_num(st, nullptr, ".@itembyuid_type",        0, id->type,         nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_subtype",     0, id->subtype,       nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_position",    0, id->equip,         nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_slot",        0, id->slots,         nullptr);
			if (id->type == IT_WEAPON)
				setd_sub_num(st, nullptr, ".@itembyuid_weaponlevel", 0, id->weapon_level, nullptr);
			if (id->type == IT_ARMOR)
				setd_sub_num(st, nullptr, ".@itembyuid_armorlevel",  0, id->armor_level,  nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_equiplvlmin", 0, id->elv,           nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_equiplvlmax", 0, id->elvmax,        nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_weight",      0, id->weight,        nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_atk",         0, id->atk,           nullptr);
			setd_sub_num(st, nullptr, ".@itembyuid_def",         0, id->def,           nullptr);
#ifdef RENEWAL
			setd_sub_num(st, nullptr, ".@itembyuid_matk",        0, id->matk,          nullptr);
#endif
		}

		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}

/* arraypush <array>, <value1>{, <value2>...}
 * Appends one or more values at the end of an array.
 */
BUILDIN_FUNC(arraypush)
{
	struct script_data *data = script_getdata(st, 2);
	if (!data_isreference(data)) {
		ShowError("script:arraypush: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	const char* name = reference_getname(data);
	struct reg_db *ref = reference_getref(data);
	int32 id = reference_getid(data);
	map_session_data* sd = nullptr;

	if (not_server_variable(*name) && !script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	uint32 idx = script_array_highest_key(st, sd, name, ref);

	if (is_string_variable(name)) {
		for (int32 i = 3; i <= script_lastdata(st) && idx < SCRIPT_MAX_ARRAYSIZE; i++, idx++)
			set_reg_str(st, sd, reference_uid(id, idx), name, script_getstr(st, i), ref);
	} else {
		for (int32 i = 3; i <= script_lastdata(st) && idx < SCRIPT_MAX_ARRAYSIZE; i++, idx++)
			set_reg_num(st, sd, reference_uid(id, idx), name, script_getnum64(st, i), ref);
	}
	return SCRIPT_CMD_SUCCESS;
}

/* arraypop <array>
 * Removes the last element of an array and returns its value.
 * Returns 0 or "" if the array is empty.
 */
BUILDIN_FUNC(arraypop)
{
	struct script_data *data = script_getdata(st, 2);
	if (!data_isreference(data)) {
		ShowError("script:arraypop: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	const char* name = reference_getname(data);
	struct reg_db *ref = reference_getref(data);
	int32 id = reference_getid(data);
	map_session_data* sd = nullptr;

	if (not_server_variable(*name) && !script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	uint32 size = script_array_highest_key(st, sd, name, ref);
	if (size == 0) {
		if (is_string_variable(name))
			script_pushconststr(st, "");
		else
			script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	uint32 last = size - 1;
	int64 uid = reference_uid(id, last);

	if (is_string_variable(name)) {
		const char* val = get_val2_str(st, uid, ref);
		script_pushstrcopy(st, val);
		script_removetop(st, -2, -1);
		set_reg_str(st, sd, uid, name, "", ref);
	} else {
		int64 val = get_val2_num(st, uid, ref);
		script_removetop(st, -1, 0);
		script_pushint(st, val);
		set_reg_num(st, sd, uid, name, 0, ref);
	}
	return SCRIPT_CMD_SUCCESS;
}

/* getiteminfobyuid("<unique_id>", <type>{, <char_id>})
 * Returns a single field from the inventory item matching the given unique_id.
 * Use INVITEMINFO_* constants for <type>.
 * Returns -1 if the item is not found or the type is invalid.
 */
BUILDIN_FUNC(getiteminfobyuid)
{
	TBL_PC *sd;
	if (!script_accid2sd(4, sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_getiteminfobyuid: invalid unique_id \"%s\"\n", uid_str);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	int32 type = script_getnum(st, 3);

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0 || it->unique_id != uid)
			continue;

		switch ((e_inviteminfo)type) {
			case INVITEMINFO_NAMEID:       script_pushint(st, it->nameid); break;
			case INVITEMINFO_IDX:          script_pushint(st, i); break;
			case INVITEMINFO_AMOUNT:       script_pushint(st, it->amount); break;
			case INVITEMINFO_EQUIP:        script_pushint(st, it->equip); break;
			case INVITEMINFO_REFINE:       script_pushint(st, it->refine); break;
			case INVITEMINFO_IDENTIFY:     script_pushint(st, it->identify); break;
			case INVITEMINFO_ATTRIBUTE:    script_pushint(st, it->attribute); break;
			case INVITEMINFO_EXPIRE:       script_pushint(st, it->expire_time); break;
			case INVITEMINFO_BOUND:        script_pushint(st, it->bound); break;
			case INVITEMINFO_ENCHANTGRADE: script_pushint(st, it->enchantgrade); break;
			case INVITEMINFO_TRADABLE:     script_pushint(st, pc_can_trade_item(sd, i)); break;
			case INVITEMINFO_FAVORITE:     script_pushint(st, it->favorite); break;
			case INVITEMINFO_CARD1:        script_pushint(st, it->card[0]); break;
			case INVITEMINFO_CARD2:        script_pushint(st, it->card[1]); break;
			case INVITEMINFO_CARD3:        script_pushint(st, it->card[2]); break;
			case INVITEMINFO_CARD4:        script_pushint(st, it->card[3]); break;
			case INVITEMINFO_OPTION_ID1:    script_pushint(st, it->option[0].id); break;
			case INVITEMINFO_OPTION_VALUE1: script_pushint(st, it->option[0].value); break;
			case INVITEMINFO_OPTION_PARAM1: script_pushint(st, it->option[0].param); break;
			case INVITEMINFO_OPTION_ID2:    script_pushint(st, it->option[1].id); break;
			case INVITEMINFO_OPTION_VALUE2: script_pushint(st, it->option[1].value); break;
			case INVITEMINFO_OPTION_PARAM2: script_pushint(st, it->option[1].param); break;
			case INVITEMINFO_OPTION_ID3:    script_pushint(st, it->option[2].id); break;
			case INVITEMINFO_OPTION_VALUE3: script_pushint(st, it->option[2].value); break;
			case INVITEMINFO_OPTION_PARAM3: script_pushint(st, it->option[2].param); break;
			case INVITEMINFO_OPTION_ID4:    script_pushint(st, it->option[3].id); break;
			case INVITEMINFO_OPTION_VALUE4: script_pushint(st, it->option[3].value); break;
			case INVITEMINFO_OPTION_PARAM4: script_pushint(st, it->option[3].param); break;
			case INVITEMINFO_OPTION_ID5:    script_pushint(st, it->option[4].id); break;
			case INVITEMINFO_OPTION_VALUE5: script_pushint(st, it->option[4].value); break;
			case INVITEMINFO_OPTION_PARAM5: script_pushint(st, it->option[4].param); break;
			default:
				ShowError("buildin_getiteminfobyuid: unknown type %d\n", type);
				script_pushint(st, -1);
				return SCRIPT_CMD_FAILURE;
		}
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, -1);
	return SCRIPT_CMD_SUCCESS;
}

/* alteritem("<unique_id>", <INVITEMINFO_*>, <value>{, <char_id>})
 * Modifies a single field of an inventory item identified by its unique_id.
 * Uses INVITEMINFO_* constants to select the field.
 * Returns 1 on success, 0 if uid not found, -1 if field is not modifiable.
 */
BUILDIN_FUNC(alteritem)
{
	TBL_PC *sd;
	if (!script_accid2sd(5, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_alteritem: invalid unique_id \"%s\"\n", uid_str);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	e_inviteminfo type = (e_inviteminfo)script_getnum(st, 3);
	int64 value = script_getnum(st, 4);

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0 || it->unique_id != uid)
			continue;

		uint32 ep = it->equip;

		switch (type) {
			case INVITEMINFO_REFINE:       it->refine       = (char)cap_value(value, 0, MAX_REFINE); break;
			case INVITEMINFO_IDENTIFY:     it->identify     = (char)cap_value(value, 0, 1);          break;
			case INVITEMINFO_ATTRIBUTE:    it->attribute    = (char)cap_value(value, 0, 1);          break;
			case INVITEMINFO_BOUND:        it->bound        = (char)cap_value(value, 0, 4);          break;
			case INVITEMINFO_ENCHANTGRADE: it->enchantgrade = (uint8)cap_value(value, 0, 4);         break;
			case INVITEMINFO_FAVORITE:     it->favorite     = (char)cap_value(value, 0, 1);          break;
			case INVITEMINFO_CARD1:        it->card[0]      = (t_itemid)value; break;
			case INVITEMINFO_CARD2:        it->card[1]      = (t_itemid)value; break;
			case INVITEMINFO_CARD3:        it->card[2]      = (t_itemid)value; break;
			case INVITEMINFO_CARD4:        it->card[3]      = (t_itemid)value; break;
			case INVITEMINFO_OPTION_ID1:    it->option[0].id    = (int16)value; break;
			case INVITEMINFO_OPTION_VALUE1: it->option[0].value = (int16)value; break;
			case INVITEMINFO_OPTION_PARAM1: it->option[0].param = (char)value;  break;
			case INVITEMINFO_OPTION_ID2:    it->option[1].id    = (int16)value; break;
			case INVITEMINFO_OPTION_VALUE2: it->option[1].value = (int16)value; break;
			case INVITEMINFO_OPTION_PARAM2: it->option[1].param = (char)value;  break;
			case INVITEMINFO_OPTION_ID3:    it->option[2].id    = (int16)value; break;
			case INVITEMINFO_OPTION_VALUE3: it->option[2].value = (int16)value; break;
			case INVITEMINFO_OPTION_PARAM3: it->option[2].param = (char)value;  break;
			case INVITEMINFO_OPTION_ID4:    it->option[3].id    = (int16)value; break;
			case INVITEMINFO_OPTION_VALUE4: it->option[3].value = (int16)value; break;
			case INVITEMINFO_OPTION_PARAM4: it->option[3].param = (char)value;  break;
			case INVITEMINFO_OPTION_ID5:    it->option[4].id    = (int16)value; break;
			case INVITEMINFO_OPTION_VALUE5: it->option[4].value = (int16)value; break;
			case INVITEMINFO_OPTION_PARAM5: it->option[4].param = (char)value;  break;
			default:
				script_pushint(st, -1);
				return SCRIPT_CMD_SUCCESS;
		}

		log_pick_pc(sd, LOG_TYPE_SCRIPT, 1, it);
		clif_equiplist(sd);
		if (ep)
			status_calc_pc(sd, SCO_FORCE);

		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}

/* equipuid("<unique_id>"{, <char_id>})
 * Equips an item from the player's inventory identified by its unique_id (as string).
 * Returns 1 on success, 0 if not found, -1 if equip failed.
 */
BUILDIN_FUNC(equipuid)
{
	TBL_PC *sd;
	if (!script_accid2sd(3, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_equipuid: invalid unique_id \"%s\"\n", uid_str);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0 || it->unique_id != uid)
			continue;

		std::shared_ptr<item_data> id = item_db.find(it->nameid);
		if (!id) {
			script_pushint(st, -1);
			return SCRIPT_CMD_SUCCESS;
		}

		script_pushint(st, pc_equipitem(sd, i, id->equip) ? 1 : -1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Sum of the refine levels of every equipped item. [Stingor]
 * Each inventory index is counted at most once, so two-handed weapons
 * (taking both HAND_L and HAND_R) and multi-slot headgears are not
 * double-counted.
 * getallrefine({<char_id>})
 * return: total refine across all equipped items (0 if nothing equipped)
 *------------------------------------------*/
BUILDIN_FUNC(getallrefine)
{
	TBL_PC *sd;

	if (!script_charid2sd(2, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	int32 total = 0;
	int16 seen[EQI_MAX];
	int32 seen_count = 0;

	for (int32 slot = 0; slot < EQI_MAX; slot++) {
		int32 idx = pc_checkequip(sd, equip_bitmask[slot]);
		if (idx < 0)
			continue;

		bool already = false;
		for (int32 k = 0; k < seen_count; k++) {
			if (seen[k] == idx) { already = true; break; }
		}
		if (already)
			continue;

		seen[seen_count++] = (int16)idx;
		total += sd->inventory.u.items_inventory[idx].refine;
	}

	script_pushint(st, total);
	return SCRIPT_CMD_SUCCESS;
}

/* delitembyuid("<unique_id>"{, <char_id>})
 * Deletes an item from the player's inventory identified by its unique_id (as string).
 * Returns nothing. Logs an error if the item is not found.
 */
BUILDIN_FUNC(delitembyuid)
{
	TBL_PC *sd;
	if (!script_accid2sd(3, sd)) {
		st->state = END;
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_delitembyuid: invalid unique_id \"%s\"\n", uid_str);
		return SCRIPT_CMD_FAILURE;
	}

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0)
			continue;
		if (it->unique_id == uid) {
			pc_delitem(sd, i, it->amount, 0, 0, LOG_TYPE_SCRIPT);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	ShowError("buildin_delitembyuid: item with unique_id \"%s\" not found in inventory of player '%s'\n",
		uid_str, sd->status.name);
	return SCRIPT_CMD_FAILURE;
}

/**
 * mobismvp(<mob_id>) -> 1 if BOSSTYPE_MVP, 0 otherwise
 * mobisboss(<mob_id>) -> 1 if BOSSTYPE_MINIBOSS, 0 otherwise
 */
BUILDIN_FUNC(mobismvp)
{
	uint16 mob_id = script_getnum(st, 2);
	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);
	script_pushint(st, mob != nullptr && mob->get_bosstype() == BOSSTYPE_MVP ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mobisboss)
{
	uint16 mob_id = script_getnum(st, 2);
	std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);
	script_pushint(st, mob != nullptr && mob->get_bosstype() == BOSSTYPE_MINIBOSS ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getmvprespawn(<mob_id>{, <spawnx_var>, <spawny_var>, <map_var$>, <killer_var$>, <killtime_var>{, <tombx_var>, <tomby_var>}})
 *   Returns seconds remaining before the MVP respawns, or -1 if not tracked.
 *   Negative values (other than -1) mean the tick is past but mob not yet appeared.
 *   Optional output variables: spawn X, spawn Y, map name, killer name, Unix kill time,
 *   tomb X (where MVP died), tomb Y (where MVP died).
 *
 * Example:
 *   .@secs = getmvprespawn(1511, .@x, .@y, .@map$, .@killer$, .@when, .@tx, .@ty);
 *   if (.@secs < 0) mes "Baphomet est en vie.";
 *   else mes "Respawn dans " + .@secs + "s, tombe en (" + .@tx + "," + .@ty + ") sur " + .@map$;
 */
BUILDIN_FUNC(getmvprespawn)
{
	int32 mob_id = script_getnum(st, 2);
	auto it = mvp_respawn_cache.find((uint16)mob_id);
	if (it == mvp_respawn_cache.end()) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	const s_mvp_respawn_info& info = it->second;
	int64 remsecs = (int64)(DIFF_TICK(info.respawn_tick, gettick())) / 1000;
	script_pushint(st, (int32)remsecs);

	// Fill optional output variables (same pattern as query_sql)
	if (script_hasdata(st, 3)) {
		map_session_data* sd = nullptr;
		struct script_data* data;
		const char* varname;

		auto fill_int = [&](int arg, int64 val) {
			if (!script_hasdata(st, arg)) return;
			data = script_getdata(st, arg);
			if (!data_isreference(data)) return;
			varname = reference_getname(data);
			if (not_server_variable(*varname) && sd == nullptr) script_rid2sd(sd);
			setd_sub_num(st, sd, varname, 0, val, reference_getref(data));
		};
		auto fill_str = [&](int arg, const char* val) {
			if (!script_hasdata(st, arg)) return;
			data = script_getdata(st, arg);
			if (!data_isreference(data)) return;
			varname = reference_getname(data);
			if (not_server_variable(*varname) && sd == nullptr) script_rid2sd(sd);
			setd_sub_str(st, sd, varname, 0, val, reference_getref(data));
		};

		fill_int(3, info.spawn_x);
		fill_int(4, info.spawn_y);
		fill_str(5, info.mapid >= 0 ? map[info.mapid].name : "");
		fill_str(6, info.killer_name);
		fill_int(7, info.kill_time);
		fill_int(8, info.tomb_x);
		fill_int(9, info.tomb_y);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getallmvprespawn(<mobid_var>, <remsecs_var>, <spawnx_var>, <spawny_var>,
 *                  <map_var$>, <killer_var$>, <killtime_var>)
 *   Returns number of tracked dead MVPs and fills arrays with respawn info.
 *   Variable scope follows the query_sql convention (.@ local, @ account-temp, etc.).
 *
 * Example:
 *   .@n = getallmvprespawn(.@id, .@secs, .@x, .@y, .@map$, .@killer$, .@when);
 *   for (.@i = 0; .@i < .@n; .@i++) {
 *       mes getmonsterinfo(.@id[.@i], MOB_NAME) + " : dans " + .@secs[.@i] + "s";
 *       mes "  " + .@map$[.@i] + " (" + .@x[.@i] + "," + .@y[.@i] + ") tue par " + .@killer$[.@i];
 *   }
 */
BUILDIN_FUNC(getallmvprespawn)
{
	// Validate all 7 output variable arguments
	map_session_data* sd = nullptr;
	for (int arg = 2; arg <= 8; arg++) {
		struct script_data* data = script_getdata(st, arg);
		if (!data_isreference(data)) {
			ShowError("buildin_getallmvprespawn: argument %d is not a variable\n", arg - 1);
			script_reportdata(data);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
		const char* name = reference_getname(data);
		if (not_server_variable(*name) && sd == nullptr) {
			if (!script_rid2sd(sd)) {
				ShowError("buildin_getallmvprespawn: player variable '%s' requires attached player\n", name);
				st->state = END;
				return SCRIPT_CMD_FAILURE;
			}
		}
	}

	t_tick now = gettick();
	int32 count = 0;

	for (const auto& pair : mvp_respawn_cache) {
		const s_mvp_respawn_info& info = pair.second;
		int64 remsecs = (int64)(DIFF_TICK(info.respawn_tick, now)) / 1000;
		const char* mapname = (info.mapid >= 0) ? map[info.mapid].name : "";

		auto set_i = [&](int arg, int64 val) {
			struct script_data* data = script_getdata(st, arg);
			setd_sub_num(st, sd, reference_getname(data), count, val, reference_getref(data));
		};
		auto set_s = [&](int arg, const char* val) {
			struct script_data* data = script_getdata(st, arg);
			setd_sub_str(st, sd, reference_getname(data), count, val, reference_getref(data));
		};

		set_i(2, info.mob_id);
		set_i(3, remsecs);
		set_i(4, info.spawn_x);
		set_i(5, info.spawn_y);
		set_s(6, mapname);
		set_s(7, info.killer_name);
		set_i(8, info.kill_time);
		count++;
	}

	script_pushint(st, count);
	return SCRIPT_CMD_SUCCESS;
}

/* getitemtype(<item_id|"item_name">) -> item type (IT_*), or -1 if unknown.
 * Raccourci direct pour getiteminfo(<id>, ITEMINFO_TYPE).
 * Accepte un ID numérique ou un nom (AegisName).
 */
BUILDIN_FUNC(getitemtype)
{
	std::shared_ptr<item_data> i_data;

	if (script_isstring(st, 2))
		i_data = item_db.searchname(script_getstr(st, 2));
	else
		i_data = item_db.find(script_getnum(st, 2));

	script_pushint(st, i_data != nullptr ? (int32)i_data->type : -1);
	return SCRIPT_CMD_SUCCESS;
}

/* addtempnpc "map", x, y, sprite, "name"{, dir} -> gid (0 = failure)
 * Spawns at a random walkable cell 3-4 cells away from (x,y), not on the exact spot.
 * dir: 0=N 1=NW 2=W 3=SW 4=S(default) 5=SE 6=E 7=NE */
BUILDIN_FUNC(addtempnpc)
{
	const char* mapname   = script_getstr(st, 2);
	int16 x               = (int16)script_getnum(st, 3);
	int16 y               = (int16)script_getnum(st, 4);
	short sprite          = (short)script_getnum(st, 5);
	const char* disp_name = script_getstr(st, 6);
	uint8 dir             = script_hasdata(st, 7) ? (uint8)script_getnum(st, 7) : (uint8)DIR_SOUTH;

	int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	// Find a walkable cell offset 3-4 cells away (avoid spawning on tomb/mob position)
	struct map_data* mapdata = map_getmapdata(m);
	int16 fx = x, fy = y;
	for (int attempt = 0; attempt < 30; attempt++) {
		int dx = (rand() % 9) - 4;  // -4 to +4
		int dy = (rand() % 9) - 4;
		if (std::abs(dx) + std::abs(dy) < 3)  // minimum Manhattan distance of 3
			continue;
		int16 nx = x + (int16)dx;
		int16 ny = y + (int16)dy;
		if (nx < 0 || ny < 0 || nx >= mapdata->xs || ny >= mapdata->ys)
			continue;
		if (map_getcell(m, nx, ny, CELL_CHKPASS)) {
			fx = nx;
			fy = ny;
			break;
		}
	}

	int gid = npc_create_temp_npc(m, fx, fy, sprite, disp_name, dir);
	script_pushint(st, gid);
	return SCRIPT_CMD_SUCCESS;
}

/* removetempnpc gid -> 1 on success, 0 if not found */
BUILDIN_FUNC(removetempnpc)
{
	int gid = script_getnum(st, 2);
	block_list* bl = map_id2bl(gid);
	if (!bl || bl->type != BL_NPC) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	npc_unload((npc_data*)bl, true);
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/* getmobxy(mob_id, "map", .@x, .@y) -> 1 if a live mob of that id is on the map (x,y set), else 0
 * Matches on the real md->mob_id (not the view sprite), reads the actual current coordinates. */
BUILDIN_FUNC(getmobxy)
{
	int32 mob_id        = script_getnum(st, 2);
	const char* mapname = script_getstr(st, 3);
	int16 m = map_mapname2mapid(mapname);
	if (m < 0) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	int16 fx = 0, fy = 0;
	bool found = false;
	struct s_mapiterator* iter = mapit_geteachmob();
	for (block_list* bl = (block_list*)mapit_first(iter); mapit_exists(iter); bl = (block_list*)mapit_next(iter)) {
		if (bl->m != m)
			continue;
		mob_data* md = (mob_data*)bl;
		if (md->mob_id != mob_id)
			continue;
		fx = md->x;
		fy = md->y;
		found = true;
		break;
	}
	mapit_free(iter);

	// Fill output vars .@x (arg 4) and .@y (arg 5)
	map_session_data* sd = nullptr;
	struct script_data* data;
	const char* varname;
	auto fill_int = [&](int arg, int64 val) {
		if (!script_hasdata(st, arg)) return;
		data = script_getdata(st, arg);
		if (!data_isreference(data)) return;
		varname = reference_getname(data);
		if (not_server_variable(*varname) && sd == nullptr) script_rid2sd(sd);
		setd_sub_num(st, sd, varname, 0, val, reference_getref(data));
	};
	fill_int(4, fx);
	fill_int(5, fy);

	script_pushint(st, found ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}

/* refineitem("<unique_id>", <goal>{, <char_id>})
 * Refine the inventory item identified by its unique_id until it reaches the
 * <goal> refine level. <goal> is capped between 1 and MAX_REFINE.
 * Refine chances and breaking rate come from the refine database (refine_db) —
 * so "safe" levels (100% chance) always succeed, and only the levels above the
 * safe limit can fail / destroy the item. The progression stops as soon as the
 * goal is reached, the item breaks, or a non-breaking failure occurs.
 * Returns:
 *    1 : the item reached the goal refine level (already at it counts too)
 *   -1 : the item was destroyed while refining
 *    0 : the goal was not reached (a failure stopped it before the goal), or
 *        the item could not be refined (not found, no_refine, broken).
 */
BUILDIN_FUNC(refineitem)
{
	TBL_PC *sd;
	if (!script_accid2sd(4, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_refineitem: invalid unique_id \"%s\"\n", uid_str);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	int32 goal = cap_value(script_getnum(st, 3), 1, MAX_REFINE);

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *item = &sd->inventory.u.items_inventory[i];
		if (item->nameid == 0 || item->unique_id != uid)
			continue;

		struct item_data *id = sd->inventory_data[i];

		// Item must exist, be refinable and not broken
		if (id == nullptr || id->flag.no_refine || item->attribute) {
			script_pushint(st, 0);
			return SCRIPT_CMD_SUCCESS;
		}

		// Already at or above the requested refine level -> nothing to do
		if (item->refine >= goal) {
			script_pushint(st, 1);
			return SCRIPT_CMD_SUCCESS;
		}

		// Refine one level at a time until the goal is reached, the item
		// breaks, or a non-breaking failure stops the progression.
		while (item->refine < goal) {
			std::shared_ptr<s_refine_level_info> info = refine_db.findLevelInfo(*id, *item);
			if (info == nullptr || info->costs.empty())
				break;

			std::shared_ptr<s_refine_cost> cost = util::umap_find(info->costs, (uint16)REFINE_COST_NORMAL);
			if (cost == nullptr)
				break;

			log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, item);

			// chance is out of 10000 — safe levels are 10000 (always succeed)
			if (cost->chance >= (rnd() % 10000)) {
				// Success
				item->refine = cap_value(item->refine + 1, 0, MAX_REFINE);
				log_pick_pc(sd, LOG_TYPE_SCRIPT, 1, item);
				if (id->type == IT_WEAPON)
					achievement_update_objective(sd, AG_ENCHANT_SUCCESS, 2, id->weapon_level, item->refine);
			} else {
				// Failure beyond the safe level
				achievement_update_objective(sd, AG_ENCHANT_FAIL, 1, 1);

				// The item breaks and is destroyed — pc_delitem unequips,
				// recalculates the status and refreshes the client by itself.
				if (cost->breaking_rate > 0 && (rnd() % 10000) < cost->breaking_rate) {
					clif_refine(*sd, i, ITEMREFINING_FAILURE);
					clif_delitem(*sd, i, 1, 3);
					pc_delitem(sd, i, 1, 0, 2, LOG_TYPE_SCRIPT);
					clif_misceffect(*sd, NOTIFYEFFECT_REFINE_FAILURE);
					script_pushint(st, -1);
					return SCRIPT_CMD_SUCCESS;
				}

				// Stop on the first failure that did not destroy the item
				break;
			}
		}

		bool reached = (item->refine >= goal);

		// Show the result and refresh the client / equip bonuses
		clif_misceffect(*sd, reached ? NOTIFYEFFECT_REFINE_SUCCESS : NOTIFYEFFECT_REFINE_FAILURE);
		clif_refine(*sd, i, reached ? ITEMREFINING_SUCCESS : ITEMREFINING_FAILURE2);
		clif_delitem(*sd, i, 1, 3);
		clif_additem(sd, i, 1, 0);
		clif_equiplist(sd);
		if (item->equip)
			status_calc_pc(sd, SCO_FORCE);

		script_pushint(st, reached ? 1 : 0);
		return SCRIPT_CMD_SUCCESS;
	}

	// unique_id not found in the inventory
	script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}

/* getrefineuid("<unique_id>"{, <char_id>})
 * Returns the refine level of the inventory item identified by its unique_id
 * (passed as a string, since unique IDs are 64-bit values).
 * Returns -1 if the unique_id is not found in the player's inventory.
 */
BUILDIN_FUNC(getrefineuid)
{
	TBL_PC *sd;
	if (!script_accid2sd(3, sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	const char* uid_str = script_getstr(st, 2);
	uint64 uid = (uint64)strtoull(uid_str, nullptr, 10);
	if (uid == 0) {
		ShowError("buildin_getrefineuid: invalid unique_id \"%s\"\n", uid_str);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];
		if (it->nameid == 0 || it->unique_id != uid)
			continue;

		script_pushint(st, it->refine);
		return SCRIPT_CMD_SUCCESS;
	}

	// unique_id not found in the inventory
	script_pushint(st, -1);
	return SCRIPT_CMD_SUCCESS;
}
