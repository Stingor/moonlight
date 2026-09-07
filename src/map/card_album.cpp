// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "card_album.hpp"

#include <algorithm>
#include <cstdlib> // strtoul
#include <unordered_map>

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>

#include "clif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp" // mmysql_handle
#include "pc.hpp"

/* ------------------------------------------------------------------------- *
 * The catalog
 * ------------------------------------------------------------------------- */

/**
 * Every card the album accepts, ascending by server nameid.
 *
 * Built once and cached, like clif_bourgeon_build_hateffect_map(): it is derived
 * from item_db, so it is the same for every player and cannot change while the
 * server runs.
 *
 * 🔴 The CARD_NORMAL test is what keeps enchants out. Both carry Type: Card;
 * enchants add SubType: Enchant and belong to gear, not to a collection. In
 * practice they are not obtainable, so this is a belt-and-braces filter - which
 * is exactly why it must stay: the day one leaks into an inventory, the album
 * refuses it instead of swallowing an item whose bonus lives in its script.
 */
const std::vector<s_card_album_card>& card_album_catalog( void ){
	static std::vector<s_card_album_card> cache;
	static bool built = false;

	if( built ){
		return cache;
	}

	built = true;

	for( const auto& entry : item_db ){
		const std::shared_ptr<item_data>& idata = entry.second;

		if( idata == nullptr ){
			continue;
		}

		if( idata->type != IT_CARD || idata->subtype != CARD_NORMAL ){
			continue;
		}

		s_card_album_card card = {};

		card.nameid = idata->nameid;
		card.equip = idata->equip;

		cache.push_back( card );
	}

	// item_db is an unordered_map: without this the catalog order would differ
	// from one boot to the next, and so would the album the player sees.
	std::sort( cache.begin(), cache.end(),
		[]( const s_card_album_card& a, const s_card_album_card& b ){ return a.nameid < b.nameid; } );

	ShowStatus( "Card album: %lu collectible cards.\n", static_cast<unsigned long>( cache.size() ) );

	return cache;
}

bool card_album_is_collectible( t_itemid nameid ){
	std::shared_ptr<item_data> idata = item_db.find( nameid );

	if( idata == nullptr ){
		return false;
	}

	return idata->type == IT_CARD && idata->subtype == CARD_NORMAL;
}

/* ------------------------------------------------------------------------- *
 * In-memory album - a CACHE of the SQL row, never the other way round
 * ------------------------------------------------------------------------- *
 *
 * 🔴 THE ROW IS THE TRUTH; sd->card_album ONLY MIRRORS IT. The album is keyed by
 * the Moonlight account, and one Moonlight account can have SEVERAL game
 * accounts online at once - each with its own map_session_data, each with its
 * own copy of the album loaded at its own login. The first version of this file
 * wrote the reserve back as an ABSOLUTE value taken from that copy: session A
 * withdrew 10, SQL went to 0, session B still remembered 10 and withdrew them
 * again. A duplication, reproduced by the author with two clients.
 *
 * Hence the rule every operation below follows:
 *   1. the mutation is ONE conditional, RELATIVE SQL statement
 *      (`amount = amount - N WHERE amount >= N`, `INSERT IGNORE`, ...);
 *   2. the number of AFFECTED ROWS decides whether the operation happened -
 *      never the cached amount;
 *   3. the inventory moves only AFTER SQL said yes;
 *   4. the cache is then re-read from the row (card_album_refresh_entry).
 * Two sessions can race all they want: MySQL serialises the row, and the loser
 * of the race gets a refusal instead of a copy.
 */

const s_card_album_entry* card_album_find( map_session_data* sd, t_itemid nameid ){
	if( sd == nullptr ){
		return nullptr;
	}

	for( const s_card_album_entry& entry : sd->card_album ){
		if( entry.nameid == nameid ){
			return &entry;
		}
	}

	return nullptr;
}

