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
 * [Stingor] num == 0/empty -> storage standard, num == 1~5 -> storage alternatif correspondant
 *------------------------------------------*/
ACMD_FUNC(storeall)
{
	int32 num = 0;
	const char* text;
	nullpo_retr(-1, sd);

	if (message && *message)
		sscanf(message, "%11d", &num);

	if (num < 0 || num > 5 || (num > 0 && !storage_exists((uint8)num))) {
		text = atcommand_help_string(command);
		if (text)
			clif_displaymessage(fd, text);
		return -1;
	}

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

	if (num > 0) { // [Stingor] Storage alternatif
		if (sd->state.pending_storeall) {
			clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
			return -1;
		}

		// storage_premiumStorage_load() may only fire an async request to the char-server
		// when switching to a storage number that isn't already cached: the transfer itself
		// is deferred to storage_premiumStorage_open(), which runs once it's truly loaded,
		// whether that happens synchronously (right here) or later (async response).
		sd->state.pending_storeall = true;

		if (storage_premiumStorage_load(sd, (uint8)num, STOR_MODE_PUT) == 0) {
			sd->state.pending_storeall = false;
			clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
			return -1;
		}

		return 0;
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
		if( sd2->state.spectator )
			continue;// a viewpoint, not a player (see SPECTATOR_USERID in mmo.hpp)
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
		// Spectators are not players (see SPECTATOR_USERID in mmo.hpp), and the
		// hidden-session test below cannot catch them: it hides an invisible
		// player only from LOWER group levels, and a spectator has no group at
		// all — so it would show up for everyone.
		if (pl_sd->state.spectator)
			continue;
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

	char path[1024];
	atcommand_normalize_npc_path(message, path, sizeof(path));

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

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_SELLSTUFF);
	return 0;
}

/*==========================================
 * @sellitem par [Stingor]
 * Protége le joueur de vendre certains items importants au npc.
 *----------------------------------------------------*/
ACMD_FUNC(sellitem)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_SELLITEM);
	return 0;
}

/*==========================================
 * @showspeed par [Stingor]
 * Affiche la valeur du changement de vitesse.
 *---------------------------------------------------*/
ACMD_FUNC(showspeed)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_SHOWSPEED);
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
 * @playertest par [Stingor]
 * Neutralise les deux bypass GM du combat pour tester avec la cadence d'un
 * joueur normal, sans changer de personnage :
 *   - pc_maxaspd()    : battle_config.gm_aspd (group_level >= 80)
 *   - skill_delayfix(): battle_config.gm_delay_rate (group_level >= 80)
 * L'equipement et les stats du personnage restent ceux du GM : la commande ne
 * simule pas un joueur, elle retire seulement le plafond artificiel.
 *----------------------------------------------------*/
ACMD_FUNC(playertest)
{
	nullpo_retr(-1, sd);

	sd->state.playertest = !sd->state.playertest;
	// Persiste par personnage : le mode survit a la deconnexion, cf. pc_reg_received().
	pc_setglobalreg(sd, add_str(PLAYERTEST_VAR), sd->state.playertest ? 1 : 0);

	// L'ASPD depend de pc_maxaspd() : il faut un recalcul complet pour que le
	// nouveau plafond soit applique et repousse au client.
	status_calc_pc(sd, SCO_FORCE);
	clif_updatestatus(*sd, SP_ASPD);

	int32 amotion = sd->battle_status.amotion;
	int32 aspd = (AMOTION_ZERO_ASPD - amotion * AMOTION_DIVIDER_PC) / AMOTION_INTERVAL;

	if (sd->state.playertest)
		clif_displaymessage(fd, "@playertest ON : plafonds GM d'ASPD et de delay de skill ignor\xe9" "s.");
	else
		clif_displaymessage(fd, "@playertest OFF : plafonds GM r\xe9" "tablis.");

	sprintf(atcmd_output, "ASPD %d (amotion %d ms, adelay %d ms) - delay de skill : %s",
		aspd, amotion, sd->battle_status.adelay,
		sd->state.playertest ? "calcul normal" : "forc\xe9 \xe0 gm_delay");
	clif_displaymessage(fd, atcmd_output);

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

	// ⚠ `pl_sd`, PAS `sd` : `skill_sit` pose (ou retire) les bonus d'assise
	// (Tumbling, Moonlit Petals). Ecrite avec `sd`, la commande les appliquait au
	// GM qui l'emploie au lieu du joueur qu'il assoit.
	if( pc_issit(pl_sd) ) {
		pc_setstand(pl_sd, true);
		skill_sit(pl_sd, false);
		clif_standing(*pl_sd);
	}
	else {
		pc_setsit(pl_sd);
		skill_sit(pl_sd, true);
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

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_ALOOT_MVP);
	return 0;
}

 /*==========================================
 * @autolootmvpreward par [Stingor]
 * Loot only reward mvps
 *------------------------------------------*/
ACMD_FUNC(autolootmvpreward)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_ALOOT_MVPRWD);
	return 0;
}

/*==========================================
 * @autolootrare par [Stingor]
 * Loot only rare items in a db
 *------------------------------------------*/
ACMD_FUNC(autolootrare)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_ALOOT_RARE);
	return 0;
}

/*==========================================
 * @showmobinfo thx [Doo~]
 * Loot only rare items in a db
 *------------------------------------------*/
