ACMD_FUNC(jobchange)
{
	int32 job = 0, upper = 0;
	const char* text;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%11d %11d", &job, &upper) < 1) {
		int32 i;
		bool found = false;

		upper = 0;

		// Normal Jobs
		for( i = JOB_NOVICE; i < JOB_MAX_BASIC && !found; i++ ) {
			if (strncmpi(message, job_name(i), 16) == 0) {
				job = i;
				found = true;
			}
		}

		// High Jobs, Babys and Third
		for( i = JOB_NOVICE_HIGH; i < JOB_MAX && !found; i++ ) {
			if (strncmpi(message, job_name(i), 16) == 0) {
				job = i;
				found = true;
			}
		}

		if (!found) {
			text = atcommand_help_string(command);
			if (text)
				clif_displaymessage(fd, text);
			return -1;
		}
	}

	if (job == JOB_KNIGHT2 || job == JOB_CRUSADER2 || job == JOB_WEDDING || job == JOB_XMAS || job == JOB_SUMMER || job == JOB_HANBOK || job == JOB_OKTOBERFEST
		|| job == JOB_LORD_KNIGHT2 || job == JOB_PALADIN2 || job == JOB_BABY_KNIGHT2 || job == JOB_BABY_CRUSADER2 || job == JOB_STAR_GLADIATOR2
		|| (job >= JOB_RUNE_KNIGHT2 && job <= JOB_MECHANIC_T2) || (job >= JOB_BABY_RUNE_KNIGHT2 && job <= JOB_BABY_MECHANIC2) || job == JOB_BABY_STAR_GLADIATOR2
		|| job == JOB_STAR_EMPEROR2 || job == JOB_BABY_STAR_EMPEROR2 || job == JOB_SUMMER2)
	{ // Deny direct transformation into dummy jobs
		clif_displaymessage(fd, msg_txt(sd,923)); //"You can not change to this job by command."
		return 0;
	}

	if (pcdb_checkid(job))
	{
		if (pc_jobchange(sd, job, upper)) {
			clif_displaymessage(fd, msg_txt(sd,12)); // Your job has been changed.
			is_atcommand(fd, sd, "@blvl 999", 1); // [Stingor]
			is_atcommand(fd, sd, "@jlvl 100", 1); // [Stingor]
			is_atcommand(fd, sd, "@allskills", 1); // [Stingor]
			is_atcommand(fd, sd, "@allstats", 1); // [Stingor]
		} else {
			clif_displaymessage(fd, msg_txt(sd,155)); // You are unable to change your job.
			return -1;
		}
	} else {
		text = atcommand_help_string(command);
		if (text)
			clif_displaymessage(fd, text);
		return -1;
	}

	return 0;
}

/*==========================================
 * @storeall by [MouseJstr]
 * Put everything into storage
 *------------------------------------------*/
ACMD_FUNC(storeall)
{
	nullpo_retr(-1, sd);

	if (pc_istrading2(sd)) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	}

	if (sd->state.storage_flag == 2) {
		clif_displaymessage(fd, msg_txt(sd,251)); // You have already opened your guild storage. Close it first.
		return -1;
	}

	if (sd->state.storage_flag == 1 || sd->state.storage_flag == 3) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}

	if( storage_storageopen(sd) ) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	}

	for (int32 i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].amount) { // [Stingor]
			if(sd->inventory.u.items_inventory[i].equip == 0 && !sd->inventory.u.items_inventory[i].favorite)
			storage_storageadd(sd, &sd->storage, i, sd->inventory.u.items_inventory[i].amount);
		}
	}
	
	storage_storageclose(sd);

	clif_displaymessage(fd, msg_txt(sd,1162)); // All items stored.
	
	return 0;
}

ACMD_FUNC(clearstorage)
{
	int32 i, j;
	nullpo_retr(-1, sd);

	if (sd->state.storage_flag == 1) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}

	if (sd->state.storage_flag == 3) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}

	j = sd->storage.amount;
	for (i = 0; i < j; ++i) {
		storage_delitem(sd, &sd->storage, i, sd->storage.u.items_storage[i].amount);
	}
	sd->state.storage_flag = 1;
	storage_storageclose(sd);

	clif_displaymessage(fd, msg_txt(sd,1394)); // Your storage was cleaned.
	return 0;
}

ACMD_FUNC(cleargstorage)
{
	int32 i, j;
	struct s_storage *gstorage;
	nullpo_retr(-1, sd);

	if (!sd->guild) {
		clif_displaymessage(fd, msg_txt(sd,43)); // You're not in a guild.
		return -1;
	}

	if (sd->state.storage_flag == 1 || sd->state.storage_flag == 3) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}
	if (sd->state.storage_flag == 2) {
		clif_displaymessage(fd, msg_txt(sd,251)); // You have already opened your guild storage. Close it first.
		return -1;
	}

	gstorage = guild2storage2(sd->status.guild_id);
	if (gstorage == nullptr) { // Doesn't have opened @gstorage yet, so we skip the deletion since *shouldn't* have any item there.
		return -1;
	}

	j = gstorage->amount;
	gstorage->lock = true; // Lock @gstorage: do not allow any item to be retrieved or stored from any guild member
	for (i = 0; i < j; ++i) {
		storage_guild_delitem(sd, gstorage, i, gstorage->u.items_guild[i].amount);
	}
	storage_guild_storageclose(sd);
	gstorage->lock = false; // Cleaning done, release lock

	clif_displaymessage(fd, msg_txt(sd,1395)); // Your guild storage was cleaned.
	return 0;
}

/// @users - displays the number of players present on each map (and percentage)
/// #users displays on the target user instead of self
ACMD_FUNC(users)
{
	char buf[CHAT_SIZE_MAX];
	char buffer[CHAT_SIZE_MAX];
	int32 i;
	int32 users_all;
	int32 users_allshop;
	struct mapinfo{
		char charid[CHAT_SIZE_MAX];
		int32 users;
		int32 shops;
	} mapinfo[MAX_MAPINDEX];
	
	struct s_mapiterator* iter;

	memset(&mapinfo,0,sizeof(mapinfo));
	users_all = users_allshop = 0;

	// count users on each map
	iter = mapit_getallusers();
	for(;;)
	{
		map_session_data* sd2 = (map_session_data*)mapit_next(iter);
		if( sd2 == nullptr )
			break;// no more users

		if( sd2->mapindex >= MAX_MAPINDEX )
			continue;// invalid mapindex
		if( !sd2->state.vending ) {
			safesnprintf(buffer, sizeof(buffer), "[%s]", sd2->status.name);
			strcat(mapinfo[sd2->mapindex].charid, buffer);
		}
		if( mapinfo[sd2->mapindex].users < INT_MAX ) {
			++mapinfo[sd2->mapindex].users;
			if (sd2->state.vending)
				++mapinfo[sd2->mapindex].shops;
		}
		if( users_all < INT_MAX ) {
			++users_all;
			if (sd2->state.vending)
				++users_allshop;
		}
	}
	mapit_free(iter);

	// display results for each map
	for( i = 0; i < MAX_MAPINDEX; ++i )
	{
		if( mapinfo[i].users == 0 )
			continue;// empty
		if( mapinfo[i].shops > 0 && mapinfo[i].users > 0)
			safesnprintf(buf, sizeof(buf), "%s: %d (%d shop) - %s", mapindex_id2name(i), (mapinfo[i].users - mapinfo[i].shops), mapinfo[i].shops, mapinfo[i].charid);
		else if( mapinfo[i].users > 0 )
			safesnprintf(buf, sizeof(buf), "%s: %d - %s", mapindex_id2name(i), mapinfo[i].users, mapinfo[i].charid);
		else if( mapinfo[i].shops > 0 )
			safesnprintf(buf, sizeof(buf), "%s: %d shop", mapindex_id2name(i), mapinfo[i].shops);
		clif_displaymessage(sd->fd, buf);
	}

	// display overall count
	if( users_allshop > 0 )
		safesnprintf(buf, sizeof(buf), "all: %d (%d shop)", (users_all - users_allshop), users_allshop);
	else
		safesnprintf(buf, sizeof(buf), "all: %d", users_all);
	clif_displaymessage(sd->fd, buf);

	return 0;
}

/*==========================================
 * Display list of online characters with
 * various info.
 *------------------------------------------*/