/// Inserts or updates the cached entry, keeping the vector sorted by nameid:
/// card_album_load() returns it that way and the packet builder relies on it.
static void card_album_cache_set( map_session_data* sd, t_itemid nameid, uint16 amount ){
	s_card_album_entry entry = {};

	entry.nameid = nameid;
	entry.amount = amount;

	auto it = std::lower_bound( sd->card_album.begin(), sd->card_album.end(), entry,
		[]( const s_card_album_entry& a, const s_card_album_entry& b ){ return a.nameid < b.nameid; } );

	if( it != sd->card_album.end() && it->nameid == nameid ){
		it->amount = amount;
	}else{
		sd->card_album.insert( it, entry );
	}
}

static void card_album_cache_erase( map_session_data* sd, t_itemid nameid ){
	sd->card_album.erase(
		std::remove_if( sd->card_album.begin(), sd->card_album.end(),
			[nameid]( const s_card_album_entry& e ){ return e.nameid == nameid; } ),
		sd->card_album.end() );
}

/**
 * Re-reads ONE row and aligns the cache on it. Returns the cached entry, or
 * nullptr if the row does not exist (slot never unlocked) or SQL failed.
 *
 * Called after every mutation, and whenever a conditional UPDATE affected no
 * row - the reason (locked slot, empty reserve, full stack) is in the row, not
 * in the cache that just proved stale.
 */
