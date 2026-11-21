# Plan — la pile d'objets passe en int32 (MAX_AMOUNT 30 000 → 99 999)

État (2026-11-21) : **lots 1 à 3 faits et commités** (moonlight 631f452cb,
c419d5cce, puis les corrections de l'audit ; moonclient 8f2466e70), **non
déployés** ; moonlight-web déployé. Reste les tests en jeu du lot 4, puis le
déploiement ci-dessous — **moonclient et serveur ensemble**.

## Pourquoi c'est un chantier

99 999 ne tient pas sur 16 bits. La quantité était en 16 bits à quatre étages :
la structure (`struct item::amount`, `int16`), la base (`smallint`), le code
(variables et paramètres `int16`/`uint16`) et le protocole (les paquets
d'objets portent la quantité sur 16 bits).

## Décisions prises

| # | Question | Décision |
|---|---|---|
| D1 | Nouveau plafond | **99 999** |
| D2 | Client officiel | **Abandonné** : seul moonclient se connecte |
| D3 | Paquets élargis | **Mêmes opcodes, champs en int32** (révisé le 2026-11-21 : le préambule 0x0F6B / 0x0F6C demandait trois fois plus de code) |

## Principe : élargir les types d'abord, monter le plafond en dernier

Les lots 1 et 2 élargissent les types **à plafond constant** : toute pile reste
≤ 30 000, donc les paquets 16 bits du lot 3 ne tronquent rien, et le serveur
est déployable entre deux lots. `MAX_AMOUNT` ne monte qu'à la fin du lot 3.

---

## Lot 1 — Structure et base (fait)

- `struct item::amount` `int16` → `int32` (`src/common/mmo.hpp`) ;
  `startitem::amount` `uint16` → `uint32`. `struct item` passe de 77 à 79 octets :
  le paquet inter-serveurs du stockage (`sizeof(struct s_storage)`, longueur sur
  16 bits) fait 600 × 79 = 47 400 octets, la marge tient jusqu'à ~829 cases.
- Liaisons SQL `SQLDT_INT16` → `SQLDT_INT32` : `char.cpp` (2), `int_storage.cpp`,
  `storage.cpp` (journal du stockage de guilde) ; `int_mail.cpp` lit en int32.
- `sql-files/upgrade_item_amount_int.sql` : `storage`, `storagealt1-4`,
  `storagecard`, `vending_items`, `buyingstore_items`, `card_album` en `int(11)`.
  **À exécuter avant de démarrer un serveur dont MAX_AMOUNT dépasse 32 767.**
  `sql-files/card_album.sql` crée directement la colonne en `int`.

## Lot 2 — Le code du serveur (fait)

Passent en 32 bits : l'échange (`s_deal::amount`, `trade_tradeadditem`,
`trade_check`), l'échoppe (`s_vending::amount`, les lectures de
`vending_purchasereq` / `vending_openvending`), l'autotrade
(`s_autotrade_entry::amount`), la recherche d'échoppes
(`s_search_store_info_item`, `s_shopsearch_vend`), le journal du stockage de
guilde (`guild_log_entry`, `storage_guild_log`), la fusion d'objets
(`clif_parse_merge_item_req`), les commandes de script `checkweight`,
`checkweight2`, `getitem` et variantes, `makeitem`, `getareadropitem`, `mail`,
et l'album de cartes (entrée, cache, `card_album_put` / `card_album_get`).

Restent en 16 bits, volontairement : les quantités **définies en base**
(recettes, récompenses, paquets d'objets, prérequis de compétences, YAML lus par
`asUInt16`), qui ne dépassent pas 65 535, et les compteurs qui ne sont pas des
piles (`clif_updatestorageamount` compte des cases, quêtes, compétences).

## Lot 3 — Le protocole : champs élargis sur place

### Le principe

Seul moonclient se connecte, et ses décodeurs sont générés depuis les en-têtes
du serveur (`tools/packetgen`) : les champs de quantité des trames d'objets
passent **en int32, aux mêmes opcodes**. Pas de trame neuve, pas de préambule.

