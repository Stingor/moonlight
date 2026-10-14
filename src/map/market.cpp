// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "market.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <common/db.hpp>
#include <common/nullpo.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "buyingstore.hpp"
#include "clif.hpp"
#include "itemdb.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "packets.hpp"
#include "pc.hpp"
#include "shop_finder.hpp"
#include "vending.hpp"

namespace {

// Ce que la demande retient, une fois lue et bornée.
struct s_market_query {
	uint16 query_id = 0;
	uint8 kind = MOONLIGHT_MARKET_VENDING;
	uint8 category = MOONLIGHT_MARKET_ANY_TYPE;
	uint8 refine_min = 0;
	uint8 refine_max = 0;
	uint8 flags = 0;
	uint32 price_min = 0;
	uint32 price_max = 0;
	uint32 store_id = 0;
	uint32 owner_aid = 0;
	std::vector<t_itemid> item_ids;
};

// La catégorie de filtre d'un type d'objet.
uint8 market_category_of(uint8 type) {
	switch (type) {
	case IT_HEALING:
	case IT_USABLE:
	case IT_DELAYCONSUME:
	case IT_CASH:
		return MOONLIGHT_MARKET_USABLE;
	case IT_WEAPON:
		return MOONLIGHT_MARKET_WEAPON;
	case IT_ARMOR:
	case IT_SHADOWGEAR:
		return MOONLIGHT_MARKET_ARMOR;
	case IT_CARD:
		return MOONLIGHT_MARKET_CARD;
	case IT_AMMO:
		return MOONLIGHT_MARKET_AMMO;
	case IT_PETEGG:
	case IT_PETARMOR:
		return MOONLIGHT_MARKET_PET;
	default:
		return MOONLIGHT_MARKET_ETC;
	}
}

// Une carte sertie : le premier emplacement n'est pas la marque d'un objet forgé,
// fabriqué ou d'un œuf, et un emplacement au moins est occupé.
bool market_has_cards(const item& it) {
	if (itemdb_isspecial(it.card[0]))
		return false;
	return std::any_of(std::begin(it.card), std::end(it.card), [](t_itemid card) { return card != 0; });
}

bool market_has_options(const item& it) {
	return std::any_of(std::begin(it.option), std::end(it.option),
	                   [](const s_item_randomoption& option) { return option.id != 0; });
}

// L'objet passe-t-il les filtres de la demande ? Le prix est jugé à part.
bool market_item_matches(const s_market_query& query, const item& it) {
	if (!query.item_ids.empty() &&
	    std::find(query.item_ids.begin(), query.item_ids.end(), it.nameid) == query.item_ids.end())
		return false;
	if (query.category != MOONLIGHT_MARKET_ANY_TYPE) {
		std::shared_ptr<item_data> data = item_db.find(it.nameid);
		if (data == nullptr || market_category_of(data->type) != query.category)
			return false;
	}
	const uint8 refine = static_cast<uint8>(it.refine);
	if (refine < query.refine_min || refine > query.refine_max)
		return false;
	if ((query.flags & MOONLIGHT_MARKET_WITH_CARDS) && !market_has_cards(it))
		return false;
	if ((query.flags & MOONLIGHT_MARKET_WITH_OPTIONS) && !market_has_options(it))
		return false;
	return true;
}

bool market_price_matches(const s_market_query& query, uint32 price) {
	return price >= query.price_min && (query.price_max == 0 || price <= query.price_max);
}

// L'entrée commune aux deux sortes d'échoppe : le marchand, sa place et son titre.
PACKET_MOONLIGHT_MARKET_ENTRY market_entry_of(const map_session_data& owner, uint32 store_id) {
	PACKET_MOONLIGHT_MARKET_ENTRY entry{};
	entry.store_id = store_id;
	entry.owner_aid = owner.status.account_id;
	entry.autotrade = owner.state.autotrade ? 1 : 0;
	if (const map_data* mapdata = map_getmapdata(owner.m); mapdata != nullptr)
		safestrncpy(entry.map, mapdata->name, sizeof(entry.map));
	entry.x = owner.x;
	entry.y = owner.y;
	safestrncpy(entry.title, owner.message, sizeof(entry.title));
	return entry;
}

void market_add_vending(const s_market_query& query, const map_session_data& owner,
                        std::vector<PACKET_MOONLIGHT_MARKET_ENTRY>& found) {
	const bool whole_store = query.store_id != 0;
	for (int32 slot = 0; slot < owner.vend_num; ++slot) {
		const s_vending& sale = owner.vending[slot];
		const item& it = owner.cart.u.items_cart[sale.index];
		if (it.nameid == 0 || sale.amount <= 0)
			continue;
		if (!whole_store && (!market_item_matches(query, it) || !market_price_matches(query, sale.value)))
			continue;
		PACKET_MOONLIGHT_MARKET_ENTRY entry = market_entry_of(owner, static_cast<uint32>(owner.vender_id));
		entry.itemId = it.nameid;
		entry.amount = static_cast<uint16>(sale.amount);
		entry.price = sale.value;
		entry.refine = static_cast<uint8>(it.refine);
		entry.grade = it.enchantgrade;
		entry.identified = it.identify ? 1 : 0;
		entry.damaged = it.attribute ? 1 : 0;
		for (int32 card = 0; card < MAX_SLOTS; ++card)
			entry.cards[card] = it.card[card];
		for (int32 option = 0; option < MAX_ITEM_RDM_OPT; ++option) {
			entry.options[option].index = it.option[option].id;
			entry.options[option].value = it.option[option].value;
			entry.options[option].param = static_cast<uint8>(it.option[option].param);
		}
		found.push_back(entry);
	}
}

void market_add_buying(const s_market_query& query, const map_session_data& owner,
                       std::vector<PACKET_MOONLIGHT_MARKET_ENTRY>& found) {
	const bool whole_store = query.store_id != 0;
	for (int32 slot = 0; slot < owner.buyingstore.slots; ++slot) {
		const s_buyingstore_item& wanted = owner.buyingstore.items[slot];
		if (wanted.nameid == 0 || wanted.amount == 0)
			continue;
		// Une boutique d'achat ne demande que des objets neufs : ni raffinement,
		// ni carte, ni option.
		item it{};
		it.nameid = wanted.nameid;
		it.identify = 1;
		const uint32 price = static_cast<uint32>(std::max<int32>(wanted.price, 0));
		if (!whole_store && (!market_item_matches(query, it) || !market_price_matches(query, price)))
			continue;
		PACKET_MOONLIGHT_MARKET_ENTRY entry = market_entry_of(owner, owner.buyer_id);
		entry.itemId = wanted.nameid;
		entry.amount = wanted.amount;
		entry.price = price;
		entry.identified = 1;
		found.push_back(entry);
	}
}

// UN PNJ QUE LE JOUEUR PEUT REJOINDRE : sur une carte, hors instance, visible.
bool market_npc_reachable(const npc_data* nd) {
	return nd != nullptr && nd->m >= 0 && nd->instance_id == 0 && nd->state != NPCVIEW_DISABLE;
}

// LE PRIX QUE CE JOUEUR PAIERAIT : celui de la liste, remisé comme npc_buylist
// le facture — et comme clif_buylist le montre — quand la boutique l'admet.
uint32 market_npc_price_for(const map_session_data& sd, const s_npc_shop_offer& offer) {
	if (!offer.discountable)
		return offer.price;
	return static_cast<uint32>(std::max<int32>(pc_modifybuyvalue(&sd, static_cast<int32>(offer.price)), 0));
}

// Le prix d'une offre de PNJ passe-t-il le filtre ? ⚠ UN FILTRE DE PRIX EST EN
// ZENY : il écarte ce qui se paie dans une autre monnaie, qu'il ne sait pas
// comparer.
bool market_npc_price_matches(const s_market_query& query, const s_npc_shop_offer& offer, uint32 price) {
	if (offer.currency == SHOP_CURRENCY_ZENY)
		return market_price_matches(query, price);
	return query.price_min == 0 && query.price_max == 0;
}

// Les boutiques de PNJ : tout ce qu'ils vendent, ou la boutique d'un PNJ seul.
// Faux si le PNJ demandé n'est plus là.
bool market_collect_npc(const map_session_data& sd, const s_market_query& query,
                        std::vector<PACKET_MOONLIGHT_MARKET_ENTRY>& found) {
	const bool whole_store = query.store_id != 0;
	if (whole_store && !market_npc_reachable(map_id2nd(query.store_id)))
		return false;
	std::set<t_itemid> wanted(query.item_ids.begin(), query.item_ids.end());
	std::vector<s_npc_shop_offer> offers;
	npc_shop_find(whole_store || wanted.empty() ? nullptr : &wanted, offers);
	// Le même PNJ ressort autant de fois qu'il ouvre de boutiques : à item, prix
	// et monnaie égaux, c'est UNE offre.
	std::set<std::tuple<int32, t_itemid, uint32, uint8, t_itemid, std::string>> seen;
	for (const s_npc_shop_offer& offer : offers) {
		if (!market_npc_reachable(offer.nd) || offer.stock == 0)
			continue;
		const uint32 price = market_npc_price_for(sd, offer);
		if (whole_store) {
			if (static_cast<uint32>(offer.nd->id) != query.store_id)
				continue;
		} else {
			item it{};
			it.nameid = offer.nameid;
			if (!market_item_matches(query, it) || !market_npc_price_matches(query, offer, price))
				continue;
		}
		if (!seen.emplace(offer.nd->id, offer.nameid, price, offer.currency, offer.currency_item,
		                  offer.currency_points).second)
			continue;
		PACKET_MOONLIGHT_MARKET_ENTRY entry{};
		entry.store_id = static_cast<uint32>(offer.nd->id);
		entry.itemId = offer.nameid;
		entry.amount = static_cast<uint16>(std::clamp<int32>(offer.stock, 0, UINT16_MAX));
		entry.price = price;
		entry.identified = 1;
		if (const map_data* mapdata = map_getmapdata(offer.nd->m); mapdata != nullptr)
			safestrncpy(entry.map, mapdata->name, sizeof(entry.map));
		entry.x = offer.nd->x;
		entry.y = offer.nd->y;
		safestrncpy(entry.title, offer.nd->name, sizeof(entry.title));
		entry.currency = offer.currency;
		entry.currency_item = offer.currency_item;
		safestrncpy(entry.currency_name, offer.currency_points.c_str(), sizeof(entry.currency_name));
		found.push_back(entry);
	}
	return true;
}

// Les entrées de la demande : une échoppe seule, ou toutes celles de sa sorte.
// Faux si l'échoppe demandée n'est plus ouverte.
// COMPARER UNE ÉCHOPPE : les items qu'elle vend — ou achète — deviennent ceux
// de la demande. Faux si elle n'est plus ouverte, ou ne porte rien.
bool market_resolve_compare(s_market_query& query) {
	const map_session_data* owner = map_id2sd(query.owner_aid);
	if (owner == nullptr)
		return false;
	std::set<t_itemid> wanted;
	if (query.kind == MOONLIGHT_MARKET_VENDING && owner->state.vending) {
		for (int32 slot = 0; slot < owner->vend_num; ++slot)
			wanted.insert(owner->cart.u.items_cart[owner->vending[slot].index].nameid);
	} else if (query.kind == MOONLIGHT_MARKET_BUYING && owner->state.buyingstore) {
		for (int32 slot = 0; slot < owner->buyingstore.slots; ++slot)
			wanted.insert(owner->buyingstore.items[slot].nameid);
	}
	wanted.erase(0);
	if (wanted.empty())
		return false;
	query.item_ids.assign(wanted.begin(), wanted.end());
	return true;
}

bool market_collect(const map_session_data& sd, const s_market_query& query,
                    std::vector<PACKET_MOONLIGHT_MARKET_ENTRY>& found) {
	if (query.kind == MOONLIGHT_MARKET_NPC)
		return market_collect_npc(sd, query, found);
	const bool vending = query.kind == MOONLIGHT_MARKET_VENDING;
	if (query.store_id != 0) {
		const map_session_data* owner = map_id2sd(query.owner_aid);
		if (owner == nullptr)
			return false;
		if (vending) {
			if (!owner->state.vending || static_cast<uint32>(owner->vender_id) != query.store_id)
				return false;
			market_add_vending(query, *owner, found);
		} else {
			if (!owner->state.buyingstore || owner->buyer_id != query.store_id)
				return false;
			market_add_buying(query, *owner, found);
		}
		return true;
	}
	DBIterator* iter = db_iterator(vending ? vending_getdb() : buyingstore_getdb());
	for (map_session_data* owner = static_cast<map_session_data*>(dbi_first(iter)); dbi_exists(iter);
	     owner = static_cast<map_session_data*>(dbi_next(iter))) {
		if (vending ? !owner->state.vending : !owner->state.buyingstore)
			continue;
		if (vending)
			market_add_vending(query, *owner, found);
		else
			market_add_buying(query, *owner, found);
	}
	dbi_destroy(iter);
	return true;
}

// Une trame de la réponse : son en-tête, et `count` entrées depuis `first`.
// `open_query` ne sert qu'au statut OPEN.
void market_send_part(const map_session_data& sd, const s_market_query& query, uint8 status, uint16 part,
                      uint16 part_count, bool truncated, const std::vector<PACKET_MOONLIGHT_MARKET_ENTRY>& entries,
                      size_t first, size_t count, const char* open_query = "") {
	const int32 fd = sd.fd;
	const size_t length = sizeof(PACKET_ZC_MOONLIGHT_MARKET) + count * sizeof(PACKET_MOONLIGHT_MARKET_ENTRY);
	WFIFOHEAD(fd, length);
	PACKET_ZC_MOONLIGHT_MARKET* reply = reinterpret_cast<PACKET_ZC_MOONLIGHT_MARKET*>(WFIFOP(fd, 0));
	reply->packetType = HEADER_ZC_MOONLIGHT_MARKET;
	reply->packetLength = static_cast<int16>(length);
	reply->query_id = query.query_id;
	reply->status = status;
	reply->kind = query.kind;
	reply->part = part;
	reply->part_count = part_count;
	reply->total = static_cast<uint16>(entries.size());
	reply->truncated = truncated ? 1 : 0;
	reply->store_id = query.store_id;
	reply->owner_aid = query.owner_aid;
	reply->reserved = 0;
	safestrncpy(reply->query, open_query, sizeof(reply->query));
	for (size_t rank = 0; rank < count; ++rank)
		reply->entries[rank] = entries[first + rank];
	WFIFOSET(fd, length);
}

// Un statut sans entrée, en une trame.
void market_send_status(const map_session_data& sd, const s_market_query& query, uint8 status,
                        const char* open_query = "") {
	market_send_part(sd, query, status, 0, 1, false, {}, 0, 0, open_query);
}

// Lit la demande ; faux si elle est mal formée.
bool market_read_query(int32 fd, s_market_query& query) {
	const size_t length = RFIFOW(fd, 2);
	if (length < sizeof(PACKET_CZ_MOONLIGHT_MARKET_REQ))
		return false;
	const size_t tail = length - sizeof(PACKET_CZ_MOONLIGHT_MARKET_REQ);
	const PACKET_CZ_MOONLIGHT_MARKET_REQ* p = reinterpret_cast<const PACKET_CZ_MOONLIGHT_MARKET_REQ*>(RFIFOP(fd, 0));
	query.query_id = p->query_id;
	query.kind = p->kind;
	query.category = p->category;
	query.refine_min = p->refine_min;
	query.refine_max = p->refine_max;
	query.flags = p->flags;
	query.price_min = p->price_min;
	query.price_max = p->price_max;
	query.store_id = p->store_id;
	query.owner_aid = p->owner_aid;
	if (tail % sizeof(PACKET_MOONLIGHT_MARKET_ITEM_ID) != 0)
		return false;
	const size_t count = tail / sizeof(PACKET_MOONLIGHT_MARKET_ITEM_ID);
	if (count > MOONLIGHT_MARKET_MAX_IDS)
		return false;
	if (query.kind > MOONLIGHT_MARKET_NPC)
		return false;
	if (query.category > MOONLIGHT_MARKET_PET || query.refine_min > query.refine_max)
		return false;
	if (query.price_max != 0 && query.price_max < query.price_min)
		return false;
	query.item_ids.reserve(count);
	for (size_t rank = 0; rank < count; ++rank)
		query.item_ids.push_back(p->items[rank].itemId);
	return true;
}

} // namespace