ACMD_FUNC(who) {
	map_session_data* pl_sd = nullptr;
	struct s_mapiterator *iter = nullptr;
	char player_name[NAME_LENGTH] = "";
	int32 count = 0;
	int32 countshop = 0;
	int32 level = 0;
	StringBuf buf;
	/**
	 * 1 = @who  : Player name, [Title], [Party name], [Guild name]
	 * 2 = @who2 : Player name, [Title], BLvl, JLvl, Job
	 * 3 = @who3 : [CID/AID] Player name [Title], Map, X, Y
	 */
	int32 display_type = 1;
	int32 map_id = -1;

	nullpo_retr(-1, sd);

	if (strstr(command, "map") != nullptr) {
		char map_name[MAP_NAME_LENGTH_EXT] = "";
		if (sscanf(message, "%15s %23s", map_name, player_name) < 1 || (map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->m;
	} else {
		sscanf(message, "%23s", player_name);
	}

	if (strstr(command, "2") != nullptr)
		display_type = 2;
	else if (strstr(command, "3") != nullptr)
		display_type = 3;

	level = pc_get_group_level(sd);
	StringBuf_Init(&buf);

	iter = mapit_getallusers();
	for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))	{
		if (!((pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) || pc_isinvisible(pl_sd)) && pc_get_group_level(pl_sd) > level)) { // you can look only lower or same level
			if (stristr(pl_sd->status.name, player_name) == nullptr // search with no case sensitive
				|| (map_id >= 0 && pl_sd->m != map_id))
				continue;
				
			if( !pl_sd->state.vending ) {
				switch (display_type) {
					case 2: {
						StringBuf_Printf(&buf, msg_txt(sd,343), pl_sd->status.name, ""); // "Name: %s%s "
						if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
							StringBuf_Printf(&buf, msg_txt(sd,344), pl_sd->group->name.c_str()); // "(%s) "
						StringBuf_Printf(&buf, msg_txt(sd,347), pl_sd->status.base_level, pl_sd->status.job_level,
							job_name(pl_sd->status.class_)); // "| Lv:%d/%d | Job: %s"
						break;
					}
					case 3: {
						if (pc_has_permission(sd, PC_PERM_WHO_DISPLAY_AID))
							StringBuf_Printf(&buf, msg_txt(sd,912), pl_sd->status.char_id, pl_sd->status.account_id);	// "(CID:%d/AID:%d) "
						StringBuf_Printf(&buf, msg_txt(sd,343), pl_sd->status.name, ""); // "Name: %s%s "
						if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
							StringBuf_Printf(&buf, msg_txt(sd,344), pl_sd->group->name.c_str()); // "(%s) "
						StringBuf_Printf(&buf, msg_txt(sd,348), mapindex_id2name(pl_sd->mapindex), pl_sd->x, pl_sd->y); // "| Location: %s %d %d"
						break;
					}
					default: {
						struct party_data *p = party_search(pl_sd->status.party_id);
						auto& g = pl_sd->guild;
								
						StringBuf_Printf(&buf, msg_txt(sd,343), pl_sd->status.name, ""); // "Name: %s%s "
						if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
							StringBuf_Printf(&buf, msg_txt(sd,344), pl_sd->group->name.c_str()); // "(%s) "
						if (p != nullptr)
							StringBuf_Printf(&buf, msg_txt(sd,345), p->party.name); // " | Party: '%s'"
						if (g != nullptr)
							StringBuf_Printf(&buf, msg_txt(sd,346), g->guild.name); // " | Guild: '%s'"
						break;
					}
				}
				clif_displaymessage(fd, StringBuf_Value(&buf));
				StringBuf_Clear(&buf);
				count++;
			}
			else
				countshop++;
		}
	}

	if( countshop ) {
		clif_displaymessage(fd, "===== Shops =====");
		for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))	{
			if( pl_sd->state.vending ) {
				if (!((pc_has_permission(pl_sd, PC_PERM_HIDE_SESSION) || pc_isinvisible(pl_sd)) && pc_get_group_level(pl_sd) > level)) { // you can look only lower or same level
					if (stristr(pl_sd->status.name, player_name) == nullptr // search with no case sensitive
						|| (map_id >= 0 && pl_sd->m != map_id))
						continue;
					switch (display_type) {
						case 2: {
							StringBuf_Printf(&buf, msg_txt(sd,343), pl_sd->status.name, " (Shop)"); // "Name: %s%s "
							if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
								StringBuf_Printf(&buf, msg_txt(sd,344), pl_sd->group->name.c_str()); // "(%s) "
							StringBuf_Printf(&buf, msg_txt(sd,347), pl_sd->status.base_level, pl_sd->status.job_level,
								job_name(pl_sd->status.class_)); // "| Lv:%d/%d | Job: %s"
							break;
						}
						case 3: {
							if (pc_has_permission(sd, PC_PERM_WHO_DISPLAY_AID))
								StringBuf_Printf(&buf, msg_txt(sd,912), pl_sd->status.char_id, pl_sd->status.account_id);	// "(CID:%d/AID:%d) "
							StringBuf_Printf(&buf, msg_txt(sd,343), pl_sd->status.name, " (Shop)"); // "Name: %s%s "
							if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
								StringBuf_Printf(&buf, msg_txt(sd,344), pl_sd->group->name.c_str()); // "(%s) "
							StringBuf_Printf(&buf, msg_txt(sd,348), mapindex_id2name(pl_sd->mapindex), pl_sd->x, pl_sd->y); // "| Location: %s %d %d"
							break;
						}
						default: {
							struct party_data *p = party_search(pl_sd->status.party_id);
							auto &g = pl_sd->guild;
									
							StringBuf_Printf(&buf, msg_txt(sd,343), pl_sd->status.name, " (Shop)"); // "Name: %s%s "
							if (pc_get_group_id(pl_sd) > 0) // Player title, if exists
								StringBuf_Printf(&buf, msg_txt(sd,344), pl_sd->group->name.c_str()); // "(%s) "
							if (p != nullptr)
								StringBuf_Printf(&buf, msg_txt(sd,345), p->party.name); // " | Party: '%s'"
							if (g != nullptr)
								StringBuf_Printf(&buf, msg_txt(sd,346), g->guild.name); // " | Guild: '%s'"
							break;
						}
					}
					clif_displaymessage(fd, StringBuf_Value(&buf));
					StringBuf_Clear(&buf);
				}
			}
		}
	}
	mapit_free(iter);

	if (map_id < 0) {
		if (count == 0 || countshop == 0)
			StringBuf_Printf(&buf, msg_txt(sd,28)); // No player found.
		else if (count == 1 || countshop == 1)
			StringBuf_Printf(&buf, msg_txt(sd,29)); // 1 player/shop found.
		else
			StringBuf_Printf(&buf, msg_txt(sd,30), count, countshop); // %d players (%d Shops) found.
	} else {
		struct map_data *mapdata = map_getmapdata(map_id);

		if (count == 0 && countshop == 0)
			StringBuf_Printf(&buf, msg_txt(sd,54), mapdata->name); // No player found in map '%s'.
		else if (count == 1 || countshop == 1)
			StringBuf_Printf(&buf, msg_txt(sd,55), mapdata->name); // 1 player/shop found in map '%s'.
		else
			StringBuf_Printf(&buf, msg_txt(sd,56), count, countshop, mapdata->name); // %d players (%d Shops) found in map '%s'.
	}
	clif_displaymessage(fd, StringBuf_Value(&buf));
	return 0;
}

/*===================================
 * vote
 *-----------------------------------*/
