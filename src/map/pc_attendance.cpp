// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pc_attendance.hpp"

#include <algorithm>
#include <cstdlib> // strtoll
#include <cstring>
#include <utility>

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

MoonlightAttendanceDatabase moonlight_attendance_db;

/// Le calendrier tel qu'il part au client, trié par jour puis par rang.
static std::vector<PACKET_ZC_MOONLIGHT_ATTENDANCE_sub> moonlight_attendance_entries;

/* ------------------------------------------------------------------------- *
 * Le calendrier
 * ------------------------------------------------------------------------- */

const std::string MoonlightAttendanceDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/import/moonlight_attendance.yml";
}

uint64 MoonlightAttendanceDatabase::parseBodyNode( const ryml::NodeRef& node ){
	uint16 day;

	if( !this->asUInt16( node, "Day", day ) ){
		return 0;
	}

	if( day < 1 || day > MOONLIGHT_ATTENDANCE_CYCLE_DAYS ){
		this->invalidWarning( node["Day"], "Day %hu hors de 1..%d.\n", day, MOONLIGHT_ATTENDANCE_CYCLE_DAYS );
		return 0;
	}

	std::shared_ptr<s_moonlight_attendance_day> entry = this->find( day );
	bool exists = entry != nullptr;

	if( !exists ){
		entry = std::make_shared<s_moonlight_attendance_day>();
		entry->day = day;
		entry->milestone = false;
		entry->event_points = 0;
	}

	if( this->nodeExists( node, "Milestone" ) ){
		bool milestone;

		if( !this->asBool( node, "Milestone", milestone ) ){
			return 0;
		}

		entry->milestone = milestone;
	}

	if( this->nodeExists( node, "EventPoints" ) ){
		uint16 event_points;

		if( !this->asUInt16( node, "EventPoints", event_points ) ){
			return 0;
		}

		entry->event_points = event_points;
	}

	if( this->nodeExists( node, "Rewards" ) ){
		std::vector<s_moonlight_attendance_reward> rewards;

		for( const auto& rewardNode : node["Rewards"] ){
			std::string item_name;

			if( !this->asString( rewardNode, "Item", item_name ) ){
				return 0;
			}

			std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

			if( item == nullptr ){
				this->invalidWarning( rewardNode["Item"], "Item %s inconnu : jour %hu refus\xe9.\n", item_name.c_str(), day );
				return 0;
			}

			uint16 amount = 1;

			if( this->nodeExists( rewardNode, "Amount" ) && !this->asUInt16( rewardNode, "Amount", amount ) ){
				return 0;
			}

			if( amount < 1 || amount > MAX_AMOUNT_UINT16 ){
				this->invalidWarning( rewardNode["Amount"], "Amount %hu hors de 1..%d : jour %hu refus\xe9.\n", amount, MAX_AMOUNT_UINT16, day );
				return 0;
			}

			// Un item non empilable occupe une case par exemplaire, dans
			// l'inventaire comme dans un courrier : il se donne à l'unité.
			if( amount > 1 && !itemdb_isstackable2( item.get() ) ){
				this->invalidWarning( rewardNode["Amount"], "%s ne s'empile pas : Amount doit valoir 1 (jour %hu refus\xe9).\n", item_name.c_str(), day );
				return 0;
			}

			rewards.push_back( { item->nameid, amount } );
		}

		if( rewards.size() > MOONLIGHT_ATTENDANCE_DAY_ITEMS_MAX ){
			this->invalidWarning( node["Rewards"], "Le jour %hu porte plus de %d items.\n", day, MOONLIGHT_ATTENDANCE_DAY_ITEMS_MAX );
			return 0;
		}

		entry->rewards = std::move( rewards );
	}

	// Un jour donne au moins un item ou des points d'event.
	if( entry->rewards.empty() && entry->event_points == 0 ){
		this->invalidWarning( node, "Le jour %hu ne donne ni item ni point d'event.\n", day );
		return 0;
	}

	if( !exists ){
		this->put( day, entry );
	}

	return 1;
}

