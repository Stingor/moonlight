// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_achievement.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <common/db.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "char.hpp"
#include "inter.hpp"
#include "int_mail.hpp"

/**
 * ── A QUI APPARTIENT UNE PROGRESSION DE SUCCES ──────────────────────────────
 *
 * Le map-server n'adresse que des PERSONNAGES ; c'est ici qu'un char_id devient
 * la cle de rangement de ses succes. Il y a TROIS formes de ligne, et une seule
 * s'applique a la fois :
 *
 *   par personnage     (char_id = X, account_id = 0, user_id = 0)
 *   par COMPTE MOONLIGHT (char_id = 0, account_id = 0, user_id = U)   <- le defaut
 *   par compte de jeu  (char_id = 0, account_id = A, user_id = 0)   <- repli
 *
 * 🔴 POURQUOI TROIS COLONNES ET NON DEUX. `account_id` et `user_id` sont deux
 * NUMEROTATIONS DIFFERENTES. Les melanger dans une seule colonne — en y mettant
 * le user_id quand il existe et l'account_id sinon — marche jusqu'au jour ou un
 * account_id tombe sur la meme valeur qu'un user_id : deux personnes sans aucun
 * rapport partagent alors leurs succes, et rien dans le schema ne l'interdit.
 * Une colonne par espace de nommage rend la collision impossible a ecrire.
 *
 * Le repli par compte de JEU sert aux comptes qu'aucun compte Moonlight ne
 * rattache (user_id = 0) : ils gardent le comportement d'avant plutot que de se
 * retrouver tous ensemble sur la ligne « user_id = 0 », qui serait un succes
 * commun a des inconnus.
 */
struct s_achievement_owner {
	uint32 char_id = 0;
	uint32 account_id = 0;  ///< compte de JEU ; 0 = resolution impossible
	uint32 user_id = 0;     ///< compte MOONLIGHT ; 0 = compte de jeu non rattache
};

/// La colonne `account_id` de la ligne PARTAGEE : renseignee seulement en repli.
static uint32 achievement_shared_account( const s_achievement_owner& owner ){
	return owner.user_id > 0 ? 0 : owner.account_id;
}

/// La colonne `user_id` de la ligne PARTAGEE : renseignee des que le compte de
/// jeu est rattache a un compte Moonlight.
static uint32 achievement_shared_user( const s_achievement_owner& owner ){
	return owner.user_id;
}

/**
 * Resout le proprietaire d'une progression depuis un char_id.
 *
 * Le char-server connait deja ce chemin : char_mmo_char_fromsql() va lire le
 * meme `login.user_id` au chargement d'un personnage. Ici on le refait a la
 * demande, parce que les requetes de succes arrivent avec un char_id nu.
 *
 * `account_id` a 0 dans le retour = resolution ECHOUEE (personnage inconnu, SQL
 * en panne) : l'appelant doit renoncer plutot qu'ecrire avec une mauvaise cle.
 */
static s_achievement_owner mapif_achievement_owner(uint32 char_id)
{
	s_achievement_owner owner;

	owner.char_id = char_id;

	// LEFT JOIN : un compte de jeu sans ligne de login (ou sans rattachement)
	// doit rendre account_id quand meme, avec user_id a 0 — c'est le repli.
	if( SQL_ERROR == Sql_Query( sql_handle,
			"SELECT `c`.`account_id`, COALESCE(`l`.`user_id`, 0) FROM `%s` AS `c` "
			"LEFT JOIN `login` AS `l` ON `l`.`account_id` = `c`.`account_id` "
			"WHERE `c`.`char_id` = '%u'",
			schema_config.char_db, char_id ) ){
		Sql_ShowDebug(sql_handle);
		return owner;
	}

	if( SQL_SUCCESS == Sql_NextRow( sql_handle ) ){
		char* data;

		Sql_GetData( sql_handle, 0, &data, nullptr );
		owner.account_id = (uint32)strtoul( data, nullptr, 10 );

		Sql_GetData( sql_handle, 1, &data, nullptr );
		owner.user_id = (uint32)strtoul( data, nullptr, 10 );
	}

	Sql_FreeResult( sql_handle );

	return owner;
}

/**
 * Load achievements for a character.
 *
 * Rend les succes propres au PERSONNAGE et ceux de sa progression PARTAGEE.
 * La ligne partagee est celle du compte Moonlight quand il y en a un, celle du
 * compte de jeu sinon — un seul des deux tests peut mordre, les deux colonnes
 * ne sont jamais renseignees ensemble (cf. mapif_achievement_owner).
 * @param owner: Proprietaire resolu (personnage, compte de jeu, compte Moonlight)
 * @param count: Pointer to return the number of found entries.
 * @return Array of found entries. It has *count entries, and it is care of the caller to aFree() it afterwards.
 */
