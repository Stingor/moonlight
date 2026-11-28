// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pc_attendance.hpp"

#include <algorithm>
#include <cstdlib> // strtoll
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "battle.hpp"
#include "chrif.hpp" // chrif_save : l'inventaire livré suit la réclamation en base
#include "clif.hpp"
#include "intif.hpp" // intif_Mail_send
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp" // mmysql_handle
#include "packets.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"

/// Blocs d'une édition au plus : ceux qui tiennent dans un paquet.
static constexpr size_t MOONLIGHT_ATTENDANCE_ENTRIES_MAX =
	( std::numeric_limits<int16>::max() - sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE ) ) / sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE_sub );

/// Exemplaires d'un item non empilable qu'un bloc donne au plus : il se donne
/// à l'unité, une case chacun, et un inventaire n'en a pas plus.
static constexpr uint32 MOONLIGHT_ATTENDANCE_UNSTACKABLE_MAX = MAX_INVENTORY;

/* ------------------------------------------------------------------------- *
 * Lecture des colonnes
 * ------------------------------------------------------------------------- */

/// Lit la colonne `col` de la ligne courante ; faux si elle est NULL.
static bool moonlight_attendance_column( size_t col, int64& out ){
	char* data = nullptr;

	if( Sql_GetData( mmysql_handle, col, &data, nullptr ) != SQL_SUCCESS || data == nullptr ){
		return false;
	}

	out = strtoll( data, nullptr, 10 );
	return true;
}

static uint32 moonlight_attendance_column_u32( size_t col ){
	int64 value = 0;

	moonlight_attendance_column( col, value );

	return static_cast<uint32>( std::clamp<int64>( value, 0, std::numeric_limits<uint32>::max() ) );
}

/* ------------------------------------------------------------------------- *
 * Les éditions du calendrier
 * ------------------------------------------------------------------------- */

/// Le calendrier d'une édition tel qu'il part au client, trié par jour puis
/// par rang. `complete` : chaque jour du cycle a au moins un bloc.
struct s_moonlight_attendance_calendar {
	uint32 edition_id = 0;
	bool complete = false;
	std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub> entries;
};