ACMD_FUNC(showmobinfo)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_SHOW_MOB_INFO);
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
			bourgeon_setting_set(sd, BOURGEON_SETTING_ALOOT_POGNON, 0, BSET_SRC_SERVER);
		}
		else {
			int32 prix;

			if( (sscanf(message, "%d", &prix) < 1) || prix <= 0 ) {
				bourgeon_setting_set(sd, BOURGEON_SETTING_ALOOT_POGNON, 0, BSET_SRC_SERVER);
				clif_displaymessage(fd, msg_txt(sd,1828));
				return -1;
			}

			bourgeon_setting_set(sd, BOURGEON_SETTING_ALOOT_POGNON, (uint32)prix, BSET_SRC_SERVER);
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

	if (sd->state.storage_flag == 1) {
		storage_storageclose(sd);
		return 0;
	}
	if (sd->state.storage_flag == 3) {
		storage_premiumStorage_close(sd);
		return 0;
	}

	if (storage_premiumStorage_load(sd, num, STOR_MODE_ALL) == 0) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
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

	if (sd->state.storage_flag == 1) {
		storage_storageclose(sd);
		return 0;
	}
	if (sd->state.storage_flag == 3) {
		storage_premiumStorage_close(sd);
		return 0;
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

// ── [Stingor] Bourgeon : @whodrops / @whereis en LIENS ────────────────────
//
// Les deux commandes ne parlent que de choses que l'interface moderne sait
// ouvrir — des monstres et des cartes — mais elles les écrivaient à plat. La
// chatbox de Bourgeon connaît déjà les balises (`<MOBL>` pour un monstre,
// `<NAVIL>` pour un lieu) : il suffit de les poser ici.
//
// 🔴 GATÉ SUR LA CAPACITÉ, pas sur `has_bourgeon`. La chatbox moderne est
// opt-in et peut être éteinte en cours de session (cf. `e_bourgeon_ui_cap`) ;
// un client qui ne la rend pas lirait les chevrons en toutes lettres. La sortie
// de ces commandes est un message SELF : personne d'autre ne la lit, donc le
// choix se fait une fois pour toute la commande, sans version de repli à poster
// en parallèle.
static bool bourgeon_chat_markup(map_session_data* sd) {
	return sd != nullptr && (sd->bourgeon_ui_caps & BOURGEON_UI_CHAT) != 0;
}

// Le nom d'un monstre. En lien, le RANG voyage dans la balise (0 normal, 1 boss,
// 2 MVP) et c'est le LECTEUR qui compose sa vignette — « <[MVP] Golden Thief
// Bug> » ; en clair, on garde le préfixe écrit à la main, tel qu'il l'était.
//
// 🔴 Le nom voyage DANS la balise, et ce n'est pas de la commodité : le client
// n'a pas mob_db, il ne sait pas nommer un monstre à partir de son seul id.
static std::string bourgeon_mob_link(map_session_data* sd, uint16 mob_id, const std::shared_ptr<s_mob_db>& mob) {
	int32 rank = 0;
	switch (mob->get_bosstype()) {
		case BOSSTYPE_MVP:      rank = 2; break;
		case BOSSTYPE_MINIBOSS: rank = 1; break;
		default: break;
	}
	char buf[NAME_LENGTH + 64];
	if (bourgeon_chat_markup(sd))
		safesnprintf(buf, sizeof(buf), "<MOBL>%u:%d:%s</MOBL>", (uint32)mob_id, rank, mob->jname.c_str());
	else
		safesnprintf(buf, sizeof(buf), "%s%s", rank == 2 ? "[MVP] " : (rank == 1 ? "[Mini-Boss] " : ""), mob->jname.c_str());
	return buf;
}

// Un lieu, en lien de NAVIGATION. `<NAVIL>` est la balise du CLIENT : le nom
// INTERNE collé aux quatre caractères base 62 des coordonnées, « 0000 » valant
// la carte entière — c'est exactement ce qu'on veut d'un lieu d'apparition, dont
// on connaît une zone et non un point. Le nom AFFICHÉ n'est pas dans la balise et
// n'a pas à y être : chacun le résout dans SA langue.
//
// Rend une chaîne VIDE si la carte n'est pas chargée sur ce serveur — à
// l'appelant de dire autre chose plutôt que d'afficher un lieu vide.
static std::string bourgeon_map_link(map_session_data* sd, uint16 mapindex) {
	const int16 mapid = map_mapindex2mapid(mapindex);
	if (mapid < 0)
		return std::string();
	const char* internal = map_getmapdata(mapid)->name;
	const char* display  = mapindex_idx2displayname(mapindex);
	char buf[MAP_DISPLAY_NAME_LENGTH + MAP_NAME_LENGTH + 24];
	if (bourgeon_chat_markup(sd))
		safesnprintf(buf, sizeof(buf), "<NAVIL>%s0000</NAVIL>", internal);
	else if (display && strcmpi(display, internal) != 0)
		safesnprintf(buf, sizeof(buf), "%s (%s)", display, internal);
	else
		safestrncpy(buf, display ? display : internal, sizeof(buf));
	return buf;
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

				// Le nom du monstre : lien cliquable pour la chatbox moderne,
				// « [MVP] Nom » en clair pour les autres (cf. bourgeon_mob_link).
				const std::string mobname = bourgeon_mob_link(sd, (uint16)mobid, mob);

				const std::vector<spawn_info> spawns = mob_get_spawns(mobid);
				if (spawns.size() < 1)
					safesnprintf(atcmd_output, sizeof(atcmd_output), "- %s (%d): %02.02f%% - %s", mobname.c_str(), mobid, (dropchance / 100.), msg_txt(sd, 1290)); // Instance/Quest/Event
				else if (spawns.size() == 1) {
					// ⚠ Une carte non chargée laissait atcmd_output tel quel, donc la
					// ligne PRÉCÉDENTE était réaffichée à l'identique : on retombe
					// désormais sur la mention sans lieu.
					const std::string location = bourgeon_map_link(sd, spawns[0].mapindex);
					if (location.empty())
						safesnprintf(atcmd_output, sizeof(atcmd_output), "- %s (%d): %02.02f%% - %s", mobname.c_str(), mobid, (dropchance / 100.), msg_txt(sd, 1290)); // Instance/Quest/Event
					else if (spawns[0].qty > 1)
						safesnprintf(atcmd_output, sizeof(atcmd_output), "- %s (%d): %02.02f%% - %d mobs dans %s", mobname.c_str(), mobid, (dropchance / 100.), spawns[0].qty, location.c_str());
					else
						safesnprintf(atcmd_output, sizeof(atcmd_output), "- %s (%d): %02.02f%% - 1 mob dans %s", mobname.c_str(), mobid, (dropchance / 100.), location.c_str());
				}
				else if (spawns.size() < 4) {
					char buf[CHAT_SIZE_MAX];

					safesnprintf(atcmd_output, sizeof(atcmd_output), "- %s (%d): %02.02f%% - ", mobname.c_str(), mobid, (dropchance / 100.));

					for (auto& spawn : spawns)
					{
						const std::string location = bourgeon_map_link(sd, spawn.mapindex);
						if (location.empty())
							continue;
						safesnprintf(buf, sizeof(buf), "[%d->%s]", spawn.qty, location.c_str());
						// La balise allonge la ligne : on borne l'ajout, là où le
						// `strcat` écrivait au-delà du tampon sans le dire.
						strncat(atcmd_output, buf, sizeof(atcmd_output) - strlen(atcmd_output) - 1);
					}
				}
				else
					safesnprintf(atcmd_output, sizeof(atcmd_output), "- %s (%d): %02.02f%% - Respawn dans %zd maps - @whereis %d", mobname.c_str(), mobid, (dropchance / 100.), spawns.size(), mobid);

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
		// Le monstre demandé est lui aussi un lien : c'est la première chose
		// qu'on veut ouvrir quand la liste des lieux ne suffit pas.
		snprintf(atcmd_output, sizeof atcmd_output, msg_txt(sd, 1289), bourgeon_mob_link(sd, mob_id, mob).c_str()); // %s spawns in:
		clif_displaymessage(fd, atcmd_output);

		const std::vector<spawn_info> spawns = mob_get_spawns(mob_id);
		if (spawns.size() <= 0) {
			// This monster does not spawn normally.
			clif_displaymessage(fd, msg_txt(sd, 1290));
		}
		else {
			for (auto& spawn : spawns)
				{
					const std::string location = bourgeon_map_link(sd, spawn.mapindex);
					if (location.empty())
						continue;
					snprintf(atcmd_output, sizeof atcmd_output, "%s (%d)", location.c_str(), spawn.qty);
					clif_displaymessage(fd, atcmd_output);
				}
		}
	}

	return 0;
}

/**
 * @separate - Toggle kill separator display
 * Displays a separator line after each mob kill for easier drop tracking
 * Useful for level 999+ characters
 **/
ACMD_FUNC(separate)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_SEPARATE);
	return 0;
}

// ── @homskin ──────────────────────────────────────────────────────────────────