struct achievement *mapif_achievements_fromsql(const s_achievement_owner& owner, int32 *count)
{
	struct achievement *achievelog = nullptr;
	struct achievement tmp_achieve;
	SqlStmt stmt{ *sql_handle };
	StringBuf buf;
	int32 i;

	if (!count)
		return nullptr;

	memset(&tmp_achieve, 0, sizeof(tmp_achieve));

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, COALESCE(UNIX_TIMESTAMP(`completed`),0), COALESCE(UNIX_TIMESTAMP(`rewarded`),0)");
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d`", i + 1);
	StringBuf_Printf(&buf,
		" FROM `%s` WHERE `char_id` = '%u' OR (`char_id` = '0' AND `account_id` = '%u' AND `user_id` = '%u')",
		schema_config.achievement_table, owner.char_id,
		achievement_shared_account(owner), achievement_shared_user(owner));

	if( SQL_ERROR == stmt.PrepareStr(StringBuf_Value(&buf))
	||  SQL_ERROR == stmt.Execute() )
	{
		SqlStmt_ShowDebug(stmt);
		*count = 0;
		return nullptr;
	}

	stmt.BindColumn(0, SQLDT_INT32, &tmp_achieve.achievement_id);
	stmt.BindColumn(1, SQLDT_INT32, &tmp_achieve.completed);
	stmt.BindColumn(2, SQLDT_INT32, &tmp_achieve.rewarded);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		stmt.BindColumn(3 + i, SQLDT_INT32, &tmp_achieve.count[i]);

	*count = (int32)stmt.NumRows();
	if (*count > 0) {
		i = 0;

		achievelog = (struct achievement *)aCalloc(*count, sizeof(struct achievement));
		while (SQL_SUCCESS == stmt.NextRow()) {
			if (i >= *count) // Sanity check, should never happen
				break;
			memcpy(&achievelog[i++], &tmp_achieve, sizeof(tmp_achieve));
		}
		if (i < *count) {
			// Should never happen. Compact array
			*count = i;
			achievelog = (struct achievement *)aRealloc(achievelog, sizeof(struct achievement) * i);
		}
	}

	ShowInfo("achievement load complete from DB - char: %d (total: %d)\n", owner.char_id, *count);

	return achievelog;
}

/**
 * Deletes an achievement from a character's achievementlog.
 *
 * Vise la ligne par personnage OU la ligne partagee qui porte cet id, pour que
 * l'appelant n'ait pas a connaitre la portee du succes.
 * @param owner: Proprietaire resolu
 * @param achievement_id: Achievement ID
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_delete(const s_achievement_owner& owner, int32 achievement_id)
{
	if (SQL_ERROR == Sql_Query(sql_handle,
			"DELETE FROM `%s` WHERE `id` = '%d' AND (`char_id` = '%u' OR (`char_id` = '0' AND `account_id` = '%u' AND `user_id` = '%u'))",
			schema_config.achievement_table, achievement_id, owner.char_id,
			achievement_shared_account(owner), achievement_shared_user(owner))) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Adds an achievement to a character's achievementlog.
 *
 * La portee du succes (ad->bound, donnee par le map-server depuis la base des
 * succes) decide de la cle : PARTAGEE (char_id 0, compte Moonlight ou compte de
 * jeu en repli) ou par PERSONNAGE (char_id X, les deux autres a 0).
 * @param owner: Proprietaire resolu
 * @param ad: Achievement data
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_add(const s_achievement_owner& owner, struct achievement* ad)
{
	StringBuf buf;
	int32 i;

	ARR_FIND( 0, MAX_ACHIEVEMENT_OBJECTIVES, i, ad->count[i] != 0 );

	if( i == MAX_ACHIEVEMENT_OBJECTIVES && ad->completed == 0 && ad->rewarded == 0 ){
		// Do not save
		return true;
	}

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `account_id`, `user_id`, `id`, `completed`, `rewarded`", schema_config.achievement_table);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d`", i + 1);
	StringBuf_AppendStr(&buf, ")");
	if( ad->bound == ACHIEVEMENT_BOUND_ACCOUNT ){
		// Progression PARTAGEE : char_id a 0, et une seule des deux autres
		// colonnes renseignee — le compte Moonlight, ou le compte de jeu en repli.
		StringBuf_Printf(&buf, " VALUES ('0', '%u', '%u', '%d',",
			achievement_shared_account(owner), achievement_shared_user(owner), ad->achievement_id);
	}else{
		// Par PERSONNAGE : char_id seul, les deux colonnes de partage a 0.
		StringBuf_Printf(&buf, " VALUES ('%u', '0', '0', '%d',", owner.char_id, ad->achievement_id);
	}
	if( ad->completed ){
		StringBuf_Printf(&buf, "FROM_UNIXTIME('%u'),", (uint32)ad->completed);
	}else{
		StringBuf_AppendStr(&buf, "NULL,");
	}
	if( ad->rewarded ){
		StringBuf_Printf(&buf, "FROM_UNIXTIME('%u')", (uint32)ad->rewarded);
	}else{
		StringBuf_AppendStr(&buf, "NULL");
	}
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", '%d'", ad->count[i]);
	StringBuf_AppendStr(&buf, ")");

	if (SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf))) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Updates an achievement in a character's achievementlog.
 *
 * Vise la ligne par personnage OU la ligne partagee qui porte cet id.
 * @param owner: Proprietaire resolu
 * @param ad: Achievement data
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_update(const s_achievement_owner& owner, struct achievement* ad)
{
	StringBuf buf;
	int32 i;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET ", schema_config.achievement_table);
	if( ad->completed ){
		StringBuf_Printf(&buf, "`completed` = FROM_UNIXTIME('%u'),", (uint32)ad->completed);
	}else{
		StringBuf_AppendStr(&buf, "`completed` = NULL,");
	}
	if( ad->rewarded ){
		StringBuf_Printf(&buf, "`rewarded` = FROM_UNIXTIME('%u')", (uint32)ad->rewarded);
	}else{
		StringBuf_AppendStr(&buf, "`rewarded` = NULL");
	}
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d` = '%d'", i + 1, ad->count[i]);
	StringBuf_Printf(&buf,
		" WHERE `id` = %d AND (`char_id` = %u OR (`char_id` = 0 AND `account_id` = %u AND `user_id` = %u))",
		ad->achievement_id, owner.char_id,
		achievement_shared_account(owner), achievement_shared_user(owner));

	if (SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf))) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Notifies the map-server of the result of saving a character's achievementlog.
 */
void mapif_achievement_save( int32 fd, uint32 char_id, bool success ){
	WFIFOHEAD(fd, 7);
	WFIFOW(fd, 0) = 0x3863;
	WFIFOL(fd, 2) = char_id;
	WFIFOB(fd, 6) = success;
	WFIFOSET(fd, 7);
}

/**
 * Handles the save request from mapserver for a character's achievementlog.
 * Received achievements are saved, and an ack is sent back to the map server.
 * @see inter_parse_frommap
 */
int32 mapif_parse_achievement_save(int32 fd)
{
	int32 i, j, k, old_n, new_n = (RFIFOW(fd, 2) - 8) / sizeof(struct achievement);
	uint32 char_id = RFIFOL(fd, 4);
	s_achievement_owner owner = mapif_achievement_owner(char_id);
	struct achievement *old_ad = nullptr, *new_ad = nullptr;
	bool success = true;

	if( owner.account_id == 0 ){
		// Proprietaire non resolu : renoncer plutot qu'ecrire avec une mauvaise
		// cle. Un user_id a 0 est LEGITIME (compte non rattache, on retombe sur
		// le compte de jeu) ; un account_id a 0 ne l'est jamais.
		mapif_achievement_save(fd, char_id, false);
		return 0;
	}

	if (new_n > 0)
		new_ad = (struct achievement *)RFIFOP(fd, 8);

	old_ad = mapif_achievements_fromsql(owner, &old_n);

	for (i = 0; i < new_n; i++) {
		ARR_FIND(0, old_n, j, new_ad[i].achievement_id == old_ad[j].achievement_id);
		if (j < old_n) { // Update existing achievements
			// Only counts, complete, and reward are changable.
			ARR_FIND(0, MAX_ACHIEVEMENT_OBJECTIVES, k, new_ad[i].count[k] != old_ad[j].count[k]);
			if (k != MAX_ACHIEVEMENT_OBJECTIVES || new_ad[i].completed != old_ad[j].completed || new_ad[i].rewarded != old_ad[j].rewarded) {
				if ((success = mapif_achievement_update(owner, &new_ad[i])) == false)
					break;
			}

			if (j < (--old_n)) {
				// Compact array
				memmove(&old_ad[j], &old_ad[j + 1], sizeof(struct achievement) * (old_n - j));
				memset(&old_ad[old_n], 0, sizeof(struct achievement));
			}
		} else { // Add new achievements
			if (new_ad[i].achievement_id) {
				if ((success = mapif_achievement_add(owner, &new_ad[i])) == false)
					break;
			}
		}
	}

	for (i = 0; i < old_n; i++) { // Achievements not in new_ad but in old_ad are to be erased.
		if ((success = mapif_achievement_delete(owner, old_ad[i].achievement_id)) == false)
			break;
	}

	if (old_ad)
		aFree(old_ad);

	mapif_achievement_save(fd, char_id, success);

	return 0;
}

/**
 * Sends the achievementlog of a character to the map-server.
 */
void mapif_achievement_load( int32 fd, uint32 char_id ){
	struct achievement *tmp_achievementlog = nullptr;
	int32 num_achievements = 0;
	s_achievement_owner owner = mapif_achievement_owner(char_id);

	tmp_achievementlog = mapif_achievements_fromsql(owner, &num_achievements);

	WFIFOHEAD(fd, num_achievements * sizeof(struct achievement) + 8);
	WFIFOW(fd, 0) = 0x3862;
	WFIFOW(fd, 2) = static_cast<int16>( num_achievements * sizeof( struct achievement ) + 8 );
	WFIFOL(fd, 4) = char_id;

	if (num_achievements > 0)
		memcpy(WFIFOP(fd, 8), tmp_achievementlog, sizeof(struct achievement) * num_achievements);

	WFIFOSET(fd, num_achievements * sizeof(struct achievement) + 8);

	if (tmp_achievementlog)
		aFree(tmp_achievementlog);
}

/**
 * Sends achievementlog to the map server
 * NOTE: Achievements sent to the player are only completed ones
 * @see inter_parse_frommap
 */
int32 mapif_parse_achievement_load(int32 fd)
{
	mapif_achievement_load( fd, RFIFOL(fd, 2) );

	return 0;
}

/**
 * Notify the map-server if claiming the reward has succeeded.
 */
void mapif_achievement_reward( int32 fd, uint32 char_id, int32 achievement_id, time_t rewarded ){
	WFIFOHEAD(fd, 14);
	WFIFOW(fd, 0) = 0x3864;
	WFIFOL(fd, 2) = char_id;
	WFIFOL(fd, 6) = achievement_id;
	WFIFOL(fd, 10) = (uint32)rewarded;
	WFIFOSET(fd, 14);
}

/**
 * Request of the map-server that a player claimed his achievement rewards.
 * @see inter_parse_frommap
 */
int32 mapif_parse_achievement_reward(int32 fd){
	time_t current = time(nullptr);
	uint32 char_id = RFIFOL(fd, 2);
	int32 achievement_id = RFIFOL(fd, 6);
	s_achievement_owner owner = mapif_achievement_owner(char_id);

	// 🔴 C'est CETTE requete qui empeche de toucher deux fois le meme lot : elle
	// n'affecte une ligne que si `rewarded` y est encore NULL. La ligne etant
	// PARTAGEE par tout le compte Moonlight, un second personnage — ou un second
	// compte de jeu — ne peut plus rien y encaisser. C'est la garde que la table
	// card_album_reward faisait a la main pour les seuls paliers de l'album.
	if( Sql_Query( sql_handle, "UPDATE `%s` SET `rewarded` = FROM_UNIXTIME('%u') WHERE (`char_id`='%u' OR (`char_id`='0' AND `account_id`='%u' AND `user_id`='%u')) AND `id` = '%d' AND `completed` IS NOT NULL AND `rewarded` IS NULL",
			schema_config.achievement_table, (uint32)current, owner.char_id,
			achievement_shared_account(owner), achievement_shared_user(owner), achievement_id ) == SQL_ERROR ||
		Sql_NumRowsAffected(sql_handle) <= 0 ){
		current = 0;
	}else if( RFIFOW(fd,10) > 0 ){ // Do not send a mail if no item reward
		char mail_sender[NAME_LENGTH];
		char mail_receiver[NAME_LENGTH];
		char mail_title[MAIL_TITLE_LENGTH];
		char mail_text[MAIL_BODY_LENGTH];
		struct item item;

		memset(&item, 0, sizeof(struct item));
		item.nameid = RFIFOL(fd, 10);
		item.amount = RFIFOW(fd, 14);
		item.identify = 1;

		safesnprintf(mail_sender, NAME_LENGTH, char_msg_txt(227)); // 227: GM
		safestrncpy(mail_receiver, RFIFOCP(fd,16), NAME_LENGTH);
		safesnprintf(mail_title, MAIL_TITLE_LENGTH, char_msg_txt(228)); // 228: Achievement Reward Mail
		safesnprintf(mail_text, MAIL_BODY_LENGTH, char_msg_txt(229), RFIFOCP(fd,16+NAME_LENGTH) ); // 229: [%s] Achievement Reward.

		if( !mail_sendmail(0, mail_sender, char_id, mail_receiver, mail_title, mail_text, 0, &item, 1) ){
			current = 0;
		}
	}

	mapif_achievement_reward(fd, char_id, achievement_id, current);

	return 0;
}

/**
 * Parses achievementlog related packets from the map server.
 * @see inter_parse_frommap
 */
int32 inter_achievement_parse_frommap(int32 fd)
{
	switch (RFIFOW(fd, 0)) {
		case 0x3062: mapif_parse_achievement_load(fd); break;
		case 0x3063: mapif_parse_achievement_save(fd); break;
		case 0x3064: mapif_parse_achievement_reward(fd); break;
		default:
			return 0;
	}
	return 1;
}