ACMD_FUNC(votes)
{
	char output[128];
	int32 value;
	int32 ret = 0;
	nullpo_retr(-1, sd);

	// Since there is no cashpoint update packet we need to force updating like this
	if (sd->state.cashshop_open) {
		clif_displaymessage(fd, msg_txt(sd, 1376)); // Please close the vot'shop before using this command.
		return -1;
	}

	if (!message || !*message || (value = atoi(message)) == 0) {
		clif_displaymessage(fd, msg_txt(sd, 1322)); // Please enter an amount.
		return -1;
	}

	parent_cmd = atcommand_alias_db.checkAlias(command + 1);

	if (!strcmpi(parent_cmd, "votes"))
	{
		if (value > 0) {
			if ((ret = pc_getcash(sd, value, 0, LOG_TYPE_COMMAND)) >= 0) {
				// If this option is set, the message is already sent by pc function
				if (!battle_config.cashshop_show_points) {
					sprintf(output, msg_txt(sd, 505), ret, sd->cashPoints); // Gained %d votes. Total %d votes.
					clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
				}
			}
			else clif_displaymessage(fd, msg_txt(sd, 149)); // Impossible to increase the number/value.
		}
		else {
			if (-value > sd->cashPoints) //By command, if cash < value, force it to remove all
				value = -sd->cashPoints;
			if ((ret = pc_paycash(sd, -value, 0, LOG_TYPE_COMMAND)) >= 0) {
				// If this option is set, the message is already sent by pc function
				if (!battle_config.cashshop_show_points) {
					sprintf(output, msg_txt(sd, 410), ret, sd->cashPoints); // Removed %d votes. Total %d votes.
					clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
				}
			}
			else clif_displaymessage(fd, msg_txt(sd, 41)); // Unable to decrease the number/value.
		}
	}
	else
	{ // @points
		if (value > 0) {
			if ((ret = pc_getcash(sd, 0, value, LOG_TYPE_COMMAND)) >= 0) {
				if (!battle_config.cashshop_show_points) {
					sprintf(output, msg_txt(sd, 506), ret, sd->kafraPoints); // Gained %d kafra points. Total %d points.
					clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
				}
			}
			else clif_displaymessage(fd, msg_txt(sd, 149)); // Impossible to increase the number/value.
		}
		else {
			if (-value > sd->kafraPoints) //By command, if cash < value, force it to remove all
				value = -sd->kafraPoints;
			if ((ret = pc_paycash(sd, 0, -value, LOG_TYPE_COMMAND)) >= 0) {
				sprintf(output, msg_txt(sd, 411), ret, sd->kafraPoints); // Removed %d kafra points. Total %d points.
				clif_messagecolor(sd, color_table[COLOR_LIGHT_GREEN], output, false, SELF);
			}
			else clif_displaymessage(fd, msg_txt(sd, 41)); // Unable to decrease the number/value.
		}
	}

	return 0;
}

ACMD_FUNC(unloadnpcfile) {

	if (!message || !*message) {
		clif_displaymessage(fd, msg_txt(sd, 1385)); // Usage: @unloadnpcfile <file name>
		return -1;
	}

	// Normalize path separators so Windows-style paths work on all platforms
	char path[1024];
	safestrncpy(path, message, sizeof(path));
	for (char* p = path; *p; ++p) if (*p == '\\') *p = '/';

	if (npc_unloadfile(path)) {
		clif_displaymessage(fd, msg_txt(sd, 1386)); // File unloaded. Be aware that mapflags and monsters spawned directly are not removed.
		ShowStatus("NPC file '" CL_WHITE "%s" CL_RESET "' was unloaded.\n", path);
	}
	else {
		clif_displaymessage(fd, msg_txt(sd, 1387)); // File not found.
		ShowDebug("Unloadnpcfile: Unknown NPC filepath'" CL_WHITE "%s" CL_RESET "'.\n", path);
		return -1;
	}

	return 0;
}

/*==========================================
 * @wings par [Stingor]
 * Afficher ou masquer les wings custom.
 *----------------------------------------------------*/
ACMD_FUNC(wings)
{
	nullpo_retr(-1, sd);

	if( sd->sc.option & OPTION_WINGS ) {
		clif_displaymessage(fd, msg_txt(sd,1835));
		pc_setoption(sd, sd->sc.option&~OPTION_WINGS);
		clif_hide_wings(sd);
	}
	else {
		clif_displaymessage(fd, msg_txt(sd,1836));
		pc_setoption(sd, sd->sc.option|OPTION_WINGS);
		clif_show_wings(sd);
	}
	return 0;
}

/*==========================================
 * @sellstuff par [Stingor]
 * Protége le joueur de vendre les cards ou item avec card au npc.
 *----------------------------------------------------*/
ACMD_FUNC(sellstuff)
{
	nullpo_retr(-1, sd);

	if( sd->state.sellstuff ) {
		clif_displaymessage(fd, msg_txt(sd,1801));
		sd->state.sellstuff = 0;
	}
	else {
		clif_displaymessage(fd, msg_txt(sd,1802));
		sd->state.sellstuff = 1;
	}

	return 0;
}

/*==========================================
 * @sellitem par [Stingor]
 * Protége le joueur de vendre certains items importants au npc.
 *----------------------------------------------------*/
ACMD_FUNC(sellitem)
{
	nullpo_retr(-1, sd);

	if( sd->state.sellitem ) {
		clif_displaymessage(fd, msg_txt(sd,1803));
		sd->state.sellitem = 0;
	}
	else {
		clif_displaymessage(fd, msg_txt(sd,1804));
		sd->state.sellitem = 1;
	}

	return 0;
}

/*==========================================
 * @showspeed par [Stingor]
 * Affiche la valeur du changement de vitesse.
 *---------------------------------------------------*/
ACMD_FUNC(showspeed)
{
	nullpo_retr(-1, sd);

	if( sd->state.showspeed )  {
		sd->state.showspeed = 0;
		clif_displaymessage(fd, msg_txt(sd,1805));
	}
	else{
		sd->state.showspeed = 1;
		clif_displaymessage(fd, msg_txt(sd,1806));
	}

	return 0;
}

/*==========================================
 * @fastmove par [Stingor]
 * Active/Désactive le teleport au click...
 *----------------------------------------------------*/
ACMD_FUNC(fastmove)
{
	nullpo_retr(-1, sd);

	if( !sd->state.gm_fast_move ) {
		sd->state.gm_fast_move = 1;
		clif_displaymessage(fd, msg_txt(sd,1807));
	}
	else {
		sd->state.gm_fast_move = 0;
		clif_displaymessage(fd, msg_txt(sd,1808));
	}
	return 0;
}

/*==========================================
 * @mvpcard par [Stingor]
 * Active/Désactive le loot d'MvP card dans la map.
 *----------------------------------------------------*/
ACMD_FUNC(mvpcard)
{
	nullpo_retr(-1, sd);

	if( map_getmapflag(sd->m, MF_NOMVPCARD) ) {
		map_setmapflag(sd->m, MF_NOMVPCARD, false);
		clif_displaymessage(fd, msg_txt(sd,1809));
	}
	else {
		map_setmapflag(sd->m, MF_NOMVPCARD, true);
		clif_displaymessage(fd, msg_txt(sd,1810));
	}
	return 0;
}

/*==========================================
 * @attackskill par [Stingor]
 * Active/Désactive les skills non support.
 *----------------------------------------------------*/
ACMD_FUNC(attackskill)
{
	nullpo_retr(-1, sd);

	if( map_getmapflag(sd->m, MF_NOATTACKSKILL) ) {
		map_setmapflag(sd->m, MF_NOATTACKSKILL, false);
		clif_displaymessage(fd, msg_txt(sd,1811));
	}
	else {
		map_setmapflag(sd->m, MF_NOATTACKSKILL, true);
		clif_displaymessage(fd, msg_txt(sd,1812));
	}
	return 0;
}

/*==========================================
 * @aoeskill par [Stingor]
 * Active/Désactive les skills AOE.
 *----------------------------------------------------*/
ACMD_FUNC(aoeskill)
{
	nullpo_retr(-1, sd);

	if( map_getmapflag(sd->m, MF_NOAOESKILL) ) {
		map_setmapflag(sd->m, MF_NOAOESKILL, false);
		clif_displaymessage(fd, msg_txt(sd,1813));
	}
	else {
		map_setmapflag(sd->m, MF_NOAOESKILL, true);
		clif_displaymessage(fd, msg_txt(sd,1814));
	}

	return 0;
}

/*==========================================
 * @playertalk <Pseudo ou Char ID>,<Message> par [Stingor]
 * Permet de parler à la place d'un joueur.
 *---------------------------------------------------*/
ACMD_FUNC(playertalk)
{
	char mes[100],temp[100];
	TBL_PC *pl_sd;
	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	nullpo_retr(-1, sd);

	if( !message || !*message || sscanf(message, "%23[^,],%99[^\n]", atcmd_player_name, mes) < 2 ) {
		clif_displaymessage(fd, msg_txt(sd,1815));
		return -1;
	}

	if( (pl_sd = map_nick2sd(atcmd_player_name, true)) == nullptr && (pl_sd = map_charid2sd(atoi(atcmd_player_name))) == nullptr) {
		clif_displaymessage(fd, msg_txt(sd,3));
		return -1;
	}

	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) ) {
		clif_displaymessage(fd, msg_txt(sd,81));
		return -1;
	}

	snprintf(temp, sizeof temp ,"%s : %s", pl_sd->status.name, mes);
	clif_GlobalMessage(*pl_sd, temp, AREA_CHAT_WOC);
	clif_displaymessage(pl_sd->fd, temp);

    log_chat(LOG_CHAT_GLOBAL, 0, pl_sd->status.name, pl_sd->status.account_id, mapindex_id2name(pl_sd->mapindex), pl_sd->x, pl_sd->y, nullptr, mes);

	return 0;
}
 /*==========================================
 * @moche <Pseudo> par [Stingor]
 * Génére un model aléatoire.
 *----------------------------------------------------*/