ACMD_FUNC(homskin)
{
	nullpo_retr(-1, sd);

	if( !sd->hd ) {
		clif_displaymessage(fd, "Vous n'avez pas d'homonculus actif.");
		return -1;
	}

	// @homskin sans argument → liste tous les sprites valides (via hom_get_viewdata)
	if( !message || !*message ) {
		clif_displaymessage(fd, "Usage : @homskin <class_id> | @homskin reset");
		clif_displaymessage(fd, "-- Sprites homonculus disponibles --");
		static const struct { int32 id; const char* name; } hom_list[] = {
			{ MER_LIF,           "Lif"              }, { MER_AMISTR,         "Amistr"            },
			{ MER_FILIR,         "Filir"             }, { MER_VANILMIRTH,     "Vanilmirth"        },
			{ MER_LIF2,          "Lif (variante)"    }, { MER_AMISTR2,        "Amistr (variante)" },
			{ MER_FILIR2,        "Filir (variante)"  }, { MER_VANILMIRTH2,    "Vanilmirth (var.)" },
			{ MER_LIF_H,         "Lif evolue"        }, { MER_AMISTR_H,       "Amistr evolue"     },
			{ MER_FILIR_H,       "Filir evolue"      }, { MER_VANILMIRTH_H,   "Vanilmirth evolue" },
			{ MER_LIF_H2,        "Lif evolue (var.)" }, { MER_AMISTR_H2,      "Amistr evo (var.)" },
			{ MER_FILIR_H2,      "Filir evo (var.)"  }, { MER_VANILMIRTH_H2,  "Vanil. evo (var.)" },
			{ MER_EIRA,          "Eira [Hom S]"      }, { MER_BAYERI,         "Bayeri [Hom S]"    },
			{ MER_SERA,          "Sera [Hom S]"      }, { MER_DIETER,         "Dieter [Hom S]"    },
			{ MER_ELEANOR,       "Eleanor [Hom S]"   },
		};
		char buf[64];
		for( const auto& e : hom_list ) {
			if( !hom_get_viewdata(e.id) ) continue; // sprite non chargé
			snprintf(buf, sizeof(buf), "  %d  %s", e.id, e.name);
			clif_displaymessage(fd, buf);
		}
		return -1;
	}

	// @homskin reset / 0 → retire le skin
	if( strncmpi(message, "reset", 5) == 0 || atoi(message) == 0 ) {
		pc_setglobalreg(sd, add_str("homskin"), 0);
		// Restaure le view data réel de l'homonculus
		status_set_viewdata(sd->hd, sd->hd->homunculus.class_);
		unit_refresh(sd->hd);
		clif_displaymessage(fd, "Skin de l'homonculus r\xe9" "initialis\xe9.");
		return 0;
	}

	int32 class_id = atoi(message);
	if( !hom_get_viewdata(class_id) ) {
		char buf[64];
		snprintf(buf, sizeof(buf), "ID %d invalide (sprite introuvable).", class_id);
		clif_displaymessage(fd, buf);
		return -1;
	}

	pc_setglobalreg(sd, add_str("homskin"), class_id);
	status_set_viewdata(sd->hd, class_id);
	unit_refresh(sd->hd);

	char buf[96];
	snprintf(buf, sizeof(buf), "Skin de l'homonculus chang\xe9 : ID %d.", class_id);
	clif_displaymessage(fd, buf);
	return 0;
}

// ── @blockexp ────────────────────────────────────────────────────────────────

ACMD_FUNC(blockexp)
{
	nullpo_retr(-1, sd);

	// Registre, message et interface du client suivent : tout est décrit
	// dans bourgeon_settings[] (clif.cpp).
	bourgeon_setting_toggle(sd, BOURGEON_SETTING_BLOCK_EXP);
	return 0;
}

// ── @flywinglast ─────────────────────────────────────────────────────────────

// Marque ou non, sur la mini-carte, la position d'arrivée d'une Fly Wing ou
// d'un Téléport niveau 1 (cf. SkillTeleport::castendNoDamageId).
ACMD_FUNC(flywinglast)
{
	nullpo_retr(-1, sd);

	bourgeon_setting_toggle(sd, BOURGEON_SETTING_FLYWING_LAST);
	return 0;
}

// ── @tri_inventaire / @tri_cart / @tri_storage / @tri_gstorage ───────────────

static e_sort_mode atcmd_parse_sort_mode(const char* msg)
{
	if( !msg || !*msg ) return (e_sort_mode)255;
	if( strncmpi(msg, "defaut",   6) == 0 ) return SORT_NAMEID;
	if( strncmpi(msg, "type",     4) == 0 ) return SORT_TYPE;
	if( strncmpi(msg, "quantite", 8) == 0 ) return SORT_AMOUNT;
	if( strncmpi(msg, "poids",    5) == 0 ) return SORT_WEIGHT;
	if( strncmpi(msg, "prix",     4) == 0 ) return SORT_PRICE;
	if( strncmpi(msg, "nom",      3) == 0 ) return SORT_NAME;
	if( strncmpi(msg, "aucun",    5) == 0 ) return SORT_NONE;
	return (e_sort_mode)255; // invalide
}

static void atcmd_show_sort_usage(const int32 fd, map_session_data* sd, const char* cmd)
{
	char buf[160];
	snprintf(buf, sizeof(buf), "Usage : @%s <defaut|type|quantite|poids|prix|nom|aucun>", cmd);
	clif_displaymessage(fd, buf);
}


ACMD_FUNC(tri_inventaire)
{
	nullpo_retr(-1, sd);
	e_sort_mode mode = atcmd_parse_sort_mode(message);
	if( mode == (e_sort_mode)255 ) { atcmd_show_sort_usage(fd, sd, "tri_inventaire"); return -1; }
	bourgeon_setting_set(sd, BOURGEON_SETTING_TRI_INV, (uint32)mode, BSET_SRC_SERVER);
	return 0;
}

ACMD_FUNC(tri_cart)
{
	nullpo_retr(-1, sd);
	e_sort_mode mode = atcmd_parse_sort_mode(message);
	if( mode == (e_sort_mode)255 ) { atcmd_show_sort_usage(fd, sd, "tri_cart"); return -1; }
	bourgeon_setting_set(sd, BOURGEON_SETTING_TRI_CART, (uint32)mode, BSET_SRC_SERVER);
	return 0;
}

ACMD_FUNC(tri_storage)
{
	nullpo_retr(-1, sd);
	if( sd->state.storage_flag ) { clif_displaymessage(fd, "Fermez d'abord votre coffre."); return -1; }
	e_sort_mode mode = atcmd_parse_sort_mode(message);
	if( mode == (e_sort_mode)255 ) { atcmd_show_sort_usage(fd, sd, "tri_storage"); return -1; }
	bourgeon_setting_set(sd, BOURGEON_SETTING_TRI_STORAGE, (uint32)mode, BSET_SRC_SERVER);
	return 0;
}

ACMD_FUNC(tri_gstorage)
{
	nullpo_retr(-1, sd);
	if( sd->state.storage_flag ) { clif_displaymessage(fd, "Fermez d'abord votre coffre."); return -1; }
	e_sort_mode mode = atcmd_parse_sort_mode(message);
	if( mode == (e_sort_mode)255 ) { atcmd_show_sort_usage(fd, sd, "tri_gstorage"); return -1; }
	bourgeon_setting_set(sd, BOURGEON_SETTING_TRI_GSTORAGE, (uint32)mode, BSET_SRC_SERVER);
	return 0;
}