/// Réécrit moonlight_attendance_calendar depuis le calendrier chargé : le YAML
/// fait foi, la table n'est que sa copie pour le site.
static void moonlight_attendance_publish_calendar( void ){
	if( mmysql_handle == nullptr ){
		return; // le générateur n'a pas de base
	}

	if( Sql_QueryStr( mmysql_handle, "START TRANSACTION" ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return;
	}

	bool ok = Sql_QueryStr( mmysql_handle, "DELETE FROM `moonlight_attendance_calendar`" ) == SQL_SUCCESS;
	uint16 slot = 0;
	uint8 previous_day = 0;

	for( size_t i = 0; ok && i < moonlight_attendance_entries.size(); i++ ){
		const PACKET_ZC_MOONLIGHT_ATTENDANCE_sub& entry = moonlight_attendance_entries[i];

		slot = entry.day == previous_day ? slot + 1 : 0;
		previous_day = entry.day;

		ok = Sql_Query( mmysql_handle,
			"INSERT INTO `moonlight_attendance_calendar` (`day`, `slot`, `item_id`, `amount`, `milestone`) VALUES ('%u', '%u', '%u', '%u', '%u')",
			entry.day, slot, entry.itemId, entry.amount, ( entry.flags & MOONLIGHT_ATTENDANCE_FLAG_MILESTONE ) ? 1 : 0 ) == SQL_SUCCESS;
	}

	if( !ok || Sql_QueryStr( mmysql_handle, "COMMIT" ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		Sql_QueryStr( mmysql_handle, "ROLLBACK" );
		ShowError( "moonlight_attendance: le calendrier n'a pas \xe9t\xe9 publi\xe9 en base.\n" );
	}
}

void MoonlightAttendanceDatabase::loadingFinished(){
	// Le cycle donne les jours dans l'ordre : un trou laisserait une
	// réclamation sans récompense. Un calendrier incomplet ferme la présence.
	if( !this->empty() ){
		bool complete = true;

		for( uint16 day = 1; day <= MOONLIGHT_ATTENDANCE_CYCLE_DAYS; day++ ){
			if( !this->exists( day ) ){
				ShowError( "moonlight_attendance: le jour %hu manque au calendrier.\n", day );
				complete = false;
			}
		}

		if( !complete ){
			ShowError( "moonlight_attendance: calendrier refus\xe9, la pr\xe9sence reste ferm\xe9" "e.\n" );
			this->clear();
		}
	}else{
		ShowWarning( "moonlight_attendance: calendrier vide, la pr\xe9sence reste ferm\xe9" "e.\n" );
	}

	moonlight_attendance_entries.clear();

	for( uint16 day = 1; day <= MOONLIGHT_ATTENDANCE_CYCLE_DAYS; day++ ){
		std::shared_ptr<s_moonlight_attendance_day> entry = this->find( day );

		if( entry == nullptr ){
			continue;
		}

		// LES POINTS D'EVENT D'ABORD : un bloc sans item, que le client et le
		// site reconnaissent à son drapeau et à son item nul.
		if( entry->event_points > 0 ){
			PACKET_ZC_MOONLIGHT_ATTENDANCE_sub sub = {};

			sub.day = static_cast<uint8>( day );
			sub.flags = MOONLIGHT_ATTENDANCE_FLAG_EVENT_POINTS | ( entry->milestone ? MOONLIGHT_ATTENDANCE_FLAG_MILESTONE : 0 );
			sub.itemId = 0;
			sub.amount = entry->event_points;
			moonlight_attendance_entries.push_back( sub );
		}

		for( const s_moonlight_attendance_reward& reward : entry->rewards ){
			PACKET_ZC_MOONLIGHT_ATTENDANCE_sub sub = {};

			sub.day = static_cast<uint8>( day );
			sub.flags = entry->milestone ? MOONLIGHT_ATTENDANCE_FLAG_MILESTONE : 0;
			sub.itemId = reward.item_id;
			sub.amount = reward.amount;
			moonlight_attendance_entries.push_back( sub );
		}
	}

	moonlight_attendance_publish_calendar();

	TypesafeYamlDatabase::loadingFinished();
}

bool moonlight_attendance_open(){
	return battle_config.feature_attendance && !moonlight_attendance_db.empty();
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
	int64 cycle_age_days = -1; ///< DATEDIFF(today, cycle_start), -1 sans cycle
	uint32 seconds_until_reset = 0;
};

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

	return static_cast<uint32>( std::max<int64>( value, 0 ) );
}

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
			"`a`.`cycle_start`+0, `a`.`claimed`, `a`.`last_claim`+0, `a`.`web_used`, DATEDIFF(CURDATE(), `a`.`cycle_start`) "
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

	int64 age = -1;

	if( moonlight_attendance_column( 6, age ) ){
		row.cycle_age_days = age;
	}

	Sql_FreeResult( mmysql_handle );
	return true;
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

enum e_moonlight_attendance_claim : uint8 {
	MOONLIGHT_ATTENDANCE_CLAIM_DONE,
	MOONLIGHT_ATTENDANCE_CLAIM_ALREADY,
	MOONLIGHT_ATTENDANCE_CLAIM_ERROR,
};

/// Réclame le jour pour le compte, par le jeu, en une transaction. En cas de
/// succès, `day` est le rang de la récompense obtenue et `log_id` la ligne du
/// journal, déjà marquée livrée à ce personnage.
static e_moonlight_attendance_claim moonlight_attendance_claim_sql( const map_session_data& sd, uint16& day, uint32& log_id ){
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
			"SELECT CURDATE()+0, `cycle_start`+0, `claimed`, `last_claim`+0, `web_used`, DATEDIFF(CURDATE(), `cycle_start`) "
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

	if( to_open ){
		row.cycle_start = row.today;
		row.claimed = 0;
		row.web_used = 0;
	}

	row.claimed += 1;

	// Le jour lu sous le verrou fait foi jusqu'au bout : un minuit qui tombe
	// entre deux requêtes ne coupe pas la réclamation en deux jours.
	if( Sql_Query( mmysql_handle,
			"UPDATE `moonlight_attendance` SET `cycle_start` = '%u', `claimed` = '%u', `last_claim` = '%u', `web_used` = '%u' WHERE `user_id` = '%u'",
			row.cycle_start, row.claimed, row.today, row.web_used, user_id ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	if( Sql_Query( mmysql_handle,
			"INSERT INTO `moonlight_attendance_log` (`user_id`, `cycle_start`, `day`, `claimed_on`, `source`, `delivered_at`, `char_id`) "
			"VALUES ('%u', '%u', '%u', '%u', 'game', NOW(), '%u')",
			user_id, row.cycle_start, row.claimed, row.today, sd.status.char_id ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	const uint64 inserted_id = Sql_LastInsertId( mmysql_handle );

	if( Sql_QueryStr( mmysql_handle, "COMMIT" ) != SQL_SUCCESS ){
		return rollback( MOONLIGHT_ATTENDANCE_CLAIM_ERROR );
	}

	day = static_cast<uint16>( row.claimed );
	log_id = static_cast<uint32>( inserted_id );
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

/// Donne les items du jour : dans l'inventaire tant qu'ils y entrent ; dès
/// qu'un item n'y entre plus (poids, cases, pile), lui et tous les suivants
/// partent par RODEX. Vrai si un courrier est parti.
static bool moonlight_attendance_deliver( map_session_data& sd, const s_moonlight_attendance_day& entry ){
	if( entry.event_points > 0 ){
		moonlight_attendance_credit_event_points( sd, entry.event_points );
	}

	std::vector<s_moonlight_attendance_reward> to_mail;

	for( const s_moonlight_attendance_reward& reward : entry.rewards ){
		if( !to_mail.empty() ){
			to_mail.push_back( reward );
			continue;
		}

		struct item it = {};

		it.nameid = reward.item_id;
		it.identify = 1;

		if( pc_additem( &sd, &it, reward.amount, LOG_TYPE_OTHER ) != ADDITEM_SUCCESS ){
			to_mail.push_back( reward );
		}
	}

	if( to_mail.empty() ){
		return false;
	}

	mail_message msg = {};

	msg.dest_id = sd.status.char_id;
	safestrncpy( msg.send_name, msg_txt( &sd, 788 ), NAME_LENGTH );
	safesnprintf( msg.title, MAIL_TITLE_LENGTH, msg_txt( &sd, 789 ), entry.day );
	safesnprintf( msg.body, MAIL_BODY_LENGTH, msg_txt( &sd, 790 ), entry.day );

	for( size_t i = 0; i < to_mail.size() && i < MAIL_MAX_ITEM; i++ ){
		msg.item[i].nameid = to_mail[i].item_id;
		msg.item[i].amount = to_mail[i].amount;
		msg.item[i].identify = 1;
	}

	msg.status = MAIL_NEW;
	msg.type = MAIL_INBOX_NORMAL;
	msg.timestamp = time( nullptr );

	intif_Mail_send( 0, &msg );
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

/// Livre les récompenses réclamées sur le site qui attendent le compte. Chaque
/// ligne se prend par un UPDATE conditionnel : une seule session la gagne.
static void moonlight_attendance_deliver_web_pending( map_session_data& sd ){
	if( Sql_Query( mmysql_handle,
			"SELECT `id`, `day` FROM `moonlight_attendance_log` WHERE `user_id` = '%u' AND `source` = 'web' AND `delivered_at` IS NULL ORDER BY `id`",
			sd.status.user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return;
	}

	std::vector<std::pair<uint32, uint16>> pending; // id, day

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		pending.emplace_back( moonlight_attendance_column_u32( 0 ), static_cast<uint16>( moonlight_attendance_column_u32( 1 ) ) );
	}

	Sql_FreeResult( mmysql_handle );

	bool delivered = false;

	for( const auto& [log_id, day] : pending ){
		std::shared_ptr<s_moonlight_attendance_day> entry = moonlight_attendance_db.find( day );

		// Sans le jour au calendrier, la ligne reste en attente : elle se
		// livrera quand le calendrier sera de nouveau complet.
		if( entry == nullptr ){
			ShowWarning( "moonlight_attendance: r\xe9" "compense web %u (jour %hu) en attente : jour absent du calendrier.\n", log_id, day );
			continue;
		}

		if( Sql_Query( mmysql_handle,
				"UPDATE `moonlight_attendance_log` SET `delivered_at` = NOW(), `char_id` = '%u' WHERE `id` = '%u' AND `delivered_at` IS NULL",
				sd.status.char_id, log_id ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
			continue;
		}

		if( Sql_NumRowsAffected( mmysql_handle ) != 1 ){
			continue; // une autre session l'a prise
		}

		const bool by_mail = moonlight_attendance_deliver( sd, *entry );

		if( by_mail ){
			moonlight_attendance_mark_by_mail( log_id );
		}

		moonlight_attendance_say_delivered( sd, day, true, by_mail );
		delivered = true;
	}

	if( delivered ){
		chrif_save( &sd, CSAVE_INVENTORY );
	}
}

/* ------------------------------------------------------------------------- *
 * Le paquet et les demandes
 * ------------------------------------------------------------------------- */

void moonlight_attendance_send( map_session_data& sd, bool open, uint8 result ){
	if( !session_isActive( sd.fd ) ){
		return;
	}

	const size_t entries_length = moonlight_attendance_entries.size() * sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE_sub );
	const size_t length = sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE ) + entries_length;

	WFIFOHEAD( sd.fd, length );

	PACKET_ZC_MOONLIGHT_ATTENDANCE& p = *reinterpret_cast<PACKET_ZC_MOONLIGHT_ATTENDANCE*>( WFIFOP( sd.fd, 0 ) );

	memset( &p, 0, sizeof( p ) );
	p.packetType = HEADER_ZC_MOONLIGHT_ATTENDANCE;
	p.packetLength = static_cast<int16>( length );
	p.open = open ? 1 : 0;
	p.result = result;
	p.cycleLength = MOONLIGHT_ATTENDANCE_CYCLE_DAYS;
	p.daysLeft = MOONLIGHT_ATTENDANCE_CYCLE_DAYS;
	p.webChargesLeft = MOONLIGHT_ATTENDANCE_WEB_CHARGES;
	p.webChargesMax = MOONLIGHT_ATTENDANCE_WEB_CHARGES;

	s_moonlight_attendance_row row;

	if( result != MOONLIGHT_ATTENDANCE_CLOSED && sd.status.user_id != 0 && moonlight_attendance_read_state( sd.status.user_id, row ) ){
		p.claimedToday = row.last_claim != 0 && row.last_claim == row.today;
		p.secondsUntilReset = row.seconds_until_reset;

		// Le cycle dont le jour vient d'être réclamé reste affiché, même plein :
		// le suivant ne s'ouvre qu'à la prochaine réclamation.
		if( !moonlight_attendance_cycle_to_open( row ) || p.claimedToday ){
			p.claimedDays = static_cast<uint8>( std::min<uint32>( row.claimed, MOONLIGHT_ATTENDANCE_CYCLE_DAYS ) );
			p.daysLeft = static_cast<uint8>( std::clamp<int64>( MOONLIGHT_ATTENDANCE_CYCLE_DAYS - row.cycle_age_days, 0, MOONLIGHT_ATTENDANCE_CYCLE_DAYS ) );
			p.webChargesLeft = static_cast<uint8>( MOONLIGHT_ATTENDANCE_WEB_CHARGES - std::min<uint32>( row.web_used, MOONLIGHT_ATTENDANCE_WEB_CHARGES ) );
			p.webClaimedMask = moonlight_attendance_web_mask( sd.status.user_id, row.cycle_start );
			p.cycleStart = row.cycle_start;
		}
	}

	if( entries_length > 0 ){
		memcpy( WFIFOP( sd.fd, sizeof( PACKET_ZC_MOONLIGHT_ATTENDANCE ) ), moonlight_attendance_entries.data(), entries_length );
	}

	WFIFOSET( sd.fd, length );
}

/// La présence est-elle permise à ce joueur ? Sinon le dit dans le chat.
static bool moonlight_attendance_allowed( map_session_data& sd ){
	if( !pc_has_permission( &sd, PC_PERM_ATTENDANCE ) ){
		clif_messagecolor( &sd, color_table[COLOR_RED], msg_txt( &sd, 791 ), false, SELF ); // You are not allowed to use the attendance system.
		return false;
	}

	return moonlight_attendance_open() && sd.status.user_id != 0;
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

	switch( moonlight_attendance_claim_sql( sd, day, log_id ) ){
		case MOONLIGHT_ATTENDANCE_CLAIM_ALREADY:
			moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_ALREADY_CLAIMED );
			return;
		case MOONLIGHT_ATTENDANCE_CLAIM_ERROR:
			clif_messagecolor( &sd, color_table[COLOR_RED], "La pr\xe9sence est momentan\xe9ment indisponible.", false, SELF );
			moonlight_attendance_send( sd, false, MOONLIGHT_ATTENDANCE_STATE );
			return;
		default:
			break;
	}

	std::shared_ptr<s_moonlight_attendance_day> entry = moonlight_attendance_db.find( day );
	bool by_mail = false;

	if( entry != nullptr ){
		by_mail = moonlight_attendance_deliver( sd, *entry );

		if( by_mail ){
			moonlight_attendance_mark_by_mail( log_id );
		}

		chrif_save( &sd, CSAVE_INVENTORY );
		moonlight_attendance_say_delivered( sd, day, false, by_mail );
	}else{
		ShowError( "moonlight_attendance: jour %hu r\xe9" "clam\xe9 par le compte %u, absent du calendrier.\n", day, sd.status.user_id );
	}

	moonlight_attendance_send( sd, false, by_mail ? MOONLIGHT_ATTENDANCE_CLAIMED_BY_MAIL : MOONLIGHT_ATTENDANCE_CLAIMED );
}

void moonlight_attendance_login( map_session_data& sd ){
	if( sd.status.user_id == 0 ){
		return;
	}

	// Une récompense réclamée sur le site est due : elle se livre même si la
	// présence a été fermée depuis.
	moonlight_attendance_deliver_web_pending( sd );

	if( !pc_has_permission( &sd, PC_PERM_ATTENDANCE ) || !moonlight_attendance_open() ){
		return;
	}

	s_moonlight_attendance_row row;

	if( moonlight_attendance_read_state( sd.status.user_id, row ) && row.last_claim != row.today ){
		moonlight_attendance_send( sd, true, MOONLIGHT_ATTENDANCE_STATE );
	}
}

void do_init_moonlight_attendance( void ){
	moonlight_attendance_db.load();
}

void do_final_moonlight_attendance( void ){
	moonlight_attendance_db.clear();
	moonlight_attendance_entries.clear();
}