ACMD_FUNC(moche)
{
	TBL_PC *pl_sd;
	int32 hair_style = rand()%MAX_HAIR_STYLE;
	int32 hair_color = rand()%MAX_HAIR_COLOR;
	int32 cloth_color = rand()%MAX_CLOTH_COLOR;

	nullpo_retr(-1,sd);

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if( !message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, msg_txt(sd,1816));
		return -1;
	}

	if( (pl_sd = map_nick2sd(atcmd_player_name, true)) == nullptr && (pl_sd = map_charid2sd(atoi(message))) == nullptr) {
		clif_displaymessage(fd, msg_txt(sd,3)); // Character not found.
		return -1;
	}

	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) ) {
		clif_displaymessage(fd, msg_txt(sd,81));
		return -1;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		char mess[256];
		if (hair_style <= 0) hair_style = 1;
		if (hair_color <= 0) hair_color = 1;
		if (cloth_color <= 0) cloth_color = 1;

		clif_changelook(pl_sd, LOOK_HAIR, hair_style);
		clif_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
		clif_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
		sprintf(mess, msg_txt(sd,1817), pl_sd->status.name);
		clif_displaymessage(fd, mess);
	}
	else
		return -1;

 	return 0;
}

/*==========================================
 * @celebrate par [Stingor]
 *----------------------------------------------------*/
ACMD_FUNC(celebrate)
{
	map_session_data* pl_sd;
	struct s_mapiterator* iter;
	char temp[100];
	int32 i;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();

	for( i = 0; i < 6; i++ )
		clif_specialeffect(sd,1038,ALL_CLIENT);

	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->m == pl_sd->m ) {
			clif_changelook(pl_sd, LOOK_HEAD_TOP, 144);
			clif_specialeffect(pl_sd,508,AREA);
			clif_specialeffect(pl_sd,435,AREA);
			snprintf(temp, sizeof temp ,"%s : CELEBRATE !!!", pl_sd->status.name);
			clif_GlobalMessage(*pl_sd, temp, AREA_CHAT_WOC);
			clif_displaymessage(pl_sd->fd, temp);
		}
	}
	mapit_free(iter);
	return 0;
}

/*==========================================
 * @trollolol par [Stingor]
 * tête d'orc pour tous
 *----------------------------------------------------*/
ACMD_FUNC(trollolol)
{
	map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();

	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->m == pl_sd->m && sd != pl_sd )
			pc_setoption(pl_sd, pl_sd->sc.option|OPTION_ORCISH);
	}
	mapit_free(iter);
	return 0;
}

/*==========================================
 * @untrollolol
 *----------------------------------------------------*/
ACMD_FUNC(untrollolol)
{
	map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();

	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->m == pl_sd->m && sd != pl_sd )
			pc_setoption(pl_sd, pl_sd->sc.option&~OPTION_ORCISH);
	}
	mapit_free(iter);
	return 0;
}

/*==========================================
 * @sitstand <Joueur> par [Stingor]
 * Fait lever ou assoir un joueur.
 *----------------------------------------------------*/
ACMD_FUNC(sitstand)
{
	TBL_PC *pl_sd;

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if( !message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, msg_txt(sd,1818));
		return -1;
	}

	if( (pl_sd = map_nick2sd(atcmd_player_name, true)) == nullptr && (pl_sd = map_charid2sd(atoi(message))) == nullptr) {
		clif_displaymessage(fd, msg_txt(sd,3)); // Character not found.
		return -1;
	}

	if( pc_get_group_level(sd) < pc_get_group_level(pl_sd) ) {
		clif_displaymessage(fd, msg_txt(sd,81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if( pc_isdead(sd) ) {
		clif_displaymessage(fd, msg_txt(sd,1819));
		return -1;
	}

	if( pc_issit(pl_sd) ) {
		pc_setstand(pl_sd, true);
		skill_sit(sd, false);
		clif_standing(*pl_sd);
	}
	else {
		pc_setsit(pl_sd);
		skill_sit(sd, true);
		clif_sitting(*pl_sd);
	}

 	return 0;
 }

 /*==========================================
 * @autolootmvp par [Stingor]
 * Loot only mvps
 *------------------------------------------*/
ACMD_FUNC(autolootmvp)
{
	nullpo_retr(-1, sd);

	if( sd->state.autolootmvp ) {
		sd->state.autolootmvp = 0;
		pc_setglobalreg(sd, add_str("alootmvp"), 0);
		clif_displaymessage(fd, msg_txt(sd,1820));
	}
	else {
		pc_setglobalreg(sd, add_str("alootmvp"), 1);
		sd->state.autolootmvp = 1;
		clif_displaymessage(fd, msg_txt(sd,1821));
	}
	return 0;
}

 /*==========================================
 * @autolootmvpreward par [Stingor]
 * Loot only reward mvps
 *------------------------------------------*/
ACMD_FUNC(autolootmvpreward)
{
	nullpo_retr(-1, sd);

	if( sd->state.autolootmvpreward ) {
		sd->state.autolootmvpreward = 0;
		pc_setglobalreg(sd, add_str("alootmvpreward"), 0);
		clif_displaymessage(fd, msg_txt(sd,1837));
	}
	else {
		pc_setglobalreg(sd, add_str("alootmvpreward"), 1);
		sd->state.autolootmvpreward = 1;
		clif_displaymessage(fd, msg_txt(sd,1838));
	}
	return 0;
}

/*==========================================
 * @autolootrare par [Stingor]
 * Loot only rare items in a db
 *------------------------------------------*/
ACMD_FUNC(autolootrare)
{
	nullpo_retr(-1, sd);
	if( sd->state.autolootrare ) {
		pc_setglobalreg(sd, add_str("alootrare"), 0);
		sd->state.autolootrare = 0;
		clif_displaymessage(fd, msg_txt(sd,1822));
	}
	else {
		pc_setglobalreg(sd, add_str("alootrare"), 1);
		sd->state.autolootrare = 1;
		clif_displaymessage(fd, msg_txt(sd,1823));
	}
	return 0;
}

/*==========================================
 * @showmobinfo thx [Doo~]
 * Loot only rare items in a db
 *------------------------------------------*/
ACMD_FUNC(showmobinfo)
{
	nullpo_retr(-1, sd);
	if( sd->state.showmobinfo ) {
		pc_setglobalreg(sd, add_str("showmobinfo"), 0);
		sd->state.showmobinfo = 0;
		clif_displaymessage(fd, msg_txt(sd,1861));
		clif_refresh(sd);
	}
	else {
		pc_setglobalreg(sd, add_str("showmobinfo"), 1);
		sd->state.showmobinfo = 1;
		clif_displaymessage(fd, msg_txt(sd,1860));
		clif_refresh(sd);
	}
	return 0;
}

/*==========================================
 * @autolootpognon par [Stingor]
 * Loot only worth selected value
 *------------------------------------------*/
ACMD_FUNC(autolootpognon)
{
	nullpo_retr(-1, sd);

	if( message && *message ) {
		if( !strcmp(message,"reset") ) {
			pc_setglobalreg(sd, add_str("alootpognon"), 0);
			sd->state.autolootpognon = 0;
			clif_displaymessage(fd, msg_txt(sd,1826));
		}
		else {
			int32 prix;

			if( (sscanf(message, "%d", &prix) < 1) || prix <= 0 ) {
				prix = sd->state.autolootpognon = 0;
				clif_displaymessage(fd, msg_txt(sd,1828));
				return -1;
			}

			pc_setglobalreg(sd, add_str("alootpognon"), prix);
			sd->state.autolootpognon = prix;
			sprintf(atcmd_output, msg_txt(sd,1829), prix);
			clif_displaymessage(fd, atcmd_output);
		}
	}
	else {
		if( sd->state.autolootpognon ) {
			sprintf(atcmd_output, msg_txt(sd,1829), sd->state.autolootpognon);
			clif_displaymessage(fd, atcmd_output);
		}
		else
			clif_displaymessage(fd, msg_txt(sd,1827));
	}
	return 0;
}

ACMD_FUNC(stats)
{
	char output[CHAT_SIZE_MAX];
	int32 i;
	struct {
		const char* format;
		int32 value;
	} output_table[] = {
		{ "Castrate - %d", 0 },
		{ "Delayrate - -%d%% - ACD (35%% soft cap)", 0 },
		{ "Hprate - %d%% - pourc. de pv en plus", 0 },
		{ "Sprate - %d%% - pourc. de sp en plus", 0 },
		{ "HPrecovRate - %d%% - Vitesse de regen pv", 0 },
		{ "SPrecovRate - %d%% - Vitesse de regen sp", 0 },
		{ "CriticalRate - %d%%", 0 },
		{ "HitRate - %d%%", 0 },
		{ "FleeRate - %d%%", 0 },
		{ "AtkRate - %d%%", 0 },
		{ "MAtkRate - %d%%", 0 },
		{ "========= Resistance Values =========", 0 },
		{ " [ %d%% ] Neutral resist", 0 },
		{ " [ %d%% ] Water resist", 0 },
		{ " [ %d%% ] Earth resist", 0 },
		{ " [ %d%% ] Fire resist", 0 },
		{ " [ %d%% ] Wind resist", 0 },
		{ " [ %d%% ] Poison resist", 0 },
		{ " [ %d%% ] Holy resist", 0 },
		{ " [ %d%% ] Dark resist", 0 },
		{ " [ %d%% ] Ghost resist", 0 },
		{ " [ %d%% ] Undead resist", 0 },
		{ NULL, 0 }
	};

	nullpo_retr(-1, sd);
	
	memset(output, '\0', sizeof(output));

	//direct array initialization with variables is not standard C compliant.
	output_table[0].value = sd->castrate;
	output_table[1].value = sd->bonus.delayrate + sd->bonus.delayrate2;
	output_table[2].value = sd->hprate;
	output_table[3].value = sd->sprate;
	output_table[4].value = sd->hprecov_rate;
	output_table[5].value = sd->sprecov_rate;
	output_table[6].value = sd->critical_rate;
	output_table[7].value = sd->hit_rate;
	output_table[8].value = sd->flee_rate;
	output_table[9].value = sd->bonus.atk_rate;
	output_table[10].value = sd->matk_rate;
	output_table[12].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_NEUTRAL] + sd->indexed_bonus.subele_script[ELE_NEUTRAL]);
	output_table[13].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_WATER] + sd->indexed_bonus.subele_script[ELE_WATER]);
	output_table[14].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_EARTH] + sd->indexed_bonus.subele_script[ELE_EARTH]);
	output_table[15].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_FIRE] + sd->indexed_bonus.subele_script[ELE_FIRE]);
	output_table[16].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_WIND] + sd->indexed_bonus.subele_script[ELE_WIND]);
	output_table[17].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_POISON] + sd->indexed_bonus.subele_script[ELE_POISON]);
	output_table[18].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_HOLY] + sd->indexed_bonus.subele_script[ELE_HOLY]);
	output_table[19].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_DARK] + sd->indexed_bonus.subele_script[ELE_DARK]);
	output_table[20].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_GHOST] + sd->indexed_bonus.subele_script[ELE_GHOST]);
	output_table[21].value = (sd->indexed_bonus.subele[ELE_ALL] + sd->indexed_bonus.subele_script[ELE_ALL] + sd->indexed_bonus.subele[ELE_UNDEAD] + sd->indexed_bonus.subele_script[ELE_UNDEAD]);

	sprintf(output, msg_txt(sd,53), sd->status.name); // '%s' stats:

	clif_displaymessage(fd, output);

	for( i = 0; output_table[i].format != nullptr; i++ ) {
		sprintf(output, output_table[i].format, output_table[i].value);
		clif_displaymessage(fd, output);
	}

	return 0;
}

