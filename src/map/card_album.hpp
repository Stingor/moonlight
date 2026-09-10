// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CARD_ALBUM_HPP
#define CARD_ALBUM_HPP

#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp> // t_itemid

class map_session_data;

/**
 * Card album [Stingor] - a card container that lives OUTSIDE the storage system.
 *
 * A storage caps at MAX_STORAGE (600) and that cap is not negotiable: `struct
 * s_storage` travels between map-server and char-server as a raw memcpy inside a
 * packet whose length is a uint16 (see mapif_storage_data_loaded in
 * src/char/int_storage.cpp). The hard ceiling of that design is 850 slots. The
 * album does not try to raise the cap - it steps out of it.
 *
 * It can, because a CARD carries no instance state: no refine, no options, no
 * inserted cards, no attribute. The pair (nameid, amount) describes it entirely,
 * where a storage slot drags 77 bytes of `struct item` around. So the album is a
 * plain SQL table written directly by the map-server, and the char-server is
 * never involved.
 *
 * ── THE RULE ────────────────────────────────────────────────────────────────
 *
 * The FIRST copy of a card is SACRIFICED: it is consumed and permanently unlocks
 * that card's slot in the album. Further copies stack there and stay
 * withdrawable. A card that was never sacrificed cannot be stored at all.
 *
 * 🔴 THE ROW'S EXISTENCE IS THE UNLOCK. There is no `unlocked` column: a boolean
 * that is always true starts lying the day an error path forgets to write it. So
 * `amount == 0` is a NORMAL state (unlocked slot, empty reserve) and a row is
 * NEVER deleted - that would hand back a sacrifice the player already paid.
 *
 * ── IDENTITY ────────────────────────────────────────────────────────────────
 *
 * Keyed by the MOONLIGHT ACCOUNT (login.user_id), never by character nor by game
 * account: same choice as user_ignore and mvp_favorite. Sacrifices already paid
 * therefore benefit every character on every game account of that person.
 *
 * See sql-files/card_album.sql for the schema and the reasoning behind it.
 */

/// One unlocked slot. The entry EXISTS as soon as the card was sacrificed once,
/// so `amount == 0` means "unlocked, reserve empty" and not "absent".
struct s_card_album_entry {
	t_itemid nameid;
	uint16 amount;
};

/// Why an album operation was refused. Travels to the client in the `result`
/// field of ZC_BOURGEON_CARD_ALBUM, which is why it is a stable uint8 enum:
/// the client maps it to a localized message.
enum e_card_album_result : uint8 {
	CARD_ALBUM_OK = 0,
	CARD_ALBUM_FAIL,             ///< generic refusal (bad index, empty request)
	CARD_ALBUM_NOT_A_CARD,       ///< not IT_CARD, or an enchant (CARD_ENCHANT)
	CARD_ALBUM_LOCKED,           ///< deposit attempted on a slot never sacrificed
	CARD_ALBUM_ALREADY_UNLOCKED, ///< sacrifice attempted on an open slot
	CARD_ALBUM_NOT_ENOUGH,       ///< withdrawing more than the reserve holds
	CARD_ALBUM_INVENTORY_FULL,   ///< pc_additem refused
	CARD_ALBUM_BUSY,             ///< trading, or a storage is open
	CARD_ALBUM_BOUND,            ///< bound or rental card: never reducible to (nameid, amount)
	CARD_ALBUM_STACK_FULL,       ///< the reserve is at MAX_AMOUNT
	/// This game account is not linked to any Moonlight account (status.user_id
	/// == 0), and the album is keyed by that account: there is nothing to write.
	/// Deliberately NOT folded into FAIL - it is a configuration problem, not a
	/// gameplay refusal, and the two are diagnosed in completely different places.
	CARD_ALBUM_NO_ACCOUNT,
	/// Another game account of the same Moonlight account holds the album open.
	/// One holder at a time, like a storage: the client-side view of the other
	/// session would be stale, and stale views are how duplications start.
	CARD_ALBUM_IN_USE,
	/// The session never opened the album (or lost it): mutations need the lock.
	CARD_ALBUM_NOT_OPEN,
};

/// One card of the catalog. `equip` is the slot mask of the gear this card goes
/// into (ARMOR, HAND_R, …): the client cannot derive it - it has no item_db - and
/// it is what lets the album be filtered by target slot, the way the storage
/// filters its card tab.
struct s_card_album_card {
	t_itemid nameid;
	uint32 equip;
};

/// Every card the album accepts, ascending by server nameid. Built once from
/// item_db and cached: it is identical for every player.
const std::vector<s_card_album_card>& card_album_catalog( void );

/// IT_CARD with subtype CARD_NORMAL. Enchants also carry Type: Card but belong
/// to gear, not to a collection - see db/CLAUDE.md.
bool card_album_is_collectible( t_itemid nameid );

/// Loads the album from SQL into sd->card_album. Called once at login, next to
/// pc_ignorechat_load().
void card_album_load( map_session_data* sd );

/// The unlocked slot for that card, or nullptr if it was never sacrificed.
const s_card_album_entry* card_album_find( map_session_data* sd, t_itemid nameid );

/**
 * ── EXCLUSIVITY ─────────────────────────────────────────────────────────────
 *
 * The album is keyed by the Moonlight account, and one Moonlight account can
 * have several GAME accounts online at once - each a separate session with its
 * own cached copy. Like a storage, the album is therefore HELD by one session at
 * a time: card_album_open() takes the lock (or refuses with CARD_ALBUM_IN_USE),
 * every mutation requires it, card_album_close() releases it - and so does
 * map_quit(), so a crash never leaves it stuck. A holder that is no longer
 * online is overridden, not waited for.
 *
 * This is the first guard; the SQL statements in the .cpp are the second. Both
 * exist because the first version of this file duplicated a stack with exactly
 * two clients open.
 */
e_card_album_result card_album_open( map_session_data* sd );
void card_album_close( map_session_data* sd );
bool card_album_is_open( map_session_data* sd );

/**
 * ── LES PALIERS DE COLLECTION ───────────────────────────────────────────────
 *
 * Neuf succes (900001..900009, db/import/achievement_db.yml) comptent les
 * pochettes ouvertes : 1, 5, 10, 20, 50, 100, 200, 400, 800. Le groupe
 * AG_CARD_ALBUM (src/map/achievement.hpp) lit sd->card_album LUI-MEME, donc la
 * boucle de rattrapage du login les accorde a TOUT personnage du compte — c'est
 * voulu : l'album est au compte, ses paliers doivent l'etre aussi.
 *
 * 🔴 Le LOT, lui, ne se touche qu'une fois : la progression de succes est rangee
 * sur le COMPTE MOONLIGHT (src/char/int_achievement.cpp), donc `rewarded` vit
 * sur une ligne unique pour toute la personne. « Creer un personnage, encaisser,
 * supprimer » ne redonne rien. Une table card_album_reward a existe ici pour
 * tenir cette garde a la main, du temps ou les succes etaient au compte de JEU ;
 * elle a disparu avec sa raison d'etre.
 */

/// Sacrifices one copy from inventory to open the slot. Fails if the slot is
/// already open - that case is a deposit, and the caller must say which it meant.
e_card_album_result card_album_unlock( map_session_data* sd, int16 inv_index );

/// Moves copies from inventory into an ALREADY unlocked reserve.
e_card_album_result card_album_put( map_session_data* sd, int16 inv_index, uint16 amount );

/// Moves copies from the reserve back to inventory. The row survives at zero.
e_card_album_result card_album_get( map_session_data* sd, t_itemid nameid, uint16 amount );

#endif /* CARD_ALBUM_HPP */