/// Exécute `query`, qui rend au plus une ligne d'une colonne : l'identifiant
/// d'une édition. 0 s'il n'y en a pas.
static uint32 moonlight_attendance_edition_query( const char* query ){
	if( Sql_QueryStr( mmysql_handle, query ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return 0;
	}

	uint32 edition_id = 0;

	if( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		edition_id = moonlight_attendance_column_u32( 0 );
	}

	Sql_FreeResult( mmysql_handle );
	return edition_id;
}

/// L'édition du jour `date` (AAAAMMJJ) : celle du plus grand mois qui
/// commence au plus tard ce jour-là. 0 s'il n'y en a pas.
static uint32 moonlight_attendance_edition_on( uint32 date ){
	if( date == 0 ){
		return 0;
	}

	char query[256];

	safesnprintf( query, sizeof( query ),
		"SELECT `id` FROM `moonlight_attendance_edition` WHERE `month` <= STR_TO_DATE('%u', '%%Y%%m%%d') ORDER BY `month` DESC LIMIT 1",
		date );

	return moonlight_attendance_edition_query( query );
}

/// L'édition d'aujourd'hui, au jour de la base.
static uint32 moonlight_attendance_edition_today(){
	return moonlight_attendance_edition_query(
		"SELECT `id` FROM `moonlight_attendance_edition` WHERE `month` <= CURDATE() ORDER BY `month` DESC LIMIT 1" );
}

/// Lit le calendrier de l'édition `edition_id`. Vrai s'il est complet ; une
/// édition absente ou incomplète le dit au journal.
static bool moonlight_attendance_load_calendar( uint32 edition_id, s_moonlight_attendance_calendar& calendar ){
	calendar = {};
	calendar.edition_id = edition_id;

	if( edition_id == 0 ){
		ShowError( "moonlight_attendance: aucune \xe9" "dition du calendrier ne couvre ce cycle, la pr\xe9sence reste ferm\xe9" "e.\n" );
		return false;
	}

	if( Sql_Query( mmysql_handle,
			"SELECT `day`, `slot`, `item_id`, `amount`, `milestone` FROM `moonlight_attendance_calendar` WHERE `edition_id` = '%u' ORDER BY `day`, `slot`",
			edition_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return false;
	}

	uint32 days_mask = 0;

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		const uint32 day = moonlight_attendance_column_u32( 0 );
		const uint32 item_id = moonlight_attendance_column_u32( 2 );
		const uint32 amount = moonlight_attendance_column_u32( 3 );
		const bool milestone = moonlight_attendance_column_u32( 4 ) != 0;

		if( day < 1 || day > MOONLIGHT_ATTENDANCE_CYCLE_DAYS || amount < 1 || amount > std::numeric_limits<uint16>::max() ){
			ShowError( "moonlight_attendance: \xe9" "dition %u, bloc (jour %u, rang %u) ignor\xe9 : jour hors de 1..%d ou quantit\xe9 %u invalide.\n",
				edition_id, day, moonlight_attendance_column_u32( 1 ), MOONLIGHT_ATTENDANCE_CYCLE_DAYS, amount );
			continue;
		}

		if( calendar.entries.size() >= MOONLIGHT_ATTENDANCE_ENTRIES_MAX ){
			ShowError( "moonlight_attendance: \xe9" "dition %u, plus de %zu blocs : les suivants sont ignor\xe9s.\n", edition_id, MOONLIGHT_ATTENDANCE_ENTRIES_MAX );
			break;
		}

		PACKET_ZC_MOONLIGHT_ATTENDANCE_sub sub = {};

		sub.day = static_cast<uint8>( day );
		sub.flags = ( milestone ? MOONLIGHT_ATTENDANCE_FLAG_MILESTONE : 0 ) | ( item_id == 0 ? MOONLIGHT_ATTENDANCE_FLAG_EVENT_POINTS : 0 );
		sub.itemId = item_id;
		sub.amount = static_cast<uint16>( amount );
		calendar.entries.push_back( sub );
		days_mask |= 1u << ( day - 1 );
	}

	Sql_FreeResult( mmysql_handle );

	// Le cycle donne les jours dans l'ordre : un trou laisserait une
	// réclamation sans récompense. Une édition incomplète ferme la présence.
	for( uint16 day = 1; day <= MOONLIGHT_ATTENDANCE_CYCLE_DAYS; day++ ){
		if( ( days_mask & ( 1u << ( day - 1 ) ) ) == 0 ){
			ShowError( "moonlight_attendance: le jour %hu manque \xe0 l'\xe9" "dition %u, la pr\xe9sence reste ferm\xe9" "e pour ceux qui la suivent.\n", day, edition_id );
			return false;
		}
	}

	calendar.complete = true;
	return true;
}

/// Les blocs du jour `day` d'un calendrier.
static std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub> moonlight_attendance_day_entries( const s_moonlight_attendance_calendar& calendar, uint16 day ){
	std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub> blocks;

	for( const PACKET_ZC_MOONLIGHT_ATTENDANCE_sub& entry : calendar.entries ){
		if( entry.day == day ){
			blocks.push_back( entry );
		}
	}

	return blocks;
}

/* ------------------------------------------------------------------------- *
 * Les bonus de date
 * ------------------------------------------------------------------------- */

/// Le bonus d'un jour : son nom, tel qu'écrit en base (latin1), et ses blocs,
/// tous du jour 0. Sans bonus, `blocks` est vide.
struct s_moonlight_attendance_bonus {
	std::string label;
	std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub> blocks;
};

/// Lit le bonus du jour `date` (AAAAMMJJ). Un bloc invalide est écarté et dit
/// au journal ; un jour sans bonus rend un bonus vide.
static s_moonlight_attendance_bonus moonlight_attendance_load_bonus( uint32 date ){
	s_moonlight_attendance_bonus bonus;

	if( date == 0 ){
		return bonus;
	}

	if( Sql_Query( mmysql_handle,
			"SELECT `b`.`slot`, `b`.`item_id`, `b`.`amount`, `d`.`label` FROM `moonlight_attendance_bonus` AS `b` "
			"LEFT JOIN `moonlight_attendance_bonus_day` AS `d` ON `d`.`on_date` = `b`.`on_date` "
			"WHERE `b`.`on_date` = STR_TO_DATE('%u', '%%Y%%m%%d') ORDER BY `b`.`slot`",
			date ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return bonus;
	}

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		const uint32 item_id = moonlight_attendance_column_u32( 1 );
		const uint32 amount = moonlight_attendance_column_u32( 2 );

		if( bonus.label.empty() ){
			char* label = nullptr;

			if( Sql_GetData( mmysql_handle, 3, &label, nullptr ) == SQL_SUCCESS && label != nullptr ){
				bonus.label = label;
			}
		}

		if( amount < 1 || amount > std::numeric_limits<uint16>::max() ){
			ShowError( "moonlight_attendance: bonus du %u, bloc %u ignor\xe9 : quantit\xe9 %u invalide.\n", date, moonlight_attendance_column_u32( 0 ), amount );
			continue;
		}

		if( bonus.blocks.size() >= MOONLIGHT_ATTENDANCE_ENTRIES_MAX ){
			ShowError( "moonlight_attendance: bonus du %u, plus de %zu blocs : les suivants sont ignor\xe9s.\n", date, MOONLIGHT_ATTENDANCE_ENTRIES_MAX );
			break;
		}

		PACKET_ZC_MOONLIGHT_ATTENDANCE_sub sub = {};

		sub.day = 0;
		sub.flags = item_id == 0 ? MOONLIGHT_ATTENDANCE_FLAG_EVENT_POINTS : 0;
		sub.itemId = item_id;
		sub.amount = static_cast<uint16>( amount );
		bonus.blocks.push_back( sub );
	}

	Sql_FreeResult( mmysql_handle );
	return bonus;
}

bool moonlight_attendance_open(){
	return battle_config.feature_attendance && mmysql_handle != nullptr && moonlight_attendance_edition_today() != 0;
}

/* ------------------------------------------------------------------------- *
 * L'état d'un compte, en SQL
 * ------------------------------------------------------------------------- */

/// La ligne moonlight_attendance d'un compte, avec le jour de la base.
/// Les dates sont en AAAAMMJJ ; 0 pour une date vide.
struct s_moonlight_attendance_row {
	uint32 today = 0;
	uint32 cycle_start = 0;
	uint32 claimed = 0;
	uint32 last_claim = 0;
	uint32 web_used = 0;
	uint32 edition_id = 0; ///< 0 : pas encore posée
	int64 cycle_age_days = -1; ///< DATEDIFF(today, cycle_start), -1 sans cycle
	uint32 seconds_until_reset = 0;
};

/// La règle : un cycle est à ouvrir s'il n'y en a pas, si sa fenêtre est
/// passée ou si ses récompenses sont toutes prises.
static bool moonlight_attendance_cycle_to_open( const s_moonlight_attendance_row& row ){
	return row.cycle_start == 0 || row.cycle_age_days >= MOONLIGHT_ATTENDANCE_CYCLE_DAYS || row.claimed >= MOONLIGHT_ATTENDANCE_CYCLE_DAYS;
}

/// Lit l'état du compte sans verrou. Un compte sans ligne rend un état vide,
/// avec le jour et le délai jusqu'au prochain.
static bool moonlight_attendance_read_state( uint32 user_id, s_moonlight_attendance_row& row ){
	if( Sql_Query( mmysql_handle,
			"SELECT CURDATE()+0, TIMESTAMPDIFF(SECOND, NOW(), CURDATE() + INTERVAL 1 DAY), "
			"`a`.`cycle_start`+0, `a`.`claimed`, `a`.`last_claim`+0, `a`.`web_used`, DATEDIFF(CURDATE(), `a`.`cycle_start`), `a`.`edition_id` "
			"FROM (SELECT 1) AS `d` LEFT JOIN `moonlight_attendance` AS `a` ON `a`.`user_id` = '%u'",
			user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return false;
	}

	if( Sql_NextRow( mmysql_handle ) != SQL_SUCCESS ){
		Sql_FreeResult( mmysql_handle );
		return false;
	}

	row = {};
	row.today = moonlight_attendance_column_u32( 0 );
	row.seconds_until_reset = moonlight_attendance_column_u32( 1 );
	row.cycle_start = moonlight_attendance_column_u32( 2 );
	row.claimed = moonlight_attendance_column_u32( 3 );
	row.last_claim = moonlight_attendance_column_u32( 4 );
	row.web_used = moonlight_attendance_column_u32( 5 );
	row.edition_id = moonlight_attendance_column_u32( 7 );

	int64 age = -1;

	if( moonlight_attendance_column( 6, age ) ){
		row.cycle_age_days = age;
	}

	Sql_FreeResult( mmysql_handle );
	return true;
}

/// L'édition du cycle en cours du compte. Un cycle qui n'en porte pas encore
/// reçoit celle de son premier jour, écrite pour qu'elle ne bouge plus.
static uint32 moonlight_attendance_cycle_edition( uint32 user_id, const s_moonlight_attendance_row& row ){
	if( row.edition_id != 0 ){
		return row.edition_id;
	}

	const uint32 edition_id = moonlight_attendance_edition_on( row.cycle_start );

	// Conditionnel : une réclamation concurrente qui ouvre un autre cycle,
	// ou qui a déjà posé l'édition, garde la sienne.
	if( edition_id != 0 && Sql_Query( mmysql_handle,
			"UPDATE `moonlight_attendance` SET `edition_id` = '%u' WHERE `user_id` = '%u' AND `edition_id` IS NULL AND `cycle_start` = STR_TO_DATE('%u', '%%Y%%m%%d')",
			edition_id, user_id, row.cycle_start ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
	}

	return edition_id;
}

/// Les récompenses du cycle `cycle_start` réclamées par le site : bit d-1.
static uint32 moonlight_attendance_web_mask( uint32 user_id, uint32 cycle_start ){
	if( Sql_Query( mmysql_handle,
			"SELECT `day` FROM `moonlight_attendance_log` WHERE `user_id` = '%u' AND `cycle_start` = '%u' AND `source` = 'web'",
			user_id, cycle_start ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return 0;
	}

	uint32 mask = 0;

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		uint32 day = moonlight_attendance_column_u32( 0 );

		if( day >= 1 && day <= MOONLIGHT_ATTENDANCE_CYCLE_DAYS ){
			mask |= 1u << ( day - 1 );
		}
	}

	Sql_FreeResult( mmysql_handle );
	return mask;
}

/// Les récompenses du cycle `cycle_start` réclamées par le site qui attendent
/// d'être récupérées en jeu : bit d-1.
static uint32 moonlight_attendance_web_pending_mask( uint32 user_id, uint32 cycle_start ){
	if( Sql_Query( mmysql_handle,
			"SELECT `day` FROM `moonlight_attendance_log` WHERE `user_id` = '%u' AND `cycle_start` = '%u' AND `source` = 'web' AND `delivered_at` IS NULL",
			user_id, cycle_start ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return 0;
	}

	uint32 mask = 0;

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		uint32 day = moonlight_attendance_column_u32( 0 );

		if( day >= 1 && day <= MOONLIGHT_ATTENDANCE_CYCLE_DAYS ){
			mask |= 1u << ( day - 1 );
		}
	}

	Sql_FreeResult( mmysql_handle );
	return mask;
}

/// Toutes les récompenses réclamées par le site qui attendent le compte,
/// cycles passés compris, ramenées à ce que porte un octet.
static uint8 moonlight_attendance_web_pending_count( uint32 user_id ){
	if( Sql_Query( mmysql_handle,
			"SELECT COUNT(*) FROM `moonlight_attendance_log` WHERE `user_id` = '%u' AND `source` = 'web' AND `delivered_at` IS NULL",
			user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return 0;
	}

	uint32 count = 0;

	if( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		count = moonlight_attendance_column_u32( 0 );
	}

	Sql_FreeResult( mmysql_handle );
	return static_cast<uint8>( std::min<uint32>( count, std::numeric_limits<uint8>::max() ) );
}

enum e_moonlight_attendance_claim : uint8 {
	MOONLIGHT_ATTENDANCE_CLAIM_DONE,
	MOONLIGHT_ATTENDANCE_CLAIM_ALREADY,
	MOONLIGHT_ATTENDANCE_CLAIM_CLOSED, ///< l'édition du cycle manque ou est incomplète
	MOONLIGHT_ATTENDANCE_CLAIM_ERROR,
};

/// Réclame le jour pour le compte, par le jeu, en une transaction. En cas de
/// succès, `day` est le rang de la récompense obtenue, `log_id` la ligne du
/// journal, déjà marquée livrée à ce personnage, et `calendar` l'édition du
/// cycle, complète, et `claimed_on` le jour de la réclamation (AAAAMMJJ).
static e_moonlight_attendance_claim moonlight_attendance_claim_sql( const map_session_data& sd, uint16& day, uint32& log_id, uint32& claimed_on, s_moonlight_attendance_calendar& calendar ){
	const uint32 user_id = sd.status.user_id;

	// La ligne se crée hors transaction : sous FOR UPDATE, un INSERT IGNORE
	// qui heurte la ligne existante pose un verrou partagé, et deux sessions
	// qui le montent ensuite en exclusif s'interbloquent.
	if( Sql_Query( mmysql_handle, "INSERT IGNORE INTO `moonlight_attendance` (`user_id`) VALUES ('%u')", user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return MOONLIGHT_ATTENDANCE_CLAIM_ERROR;
	}

	if( Sql_QueryStr( mmysql_handle, "START TRANSACTION" ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return MOONLIGHT_ATTENDANCE_CLAIM_ERROR;
	}

	auto rollback = []( e_moonlight_attendance_claim outcome ){
		if( outcome == MOONLIGHT_ATTENDANCE_CLAIM_ERROR ){
			Sql_ShowDebug( mmysql_handle );
		}

		Sql_QueryStr( mmysql_handle, "ROLLBACK" );
		return outcome;
	};

	if( Sql_Query( mmysql_handle,
			"SELECT CURDATE()+0, `cycle_start`+0, `claimed`, `last_claim`+0, `web_used`, DATEDIFF(CURDATE(), `cycle_start`), `edition_id` "
			"FROM `moonlight_attendance` WHERE `user_id` = '%u' FOR UPDATE",
			user_id ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	if( Sql_NextRow( mmysql_handle ) != SQL_SUCCESS ){
		Sql_FreeResult( mmysql_handle );
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	s_moonlight_attendance_row row;

	row.today = moonlight_attendance_column_u32( 0 );
	row.cycle_start = moonlight_attendance_column_u32( 1 );
	row.claimed = moonlight_attendance_column_u32( 2 );
	row.last_claim = moonlight_attendance_column_u32( 3 );
	row.web_used = moonlight_attendance_column_u32( 4 );
	row.edition_id = moonlight_attendance_column_u32( 6 );

	int64 age = -1;

	if( moonlight_attendance_column( 5, age ) ){
		row.cycle_age_days = age;
	}

	Sql_FreeResult( mmysql_handle );

	if( row.today == 0 ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	const bool to_open = moonlight_attendance_cycle_to_open( row );

	if( row.last_claim == row.today ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ALREADY );
	}

	// Le jour lu sous le verrou fait foi jusqu'au bout : un minuit qui tombe
	// entre deux requêtes ne coupe pas la réclamation en deux jours, ni ne
	// change l'édition du cycle qui s'ouvre.
	if( to_open ){
		row.cycle_start = row.today;
		row.claimed = 0;
		row.web_used = 0;
		row.edition_id = moonlight_attendance_edition_on( row.today );
	}else if( row.edition_id == 0 ){
		row.edition_id = moonlight_attendance_edition_on( row.cycle_start );
	}

	// Rien ne se réclame qui ne pourrait se livrer.
	if( !moonlight_attendance_load_calendar( row.edition_id, calendar ) ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_CLOSED );
	}

	row.claimed += 1;

	if( Sql_Query( mmysql_handle,
			"UPDATE `moonlight_attendance` SET `cycle_start` = '%u', `claimed` = '%u', `last_claim` = '%u', `web_used` = '%u', `edition_id` = '%u' WHERE `user_id` = '%u'",
			row.cycle_start, row.claimed, row.today, row.web_used, row.edition_id, user_id ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	if( Sql_Query( mmysql_handle,
			"INSERT INTO `moonlight_attendance_log` (`user_id`, `cycle_start`, `edition_id`, `day`, `claimed_on`, `source`, `delivered_at`, `char_id`) "
			"VALUES ('%u', '%u', '%u', '%u', '%u', 'game', NOW(), '%u')",
			user_id, row.cycle_start, row.edition_id, row.claimed, row.today, sd.status.char_id ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	const uint64 inserted_id = Sql_LastInsertId( mmysql_handle );

	if( Sql_QueryStr( mmysql_handle, "COMMIT" ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	day = static_cast<uint16>( row.claimed );
	log_id = static_cast<uint32>( inserted_id );
	claimed_on = row.today;
	return MOONLIGHT_ATTENDANCE_CLAIM_DONE;
}

/* ------------------------------------------------------------------------- *
 * La livraison
 * ------------------------------------------------------------------------- */

/// Crédite des points d'event (#KAFRAPOINTS) au compte de jeu du personnage
/// qui reçoit : ceux que donnent les events et les MVP.
static void moonlight_attendance_credit_event_points( map_session_data& sd, uint16 points ){
	if( pc_getcash( &sd, 0, points, LOG_TYPE_OTHER ) <= 0 ){
		ShowError( "moonlight_attendance: %hu point(s) d'event non cr\xe9" "dit\xe9s au compte %u.\n", points, sd.status.account_id );
		return;
	}

	char message[CHAT_SIZE_MAX];

	safesnprintf( message, sizeof( message ), "Pr\xe9sence : %hu point(s) d'event cr\xe9" "dit\xe9s.", points );
	clif_messagecolor( &sd, color_table[COLOR_LIGHT_GREEN], message, false, SELF );
}

/// Un lot à remettre : un item et sa quantité, qui tient en une case.
struct s_moonlight_attendance_parcel {
	t_itemid item_id;
	uint32 amount;
};

/// Les lots des blocs d'items d'un jour. Un item inconnu est écarté ; un item
/// non empilable se donne à l'unité, autant de fois que la quantité le dit.
static std::vector<s_moonlight_attendance_parcel> moonlight_attendance_parcels( const std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub>& blocks ){
	std::vector<s_moonlight_attendance_parcel> parcels;

	for( const PACKET_ZC_MOONLIGHT_ATTENDANCE_sub& block : blocks ){
		if( block.flags & MOONLIGHT_ATTENDANCE_FLAG_EVENT_POINTS ){
			continue;
		}

		std::shared_ptr<item_data> item = item_db.find( block.itemId );

		if( item == nullptr ){
			ShowError( "moonlight_attendance: item %u inconnu (jour %u) : il n'est pas livr\xe9.\n", block.itemId, block.day );
			continue;
		}

		if( itemdb_isstackable2( item.get() ) ){
			parcels.push_back( { item->nameid, block.amount } );
			continue;
		}

		uint32 copies = block.amount;

		if( copies > MOONLIGHT_ATTENDANCE_UNSTACKABLE_MAX ){
			ShowError( "moonlight_attendance: item %u non empilable, quantit\xe9 %u ramen\xe9" "e \xe0 %u (jour %u).\n",
				block.itemId, copies, MOONLIGHT_ATTENDANCE_UNSTACKABLE_MAX, block.day );
			copies = MOONLIGHT_ATTENDANCE_UNSTACKABLE_MAX;
		}

		for( uint32 i = 0; i < copies; i++ ){
			parcels.push_back( { item->nameid, 1 } );
		}
	}

	return parcels;
}

/// Envoie `parcels` par RODEX, MAIL_MAX_ITEM lots par courrier.
static void moonlight_attendance_mail( map_session_data& sd, uint16 day, const std::vector<s_moonlight_attendance_parcel>& parcels ){
	for( size_t first = 0; first < parcels.size(); first += MAIL_MAX_ITEM ){
		mail_message msg = {};

		msg.dest_id = sd.status.char_id;
		safestrncpy( msg.send_name, msg_txt( &sd, 788 ), NAME_LENGTH );
		safesnprintf( msg.title, MAIL_TITLE_LENGTH, msg_txt( &sd, 789 ), day );
		safesnprintf( msg.body, MAIL_BODY_LENGTH, msg_txt( &sd, 790 ), day );

		for( size_t i = 0; i < MAIL_MAX_ITEM && first + i < parcels.size(); i++ ){
			msg.item[i].nameid = parcels[first + i].item_id;
			msg.item[i].amount = static_cast<decltype( msg.item[i].amount )>( parcels[first + i].amount );
			msg.item[i].identify = 1;
		}

		msg.status = MAIL_NEW;
		msg.type = MAIL_INBOX_NORMAL;
		msg.timestamp = time( nullptr );

		intif_Mail_send( 0, &msg );
	}
}

/// Donne les blocs du jour `day` : les points d'event au compte, les items
/// dans l'inventaire tant qu'ils y entrent ; dès qu'un lot n'y entre plus
/// (poids, cases, pile), lui et tous les suivants partent par RODEX, en autant
/// de courriers qu'il en faut. Vrai si un courrier est parti.
static bool moonlight_attendance_deliver( map_session_data& sd, uint16 day, const std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub>& blocks ){
	for( const PACKET_ZC_MOONLIGHT_ATTENDANCE_sub& block : blocks ){
		if( block.flags & MOONLIGHT_ATTENDANCE_FLAG_EVENT_POINTS ){
			moonlight_attendance_credit_event_points( sd, block.amount );
		}
	}

	std::vector<s_moonlight_attendance_parcel> to_mail;

	for( const s_moonlight_attendance_parcel& parcel : moonlight_attendance_parcels( blocks ) ){
		if( !to_mail.empty() ){
			to_mail.push_back( parcel );
			continue;
		}

		struct item it = {};

		it.nameid = parcel.item_id;
		it.identify = 1;

		if( pc_additem( &sd, &it, parcel.amount, LOG_TYPE_OTHER ) != ADDITEM_SUCCESS ){
			to_mail.push_back( parcel );
		}
	}

	if( to_mail.empty() ){
		return false;
	}

	moonlight_attendance_mail( sd, day, to_mail );
	return true;
}

static void moonlight_attendance_mark_by_mail( uint32 log_id ){
	if( Sql_Query( mmysql_handle, "UPDATE `moonlight_attendance_log` SET `by_mail` = '1' WHERE `id` = '%u'", log_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
	}
}

/// Dit dans le chat où est la récompense du jour `day`.
static void moonlight_attendance_say_delivered( map_session_data& sd, uint16 day, bool from_web, bool by_mail ){
	char message[CHAT_SIZE_MAX];

	safesnprintf( message, sizeof( message ), "Pr\xe9sence : r\xe9" "compense du jour %hu%s %s.",
		day,
		from_web ? " r\xe9" "clam\xe9" "e sur le site" : "",
		by_mail ? "envoy\xe9" "e par RODEX, faute de place ou de poids dans l'inventaire" : "re\xe7ue dans l'inventaire" );

	clif_messagecolor( &sd, color_table[COLOR_LIGHT_GREEN], message, false, SELF );
}

/// Livre le bonus du jour `claimed_on` (AAAAMMJJ) qui accompagne la récompense
/// du jour `day`, par le même chemin qu'elle, et le dit dans le chat. Vrai si
/// un courrier est parti ; faux aussi sans bonus.
static bool moonlight_attendance_deliver_bonus( map_session_data& sd, uint16 day, uint32 claimed_on ){
	const s_moonlight_attendance_bonus bonus = moonlight_attendance_load_bonus( claimed_on );

	if( bonus.blocks.empty() ){
		return false;
	}

	const bool by_mail = moonlight_attendance_deliver( sd, day, bonus.blocks );
	const bool has_items = std::any_of( bonus.blocks.begin(), bonus.blocks.end(), []( const PACKET_ZC_MOONLIGHT_ATTENDANCE_sub& block ){
		return ( block.flags & MOONLIGHT_ATTENDANCE_FLAG_EVENT_POINTS ) == 0;
	} );
	char message[CHAT_SIZE_MAX];

	safesnprintf( message, sizeof( message ), "Pr\xe9sence : bonus %s %s.",
		bonus.label.empty() ? "du jour" : bonus.label.c_str(),
		by_mail ? "envoy\xe9 par RODEX, faute de place ou de poids dans l'inventaire" : ( has_items ? "re\xe7u dans l'inventaire" : "re\xe7u" ) );

	clif_messagecolor( &sd, color_table[COLOR_LIGHT_GREEN], message, false, SELF );
	return by_mail;
}

/// Ce qu'une récupération a remis : `delivered`, au moins une récompense ;
/// `by_mail`, au moins une part partie par RODEX.
struct s_moonlight_attendance_collect {
	bool delivered = false;
	bool by_mail = false;
};

/// Livre au personnage `sd` les récompenses réclamées sur le site qui attendent
/// le compte, chacune selon l'édition de sa réclamation et avec le bonus de son
/// jour. Chaque ligne se prend par un UPDATE conditionnel : une seule session
/// la gagne.
static s_moonlight_attendance_collect moonlight_attendance_collect_web_pending( map_session_data& sd ){
	s_moonlight_attendance_collect outcome;

	if( Sql_Query( mmysql_handle,
			"SELECT `id`, `day`, `edition_id`, `cycle_start`+0, `claimed_on`+0 FROM `moonlight_attendance_log` "
			"WHERE `user_id` = '%u' AND `source` = 'web' AND `delivered_at` IS NULL ORDER BY `id`",
			sd.status.user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return outcome;
	}

	std::vector<std::tuple<uint32, uint16, uint32, uint32, uint32>> pending; // id, day, edition_id, cycle_start, claimed_on

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		pending.emplace_back( moonlight_attendance_column_u32( 0 ), static_cast<uint16>( moonlight_attendance_column_u32( 1 ) ),
			moonlight_attendance_column_u32( 2 ), moonlight_attendance_column_u32( 3 ), moonlight_attendance_column_u32( 4 ) );
	}

	Sql_FreeResult( mmysql_handle );

	std::map<uint32, s_moonlight_attendance_calendar> calendars; // par édition

	for( auto [log_id, day, edition_id, cycle_start, claimed_on] : pending ){
		if( edition_id == 0 ){
			edition_id = moonlight_attendance_edition_on( cycle_start );
		}

		auto found = calendars.find( edition_id );

		if( found == calendars.end() ){
			found = calendars.emplace( edition_id, s_moonlight_attendance_calendar{} ).first;
			moonlight_attendance_load_calendar( edition_id, found->second );
		}

		// Une réclamation se livre dès que son jour a des blocs, même si son
		// édition est incomplète : elle est due. Sans eux, elle reste en
		// attente jusqu'à ce que l'édition les ait.
		const std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub> blocks = moonlight_attendance_day_entries( found->second, day );

		if( blocks.empty() ){
			ShowWarning( "moonlight_attendance: r\xe9" "compense web %u (jour %hu, \xe9" "dition %u) en attente : jour absent du calendrier.\n", log_id, day, edition_id );
			continue;
		}

		if( Sql_Query( mmysql_handle,
				"UPDATE `moonlight_attendance_log` SET `delivered_at` = NOW(), `char_id` = '%u', `edition_id` = '%u' WHERE `id` = '%u' AND `delivered_at` IS NULL",
				sd.status.char_id, edition_id, log_id ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
			continue;
		}

		if( Sql_NumRowsAffected( mmysql_handle ) != 1 ){
			continue; // une autre session l'a prise
		}

		const bool by_mail = moonlight_attendance_deliver( sd, day, blocks );

		moonlight_attendance_say_delivered( sd, day, true, by_mail );

		if( moonlight_attendance_deliver_bonus( sd, day, claimed_on ) || by_mail ){
			moonlight_attendance_mark_by_mail( log_id );
			outcome.by_mail = true;
		}

		outcome.delivered = true;
	}

	if( outcome.delivered ){
		chrif_save( &sd, CSAVE_INVENTORY );
	}

	return outcome;
}

/* ------------------------------------------------------------------------- *
 * Le paquet et les demandes
 * ------------------------------------------------------------------------- */

void moonlight_attendance_send( map_session_data& sd, bool open, uint8 result ){
	if( !session_isActive( sd.fd ) ){
		return;
	}

	uint8 claimed_days = 0;
	uint8 claimed_today = 0;
	uint8 days_left = MOONLIGHT_ATTENDANCE_CYCLE_DAYS;
	uint8 web_charges_left = MOONLIGHT_ATTENDANCE_WEB_CHARGES;
	uint32 web_claimed_mask = 0;
	uint32 web_pending_mask = 0;
	uint8 web_pending_count = 0;
	uint32 seconds_until_reset = 0;
	uint32 cycle_start = 0;
	s_moonlight_attendance_calendar calendar;
	s_moonlight_attendance_bonus bonus;
	s_moonlight_attendance_row row;

	if( result != MOONLIGHT_ATTENDANCE_CLOSED && sd.status.user_id != 0 && moonlight_attendance_read_state( sd.status.user_id, row ) ){
		claimed_today = row.last_claim != 0 && row.last_claim == row.today;
		seconds_until_reset = row.seconds_until_reset;

		// Le cycle dont le jour vient d'être réclamé reste affiché, même plein :
		// le suivant ne s'ouvre qu'à la prochaine réclamation. Un cycle à
		// ouvrir montre l'édition du jour, celle qu'il prendra.
		const bool shows_cycle = !moonlight_attendance_cycle_to_open( row ) || claimed_today;
		const uint32 edition_id = shows_cycle ? moonlight_attendance_cycle_edition( sd.status.user_id, row ) : moonlight_attendance_edition_today();

		if( !moonlight_attendance_load_calendar( edition_id, calendar ) ){
			result = MOONLIGHT_ATTENDANCE_CLOSED;
			calendar.entries.clear();
			claimed_today = 0;
			seconds_until_reset = 0;
		}else if( shows_cycle ){
			claimed_days = static_cast<uint8>( std::min<uint32>( row.claimed, MOONLIGHT_ATTENDANCE_CYCLE_DAYS ) );
			days_left = static_cast<uint8>( std::clamp<int64>( MOONLIGHT_ATTENDANCE_CYCLE_DAYS - row.cycle_age_days, 0, MOONLIGHT_ATTENDANCE_CYCLE_DAYS ) );
			web_charges_left = static_cast<uint8>( MOONLIGHT_ATTENDANCE_WEB_CHARGES - std::min<uint32>( row.web_used, MOONLIGHT_ATTENDANCE_WEB_CHARGES ) );
			web_claimed_mask = moonlight_attendance_web_mask( sd.status.user_id, row.cycle_start );
			web_pending_mask = moonlight_attendance_web_pending_mask( sd.status.user_id, row.cycle_start );
			cycle_start = row.cycle_start;
		}

		if( result != MOONLIGHT_ATTENDANCE_CLOSED ){
			bonus = moonlight_attendance_load_bonus( row.today );
		}
	}

	// Ce qui attend se compte même présence fermée : une récompense due se
	// récupère toujours.
	if( sd.status.user_id != 0 && mmysql_handle != nullptr ){
		web_pending_count = moonlight_attendance_web_pending_count( sd.status.user_id );
	}

	// Le bonus d'aujourd'hui part en tête, jour 0, dans ce qui reste de place.
	const size_t bonus_count = std::min( bonus.blocks.size(), MOONLIGHT_ATTENDANCE_ENTRIES_MAX - calendar.entries.size() );
	const size_t bonus_length = bonus_count * sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE_sub );
	const size_t entries_length = bonus_length + calendar.entries.size() * sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE_sub );
	const size_t length = sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE ) + entries_length;

	WFIFOHEAD( sd.fd, length );

	PACKET_ZC_MOONLIGHT_ATTENDANCE& p = *reinterpret_cast<PACKET_ZC_MOONLIGHT_ATTENDANCE*>( WFIFOP( sd.fd, 0 ) );

	memset( &p, 0, sizeof( p ) );
	p.packetType = HEADER_ZC_MOONLIGHT_ATTENDANCE;
	p.packetLength = static_cast<int16>( length );
	p.open = open ? 1 : 0;
	p.result = result;
	p.cycleLength = MOONLIGHT_ATTENDANCE_CYCLE_DAYS;
	p.claimedDays = claimed_days;
	p.claimedToday = claimed_today;
	p.daysLeft = days_left;
	p.webChargesLeft = web_charges_left;
	p.webChargesMax = MOONLIGHT_ATTENDANCE_WEB_CHARGES;
	p.webClaimedMask = web_claimed_mask;
	p.webPendingMask = web_pending_mask;
	p.webPendingCount = web_pending_count;
	p.secondsUntilReset = seconds_until_reset;
	p.cycleStart = cycle_start;

	if( bonus_length > 0 ){
		memcpy( WFIFOP( sd.fd, sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE ) ), bonus.blocks.data(), bonus_length );
	}

	if( !calendar.entries.empty() ){
		memcpy( WFIFOP( sd.fd, sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE ) + bonus_length ), calendar.entries.data(), entries_length - bonus_length );
	}

	WFIFOSET( sd.fd, length );
}

/// La présence est-elle permise à ce joueur ? Sinon le dit dans le chat.
static bool moonlight_attendance_allowed( map_session_data& sd ){
	if( !pc_has_permission( &sd, PC_PERM_ATTENDANCE ) ){
		clif_messagecolor( &sd, color_table[COLOR_RED], msg_txt( &sd, 791 ), false, SELF ); // You are not allowed to use the attendance system.
		return false;
	}

	return sd.status.user_id != 0 && moonlight_attendance_open();
}

void moonlight_attendance_request_state( map_session_data& sd ){
	moonlight_attendance_send( sd, true, moonlight_attendance_allowed( sd ) ? MOONLIGHT_ATTENDANCE_STATE : MOONLIGHT_ATTENDANCE_CLOSED );
}

void moonlight_attendance_request_claim( map_session_data& sd ){
	if( !moonlight_attendance_allowed( sd ) ){
		moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_CLOSED );
		return;
	}

	uint16 day = 0;
	uint32 log_id = 0;
	uint32 claimed_on = 0;
	s_moonlight_attendance_calendar calendar;

	switch( moonlight_attendance_claim_sql( sd, day, log_id, claimed_on, calendar ) ){
		case MOONLIGHT_ATTENDANCE_CLAIM_ALREADY:
			moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_ALREADY_CLAIMED );
			return;
		case MOONLIGHT_ATTENDANCE_CLAIM_CLOSED:
			moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_CLOSED );
			return;
		case MOONLIGHT_ATTENDANCE_CLAIM_ERROR:
			clif_messagecolor( &sd, color_table[COLOR_RED], "La pr\xe9sence est momentan\xe9ment indisponible.", false, SELF );
			moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_STATE );
			return;
		default:
			break;
	}

	// L'édition est complète : le jour réclamé a au moins un bloc.
	const bool day_by_mail = moonlight_attendance_deliver( sd, day, moonlight_attendance_day_entries( calendar, day ) );

	moonlight_attendance_say_delivered( sd, day, false, day_by_mail );

	// Le bonus du jour de la réclamation, s'il y en a un, la suit.
	const bool by_mail = moonlight_attendance_deliver_bonus( sd, day, claimed_on ) || day_by_mail;

	if( by_mail ){
		moonlight_attendance_mark_by_mail( log_id );
	}

	chrif_save( &sd, CSAVE_INVENTORY );
	moonlight_attendance_send( sd, false, by_mail ? MOONLIGHT_ATTENDANCE_CLAIMED_BY_MAIL : MOONLIGHT_ATTENDANCE_CLAIMED );
}

void moonlight_attendance_request_collect( map_session_data& sd ){
	if( !pc_has_permission( &sd, PC_PERM_ATTENDANCE ) ){
		clif_messagecolor( &sd, color_table[COLOR_RED], msg_txt( &sd, 791 ), false, SELF ); // You are not allowed to use the attendance system.
		moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_CLOSED );
		return;
	}

	if( sd.status.user_id == 0 || mmysql_handle == nullptr ){
		moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_CLOSED );
		return;
	}

	// Une récompense réclamée sur le site est due : elle se récupère même si la
	// présence a été fermée depuis.
	const s_moonlight_attendance_collect outcome = moonlight_attendance_collect_web_pending( sd );
	uint8 result = MOONLIGHT_ATTENDANCE_STATE;

	if( outcome.delivered ){
		result = outcome.by_mail ? MOONLIGHT_ATTENDANCE_COLLECTED_BY_MAIL : MOONLIGHT_ATTENDANCE_COLLECTED;
	}

	moonlight_attendance_send( sd, false, result );
}

void moonlight_attendance_login( map_session_data& sd ){
	if( sd.status.user_id == 0 || !pc_has_permission( &sd, PC_PERM_ATTENDANCE ) || !moonlight_attendance_open() ){
		return;
	}

	// La fenêtre s'ouvre quand le jour reste à prendre, ou qu'une récompense
	// réclamée sur le site attend que le joueur la récupère, avec le
	// personnage de son choix.
	s_moonlight_attendance_row row;

	if( !moonlight_attendance_read_state( sd.status.user_id, row ) ){
		return;
	}

	if( row.last_claim != row.today || moonlight_attendance_web_pending_count( sd.status.user_id ) > 0 ){
		moonlight_attendance_send( sd, true, MOONLIGHT_ATTENDANCE_STATE );
	}
}