/*==========================================
 * @whosell <Nom ou ID d'item>
 *----------------------------------------------------*/
ACMD_FUNC(shopsearch)
{
	char item_name[100], output[255];
	struct s_mapiterator* iter;
	int32 j, count = 0;
	map_session_data *pl_sd;
	uint32 MinPrize = battle_config.vending_max_value, MaxPrize = 0;
	
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));

	if( !message || !*message || sscanf(message, "%99[^\n]", item_name) < 1 ) {
		clif_displaymessage(fd, msg_txt(sd,1830));
		return -1;
	}

	std::shared_ptr<item_data> item = item_db.searchname(item_name);

	if (item == nullptr) {
		item = item_db.find(strtoul(item_name, nullptr, 10));
	}

	if (item == nullptr) {
		clif_displaymessage(fd, msg_txt(sd, 19)); // Invalid item ID or name.
		return -1;
	}

	iter = mapit_getallusers();
	
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( pl_sd->vender_id ) {
			// One player Online and are in the same map and are vending
			for( j = 0; j < pl_sd->vend_num; j++ ) {
				if( pl_sd->cart.u.items_cart[pl_sd->vending[j].index].nameid == item->nameid ) {
					// If the item is the same, it will show this? Computer translations are bad.
					sprintf(output, "Vendeur: %s | Quantit� %d | Prix: %d | Shop: %s | Coord: %d,%d", pl_sd->status.name, pl_sd->vending[j].amount, pl_sd->vending[j].value,pl_sd->message,pl_sd->x,pl_sd->y);
					clif_viewpoint(*sd, 1, 1, pl_sd->x, pl_sd->y, count+1, 0xFFFFFF);
					
					if(pl_sd->vending[j].value < MinPrize)
						MinPrize = pl_sd->vending[j].value;
					
					if(pl_sd->vending[j].value > MaxPrize)
						MaxPrize = pl_sd->vending[j].value;
					
					clif_displaymessage(fd, output); // Message sent to player
					count++;
				}
			}
		}
	}
	mapit_free(iter);

	if (count > 0)  {
		sprintf(output, msg_txt(sd,1831), count, MinPrize, MaxPrize);
		clif_displaymessage(fd, output);
	}
	else
		clif_displaymessage(fd, msg_txt(sd,1832));

	return 0;
}

/*==========================================
 * @storagealt [Stingor]
 * Ouverture des storages alternatifs
 *------------------------------------------*/
ACMD_FUNC(storagealt)
{
	nullpo_retr(-1, sd);

	if (pc_istrading2(sd)) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	}

	if (sd->state.storage_flag == 2) {
		clif_displaymessage(fd, msg_txt(sd,251)); // You have already opened your guild storage. Close it first.
		return -1;
	}

	if (sd->state.storage_flag == 1 || sd->state.storage_flag == 3) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}

	// Résoudre l'alias vers la commande principale (ex: "cardstorage" -> "storagealt5")
	const char *resolved = atcommand_alias_db.checkAlias(command + 1);
	const char *p = resolved;
	uint8 num = 1;

	while (*p) {
		if (*p >= '1' && *p <= '5') {
			num = (uint8)(*p - '0');
			break;
		}
		p++;
	}

	if (storage_premiumStorage_load(sd, num, STOR_MODE_ALL) == 0) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}
	char output[128];
	sprintf(output, msg_txt(sd,1850), num); // Storage alternatif %d ouvert.
	clif_displaymessage(fd, output);

	return 0;
}

/*==========================================
 * @storecard [Stingor]
 * Met toutes les cards dans le card storage
 *------------------------------------------*/
ACMD_FUNC(storecard)
{	
	nullpo_retr(-1, sd);

	if (pc_istrading2(sd)) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	}

	if (sd->state.storage_flag == 2) {
		clif_displaymessage(fd, msg_txt(sd,251)); // You have already opened your guild storage. Close it first.
		return -1;
	}

	if (sd->state.storage_flag == 1 || sd->state.storage_flag == 3) {
		clif_displaymessage(fd, msg_txt(sd,250)); // You have already opened your storage. Close it first.
		return -1;
	}

	if (storage_premiumStorage_load(sd, 5, STOR_MODE_PUT) == 0) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	}

	for (int i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].amount && itemdb_type(sd->inventory.u.items_inventory[i].nameid) == IT_CARD)
			storage_storageadd(sd, &sd->premiumStorage, i, sd->inventory.u.items_inventory[i].amount);
	}

	storage_premiumStorage_close(sd);

	clif_displaymessage(fd, msg_txt(sd,1851)); // Toutes les Cards sont stockées dans le storage alternatif 5.
	
	return 0;
}

/*==========================================
 * @partybuff [Vykimo]
 *------------------------------------------*/