/*==========================================
 * [Stingor] @ignore <nom> : masque TOUT le chat d'une personne
 *
 * Là où /ex ne bloque que les chuchotements, @ignore rend la cible entièrement
 * muette pour l'auteur de la commande : chat de zone, chatroom, chuchotements,
 * party, guilde, clan, BG, channels et texte au-dessus de la tête.
 *
 * L'ignore porte sur le compte Moonlight, pas sur le personnage nommé : la cible
 * reste masquée sur tous ses persos et tous ses comptes de jeu, et la liste vous
 * suit sur les vôtres. Persistée dans `user_ignore` (cf. pc_ignorechat_*).
 *------------------------------------------*/

/**
 * Extrait le nom de personnage du paramètre de la commande.
 * Les noms peuvent contenir des espaces, on ne coupe donc qu'aux extrémités.
 * @param message: Paramètre brut de la commande
 * @param out_name: Nom extrait (taille NAME_LENGTH)
 * @return false si aucun nom exploitable
 */
static bool atcommand_ignore_parse( const char* message, char* out_name ){
	memset( out_name, '\0', NAME_LENGTH );

	if( message == nullptr || *message == '\0' ){
		return false;
	}

	while( *message == ' ' ){
		message++;
	}

	if( sscanf( message, "%23[^\n]", out_name ) < 1 ){
		return false;
	}

	size_t len = strlen( out_name );

	while( len > 0 && out_name[len - 1] == ' ' ){
		out_name[--len] = '\0';
	}

	return len > 0;
}

ACMD_FUNC(ignore)
{
	char name[NAME_LENGTH];

	nullpo_retr(-1, sd);

	if( !atcommand_ignore_parse( message, name ) ){
		clif_displaymessage( fd, msg_txt( sd, 1870 ) ); // Usage: @ignore <character name>
		return -1;
	}

	// name est réécrit avec le nom exact au passage : corrige la casse saisie.
	uint32 target_id = pc_ignorechat_name2userid( name, name );

	if( target_id == 0 ){
		sprintf( atcmd_output, msg_txt( sd, 1875 ), name ); // Character '%s' not found.
		clif_displaymessage( fd, atcmd_output );
		return -1;
	}

	switch( pc_ignorechat_add( sd, target_id, name ) ){
		case IGNORECHAT_ADD_SELF:
			clif_displaymessage( fd, msg_txt( sd, 1874 ) ); // You cannot ignore yourself.
			return -1;

		// Le contrôle vit dans pc_ignorechat_add() et non ici : il passe par la
		// base, donc il vaut aussi pour un membre de l'équipe hors ligne.
		case IGNORECHAT_ADD_PROTECTED:
			clif_displaymessage( fd, msg_txt( sd, 1881 ) ); // You cannot ignore a member of the server staff.
			return -1;

		case IGNORECHAT_ADD_DUPLICATE:
			sprintf( atcmd_output, msg_txt( sd, 1872 ), name ); // You are already ignoring '%s'.
			clif_displaymessage( fd, atcmd_output );
			return -1;

		case IGNORECHAT_ADD_FULL:
			sprintf( atcmd_output, msg_txt( sd, 1873 ), MAX_IGNORECHAT_LIST ); // Your ignore list is full (%d entries maximum).
			clif_displaymessage( fd, atcmd_output );
			return -1;

		case IGNORECHAT_ADD_OK:
			sprintf( atcmd_output, msg_txt( sd, 1871 ), name ); // '%s' is now ignored: you will no longer see any of their messages.
			clif_displaymessage( fd, atcmd_output );
			return 0;
	}

	return -1;
}

ACMD_FUNC(unignore)
{
	char name[NAME_LENGTH];

	nullpo_retr(-1, sd);

	if( !atcommand_ignore_parse( message, name ) ){
		clif_displaymessage( fd, msg_txt( sd, 1876 ) ); // Usage: @unignore <character name>
		return -1;
	}

	// N'importe lequel des personnages de la personne ignorée fait l'affaire,
	// puisque tous mènent au même compte. L'échec de résolution n'est pas
	// bloquant : pc_ignorechat_del() se rabat alors sur le libellé stocké, ce qui
	// garde retirable une entrée dont le personnage a été supprimé entre-temps.
	uint32 target_id = pc_ignorechat_name2userid( name );

	if( !pc_ignorechat_del( sd, target_id, name ) ){
		sprintf( atcmd_output, msg_txt( sd, 1878 ), name ); // You are not ignoring '%s'.
		clif_displaymessage( fd, atcmd_output );
		return -1;
	}

	sprintf( atcmd_output, msg_txt( sd, 1877 ), name ); // '%s' is no longer ignored.
	clif_displaymessage( fd, atcmd_output );
	return 0;
}

ACMD_FUNC(ignorelist)
{
	nullpo_retr(-1, sd);

	int32 count = 0;

	while( count < MAX_IGNORECHAT_LIST && sd->ignoreChats[count].user_id != 0 ){
		count++;
	}

	if( count == 0 ){
		clif_displaymessage( fd, msg_txt( sd, 1879 ) ); // You are not ignoring anyone.
		return 0;
	}

	sprintf( atcmd_output, msg_txt( sd, 1880 ), count, MAX_IGNORECHAT_LIST ); // Ignored characters (%d/%d):
	clif_displaymessage( fd, atcmd_output );

	// Le libellé est le personnage principal de la personne visée ; ses autres
	// personnages sont couverts par la même entrée. Vide = compte sans aucun
	// personnage, on affiche son identifiant pour garder l'entrée identifiable.
	for( int32 i = 0; i < count; i++ ){
		if( sd->ignoreChats[i].name[0] != '\0' ){
			sprintf( atcmd_output, " - %s", sd->ignoreChats[i].name );
		}else{
			sprintf( atcmd_output, " - #%u", sd->ignoreChats[i].user_id );
		}

		clif_displaymessage( fd, atcmd_output );
	}

	return 0;
}

// ── [Stingor] Bourgeon : @shopsearch / @whosell — où acheter cet item ────────
//
// UNE seule commande pour les deux moitiés de la question. Le joueur ne cherche
// pas « un NPC » ou « un vendeur » : il cherche où acheter, et le NPC à 5000z
// ne vaut d'être connu qu'à côté du joueur qui brade à 3000z deux cases plus
// loin. Deux commandes séparées auraient forcé à les connaître toutes les deux
// et empêché la seule comparaison qui compte.
//
// 🔴 Côté NPC, on lit les boutiques EN MÉMOIRE, pas les scripts. Les shops
// remaniés à chaud (npcshopitem / npcshopadditem / npcshopdelitem) et les
// duplicates — qui partagent le tableau de leur original — sont donc vus tels
// qu'ils sont RÉELLEMENT au moment de la recherche, ce qu'un grep dans npc/ ne
// saurait pas faire. Reste hors de portée ce qu'aucune méthode ne verrait : un
// dialogue scripté qui fait getitem contre des zeny sans passer par une
// boutique.
//
// 🔴 La réponse nomme le NPC QUE LE JOUEUR VOIT, jamais la boutique interne.
// Une bonne part des boutiques du serveur sont FLOTTANTES (carte `-`) et ne
// s'ouvrent que par callshop depuis un script : « shop_mg (hors carte) » ne dit
// rien à personne. On remonte donc au NPC qui les ouvre, et c'est SA position
// qu'on affiche — cf. shopsearch_scan_script.
#define SHOPSEARCH_MAX_HITS 15		// lignes affichées par section et par item
#define SHOPSEARCH_MAX_MARKS 20		// points posés sur la minimap
#define SHOPSEARCH_TITLE_LENGTH 40	// titre de boutique joueur, tronqué

