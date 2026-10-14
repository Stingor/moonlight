// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// [Stingor] OÙ ACHETER UN ITEM CHEZ UN PNJ : les boutiques de PNJ lues EN
// MÉMOIRE, telles qu'elles sont au moment de la recherche. Partagé par l'onglet
// « PNJ » du Marché (market.cpp) et par l'ancien @shopsearch (custom/atcommand.inc).
//
// 🔴 On lit les boutiques, pas les scripts. Les boutiques remaniées à chaud
// (npcshopitem / npcshopadditem / npcshopdelitem) et les duplicates — qui
// partagent le tableau de leur original — sont donc vues telles qu'elles sont
// RÉELLEMENT. Reste hors de portée un dialogue scripté qui fait getitem contre
// des zeny sans passer par une boutique.
//
// 🔴 Une offre nomme le PNJ QUE LE JOUEUR VOIT, jamais la boutique interne. Une
// bonne part des boutiques sont FLOTTANTES (carte `-`) et ne s'ouvrent que par
// callshop depuis un script : on remonte au PNJ qui les ouvre, et c'est SA
// position qui compte.

#ifndef MAP_SHOP_FINDER_HPP
#define MAP_SHOP_FINDER_HPP

#include <set>
#include <string>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct npc_data;

// La monnaie d'une boutique. ⚠ Valeurs sur le fil du Marché (ZC 0x0F42) : on
// AJOUTE en fin.
enum e_shop_currency : uint8 {
	SHOP_CURRENCY_ZENY   = 0,
	SHOP_CURRENCY_CASH   = 1,
	SHOP_CURRENCY_ITEM   = 2, ///< itemshop : `currency_item`
	SHOP_CURRENCY_POINTS = 3, ///< pointshop : `currency_points`, le nom de la variable
	SHOP_CURRENCY_BARTER = 4, ///< troc : des items, et `price` zeny en plus
};

// Un item en vente chez un PNJ.
struct s_npc_shop_offer {
	/// Le PNJ que le joueur voit. nullptr : boutique flottante qu'aucun script
	/// sur une carte ne nomme — orpheline, ou nom composé à l'exécution.
	npc_data* nd = nullptr;
	/// La boutique interne, pour l'orpheline que seul le staff saura suivre.
	std::string shop_exname;
	t_itemid nameid = 0;
	/// Le prix de la liste, dans la monnaie de la boutique.
	uint32 price = 0;
	e_shop_currency currency = SHOP_CURRENCY_ZENY;
	t_itemid currency_item = 0;
	std::string currency_points;
	/// Le stock d'un marketshop ou d'un troc limité ; -1 : illimité.
	int32 stock = -1;
	/// LA REMISE DE L'ACHETEUR S'APPLIQUE-T-ELLE ? Celle de pc_modifybuyvalue
	/// (Discount, Compulsion Discount), que la boutique de zeny (shop, marketshop)
	/// facture quand npc_shop_discount le dit — lu sur la boutique qui porte la
	/// liste, pas sur le PNJ qui l'ouvre. Jamais pour cash, item, points ni troc :
	/// leurs achats ne l'appliquent pas.
	bool discountable = false;
};

// Toutes les offres des PNJ pour ces items ; `wanted` nul : pour tous les items.
// UN SEUL balayage des PNJ quel que soit le nombre d'items, et un seul examen
// par bytecode DISTINCT pour remonter aux porteurs des boutiques flottantes.
// Une boutique flottante portée par plusieurs PNJ donne une offre par PNJ.
void npc_shop_find(const std::set<t_itemid>* wanted, std::vector<s_npc_shop_offer>& found);

#endif /* MAP_SHOP_FINDER_HPP */