ACMD_FUNC(partybuff)
{
	struct party_data* p = nullptr;
	nullpo_retr(-1, sd);

	if( !sd->status.party_id ) {
		clif_displaymessage(fd, msg_txt(sd,1839)); // You're not in a party.
		return -1;
	}

	p = party_search(sd->status.party_id);

	if( sd->state.spb ) {
		sd->state.spb = 0;
		clif_displaymessage(fd, msg_txt(sd,1840)); // Displaying party member's buffs disabled.
	} else {
		sd->state.spb = 1;
		clif_displaymessage(fd, msg_txt(sd,1841)); // Displaying party member's buffs enabled.
	}

    if (p)
		clif_party_info(*p, sd);
	return 0;
}

/*==========================================
 * Show Monster DB Info   v 1.0
 * originally by [Lupus]
 *------------------------------------------*/
ACMD_FUNC(mobinfo)
{
	unsigned char msize[SZ_ALL][7] = { "Small", "Medium", "Large" };
	unsigned char mrace[RC_ALL][11] = { "Formless", "Undead", "Beast", "Plant", "Insect", "Fish", "Demon", "Demi-Human", "Angel", "Dragon", "Player" };
	unsigned char melement[ELE_ALL][8] = { "Neutral", "Water", "Earth", "Fire", "Wind", "Poison", "Holy", "Dark", "Ghost", "Undead" };
	char atcmd_output2[CHAT_SIZE_MAX];
	char mob_name[256];
	uint16 mob_ids[MAX_SEARCH], count;
	uint16 i;
	int32 hardmode = 1;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_output2, '\0', sizeof(atcmd_output2));

	if (!message || !*message) {
		clif_displaymessage(fd, msg_txt(sd, 1239)); // Please enter a monster name/ID (usage: @mobinfo <monster_name_or_monster_ID> [hardmode 1-20]).
		return -1;
	}

	// Parse optional hardmode level from end of message: @mobinfo <mob> [1-20]
	strncpy(mob_name, message, sizeof(mob_name) - 1);
	mob_name[sizeof(mob_name) - 1] = '\0';
	{
		char* last_space = strrchr(mob_name, ' ');
		if (last_space != nullptr) {
			char* endptr;
			long hm = strtol(last_space + 1, &endptr, 10);
			if (*endptr == '\0' && hm >= 1 && hm <= 20) {
				hardmode = (int32)hm;
				*last_space = '\0';
			}
		}
	}

	// If monster identifier/name argument is an ID
	if ((i = mobdb_checkid(strtoul(mob_name, nullptr, 10))))
	{
		mob_ids[0] = i;
		count = 1;
	}
	else
		count = mobdb_searchname_array(mob_name, mob_ids, MAX_SEARCH);

	if (!count) {
		clif_displaymessage(fd, msg_txt(sd, 40)); // Invalid monster ID or name.
		return -1;
	}

	if (count >= MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(sd, 269), MAX_SEARCH); // Displaying first %d matches
		clif_displaymessage(fd, atcmd_output);
		count = MAX_SEARCH;
	}
	for (uint16 k = 0; k < count; k++) {
		std::shared_ptr<s_mob_db> mob = mob_db.find(mob_ids[k]);

		if (mob == nullptr)
			continue;

		t_exp base_exp = mob->base_exp;
		t_exp job_exp = mob->job_exp;

		if (event_exp) {// Stingor
			base_exp = apply_rate(base_exp, battle_config.mult_eventexp);
			job_exp = apply_rate(job_exp, battle_config.mult_eventexp);
		}

		// stats
		if (mob->get_bosstype() == BOSSTYPE_MVP)
			sprintf(atcmd_output, msg_txt(sd, 1240), mob->name.c_str(), mob->jname.c_str(), mob->sprite.c_str(), mob->id); // MVP Monster: '%s'/'%s'/'%s' (%d)
		else
			sprintf(atcmd_output, msg_txt(sd, 1241), mob->name.c_str(), mob->jname.c_str(), mob->sprite.c_str(), mob->id); // Monster: '%s'/'%s'/'%s' (%d)
		clif_displaymessage(fd, atcmd_output);

		double base_pct = 0, job_pct = 0;
		t_exp base_exp2 = base_exp;
		if (sd->status.base_level > 200 && base_exp2 > 0) { // [Stingor]
			base_exp2 = (t_exp)((float)base_exp2 / exp(((float)(sd->status.base_level) - 200.0f) / 250.0f));
			if (sd->status.class_ == JOB_TAEKWON)
				base_exp2 /= 2;
		}
		t_exp nextbase = pc_nextbaseexp(sd);
		t_exp nextjob = pc_nextjobexp(sd);
		if (nextbase)
			base_pct = (double)base_exp2 * 100.0 / nextbase;
		if (nextjob)
			job_pct = (double)job_exp * 100.0 / nextjob;
		sprintf(atcmd_output, msg_txt(sd, 1242), mob->lv, mob->status.max_hp, base_exp, base_pct, job_exp, job_pct, MOB_HIT(mob), MOB_FLEE(mob)); //  Lv:%d  HP:%d  Base EXP:%llu  Job EXP:%llu  HIT:%d  FLEE:%d
		clif_displaymessage(fd, atcmd_output);

		sprintf(atcmd_output, msg_txt(sd, 1243), //  DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d
			mob->status.def, mob->status.mdef, mob->status.str, mob->status.agi,
			mob->status.vit, mob->status.int_, mob->status.dex, mob->status.luk);
		clif_displaymessage(fd, atcmd_output);

		sprintf(atcmd_output, msg_txt(sd, 1244), //  ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)
			mob->status.batk + mob->status.rhw.atk, mob->status.batk + mob->status.rhw.atk2, mob->status.rhw.range,
			mob->range2, mob->range3, msize[mob->status.size],
			mrace[mob->status.race], melement[mob->status.def_ele], mob->status.ele_lv);
		clif_displaymessage(fd, atcmd_output);
#ifdef RENEWAL
		sprintf(atcmd_output, msg_txt(sd, 827), mob->status.res, mob->status.mres);//  MDEF:%d  RES:%d  MRES:%d
		clif_displaymessage(fd, atcmd_output);
#endif
		// hardmode simulation
		if (hardmode > 1) {
			int32 hm_cap   = cap_value(hardmode, 1, 15);
			int32 hm_level = (int32)mob->lv * hardmode;

			uint64 hp_floor = (uint64)(5000 * hardmode);
			uint64 hm_hp    = ((uint64)mob->status.max_hp < hp_floor ? hp_floor : (uint64)mob->status.max_hp) * (uint64)hardmode * 3ULL;

			uint64 hm_atkmin, hm_atkmax, hm_matkmin, hm_matkmax;
			if (hardmode >= 15) {
				uint64 af_min  = (uint64)(2000  * hardmode);
				uint64 af_max  = (uint64)(10000 * hardmode);
				uint64 amin_b  = ((uint64)mob->status.rhw.atk  < af_min) ? af_min : (uint64)mob->status.rhw.atk;
				uint64 amax_b  = ((uint64)mob->status.rhw.atk2 < af_max) ? af_max : (uint64)mob->status.rhw.atk2;
				uint64 mamin_b = ((uint64)mob->status.matk_min < af_min) ? af_min : (uint64)mob->status.matk_min;
				uint64 mamax_b = ((uint64)mob->status.matk_max < af_max) ? af_max : (uint64)mob->status.matk_max;
				hm_atkmin  = amin_b  * (uint64)(hardmode * 2) * (uint64)hardmode / 2ULL;
				hm_atkmax  = amax_b  * (uint64)(hardmode * 2) * (uint64)hardmode / 2ULL;
				hm_matkmin = mamin_b * (uint64)(hardmode * 2) * (uint64)hardmode;
				hm_matkmax = mamax_b * (uint64)(hardmode * 2) * (uint64)hardmode;
			} else {
				uint64 af_min  = (uint64)(1000 * hardmode);
				uint64 af_max  = (uint64)(5000 * hardmode);
				uint64 amin_b  = ((uint64)mob->status.rhw.atk  < af_min) ? af_min : (uint64)mob->status.rhw.atk;
				uint64 amax_b  = ((uint64)mob->status.rhw.atk2 < af_max) ? af_max : (uint64)mob->status.rhw.atk2;
				uint64 mamin_b = ((uint64)mob->status.matk_min < af_min) ? af_min : (uint64)mob->status.matk_min;
				uint64 mamax_b = ((uint64)mob->status.matk_max < af_max) ? af_max : (uint64)mob->status.matk_max;
				hm_atkmin  = amin_b  * 30ULL * (uint64)hardmode / 2ULL;
				hm_atkmax  = amax_b  * 30ULL * (uint64)hardmode / 2ULL;
				hm_matkmin = mamin_b * 30ULL * (uint64)hardmode;
				hm_matkmax = mamax_b * 30ULL * (uint64)hardmode;
			}

			int32 hm_def     = (mob->status.def  < 5 ? 5 : (int32)mob->status.def)  + hm_cap;
			int32 hm_mdef    = (mob->status.mdef < 5 ? 5 : (int32)mob->status.mdef) + hm_cap;
			int32 hm_hit     = (mob->status.hit  < 300 ? 300 : (int32)mob->status.hit)  * hm_cap;
			int32 hm_flee    = (mob->status.flee > 60  ? 60  : (int32)mob->status.flee) * hm_cap;
			int32 hm_crit    = (mob->status.cri < 25 ? 25 : (int32)mob->status.cri) + hardmode * 30;
			int32 hm_adelay  = cap_value((int32)mob->status.adelay  / hardmode * 2, 50, 1200);
			int32 hm_speed   = cap_value((int32)mob->status.speed   / hardmode * 2, 40, 400);
			float hm_atkspd  = 1000.0f / (float)hm_adelay;
			float hm_movspd  = 1000.0f / (float)hm_speed;

			// Esquive: label qualitatif (FLEE - HIT_joueur + 100 = taux reel, depend du HIT de l'attaquant)
			const char* flee_label;
			if      (hm_flee < 100) flee_label = "Nulle";
			else if (hm_flee < 300) flee_label = "Faible";
			else if (hm_flee < 500) flee_label = "Moderee";
			else if (hm_flee < 700) flee_label = "Elevee";
			else                    flee_label = "Extreme";

			snprintf(atcmd_output, sizeof(atcmd_output), " -- [ Hard Mode Lv.%d ] --", hardmode);
			clif_displaymessage(fd, atcmd_output);
			snprintf(atcmd_output, sizeof(atcmd_output), "  Lv: %d  HP: %llu", hm_level, (unsigned long long)hm_hp);
			clif_displaymessage(fd, atcmd_output);
			snprintf(atcmd_output, sizeof(atcmd_output), "  ATK: %llu~%llu  MATK: %llu~%llu",
				(unsigned long long)hm_atkmin, (unsigned long long)hm_atkmax,
				(unsigned long long)hm_matkmin, (unsigned long long)hm_matkmax);
			clif_displaymessage(fd, atcmd_output);
			snprintf(atcmd_output, sizeof(atcmd_output), "  DEF: %d  MDEF: %d", hm_def, hm_mdef);
			clif_displaymessage(fd, atcmd_output);
			snprintf(atcmd_output, sizeof(atcmd_output), "  HIT: %d  Esquive: %d (%s)  CRIT: %.1f%%", hm_hit, hm_flee, flee_label, hm_crit / 10.0f);
			clif_displaymessage(fd, atcmd_output);
			// Cadence : attaques/sec | Deplacement : cellules/sec
			snprintf(atcmd_output, sizeof(atcmd_output), " Cadence d'attaque: %.1f atk/s  Deplacement: %.1f cel/s", hm_atkspd, hm_movspd);
			clif_displaymessage(fd, atcmd_output);
		}
		// drops
		clif_displaymessage(fd, msg_txt(sd, 1245)); //  Drops:
		strcpy(atcmd_output, " ");
		if (mob->dropitem.empty()) {
			clif_displaymessage(fd, msg_txt(sd, 1246)); // This monster has no drops.
		}
		else {
			uint32 j = 0;
			int32 drop_modifier = 100;
#ifdef RENEWAL_DROP
			if (battle_config.atcommand_mobinfo_type) {
				drop_modifier = pc_level_penalty_mod(sd, PENALTY_DROP, mob);
			}
#endif

			for (const std::shared_ptr<s_mob_drop>& entry : mob->dropitem) {
				if (entry->nameid == 0 || entry->rate < 1)
					continue;

				std::shared_ptr<item_data> id = item_db.find(entry->nameid);

				if (id == nullptr)
					continue;

				int32 droprate = mob_getdroprate(sd, mob, entry->rate, drop_modifier);

				if (event_drop) { // Stingor
					droprate = apply_rate(droprate, battle_config.mult_eventdrop);
					droprate = min(droprate, 10000); //cap it to 100%
				}

				sprintf(atcmd_output2, " - %s  %02.02f%%", item_db.create_item_link(id).c_str(), (float)droprate / 100);
				strcat(atcmd_output, atcmd_output2);
				if (++j % 3 == 0) {
					clif_displaymessage(fd, atcmd_output);
					strcpy(atcmd_output, " ");
				}
			}

			if (j % 3 != 0) {
				clif_displaymessage(fd, atcmd_output);
			}
		}

		// mvp
		if (mob->get_bosstype() == BOSSTYPE_MVP) {
			sprintf(atcmd_output, msg_txt(sd, 1247), mob->mexp); //  MVP Bonus EXP:%llu
			clif_displaymessage(fd, atcmd_output);
			clif_displaymessage(fd, msg_txt(sd, 1248)); //  MVP drops:
			strcpy(atcmd_output, " ");

			if (mob->mvpitem.empty()) {
				clif_displaymessage(fd, msg_txt(sd, 1249)); // This monster has no MVP drops.
			}
			else {
				float mvpremain = 100.0; //Remaining drop chance for official mvp drop mode
				uint32 j = 0;
				for (const std::shared_ptr<s_mob_drop>& entry : mob->mvpitem) {
					if (entry->nameid == 0)
						continue;

					std::shared_ptr<item_data> id = item_db.find(entry->nameid);

					if (id == nullptr)
						continue;

					//Because if there are 3 MVP drops at 50%, the first has a chance of 50%, the second 25% and the third 12.5%
					float mvppercent = (float)entry->rate * mvpremain / 10000.0f;
					if (battle_config.item_drop_mvp_mode == 0) {
						mvpremain -= mvppercent;
					}
					if (mvppercent > 0) {
						sprintf(atcmd_output2, " - %s  %02.02f%%", item_db.create_item_link(id).c_str(), mvppercent);
						strcat(atcmd_output, atcmd_output2);
						if (++j % 3 == 0) {
							clif_displaymessage(fd, atcmd_output);
							strcpy(atcmd_output, " ");
						}
					}
				}

				if (j % 3 != 0) {
					clif_displaymessage(fd, atcmd_output);
				}
			}
		}
	}
	return 0;
}
/*==========================================
 * Show Items DB Info   v 1.0
 * originally by [Lupus]
 *------------------------------------------*/