// Un lieu PRÉCIS, en lien de navigation. `bourgeon_map_link` vise la carte
// entière (« 0000 ») parce qu'un lieu d'apparition est une zone ; ici on connaît
// le point exact, alors on l'encode — base 62, poids faible d'abord, deux
// caractères par coordonnée : la forme qu'écrit le bouton « Partager » du
// client. La boucle native ne déborde qu'au-delà de 62², et une coordonnée de
// carte est bornée par 512, donc deux caractères suffisent toujours.
//
// Rend une chaîne VIDE pour un NPC hors carte — à l'appelant de dire autre
// chose plutôt que d'afficher un lieu vide.
static std::string bourgeon_pos_link(map_session_data* sd, int16 mapid, uint16 x, uint16 y) {
	if (mapid < 0)
		return std::string();

	const char* internal = map_getmapdata(mapid)->name;
	char buf[MAP_DISPLAY_NAME_LENGTH + MAP_NAME_LENGTH + 32];

	if (bourgeon_chat_markup(sd)) {
		static const char base62[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		const char coords[5] = { base62[x % 62], base62[(x / 62) % 62], base62[y % 62], base62[(y / 62) % 62], '\0' };

		safesnprintf(buf, sizeof(buf), "<NAVIL>%s%s</NAVIL>", internal, coords);
	}
	else {
		// Le nom INTERNE, pas le nom d'affichage : sans lien cliquable, la ligne
		// ne vaut que si elle se recopie telle quelle dans un @mapmove.
		safesnprintf(buf, sizeof(buf), "%s %d,%d", internal, x, y);
	}

	return buf;
}

// La monnaie dépend du type de boutique, et le prix seul serait trompeur : 200
// dans un pointshop n'est pas 200 zeny.
static std::string shopsearch_price(const npc_data* nd, const struct npc_item_list& entry) {
	char buf[128];

	switch (nd->subtype) {
		case NPCTYPE_CASHSHOP:
			safesnprintf(buf, sizeof(buf), "%u cash", entry.value);
			break;
		case NPCTYPE_ITEMSHOP: {
			std::shared_ptr<item_data> currency = item_db.find(nd->u.shop.itemshop_nameid);

			safesnprintf(buf, sizeof(buf), "%u x %s", entry.value, currency != nullptr ? currency->ename.c_str() : "???");
			break;
		}
		case NPCTYPE_POINTSHOP:
			safesnprintf(buf, sizeof(buf), "%u %s", entry.value, nd->u.shop.pointshop_str);
			break;
		default:
			safesnprintf(buf, sizeof(buf), "%uz", entry.value);
			break;
	}

	return buf;
}

// Le NPC à aller voir, et ce qu'il demande.
struct s_shopsearch_hit {
	npc_data* nd;			// nullptr : boutique flottante sans porteur identifié
	std::string label;		// nom TECHNIQUE de la boutique, seulement si nd est nul
	std::string price;
};

// Une boutique flottante en attente de porteur.
struct s_shopsearch_pending {
	std::string exname;
	std::string price;
};

// Un étal de joueur.
struct s_shopsearch_vend {
	map_session_data* sd;
	int16 amount;
	uint32 value;
};

// Quelles boutiques flottantes ce script ouvre-t-il ?
//
// callshop ne laisse aucun lien vers sa cible : le nom voyage en clair dans le
// bytecode, empilé comme littéral. On cherche donc ce littéral — l'octet C_STR
// suivi de la chaîne terminée par un zéro. C'est une HEURISTIQUE assumée : elle
// attrape aussi un nom seulement cité dans une chaîne, mais un nom de boutique
// flottante ne se promène pas ailleurs, et le pire cas est une ligne de trop.
// Décoder le flux d'opcodes serait exact, mais demanderait de recopier get_com()
// hors de script.cpp où il est privé : bien plus fragile pour le même résultat.
static void shopsearch_scan_script(const struct script_code* code, const std::set<std::string>& names, size_t shortest, size_t longest, std::vector<std::string>& found) {
	if (code == nullptr || code->script_buf == nullptr)
		return;

	for (int32 i = 0; i < code->script_size; i++) {
		if (code->script_buf[i] != C_STR)
			continue;

		const char* literal = (const char*)code->script_buf + i + 1;
		const size_t left = (size_t)(code->script_size - i - 1);
		// ⚠ Le tampon n'est pas garanti nul-terminé à la fin : sans cette borne,
		// un C_STR en dernier octet ferait lire au-delà.
		const size_t length = strnlen(literal, left);

		if (length == left)
			break;

		// Le test de longueur AVANT la construction de la std::string : l'octet 5
		// se rencontre partout dans du bytecode, et allouer à chaque rencontre
		// coûterait bien plus cher que la recherche elle-même.
		if (length >= shortest && length <= longest && names.count(literal) != 0) {
			found.push_back(literal);
			// Un vrai littéral : rien à y chercher. Ailleurs on avance d'UN octet,
			// car l'octet 5 croisé au milieu d'un nombre ou d'un nom n'est pas un
			// C_STR : sauter sa pseudo-longueur enjamberait un vrai littéral posé
			// juste derrière.
			i += (int32)length;
		}
	}
}

ACMD_FUNC(shopsearch)
{
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, msg_txt(sd, 1830)); // @shopsearch <nom ou ID d'item>
		return -1;
	}

	std::map<t_itemid, std::shared_ptr<item_data>> item_array = {};
	uint16 count = 1;
	t_itemid itemid = strtoul(message, nullptr, 10);

	if (itemid == 0) // Un nom a été saisi
		count = itemdb_searchname_array(item_array, MAX_SEARCH, message);
	else if ((item_array[0] = item_db.find(itemid)) == nullptr)
		count = 0;

	if (!count) {
		clif_displaymessage(fd, msg_txt(sd, 19)); // Invalid item ID or name.
		return -1;
	}

	if (count == MAX_SEARCH) {
		safesnprintf(atcmd_output, sizeof(atcmd_output), msg_txt(sd, 269), MAX_SEARCH); // Displaying first %d matches
		clif_displaymessage(fd, atcmd_output);
	}

	// UN SEUL balayage des NPC quel que soit le nombre d'items retenus : une
	// recherche par nom en ramène jusqu'à MAX_SEARCH, et id_db compte quelques
	// milliers d'entrées. Le set évite de rejouer la liste des items pour chaque
	// ligne de chaque boutique.
	//
	// ⚠ La clé d'`item_array` n'est PAS l'id de l'item quand la recherche s'est
	// faite par ID (rAthena y range le résultat en 0) : c'est `nameid` qui fait
	// foi, ici comme à l'affichage.
	std::set<t_itemid> wanted;

	for (const auto& result : item_array)
		wanted.insert(result.second->nameid);

	std::map<t_itemid, std::vector<s_shopsearch_hit>> hits;
	std::map<t_itemid, std::vector<s_shopsearch_pending>> pending;
	std::set<std::string> floating_names;

	auto record = [&](npc_data* nd, t_itemid nameid, const std::string& price) {
		if (nd->m < 0) {
			// Boutique flottante : on ne sait pas encore où l'on envoie le
			// joueur, ça se règle à la passe suivante.
			pending[nameid].push_back({ nd->exname, price });
			floating_names.insert(nd->exname);
			return;
		}

		hits[nameid].push_back({ nd, std::string(), price });
	};

	struct s_mapiterator* iter = mapit_geteachnpc();

	for (npc_data* nd = (TBL_NPC*)mapit_first(iter); mapit_exists(iter); nd = (TBL_NPC*)mapit_next(iter)) {
		if (nd == nullptr)
			continue;

		switch (nd->subtype) {
			case NPCTYPE_SHOP:
			case NPCTYPE_CASHSHOP:
			case NPCTYPE_ITEMSHOP:
			case NPCTYPE_POINTSHOP:
			case NPCTYPE_MARKETSHOP:
				if (nd->u.shop.shop_item == nullptr)
					break;

				for (uint16 i = 0; i < nd->u.shop.count; i++) {
					const struct npc_item_list& entry = nd->u.shop.shop_item[i];

					if (entry.nameid == 0 || wanted.find(entry.nameid) == wanted.end())
						continue;

					std::string price = shopsearch_price(nd, entry);

#if PACKETVER >= 20131223
					// Un marketshop tient un stock : épuisé, il ne vend plus rien,
					// et le dire évite un aller-retour pour rien. -1 = illimité.
					if (nd->subtype == NPCTYPE_MARKETSHOP && entry.qty >= 0) {
						char stock[32];

						safesnprintf(stock, sizeof(stock), " (stock: %d)", entry.qty);
						price += stock;
					}
#endif

					record(nd, entry.nameid, price);
				}
				break;

			case NPCTYPE_BARTER: {
				// Un barter ne range pas ses articles dans l'union : le npc_data
				// n'en porte que le drapeau « étendu », la marchandise vit dans
				// barter_db, indexée par le nom UNIQUE du NPC.
				std::shared_ptr<s_npc_barter> barter = barter_db.find(nd->exname);

				if (barter == nullptr)
					break;

				for (const auto& it : barter->items) {
					std::shared_ptr<s_npc_barter_item> entry = it.second;

					if (entry == nullptr || wanted.find(entry->nameid) == wanted.end())
						continue;

					char price[64];

					if (entry->price > 0)
						safesnprintf(price, sizeof(price), "troc + %uz", entry->price);
					else
						safestrncpy(price, "troc", sizeof(price));

					record(nd, entry->nameid, price);
				}
				break;
			}

			default:
				break;
		}
	}

	mapit_free(iter);

	// Qui ouvre les boutiques flottantes ? Un seul scan par bytecode DISTINCT :
	// les duplicates partagent celui de leur original, et le NPC des villes en
	// compte une trentaine. Les relire chacun serait trente fois le même travail.
	std::map<std::string, std::vector<npc_data*>> callers;

	if (!floating_names.empty()) {
		size_t shortest = SIZE_MAX, longest = 0;

		for (const std::string& name : floating_names) {
			// std:: explicite : le min()/max() maison de rAthena est typé int32 et
			// tronquerait les size_t.
			shortest = std::min(shortest, name.length());
			longest = std::max(longest, name.length());
		}

		std::map<const struct script_code*, std::vector<std::string>> scanned;

		iter = mapit_geteachnpc();

		for (npc_data* nd = (TBL_NPC*)mapit_first(iter); mapit_exists(iter); nd = (TBL_NPC*)mapit_next(iter)) {
			// Un porteur lui-même hors carte ne mènerait nulle part : autant
			// garder le nom technique, que le staff saura suivre.
			if (nd == nullptr || nd->subtype != NPCTYPE_SCRIPT || nd->m < 0 || nd->u.scr.script == nullptr)
				continue;

			auto cached = scanned.find(nd->u.scr.script);

			if (cached == scanned.end()) {
				std::vector<std::string> found;

				shopsearch_scan_script(nd->u.scr.script, floating_names, shortest, longest, found);
				cached = scanned.emplace(nd->u.scr.script, std::move(found)).first;
			}

			for (const std::string& name : cached->second)
				callers[name].push_back(nd);
		}

		mapit_free(iter);
	}

	for (const auto& waiting : pending) {
		for (const s_shopsearch_pending& shop : waiting.second) {
			const auto found = callers.find(shop.exname);

			if (found == callers.end()) {
				// Aucun script ne la nomme : boutique orpheline, ou nom composé
				// à l'exécution. Le joueur n'en ferait rien — le staff, si.
				if (pc_get_group_level(sd) > 0)
					hits[waiting.first].push_back({ nullptr, shop.exname, shop.price });
				continue;
			}

			for (npc_data* nd : found->second)
				hits[waiting.first].push_back({ nd, std::string(), shop.price });
		}
	}

	// Les étals de joueurs, seconde moitié de la réponse.
	std::map<t_itemid, std::vector<s_shopsearch_vend>> vends;

	iter = mapit_getallusers();

	for (map_session_data* pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter)) {
		if (pl_sd == nullptr || !pl_sd->vender_id)
			continue;

		for (int32 j = 0; j < pl_sd->vend_num; j++) {
			const t_itemid nameid = pl_sd->cart.u.items_cart[pl_sd->vending[j].index].nameid;

			if (wanted.find(nameid) == wanted.end())
				continue;

			vends[nameid].push_back({ pl_sd, pl_sd->vending[j].amount, pl_sd->vending[j].value });
		}
	}

	mapit_free(iter);

	int32 marks = 0;

	for (const auto& result : item_array) {
		std::shared_ptr<item_data> id = result.second;

		safesnprintf(atcmd_output, sizeof(atcmd_output), "Item: %s (ID:%u)", item_db.create_item_link(id).c_str(), id->nameid);
		clif_displaymessage(fd, atcmd_output);

		const auto found = hits.find(id->nameid);
		const auto sold = vends.find(id->nameid);
		const bool has_npc = found != hits.end() && !found->second.empty();
		const bool has_vend = sold != vends.end() && !sold->second.empty();

		if (!has_npc && !has_vend) {
			clif_displaymessage(fd, " - Aucun NPC ni joueur ne vend cet item.");
			continue;
		}

		if (has_npc) {
			// Le même NPC ressort autant de fois qu'il ouvre de boutiques —
			// shop_mg, shop_wz, shop_sa… tous derrière la même marchande. À prix
			// égal, c'est UNE adresse pour le joueur, pas trois.
			std::set<std::string> seen;
			std::vector<s_shopsearch_hit> unique;

			for (const s_shopsearch_hit& hit : found->second) {
				char key[64];

				if (hit.nd != nullptr)
					safesnprintf(key, sizeof(key), "%d|", hit.nd->id);
				else
					safestrncpy(key, "0|", sizeof(key));

				if (seen.insert(std::string(key) + hit.price).second)
					unique.push_back(hit);
			}

			// La boutique du coin d'abord : c'est la seule réponse qui intéresse
			// quelqu'un qui cherche où acheter tout de suite.
			std::stable_sort(unique.begin(), unique.end(), [&](const s_shopsearch_hit& a, const s_shopsearch_hit& b) {
				const bool a_here = a.nd != nullptr && a.nd->m == sd->m;
				const bool b_here = b.nd != nullptr && b.nd->m == sd->m;

				return a_here > b_here;
			});

			safesnprintf(atcmd_output, sizeof(atcmd_output), " Boutiques NPC (%d) :", (int32)unique.size());
			clif_displaymessage(fd, atcmd_output);

			int32 shown = 0;

			for (const s_shopsearch_hit& hit : unique) {
				if (shown++ >= SHOPSEARCH_MAX_HITS) {
					safesnprintf(atcmd_output, sizeof(atcmd_output), " ... et %d autres.", (int32)unique.size() - SHOPSEARCH_MAX_HITS);
					clif_displaymessage(fd, atcmd_output);
					break;
				}

				if (hit.nd == nullptr) {
					safesnprintf(atcmd_output, sizeof(atcmd_output), " - %s : %s (boutique sans NPC)", hit.label.c_str(), hit.price.c_str());
					clif_displaymessage(fd, atcmd_output);
					continue;
				}

				const std::string place = bourgeon_pos_link(sd, hit.nd->m, hit.nd->x, hit.nd->y);
				const char* flag = "";

				if (hit.nd->instance_id != 0)
					flag = " (instance)";
				else if (hit.nd->state == NPCVIEW_DISABLE)
					flag = " (d\xe9sactiv\xe9)";

				safesnprintf(atcmd_output, sizeof(atcmd_output), " - %s @ %s : %s%s", hit.nd->name, place.c_str(), hit.price.c_str(), flag);
				clif_displaymessage(fd, atcmd_output);

				// Le point sur la minimap ne veut rien dire ailleurs que sur la
				// carte où se tient le joueur.
				if (hit.nd->m == sd->m && marks < SHOPSEARCH_MAX_MARKS)
					clif_viewpoint(*sd, 1, 0, hit.nd->x, hit.nd->y, ++marks, 0xFFFFFF);
			}
		}

		if (has_vend) {
			std::vector<s_shopsearch_vend> stalls = sold->second;

			// Le moins cher d'abord : chez les joueurs, c'est le prix qui décide,
			// pas la distance.
			std::stable_sort(stalls.begin(), stalls.end(), [](const s_shopsearch_vend& a, const s_shopsearch_vend& b) {
				return a.value < b.value;
			});

			safesnprintf(atcmd_output, sizeof(atcmd_output), " Boutiques joueurs (%d) :", (int32)stalls.size());
			clif_displaymessage(fd, atcmd_output);

			int32 shown = 0;

			for (const s_shopsearch_vend& stall : stalls) {
				if (shown++ >= SHOPSEARCH_MAX_HITS) {
					safesnprintf(atcmd_output, sizeof(atcmd_output), " ... et %d autres.", (int32)stalls.size() - SHOPSEARCH_MAX_HITS);
					clif_displaymessage(fd, atcmd_output);
					break;
				}

				const std::string place = bourgeon_pos_link(sd, stall.sd->m, stall.sd->x, stall.sd->y);
				char title[SHOPSEARCH_TITLE_LENGTH];

				// Le titre de l'étal porte souvent l'essentiel de l'annonce, mais
				// il monte à 80 caractères : tronqué, il tient sur la ligne.
				safestrncpy(title, stall.sd->message, sizeof(title));

				safesnprintf(atcmd_output, sizeof(atcmd_output), " - %s @ %s : %uz (x%d) %s", stall.sd->status.name, place.c_str(), stall.value, stall.amount, title);
				clif_displaymessage(fd, atcmd_output);

				if (stall.sd->m == sd->m && marks < SHOPSEARCH_MAX_MARKS)
					clif_viewpoint(*sd, 1, 0, stall.sd->x, stall.sd->y, ++marks, 0xFFFFFF);
			}

			safesnprintf(atcmd_output, sizeof(atcmd_output), msg_txt(sd, 1831), (int32)stalls.size(), stalls.front().value, stalls.back().value); // %d shop trouve - Fourchette des prix: %d ~ %d.
			clif_displaymessage(fd, atcmd_output);
		}
	}

	return 0;
}

