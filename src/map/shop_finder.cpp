// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shop_finder.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>

#include "map.hpp"
#include "npc.hpp"
#include "script.hpp"

namespace {

bool shop_finder_wants(const std::set<t_itemid>* wanted, t_itemid nameid) {
	return nameid != 0 && (wanted == nullptr || wanted->count(nameid) != 0);
}

// L'offre d'une ligne de boutique classique, sans son PNJ.
s_npc_shop_offer shop_finder_offer_of(const npc_data& nd, const npc_item_list& entry) {
	s_npc_shop_offer offer;
	offer.shop_exname = nd.exname;
	offer.nameid = entry.nameid;
	offer.price = entry.value;
	switch (nd.subtype) {
	case NPCTYPE_CASHSHOP:
		offer.currency = SHOP_CURRENCY_CASH;
		break;
	case NPCTYPE_ITEMSHOP:
		offer.currency = SHOP_CURRENCY_ITEM;
		offer.currency_item = nd.u.shop.itemshop_nameid;
		break;
	case NPCTYPE_POINTSHOP:
		offer.currency = SHOP_CURRENCY_POINTS;
		offer.currency_points = nd.u.shop.pointshop_str;
		break;
	default:
		offer.currency = SHOP_CURRENCY_ZENY;
		// npc_buylist, le seul achat qui remise, ne sert que shop et marketshop.
		offer.discountable = (nd.subtype == NPCTYPE_SHOP || nd.subtype == NPCTYPE_MARKETSHOP) && npc_shop_discount(&nd);
		break;
	}
#if PACKETVER >= 20131223
	// Un marketshop tient un stock ; -1 : illimité.
	if (nd.subtype == NPCTYPE_MARKETSHOP)
		offer.stock = entry.qty;
#endif
	return offer;
}

// Quelles boutiques flottantes ce script ouvre-t-il ?
//
// callshop ne laisse aucun lien vers sa cible : le nom voyage en clair dans le
// bytecode, empilé comme littéral. On cherche donc ce littéral — l'octet C_STR
// suivi de la chaîne terminée par un zéro. C'est une HEURISTIQUE assumée : elle
// attrape aussi un nom seulement cité dans une chaîne, mais un nom de boutique
// flottante ne se promène pas ailleurs, et le pire cas est une ligne de trop.
// Décoder le flux d'opcodes serait exact, mais demanderait de recopier get_com()
// hors de script.cpp où il est privé : bien plus fragile pour le même résultat.
void shop_finder_scan_script(const script_code* code, const std::set<std::string>& names, size_t shortest,
                             size_t longest, std::vector<std::string>& found) {
	if (code == nullptr || code->script_buf == nullptr)
		return;

	for (int32 i = 0; i < code->script_size; i++) {
		if (code->script_buf[i] != C_STR)
			continue;

		const char* literal = reinterpret_cast<const char*>(code->script_buf) + i + 1;
		const size_t left = static_cast<size_t>(code->script_size - i - 1);
		// ⚠ Le tampon n'est pas garanti nul-terminé à la fin : sans cette borne,
		// un C_STR en dernier octet ferait lire au-delà.
		const size_t length = strnlen(literal, left);

		if (length == left)
			break;

		// Le test de longueur AVANT la construction de la std::string : l'octet
		// C_STR se rencontre partout dans du bytecode, et allouer à chaque rencontre
		// coûterait bien plus cher que la recherche elle-même.
		if (length >= shortest && length <= longest && names.count(literal) != 0) {
			found.push_back(literal);
			// Un vrai littéral : rien à y chercher. Ailleurs on avance d'UN octet,
			// car l'octet C_STR croisé au milieu d'un nombre ou d'un nom n'en est
			// pas un : sauter sa pseudo-longueur enjamberait un vrai littéral posé
			// juste derrière.
			i += static_cast<int32>(length);
		}
	}
}

} // namespace