ACMD_FUNC(iteminfo)
{
	if (!message || !*message) {
		clif_displaymessage(fd, msg_txt(sd, 1276)); // Please enter an item name/ID (usage: @ii/@iteminfo <item name/ID>).
		return -1;
	}

	std::map<t_itemid, std::shared_ptr<item_data>> item_array = {};
	uint16 count = 1;
	t_itemid itemid = strtoul(message, nullptr, 10);

	if (itemid == 0) // Entered a string
		count = itemdb_searchname_array(item_array, MAX_SEARCH, message);
	else {
		if ((item_array[0] = item_db.find(itemid)) == nullptr)
			count = 0;
	}

	if (!count) {
		clif_displaymessage(fd, msg_txt(sd, 19));	// Invalid item ID or name.
		return -1;
	}

	if (count == MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(sd, 269), MAX_SEARCH); // Displaying first %d matches
		clif_displaymessage(fd, atcmd_output);
	}
	for (const auto& result : item_array) {
		std::shared_ptr<item_data> item_data = result.second;

		sprintf(atcmd_output, msg_txt(sd, 1277), // Item: '%s'/'%s' (%u) Type: %s | Extra Effect: %s
			item_db.create_item_link(item_data).c_str(), item_data->ename.c_str(), item_data->nameid,
			(item_data->type != IT_AMMO) ? itemdb_typename((enum item_types)item_data->type) : itemdb_typename_ammo((e_ammo_type)item_data->subtype),
			(item_data->script == nullptr) ? msg_txt(sd, 1278) : msg_txt(sd, 1279) // None / With script
		);
		clif_displaymessage(fd, atcmd_output);

		sprintf(atcmd_output, msg_txt(sd, 1280), item_data->value_buy, item_data->value_sell, item_data->weight / 10.); // NPC Buy:%dz, Sell:%dz | Weight: %.1f
		clif_displaymessage(fd, atcmd_output);

		if (item_data->maxchance == -1) {
			strcpy(atcmd_output, msg_txt(sd, 1281)); //  - Available in the shops only.
			clif_displaymessage(fd, atcmd_output);
		}
		else if (!battle_config.atcommand_mobinfo_type) {
			if (item_data->maxchance)
				sprintf(atcmd_output, msg_txt(sd, 1282), (float)item_data->maxchance / 100); //  - Maximal monsters drop chance: %02.02f%%
			else
				strcpy(atcmd_output, msg_txt(sd, 1283)); //  - Monsters don't drop this item.
			clif_displaymessage(fd, atcmd_output);
		}
	}
	return 0;
}