/*==========================================
 * @mvpslots : liste le registre des créneaux du MVP tracker.
 *
 * Recette du lot 1 : le total doit valoir « nombre de boss_monster + 4 »,
 * et chaque délai doit être celui écrit dans le script de spawn — aucune
 * constante n'est recopiée en C++, sauf les quatre créneaux scriptés.
 * Un argument facultatif filtre sur le nom de carte.
 *------------------------------------------*/
ACMD_FUNC(mvpslots)
{
	nullpo_retr(-1, sd);

	const std::vector<s_mvp_slot>& slots = mvp_tracker_slots();
	char filter[MAP_NAME_LENGTH_EXT];

	filter[0] = '\0';

	if (message && *message)
		safestrncpy(filter, message, sizeof(filter));

	static const char* kind_label[] = { "boss", "timer", "invasion", "verrou" };
	int32 shown = 0;

	for (const s_mvp_slot& slot : slots) {
		const char* mapname = (slot.mapid >= 0) ? map_getmapdata(slot.mapid)->name : "?";

		if (filter[0] != '\0' && strstr(mapname, filter) == nullptr)
			continue;

		const char* mobname = "(tire par le script)";
		std::shared_ptr<s_mob_db> mob = (slot.mob_id != 0) ? mob_db.find(slot.mob_id) : nullptr;

		if (mob != nullptr)
			mobname = mob->jname.c_str();

		// La LOI, jamais le tirage : fenêtre = [delay1, delay1 + delay2].
		safesnprintf(atcmd_output, sizeof(atcmd_output), "#%d %s @ %s [%s] %u-%u min",
			slot.slot_id, mobname, mapname, kind_label[slot.kind],
			slot.delay1 / 60000, (slot.delay1 + slot.delay2) / 60000);
		clif_displaymessage(fd, atcmd_output);
		shown++;
	}

	safesnprintf(atcmd_output, sizeof(atcmd_output), "MVP tracker : %d creneaux affiches sur %d.", shown, (int32)slots.size());
	clif_displaymessage(fd, atcmd_output);
	return 0;
}

