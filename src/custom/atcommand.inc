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

	if( map_getmapflag(sd->bl.m, MF_NOMVPCARD) ) {
		map_setmapflag(sd->bl.m, MF_NOMVPCARD, false);
		clif_displaymessage(fd, msg_txt(sd,1809));
	}
	else {
		map_setmapflag(sd->bl.m, MF_NOMVPCARD, true);
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

	if( map_getmapflag(sd->bl.m, MF_NOATTACKSKILL) ) {
		map_setmapflag(sd->bl.m, MF_NOATTACKSKILL, false);
		clif_displaymessage(fd, msg_txt(sd,1811));
	}
	else {
		map_setmapflag(sd->bl.m, MF_NOATTACKSKILL, true);
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

	if( map_getmapflag(sd->bl.m, MF_NOAOESKILL) ) {
		map_setmapflag(sd->bl.m, MF_NOAOESKILL, false);
		clif_displaymessage(fd, msg_txt(sd,1813));
	}
	else {
		map_setmapflag(sd->bl.m, MF_NOAOESKILL, true);
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
	struct map_session_data *pl_sd;
	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	nullpo_retr(-1, sd);

	if( !message || !*message || sscanf(message, "%23[^,],%99[^\n]", atcmd_player_name, mes) < 2 ) {
		clif_displaymessage(fd, msg_txt(sd,1815));
		return -1;
	}

	if( (pl_sd = map_nick2sd(atcmd_player_name, true)) == NULL && (pl_sd = map_charid2sd(atoi(atcmd_player_name))) == NULL ) {
		clif_displaymessage(fd, msg_txt(sd,3));
		return -1;
	}

	if ( pc_get_group_level(sd) < pc_get_group_level(pl_sd) ) {
		clif_displaymessage(fd, msg_txt(sd,81));
		return -1;
	}

	snprintf(temp, sizeof temp ,"%s : %s", pl_sd->status.name, mes);
	clif_GlobalMessage(&pl_sd->bl, temp, AREA_CHAT_WOC);
	clif_displaymessage(pl_sd->fd, temp);

    log_chat(LOG_CHAT_GLOBAL, 0, pl_sd->status.name, pl_sd->status.account_id, mapindex_id2name(pl_sd->mapindex), pl_sd->bl.x, pl_sd->bl.y, NULL, mes);

	return 0;
}
 /*==========================================
 * @moche <Pseudo> par [Stingor]
 * Génére un model aléatoire.
 *----------------------------------------------------*/
ACMD_FUNC(moche)
{
	struct map_session_data *pl_sd;
	int hair_style = rand()%MAX_HAIR_STYLE;
	int hair_color = rand()%MAX_HAIR_COLOR;
	int cloth_color = rand()%MAX_CLOTH_COLOR;

	nullpo_retr(-1,sd);

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if( !message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, msg_txt(sd,1816));
		return -1;
	}

	if( (pl_sd = map_nick2sd(atcmd_player_name, true)) == NULL && (pl_sd = map_charid2sd(atoi(message))) == NULL ) {
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

		clif_changelook(&pl_sd->bl, LOOK_HAIR, hair_style);
		clif_changelook(&pl_sd->bl, LOOK_HAIR_COLOR, hair_color);
		clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, cloth_color);
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
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	char temp[100];
	int i;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();

	for( i = 0; i < 6; i++ )
		clif_specialeffect(&sd->bl,1038,ALL_CLIENT);

	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->bl.m == pl_sd->bl.m ) {
			clif_changelook(&pl_sd->bl, LOOK_HEAD_TOP, 144);
			clif_specialeffect(&pl_sd->bl,508,AREA);
			clif_specialeffect(&pl_sd->bl,435,AREA);
			snprintf(temp, sizeof temp ,"%s : CELEBRATE !!!", pl_sd->status.name);
			clif_GlobalMessage(&pl_sd->bl, temp, AREA_CHAT_WOC);
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
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();

	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->bl.m == pl_sd->bl.m && sd != pl_sd )
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
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;

	nullpo_retr(-1, sd);

	iter = mapit_getallusers();

	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) ) {
		if( sd->bl.m == pl_sd->bl.m && sd != pl_sd )
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
	struct map_session_data *pl_sd;

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if( !message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, msg_txt(sd,1818));
		return -1;
	}

	if( (pl_sd = map_nick2sd(atcmd_player_name, true)) == NULL && (pl_sd = map_charid2sd(atoi(message))) == NULL ) {
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
		clif_standing(&pl_sd->bl);
	}
	else {
		pc_setsit(pl_sd);
		skill_sit(sd, true);
		clif_sitting(&pl_sd->bl);
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
			int prix;

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
	int i;
	struct {
		const char* format;
		int value;
	} output_table[] = {
		{ "Castrate - %d", 0 },
		{ "Delayrate - -%d%% - After Cast Delay", 0 },
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
	output_table[1].value = sd->delayrate;
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

	for( i = 0; output_table[i].format != NULL; i++ ) {
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
	int j, count = 0;
	struct map_session_data *pl_sd;
	unsigned int MinPrize = battle_config.vending_max_value, MaxPrize = 0;
	
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
					sprintf(output, "Vendeur: %s | Quantit? %d | Prix: %d | Shop: %s | Coord: %d,%d", pl_sd->status.name, pl_sd->vending[j].amount, pl_sd->vending[j].value,pl_sd->message,pl_sd->bl.x,pl_sd->bl.y);
					clif_viewpoint(sd, 1, 1, pl_sd->bl.x, pl_sd->bl.y, count+1, 0xFFFFFF);
					
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

	if (!intif_storage_request(sd, TABLE_STORAGE, 5, STOR_MODE_PUT)) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	} else if (storage_premiumStorage_load(sd, 5, STOR_MODE_PUT) == 0) {
		clif_displaymessage(fd, msg_txt(sd,1161)); // You currently cannot open your storage.
		return -1;
	}

	if (sd->premiumStorage.stor_id == 5) {
		for (int i = 0; i < MAX_INVENTORY; i++) {
			if (sd->inventory.u.items_inventory[i].amount && itemdb_iscard(sd->inventory.u.items_inventory[i].nameid)) 
				storage_storageadd(sd, &sd->premiumStorage, i, sd->inventory.u.items_inventory[i].amount);
		}
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
	struct party_data* p = NULL;
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

	clif_party_info(p,sd);
	return 0;
}