/*==========================================
 * Show who drops the item.
 *------------------------------------------*/
ACMD_FUNC(whodrops)
{
	if (!message || !*message) {
		clif_displaymessage(fd, msg_txt(sd, 1284)); // Please enter item name/ID (usage: @whodrops <item name/ID>).
		return -1;
	}

	std::map<t_itemid, std::shared_ptr<item_data>> item_array = {};
	uint16 count = 1;
	t_itemid itemid = strtoul(message, nullptr, 10);

	if (itemid == 0) // Entered a string
		count = itemdb_searchname_array(item_array, MAX_SEARCH, message);
	else {
		if ((item_array[0] = item_db.find(itemid)) == nullptr)
			count = 0;
	}

	if (!count) {
		clif_displaymessage(fd, msg_txt(sd, 19));	// Invalid item ID or name.
		return -1;
	}

	if (count == MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(sd, 269), MAX_SEARCH); // Displaying first %d matches
		clif_displaymessage(fd, atcmd_output);
	}
	for (const auto& result : item_array) {
		std::shared_ptr<item_data> id = result.second;

		sprintf(atcmd_output, msg_txt(sd, 1285), item_db.create_item_link(id).c_str(), id->nameid); // Item: '%s' (ID:%u)
		clif_displaymessage(fd, atcmd_output);

		if (id->mob[0].chance == 0) {
			strcpy(atcmd_output, msg_txt(sd, 1286)); //  - Item is not dropped by mobs.
			clif_displaymessage(fd, atcmd_output);
		}
		else {
			sprintf(atcmd_output, msg_txt(sd, 1287), MAX_SEARCH); //  - Common mobs with highest drop chance (only max %d are listed):
			clif_displaymessage(fd, atcmd_output);

			for (uint16 j = 0; j < MAX_SEARCH && id->mob[j].chance > 0; j++)
			{
				int32 dropchance = id->mob[j].chance;
				int32 drop_rate_bonus = 0;
				int32 mobid = id->mob[j].id;

				std::shared_ptr<s_mob_db> mob = mob_db.find(id->mob[j].id);
				if (!mob) continue;
				// Add class and race specific bonuses
				drop_rate_bonus += sd->indexed_bonus.dropaddclass[mob->status.class_] + sd->indexed_bonus.dropaddclass[CLASS_ALL];
				drop_rate_bonus += sd->indexed_bonus.dropaddrace[mob->status.race] + sd->indexed_bonus.dropaddrace[RC_ALL];

				// Increase drop rate if user has SC_ITEMBOOST
				if (sd->sc.getSCE(SC_ITEMBOOST))
					drop_rate_bonus += sd->sc.getSCE(SC_ITEMBOOST)->val1;

				drop_rate_bonus = (int)(0.5 + dropchance * drop_rate_bonus / 100.);
				dropchance = i32max(dropchance, cap_value(drop_rate_bonus, 0, 9000));

#ifdef RENEWAL_DROP
				if (battle_config.atcommand_mobinfo_type) {
					dropchance = dropchance * pc_level_penalty_mod(sd, PENALTY_DROP, mob) / 100;
					if (dropchance <= 0 && !battle_config.drop_rate0item)
						dropchance = 1;
				}
#endif
				if (pc_isvip(sd)) // Display item rate increase for VIP
					dropchance += (dropchance * battle_config.vip_drop_increase) / 100;

				if (event_drop) // Stingor
					dropchance = apply_rate(dropchance, battle_config.mult_eventdrop);

				dropchance = min(dropchance, 10000); //cap it to 100%

				const std::vector<spawn_info> spawns = mob_get_spawns(mobid);
				if (spawns.size() < 1)
					sprintf(atcmd_output, "- %s (%d): %02.02f%% - %s", mob->jname.c_str(), mobid, (dropchance / 100.), msg_txt(sd, 1290)); // Instance/Quest/Event
				else if (spawns.size() == 1) {
					int16 mapid = map_mapindex2mapid(spawns[0].mapindex);
					if (mapid > 1) {
							const char* display = mapindex_idx2displayname(spawns[0].mapindex);
							const char* internal = map_getmapdata(mapid)->name;
							char location[MAP_DISPLAY_NAME_LENGTH + MAP_NAME_LENGTH + 3];
							if (display && strcmpi(display, internal) != 0)
								safesnprintf(location, sizeof(location), "%s (%s)", display, internal);
							else
								safestrncpy(location, display ? display : internal, sizeof(location));
							if (spawns[0].qty > 1)
								sprintf(atcmd_output, "- %s (%d): %02.02f%% - %d mobs dans %s", mob->jname.c_str(), mobid, (dropchance / 100.), spawns[0].qty, location);
							else
								sprintf(atcmd_output, "- %s (%d): %02.02f%% - 1 mob dans %s", mob->jname.c_str(), mobid, (dropchance / 100.), location);
						}
				}
				else if (spawns.size() < 4) {
					char buf[CHAT_SIZE_MAX];

					sprintf(atcmd_output, "- %s (%d): %02.02f%% - ", mob->jname.c_str(), mobid, (dropchance / 100.));

					for (auto& spawn : spawns)
					{
						int16 mapid = map_mapindex2mapid(spawn.mapindex);
						if (mapid < 0)
							continue;
						safesnprintf(buf, sizeof(buf), "[%d->%s]", spawn.qty, map_getmapdata(mapid)->name);
						strcat(atcmd_output, buf);
					}
				}
				else
					sprintf(atcmd_output, "- %s (%d): %02.02f%% - Respawn dans %zd maps - @whereis %d", mob->jname.c_str(), mobid, (dropchance / 100.), spawns.size(), mobid);

				clif_displaymessage(fd, atcmd_output);
			}
		}
	}
	return 0;
}

ACMD_FUNC(whereis)
{
	uint16 mob_ids[MAX_SEARCH] = { 0 }, count;

	if (!message || !*message) {
		clif_displaymessage(fd, msg_txt(sd, 1288)); // Please enter a monster name/ID (usage: @whereis <monster_name_or_monster_ID>).
		return -1;
	}

	int32 i_message = atoi(message);
	if (mobdb_checkid(i_message)) {
		// ID given
		mob_ids[0] = i_message;
		count = 1;
	}
	else {
		// Name given, get all monster associated whith this name
		count = mobdb_searchname_array(message, mob_ids, MAX_SEARCH);
	}

	if (count <= 0) {
		clif_displaymessage(fd, msg_txt(sd, 40)); // Invalid monster ID or name.
		return -1;
	}

	if (count >= MAX_SEARCH) {
		sprintf(atcmd_output, msg_txt(sd, 269), MAX_SEARCH); // Displaying first %d matches
		clif_displaymessage(fd, atcmd_output);
		count = MAX_SEARCH;
	}

	for (int i = 0; i < count; i++) {
		uint16 mob_id = mob_ids[i];
		std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);

		if (!mob) continue;
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(sd, 1289), mob->jname.c_str()); // %s spawns in:
		clif_displaymessage(fd, atcmd_output);

		const std::vector<spawn_info> spawns = mob_get_spawns(mob_id);
		if (spawns.size() <= 0) {
			// This monster does not spawn normally.
			clif_displaymessage(fd, msg_txt(sd, 1290));
		}
		else {
			for (auto& spawn : spawns)
				{
					int16 mapid = map_mapindex2mapid(spawn.mapindex);
					if (mapid < 0)
						continue;
					const char* display = mapindex_idx2displayname(spawn.mapindex);
					const char* internal = map_getmapdata(mapid)->name;
					char location[MAP_DISPLAY_NAME_LENGTH + MAP_NAME_LENGTH + 3];
					if (display && strcmpi(display, internal) != 0)
						safesnprintf(location, sizeof(location), "%s (%s)", display, internal);
					else
						safestrncpy(location, display ? display : internal, sizeof(location));
					snprintf(atcmd_output, sizeof atcmd_output, "%s (%d)", location, spawn.qty);
					clif_displaymessage(fd, atcmd_output);
				}
		}
	}

	return 0;
}