/*==========================================
 * Libelle d'un code de retour du MVP tracker.
 * Les messages restent en ASCII : ce sont des chaines serveur, envoyees telles
 * quelles au client, comme les msg_txt du dossier conf/msg_conf.
 *------------------------------------------*/
static const char* mvp_group_result_text(e_mvp_group_result result)
{
	switch (result) {
		case MVP_GROUP_OK:                  return "C'est fait.";
		case MVP_GROUP_ERR_NO_ACCOUNT:      return "Ce compte de jeu n'est rattache a aucun compte Moonlight.";
		case MVP_GROUP_ERR_ALREADY_MEMBER:  return "Vous etes deja dans un groupe : quittez-le d'abord (@mvpgroup leave).";
		case MVP_GROUP_ERR_NOT_MEMBER:      return "Vous n'etes dans aucun groupe.";
		case MVP_GROUP_ERR_NOT_OWNER:       return "Seul le proprietaire du groupe peut faire cela.";
		case MVP_GROUP_ERR_NO_SUCH_USER:    return "Personnage introuvable : verifiez l'orthographe.";
		case MVP_GROUP_ERR_NOT_INVITABLE:   return "Ce personnage n'est ni dans votre guilde ni dans vos amis.";
		case MVP_GROUP_ERR_SELF:            return "Pas sur vous-meme.";
		case MVP_GROUP_ERR_FULL:            return "Le groupe est complet.";
		case MVP_GROUP_ERR_BAD_NAME:        return "Nom de groupe invalide (1 a 31 caracteres).";
		case MVP_GROUP_ERR_NO_INVITE:       return "Aucune invitation en attente.";
		case MVP_GROUP_ERR_TARGET_IN_GROUP: return "Cette personne appartient deja a un autre groupe.";
		case MVP_GROUP_ERR_TARGET_SAME_GROUP: return "Ce personnage est deja dans votre groupe : son compte Moonlight y est, avec tous ses personnages.";
		case MVP_GROUP_ERR_SQL:             return "Erreur de base de donnees, rien n'a ete change.";
	}

	return "Erreur inconnue.";
}