bool market_announced(const map_session_data& sd) {
	return sd.state.has_bourgeon && (sd.bourgeon_ui_caps & BOURGEON_UI_MARKET);
}

void clif_parse_moonlight_market_req(int32 fd, map_session_data* sd) {
	nullpo_retv(sd);
	if (!market_announced(*sd))
		return;

	s_market_query query;
	if (!market_read_query(fd, query)) {
		market_send_status(*sd, query, MOONLIGHT_MARKET_BAD_REQUEST);
		return;
	}
	const t_tick now = gettick();
	if (sd->moon_market_last_tick != 0 &&
	    DIFF_TICK(now, sd->moon_market_last_tick) < battle_config.moon_market_query_delay) {
		market_send_status(*sd, query, MOONLIGHT_MARKET_TOO_SOON);
		return;
	}
	sd->moon_market_last_tick = now;

	std::vector<PACKET_MOONLIGHT_MARKET_ENTRY> found;
	const bool compare = (query.flags & MOONLIGHT_MARKET_COMPARE) != 0 && query.store_id == 0;
	if ((compare && !market_resolve_compare(query)) || !market_collect(*sd, query, found)) {
		market_send_status(*sd, query, MOONLIGHT_MARKET_STORE_GONE);
		return;
	}
	// LE PLAFOND GARDE LES MEILLEURES OFFRES : les ventes les moins chères, les
	// achats les mieux payés.
	const bool buying = query.kind == MOONLIGHT_MARKET_BUYING;
	std::stable_sort(found.begin(), found.end(),
	                 [buying](const PACKET_MOONLIGHT_MARKET_ENTRY& a, const PACKET_MOONLIGHT_MARKET_ENTRY& b) {
		                 return buying ? a.price > b.price : a.price < b.price;
	                 });
	const bool truncated = found.size() > MOONLIGHT_MARKET_MAX_RESULTS;
	if (truncated)
		found.resize(MOONLIGHT_MARKET_MAX_RESULTS);

	const size_t part_count = std::max<size_t>(1, (found.size() + MOONLIGHT_MARKET_PER_PART - 1) / MOONLIGHT_MARKET_PER_PART);
	for (size_t part = 0; part < part_count; ++part) {
		const size_t first = part * MOONLIGHT_MARKET_PER_PART;
		const size_t count = std::min<size_t>(MOONLIGHT_MARKET_PER_PART, found.size() - first);
		market_send_part(*sd, query, MOONLIGHT_MARKET_OK, static_cast<uint16>(part),
		                 static_cast<uint16>(part_count), truncated, found, first, count);
	}
}

void market_open(map_session_data& sd, const char* query) {
	const s_market_query none;
	market_send_status(sd, none, MOONLIGHT_MARKET_OPEN, query != nullptr ? query : "");
}