void npc_shop_find(const std::set<t_itemid>* wanted, std::vector<s_npc_shop_offer>& found) {
	std::vector<s_npc_shop_offer> floating;
	std::set<std::string> floating_names;

	auto record = [&](npc_data* nd, s_npc_shop_offer offer) {
		if (nd->m < 0) {
			// Boutique flottante : on ne sait pas encore où l'on envoie le
			// joueur, ça se règle à la passe suivante.
			floating_names.insert(nd->exname);
			floating.push_back(std::move(offer));
			return;
		}
		offer.nd = nd;
		found.push_back(std::move(offer));
	};

	s_mapiterator* iter = mapit_geteachnpc();

	for (npc_data* nd = static_cast<npc_data*>(mapit_first(iter)); mapit_exists(iter);
	     nd = static_cast<npc_data*>(mapit_next(iter))) {
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
				const npc_item_list& entry = nd->u.shop.shop_item[i];
				if (shop_finder_wants(wanted, entry.nameid))
					record(nd, shop_finder_offer_of(*nd, entry));
			}
			break;

		case NPCTYPE_BARTER: {
			// Un troc ne range pas ses articles dans l'union : la marchandise vit
			// dans barter_db, indexée par le nom UNIQUE du PNJ.
			std::shared_ptr<s_npc_barter> barter = barter_db.find(nd->exname);
			if (barter == nullptr)
				break;
			for (const auto& it : barter->items) {
				const std::shared_ptr<s_npc_barter_item>& entry = it.second;
				if (entry == nullptr || !shop_finder_wants(wanted, entry->nameid))
					continue;
				s_npc_shop_offer offer;
				offer.shop_exname = nd->exname;
				offer.nameid = entry->nameid;
				offer.price = entry->price;
				offer.currency = SHOP_CURRENCY_BARTER;
				offer.stock = entry->stockLimited ? static_cast<int32>(entry->stock) : -1;
				record(nd, std::move(offer));
			}
			break;
		}

		default:
			break;
		}
	}

	mapit_free(iter);

	if (floating.empty())
		return;

	// Qui ouvre les boutiques flottantes ? Un seul examen par bytecode DISTINCT :
	// les duplicates partagent celui de leur original, et le PNJ des villes en
	// compte une trentaine.
	size_t shortest = SIZE_MAX, longest = 0;
	for (const std::string& name : floating_names) {
		// std:: explicite : le min()/max() maison de rAthena est typé int32 et
		// tronquerait les size_t.
		shortest = std::min(shortest, name.length());
		longest = std::max(longest, name.length());
	}

	std::map<std::string, std::vector<npc_data*>> callers;
	std::map<const script_code*, std::vector<std::string>> scanned;

	iter = mapit_geteachnpc();

	for (npc_data* nd = static_cast<npc_data*>(mapit_first(iter)); mapit_exists(iter);
	     nd = static_cast<npc_data*>(mapit_next(iter))) {
		// Un porteur lui-même hors carte ne mènerait nulle part.
		if (nd == nullptr || nd->subtype != NPCTYPE_SCRIPT || nd->m < 0 || nd->u.scr.script == nullptr)
			continue;

		auto cached = scanned.find(nd->u.scr.script);
		if (cached == scanned.end()) {
			std::vector<std::string> names;
			shop_finder_scan_script(nd->u.scr.script, floating_names, shortest, longest, names);
			cached = scanned.emplace(nd->u.scr.script, std::move(names)).first;
		}
		for (const std::string& name : cached->second)
			callers[name].push_back(nd);
	}

	mapit_free(iter);

	for (s_npc_shop_offer& offer : floating) {
		const auto carriers = callers.find(offer.shop_exname);
		if (carriers == callers.end()) {
			// Aucun script ne la nomme : l'offre reste sans PNJ.
			found.push_back(std::move(offer));
			continue;
		}
		for (npc_data* nd : carriers->second) {
			s_npc_shop_offer carried = offer;
			carried.nd = nd;
			found.push_back(std::move(carried));
		}
	}
}