/*==========================================
 * @mvpgroup : gestion du groupe de chasse MVP.
 *
 * Recette des lots 2 et 3 : cree un groupe, invite, verifie qu'un kill credite
 * le groupe du tueur et lui seul (@mvpobs), et qu'un redemarrage garde le groupe
 * mais perd les observations. Le protocole du client Bourgeon appellera
 * exactement les memes fonctions.
 *------------------------------------------*/
ACMD_FUNC(mvpgroup)
{
	nullpo_retr(-1, sd);

	char action[16];
	char argument[NAME_LENGTH + MVP_GROUP_NAME_LEN];

	action[0] = '\0';
	argument[0] = '\0';

	if (!message || !*message || sscanf(message, "%15s %55[^\n]", action, argument) < 1) {
		clif_displaymessage(fd, "Usage : @mvpgroup <create|info|invite|accept|decline|leave|kick|dissolve> [nom]");
		return -1;
	}

	if (strcmpi(action, "info") == 0) {
		s_mvp_group* group = mvp_tracker_group_of(*sd);

		if (group == nullptr) {
			char invite_name[MVP_GROUP_NAME_LEN];

			if (mvp_group_pending_invite(*sd, invite_name) != 0) {
				safesnprintf(atcmd_output, sizeof(atcmd_output), "Invitation en attente du groupe '%s' (@mvpgroup accept).", invite_name);
				clif_displaymessage(fd, atcmd_output);
			} else {
				clif_displaymessage(fd, mvp_group_result_text(MVP_GROUP_ERR_NOT_MEMBER));
			}

			return 0;
		}

		safesnprintf(atcmd_output, sizeof(atcmd_output), "Groupe '%s' (#%u) : %d membres, %d observations.",
			group->name, group->group_id, (int32)group->members.size(), (int32)group->obs.size());
		clif_displaymessage(fd, atcmd_output);

		for (uint32 user_id : group->members) {
			safesnprintf(atcmd_output, sizeof(atcmd_output), " - compte %u%s", user_id,
				user_id == group->owner_user_id ? " (proprietaire)" : "");
			clif_displaymessage(fd, atcmd_output);
		}

		return 0;
	}

	e_mvp_group_result result;

	if (strcmpi(action, "create") == 0)
		result = mvp_group_create(*sd, argument);
	else if (strcmpi(action, "invite") == 0)
		result = mvp_group_invite(*sd, argument);
	else if (strcmpi(action, "accept") == 0)
		result = mvp_group_accept(*sd);
	else if (strcmpi(action, "decline") == 0)
		result = mvp_group_decline(*sd);
	else if (strcmpi(action, "leave") == 0)
		result = mvp_group_leave(*sd);
	else if (strcmpi(action, "kick") == 0)
		result = mvp_group_kick(*sd, argument);
	else if (strcmpi(action, "dissolve") == 0)
		result = mvp_group_dissolve(*sd);
	else {
		clif_displaymessage(fd, "Usage : @mvpgroup <create|info|invite|accept|decline|leave|kick|dissolve> [nom]");
		return -1;
	}

	clif_displaymessage(fd, mvp_group_result_text(result));
	return result == MVP_GROUP_OK ? 0 : -1;
}

/*==========================================
 * @mvpobs : ce que le groupe SAIT, sans passer par le client.
 *
 * Affiche la source, l'age de l'information et, quand elle a ete meritee,
 * l'instant exact. Sinon la FENETRE deduite de la loi (delay1, delay2) : c'est
 * la regle de non-triche rendue visible.
 *------------------------------------------*/
ACMD_FUNC(mvpobs)
{
	nullpo_retr(-1, sd);

	s_mvp_group* group = mvp_tracker_group_of(*sd);

	if (group == nullptr) {
		clif_displaymessage(fd, mvp_group_result_text(MVP_GROUP_ERR_NOT_MEMBER));
		return -1;
	}

	if (group->obs.empty()) {
		clif_displaymessage(fd, "Aucune observation : le groupe n'a encore rien vu.");
		return 0;
	}

	static const char* source_label[] = { "saisi", "tombe", "kill", "miroir" };
	const std::vector<s_mvp_slot>& slots = mvp_tracker_slots();
	int64 now = (int64)time(nullptr);

	for (const auto& entry : group->obs) {
		if (entry.first >= slots.size())
			continue;

		const s_mvp_slot& slot = slots[entry.first];
		const s_mvp_obs& obs = entry.second;
		std::shared_ptr<s_mob_db> mob = (obs.mob_id != 0) ? mob_db.find(obs.mob_id) : nullptr;
		const char* mapname = (slot.mapid >= 0) ? map_getmapdata(slot.mapid)->name : "?";

		if (obs.exact_respawn != 0) {
			safesnprintf(atcmd_output, sizeof(atcmd_output), "#%d %s @ %s [%s] exact dans %dmin (vu il y a %dmin)",
				slot.slot_id, mob != nullptr ? mob->jname.c_str() : "?", mapname, source_label[obs.source],
				(int32)((obs.exact_respawn - now) / 60), (int32)((now - obs.reported_at) / 60));
		} else {
			// La LOI, pas le tirage : une fenetre, jamais un point.
			int64 from = obs.kill_time + slot.delay1 / 1000;
			int64 to = from + slot.delay2 / 1000;

			safesnprintf(atcmd_output, sizeof(atcmd_output), "#%d %s @ %s [%s] fenetre dans %d-%dmin (vu il y a %dmin)",
				slot.slot_id, mob != nullptr ? mob->jname.c_str() : "?", mapname, source_label[obs.source],
				(int32)((from - now) / 60), (int32)((to - now) / 60), (int32)((now - obs.reported_at) / 60));
		}

		clif_displaymessage(fd, atcmd_output);

		if (obs.tomb_x >= 0) {
			safesnprintf(atcmd_output, sizeof(atcmd_output), "     tombe en (%d,%d), tue par %s",
				obs.tomb_x, obs.tomb_y, obs.killer_name[0] != '\0' ? obs.killer_name : "?");
			clif_displaymessage(fd, atcmd_output);
		}
	}

	return 0;
}