Un premier essai par préambule (ZC 0x0F6B / CZ 0x0F6C, champs d'origine bornés
et vraie valeur à côté) a été écarté : trois fois plus de code, et une classe de
défauts propre (deux lignes d'une même case héritant de la même vraie valeur).

Contrepartie acceptée : chaque champ élargi est une ligne de `packets_struct.hpp`
ou de `packets.hpp` que l'amont peut toucher (conflit possible aux synchros), et
moonclient ne lit plus ces trames chez un rAthena d'usine.

### Ce qui change (serveur)

- 29 champs en int32, dans la variante active au PACKETVER 20250716 (et dans les
  autres variantes de trois structures, que packetgen veut de même signe) :
  listes d'objets (`NORMALITEM_INFO`), ramassage, suppression, jet, utilisation,
  fusion, objets au sol, échoppes (listes, achat, rapport, CZ d'achat),
  recherche d'échoppes, boutique d'achat (listes, mise à jour, CZ d'ouverture et
  de vente), rodex (accusés, lecture), achat et vente au PNJ, boutique à points,
  boutique marché, album de cartes (`CARD_ALBUM_ENTRY`, CZ 0x0F34), Marché
  (`PACKET_MOONLIGHT_MARKET_ENTRY`).
- Trames écrites ou lues à la main : jet au sol 0x0363 (8 octets, déclaré dans
  `clif_shuffle.hpp`, qui écrase `clif_packetdb.hpp` à ce PACKETVER), rodex 0x0A04
  et 0x0A06 (8 octets), 0x081C (12 octets), entrée d'album (14 octets),
  ouverture d'échoppe (`VENDING_OPEN_ENTRY_SIZE` = 10 octets par ligne, reprise
  en autotrade comprise), achat en échoppe lu par sa structure.
- Règle 0x0F38, clé 31 : `MOONCLIENT_RULE_ITEM_AMOUNT_MAX` = `MAX_AMOUNT`.
- Corrigé au passage : le Marché tronquait la quantité, le journal du stockage
  de guilde la coupait à 16 bits, les cumuls de poids et le coût d'un achat à
  points débordaient un int32 avec des piles pleines.
- Déjà en 32 bits, rien à faire : échange, déplacements inventaire ↔ chariot ↔
  stockage, ajout et retrait au chariot et au stockage, journal du stockage de
  guilde (champ du fil).

### Côté moonclient

Régénérer `libs/proto`, adapter les paquets manuels, lire et écrire l'int32
dans les traducteurs et les encodeurs, modèle en int32, plafonds qui suivent la
règle 31.

## Lot 4 — Autour, et le banc de test en jeu

Fait :

- serveur : `@item` lit sa quantité en int32, `pc_additem` borne à
  `MAX_AMOUNT` ; `doc/script_commands.txt` ne cite pas l'ancien plafond ;
- audit du 2026-11-21 : élargir un champ CZ en int32 y rend possibles des
  valeurs NÉGATIVES ou énormes qu'un uint16 empêchait. Corrigé : livraison des
  achats au PNJ (`get_amt` resté en uint16 : gel du serveur à 65 536), quantité
  ≤ 0 refusée (achat au PNJ, boutique d'achat, ouverture d'échoppe), album borné
  à `MAX_AMOUNT` avant conversion, vente au PNJ jugée par case (deux lignes d'une
  même case), anciens opcodes de jet et 0x0246 lus selon leur longueur, objets
  de départ en `%u`, structures rodex CZ élargies ;
- relecture neuve du 2026-11-21 : troc borné à MAX_AMOUNT (hérité d'amont :
  aucune borne, objets gratuits) et compté en 64 bits, liste d'achat au PNJ
  bornée à MAX_INVENTORY (`market_index` débordait la pile), cumul par objet
  avant `pc_checkadditem` (PNJ, points, échoppe, boutique d'achat),
  `checkweight` en int32, `makeitem` refuse une quantité ≤ 0, changement de
  matériau par piles de MAX_AMOUNT, exigences Laphine bornées à INT16_MAX ;
- moonlight-web : la fusion de comptes plafonne la réserve de l'album à
  `MAX_AMOUNT` (`config/moonlight.php`, `merge.max_amount` = 99 999) ; la base de
  test (`FakeRathena`) déclare `amount` en int comme le serveur ; suite verte
  (499 tests).

### Banc de test en jeu

Préalable : `sql-files/upgrade_item_amount_int.sql` sur la base de test,
char-server et map-server du même build, moonclient du lot 3. Tester chaque
ligne avec des piles de **32 767**, **32 768**, **65 536** et **99 999**
(`@item <id> <n>`, objet empilable et léger, puis un objet lourd pour le poids).

| # | Cas | Ce qui doit se voir |
|---|---|---|
| 1 | Sauvegarde : se déconnecter, se reconnecter | La pile garde sa quantité exacte |
| 2 | Changer de carte (autre map-server si possible) | Inventaire et chariot exacts dès l'arrivée |
| 3 | Ramasser une pile au sol, en jeter une | Au sol, au ramassage et dans le sac : la vraie quantité |
| 4 | Utiliser un consommable d'une pile de 99 999 | Le reste affiché est 99 998 |
| 5 | Stockage : déposer, retirer, depuis le sac et le chariot | Les deux fenêtres exactes |
| 6 | Stockage de guilde : idem, puis son journal | Quantités exactes, journal compris |
| 7 | Chariot : poser, reprendre | Exact |
| 8 | Échange entre joueurs d'une pile de 99 999 | Les deux voient 99 999, le receveur l'a |
| 9 | Échoppe : ouvrir avec 99 999, un autre achète 70 000 | Liste, achat, rapport de vente, reste 29 999 |
| 10 | Échoppe en autotrade, puis redémarrage du serveur | Elle rouvre avec la bonne quantité |
| 11 | Recherche d'échoppes et Marché | La vraie quantité dans les deux |
| 12 | Boutique d'achat : demander 99 999, en vendre 70 000 | Les deux listes, la vente, la mise à jour |
| 13 | Boutique d'achat en autotrade, puis redémarrage | Elle rouvre avec la bonne quantité |
| 14 | Vente au PNJ de 99 999 (et 70 000 + 20 000 d'une même case) | Payé pour la vraie quantité, une seule ligne |
| 15 | Achat au PNJ de 99 999, boutique à points, boutique marché | Reçu et facturé exactement |
| 16 | Achat d'objets lourds au PNJ au-delà du poids | Refus pour surpoids, zeny intact |
| 17 | Rodex : joindre 99 999, envoyer, lire, retirer la pièce jointe | Exact à chaque étape |
| 18 | Album de cartes : déposer 99 999, retirer 70 000 | Réserve exacte, refus au-delà de 99 999 |
| 19 | Pile pleine : ajouter 1 à 99 999 | Refus (sac plein / pile pleine), rien de perdu |

## Déploiement

🔴 **moonclient ET serveur ensemble.** Les trames d'objets changent de taille :
un moonclient antérieur ne les lit plus, et un nouveau moonclient ne lit plus un
serveur antérieur. Les deux se déploient au même moment.

1. Serveurs arrêtés, `sql-files/upgrade_item_amount_int.sql` sur la base du jeu.
2. Déployer char-server **et** map-server du même build (la taille de
   `struct item` change), et publier le moonclient du lot 3 dans la même
   fenêtre de maintenance, avant la réouverture.
3. Démarrer.