static const s_card_album_entry* card_album_refresh_entry( map_session_data* sd, t_itemid nameid ){
	if( Sql_Query( mmysql_handle,
			"SELECT `amount` FROM `card_album` WHERE `user_id` = '%u' AND `nameid` = '%u'",
			sd->status.user_id, nameid ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return nullptr;
	}

	bool found = false;
	uint16 amount = 0;

	if( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* amount_data = nullptr;

		if( Sql_GetData( mmysql_handle, 0, &amount_data, nullptr ) == SQL_SUCCESS && amount_data != nullptr ){
			found = true;
			amount = static_cast<uint16>( strtoul( amount_data, nullptr, 10 ) );
		}
	}

	Sql_FreeResult( mmysql_handle );

	if( !found ){
		card_album_cache_erase( sd, nameid );
		return nullptr;
	}

	card_album_cache_set( sd, nameid, amount );

	return card_album_find( sd, nameid );
}

/**
 * Loads the album at login, in one query, the way pc_ignorechat_load() does -
 * and again on every explicit refresh from the client, so a second session of
 * the same Moonlight account sees what the first one did.
 *
 * Ordered by nameid so the vector stays sorted without a second pass, and so the
 * packet the client receives is stable across sessions.
 */
void card_album_load( map_session_data* sd ){
	nullpo_retv( sd );

	sd->card_album.clear();

	// No Moonlight account means no album to key. Happens on a game account that
	// was never linked; the player simply has no album rather than a broken one.
	if( sd->status.user_id == 0 ){
		return;
	}

	if( Sql_Query( mmysql_handle,
			"SELECT `nameid`, `amount` FROM `card_album` WHERE `user_id` = '%u' ORDER BY `nameid` ASC",
			sd->status.user_id ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return;
	}

	while( Sql_NextRow( mmysql_handle ) == SQL_SUCCESS ){
		char* nameid_data;
		char* amount_data;

		if( Sql_GetData( mmysql_handle, 0, &nameid_data, nullptr ) != SQL_SUCCESS || nameid_data == nullptr ){
			continue;
		}

		if( Sql_GetData( mmysql_handle, 1, &amount_data, nullptr ) != SQL_SUCCESS || amount_data == nullptr ){
			continue;
		}

		s_card_album_entry entry = {};

		entry.nameid = static_cast<t_itemid>( strtoul( nameid_data, nullptr, 10 ) );
		entry.amount = static_cast<uint16>( strtoul( amount_data, nullptr, 10 ) );

		// A card removed from item_db since the row was written stays in SQL - we
		// do not delete player data on a db edit - but it is not shown or moved.
		if( entry.nameid == 0 || !card_album_is_collectible( entry.nameid ) ){
			continue;
		}

		sd->card_album.push_back( entry );
	}

	Sql_FreeResult( mmysql_handle );
}

/* ------------------------------------------------------------------------- *
 * Exclusivity - one holder per Moonlight account
 * ------------------------------------------------------------------------- */

/// user_id -> account_id of the session holding the album. A session is in
/// here exactly while its `state.card_album_open` bit is set.
static std::unordered_map<uint32, uint32> card_album_holders;

bool card_album_is_open( map_session_data* sd ){
	return sd != nullptr && sd->state.card_album_open != 0;
}

e_card_album_result card_album_open( map_session_data* sd ){
	nullpo_retr( CARD_ALBUM_FAIL, sd );

	if( sd->status.user_id == 0 ){
		return CARD_ALBUM_NO_ACCOUNT;
	}

	if( card_album_is_open( sd ) ){
		return CARD_ALBUM_OK;
	}

	auto it = card_album_holders.find( sd->status.user_id );

	if( it != card_album_holders.end() && it->second != sd->status.account_id ){
		map_session_data* holder = map_id2sd( it->second );

		// Still online and still holding: refused. Gone (or somehow without the
		// bit): the lock is stale, and we take it - map_quit() should have
		// released it, this is the belt to that pair of braces.
		if( holder != nullptr && card_album_is_open( holder ) ){
			ShowInfo( "card_album_open: user %u refused to account %u, held by account %u.\n",
				sd->status.user_id, sd->status.account_id, it->second );
			return CARD_ALBUM_IN_USE;
		}

		ShowWarning( "card_album_open: stale album lock of user %u (account %u) taken over by account %u.\n",
			sd->status.user_id, it->second, sd->status.account_id );
	}

	card_album_holders[sd->status.user_id] = sd->status.account_id;
	sd->state.card_album_open = 1;

	return CARD_ALBUM_OK;
}

void card_album_close( map_session_data* sd ){
	if( sd == nullptr || !card_album_is_open( sd ) ){
		return;
	}

	sd->state.card_album_open = 0;

	auto it = card_album_holders.find( sd->status.user_id );

	if( it != card_album_holders.end() && it->second == sd->status.account_id ){
		card_album_holders.erase( it );
	}
}

/* ------------------------------------------------------------------------- *
 * Operations
 * ------------------------------------------------------------------------- */

/**
 * Shared gate for anything that moves a card between inventory and album.
 *
 * Returns CARD_ALBUM_OK and fills `nameid` when the inventory slot holds a card
 * this album may take; a refusal code otherwise.
 *
 * 🔴 Bound and rental cards are refused outright. Reducing an item to
 * (nameid, amount) throws away `bound` and `expire_time`, so accepting one would
 * silently launder a bound card into a free one. Refusing is correct today (no
 * such card exists) and stays correct the day a script hands one out.
 */
static e_card_album_result card_album_check_inventory( map_session_data* sd, int16 inv_index, t_itemid* nameid, uint16 amount ){
	if( sd->status.user_id == 0 ){
		return CARD_ALBUM_NO_ACCOUNT;
	}

	// 🔴 The lock, before anything moves. A session that does not hold the
	// album cannot touch it - whatever its cache believes.
	if( !card_album_is_open( sd ) ){
		return CARD_ALBUM_NOT_OPEN;
	}

	// Same guards as storage: a card must not move while its owner is trading or
	// has a storage open, or the two containers could disagree on who holds it.
	if( pc_istrading( sd ) || sd->state.storage_flag != 0 ){
		return CARD_ALBUM_BUSY;
	}

	if( inv_index < 0 || inv_index >= MAX_INVENTORY ){
		return CARD_ALBUM_FAIL;
	}

	const struct item& it = sd->inventory.u.items_inventory[inv_index];

	if( it.nameid == 0 || it.amount <= 0 ){
		return CARD_ALBUM_FAIL;
	}

	// it.amount est un int16 et `amount` un uint16 : on compare en int32 pour que
	// la promotion ne dépende pas du compilateur.
	if( amount == 0 || static_cast<int32>( amount ) > static_cast<int32>( it.amount ) ){
		return CARD_ALBUM_FAIL;
	}

	if( !card_album_is_collectible( it.nameid ) ){
		return CARD_ALBUM_NOT_A_CARD;
	}

	if( it.bound != BOUND_NONE || it.expire_time != 0 ){
		return CARD_ALBUM_BOUND;
	}

	*nameid = it.nameid;

	return CARD_ALBUM_OK;
}

e_card_album_result card_album_unlock( map_session_data* sd, int16 inv_index ){
	nullpo_retr( CARD_ALBUM_FAIL, sd );

	t_itemid nameid = 0;
	e_card_album_result check = card_album_check_inventory( sd, inv_index, &nameid, 1 );

	if( check != CARD_ALBUM_OK ){
		return check;
	}

	// Already open: this is a deposit, and the caller has to say so. Silently
	// turning a sacrifice into a deposit (or the reverse) would cost the player a
	// card they meant to keep. The cache answers first; SQL has the last word.
	if( card_album_find( sd, nameid ) != nullptr ){
		return CARD_ALBUM_ALREADY_UNLOCKED;
	}

	if( Sql_Query( mmysql_handle,
			"INSERT IGNORE INTO `card_album` (`user_id`, `nameid`, `amount`) VALUES ('%u', '%u', '0')",
			sd->status.user_id, nameid ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return CARD_ALBUM_FAIL;
	}

	// Ignored = the row was there already: another session of this Moonlight
	// account opened the slot first. The card is NOT consumed - the player would
	// pay a sacrifice for nothing - and the cache learns about the row.
	if( Sql_NumRowsAffected( mmysql_handle ) == 0 ){
		card_album_refresh_entry( sd, nameid );
		return CARD_ALBUM_ALREADY_UNLOCKED;
	}

	// SQL first, inventory second: if the INSERT fails the player keeps the card.
	// The reverse order could eat it and unlock nothing.
	pc_delitem( sd, inv_index, 1, 0, 4, LOG_TYPE_STORAGE );

	card_album_cache_set( sd, nameid, 0 );

	return CARD_ALBUM_OK;
}

e_card_album_result card_album_put( map_session_data* sd, int16 inv_index, uint16 amount ){
	nullpo_retr( CARD_ALBUM_FAIL, sd );

	t_itemid nameid = 0;
	e_card_album_result check = card_album_check_inventory( sd, inv_index, &nameid, amount );

	if( check != CARD_ALBUM_OK ){
		return check;
	}

	// Never unlocked: the album has no slot for this card yet, and opening one
	// costs a sacrifice the player has to accept explicitly. Quick answer from
	// the cache; a stale cache is corrected by the UPDATE below, which cannot
	// touch a row that does not exist.
	if( card_album_find( sd, nameid ) == nullptr ){
		return CARD_ALBUM_LOCKED;
	}

	// One RELATIVE, CONDITIONAL statement: the reserve grows by `amount` only if
	// the row exists AND the stack has that much room. Whatever another session
	// did meanwhile is already in the row.
	auto try_put = [&]( uint16 n ) -> bool {
		if( Sql_Query( mmysql_handle,
				"UPDATE `card_album` SET `amount` = `amount` + '%u' WHERE `user_id` = '%u' AND `nameid` = '%u' AND `amount` <= '%u'",
				n, sd->status.user_id, nameid, static_cast<uint32>( MAX_AMOUNT ) - n ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
			return false;
		}

		return Sql_NumRowsAffected( mmysql_handle ) > 0;
	};

	if( !try_put( amount ) ){
		// No row moved: locked after all, or not enough room. Ask the row.
		const s_card_album_entry* entry = card_album_refresh_entry( sd, nameid );

		if( entry == nullptr ){
			return CARD_ALBUM_LOCKED;
		}

		if( entry->amount >= MAX_AMOUNT ){
			return CARD_ALBUM_STACK_FULL;
		}

		// Partial room: store what fits, as the first version did - but with the
		// room read from the row a moment ago, and still under the same guard.
		amount = static_cast<uint16>( MAX_AMOUNT - entry->amount );

		if( !try_put( amount ) ){
			card_album_refresh_entry( sd, nameid );
			return CARD_ALBUM_STACK_FULL;
		}
	}

	// SQL said yes for exactly `amount`: only now does the inventory lose them.
	pc_delitem( sd, inv_index, amount, 0, 4, LOG_TYPE_STORAGE );
	card_album_refresh_entry( sd, nameid );

	return CARD_ALBUM_OK;
}

e_card_album_result card_album_get( map_session_data* sd, t_itemid nameid, uint16 amount ){
	nullpo_retr( CARD_ALBUM_FAIL, sd );

	if( sd->status.user_id == 0 ){
		return CARD_ALBUM_NO_ACCOUNT;
	}

	if( !card_album_is_open( sd ) ){
		return CARD_ALBUM_NOT_OPEN;
	}

	if( pc_istrading( sd ) || sd->state.storage_flag != 0 ){
		return CARD_ALBUM_BUSY;
	}

	if( amount == 0 || amount > MAX_AMOUNT || !card_album_is_collectible( nameid ) ){
		return CARD_ALBUM_FAIL;
	}

	// Quick refusals from the cache. Both are re-checked by the UPDATE: a cache
	// that says "10 in reserve" while another session just took them is exactly
	// the case this function exists to survive.
	const s_card_album_entry* cached = card_album_find( sd, nameid );

	if( cached == nullptr ){
		return CARD_ALBUM_LOCKED;
	}

	if( cached->amount < amount ){
		return CARD_ALBUM_NOT_ENOUGH;
	}

	// Inventory room checked BEFORE the reserve is touched: the common refusal
	// (full inventory, over-stacked slot) must not need a compensation write.
	if( pc_checkadditem( sd, nameid, amount ) == CHKADDITEM_OVERAMOUNT ){
		return CARD_ALBUM_INVENTORY_FULL;
	}

	if( pc_checkadditem( sd, nameid, amount ) == CHKADDITEM_NEW && pc_inventoryblank( sd ) == 0 ){
		return CARD_ALBUM_INVENTORY_FULL;
	}

	// The reserve shrinks by `amount` only if it holds that much - RIGHT NOW, in
	// the row. Zero rows affected = someone else emptied it first.
	if( Sql_Query( mmysql_handle,
			"UPDATE `card_album` SET `amount` = `amount` - '%u' WHERE `user_id` = '%u' AND `nameid` = '%u' AND `amount` >= '%u'",
			amount, sd->status.user_id, nameid, amount ) != SQL_SUCCESS ){
		Sql_ShowDebug( mmysql_handle );
		return CARD_ALBUM_FAIL;
	}

	if( Sql_NumRowsAffected( mmysql_handle ) == 0 ){
		// The cache said yes, the row said no: something moved this reserve
		// behind this session's back. Worth a line in the log - it is either a
		// second session (the lock should have prevented it) or a hand edit.
		ShowWarning( "card_album_get: stale reserve for user %u (account %u): cache %u, wanted %u x %u - refused.\n",
			sd->status.user_id, sd->status.account_id, cached->amount, amount, nameid );
		return card_album_refresh_entry( sd, nameid ) == nullptr ? CARD_ALBUM_LOCKED : CARD_ALBUM_NOT_ENOUGH;
	}

	struct item it = {};

	it.nameid = nameid;
	it.identify = 1;

	if( pc_additem( sd, &it, amount, LOG_TYPE_STORAGE ) != ADDITEM_SUCCESS ){
		// The row already gave the cards away (weight, most likely - not covered
		// by the checks above). Put them back the same relative way; the player
		// keeps his reserve and gets a refusal.
		if( Sql_Query( mmysql_handle,
				"UPDATE `card_album` SET `amount` = `amount` + '%u' WHERE `user_id` = '%u' AND `nameid` = '%u'",
				amount, sd->status.user_id, nameid ) != SQL_SUCCESS ){
			Sql_ShowDebug( mmysql_handle );
			ShowError( "card_album_get: could not return %u x %u to user %u after a failed pc_additem.\n",
				amount, nameid, sd->status.user_id );
		}

		card_album_refresh_entry( sd, nameid );
		return CARD_ALBUM_INVENTORY_FULL;
	}

	// The row stays at zero on purpose: it is the proof the slot was paid for.
	card_album_refresh_entry( sd, nameid );

	return CARD_ALBUM_OK;
}
