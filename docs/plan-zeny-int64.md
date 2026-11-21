# Plan — le zeny en poche passe en int64

État : **lots 0 et 1 faits** (2026-11-21), lots 2 à 7 à faire. `MAX_ZENY` vaut
encore INT_MAX : il ne monte qu'une fois les lots 2 (base) et 3 (paquets) prêts,
pour que chaque commit reste déployable seul.

Lot 1, en plus du tableau ci-dessous : `pc_payzeny` ne borne plus (borner un
paiement le ferait payer moins que dû) ; le char-server lit et écrit déjà le
zeny en int64 (`SQLDT_INT64`, `PRId64`), et l'écran de sélection le borne à
INT32_MAX — imposé par le changement de type de `mmo_charstatus` ; les réponses
de la banque bornent leur champ `zeny` ; une enchère au-delà de
`MAX_ZENY_AMOUNT` est refusée ; Zenynage paie en int64 ; le rapport de vente
(`clif_vendingreport`) borne son gain par ligne, qui débordait déjà au-delà de 2^31.

## Pourquoi maintenant

Le plafond `MAX_ZENY = INT_MAX` ne venait pas du Ragexe 32 bits, mais du format
des paquets (`int32`) et d'un client qu'on ne pouvait pas modifier. Seul
moonclient se connecte désormais, et il garde déjà le zeny en `std::int64_t`
partout ; il ne borne qu'avec des constantes codées en dur (`kPocketZenyMax`,
`kZenyCeiling`…). La banque est déjà en int64 (62645045b).

## Décisions à prendre

| # | Question | Recommandation |
|---|---|---|
| D1 | Nouveau plafond en poche | **999 999 999 999** (12 chiffres) : ×465 par rapport à aujourd'hui, lisible dans l'interface, et `MAX_BANK_ZENY` reste au-dessus |
| D2 | Écran de sélection (`CHARACTER_INFO.money`, int32) | **Borner à INT32_MAX** pour l'affichage ; élargir la structure plus tard si besoin (elle change de taille selon PACKETVER, risque élevé pour peu de gain) |
| D3 | Échange, courrier, dépôt/retrait, prix : un montant par opération au-delà de 2^31 ? | **Non pour ce chantier** : ils restent en int32 sur le réseau, sous une constante dédiée. Un échange > 2^31 demanderait un nouveau paquet, lot à part |

## Principe directeur : séparer deux notions

Aujourd'hui `MAX_ZENY` veut dire deux choses :

- **le solde** d'un personnage → `MAX_ZENY` (int64, D1) ;
- **le montant d'une seule opération ou d'un prix** (stocké en `uint32`, transmis
  en `int32`) → nouvelle constante `MAX_ZENY_AMOUNT = INT_MAX`.

Le lot 0 fait ce partage **à valeur constante** (les deux valent INT_MAX) : il ne
change aucun comportement, ce qui permet de le compiler et de le vérifier seul.

---

## Lot 0 — Partage des constantes (aucun changement de comportement)

`src/common/mmo.hpp` : ajouter `MAX_ZENY_AMOUNT INT_MAX`, `static_assert`
`MAX_ZENY_AMOUNT <= MAX_ZENY <= MAX_BANK_ZENY`.

Passent à `MAX_ZENY_AMOUNT` (prix et montants d'opération) :

- `itemdb.cpp` 208, 227, 2523, 2595, 2748, 2821 (Buy/Sell, prix d'enchantement…)
- `npc.cpp` 146 (styliste), 603 (barter)
- `guild.cpp` 444, 462 (WarpCost)
- `status.cpp` 326
- `mail.cpp` 127, 135 ; `script.cpp` 26585 (`buildin_mail`)
- `chat.cpp` 455 (prix d'entrée d'un salon)
- `battle.cpp` 8660, 8822, 8838 (`vending_max_value`, `vending_tax_min`, `auction_maximumprice`)

Restent `MAX_ZENY` (le solde) : `pc.cpp` 6275, 6309 (le `cap_value` d'entrée de
`pc_payzeny`/`pc_getzeny` : un total d'achat peut dépasser 2^31 une fois le solde
élargi, et le borner à `MAX_ZENY_AMOUNT` le ferait payer tronqué), 6316,
11084-11085, 15708 ;
`trade.cpp` 267, 269, 497 ; `vending.cpp` 187, 191, 372 ; `npc.cpp` 3245 ;
`clif.cpp` 24753.

Exporter aussi `MAX_BANK_ZENY` et `MAX_ZENY_AMOUNT` dans `script_constants.hpp`
(voir lot 5).

## Lot 1 — Le type, côté map-server

1. `mmo_charstatus::zeny` : `int32` → `int64` (`mmo.hpp:654`). Retirer le
   `#error` quand `MAX_ZENY > INT_MAX` (`mmo.hpp:1258`). Passer `MAX_ZENY` à la valeur D1.
2. `pc_payzeny` / `pc_getzeny` : paramètre `int64`, `sprintf` en `PRId64`.
3. Troncatures relevées, à corriger une par une :

| Endroit | Problème |
|---|---|
| `pc.cpp:10662` | pénalité de mort castée en `uint32` |
| `skill.cpp:9819` | `require.zeny = sd->status.zeny` (int32) : borner |
| `buyingstore.cpp:389` | `zenylimit = status.zeny` (int32) : borner |
| `script.cpp:13425` | `(uint32)sd->status.zeny < cd->zeny` |
| `npc.cpp:3024, 3248, 3481` | `(int32)z` sur le total d'un achat ou d'une vente : int64 |
| `vending.cpp:229, 232` | `(int32)z` : int64 (borné par la ligne 187) |
| `pc.cpp:6329` | `AG_GET_ZENY` passe le solde à un `va_arg(ap, int32)` (`achievement.cpp:1174`) : UB → borner à INT32_MAX |
| `atcommand.cpp:2979`, `10451` | @zeny et la table de @charinfo en `%d` |
| `trade.cpp:483` + `s_deal::zeny` | restent int32 (D3), mais les comparaisons avec le solde se font en int64 |
| `pc.cpp:15679, 15703` | banque : déjà en int64, à relire après le changement de type |

4. Relire tous les `sd->status.zeny` (environ 60 lignes) et les 41 appels de
   `pc_payzeny`/`pc_getzeny` : toute variable intermédiaire `int32` qui reçoit
   le solde est une duplication potentielle.
5. ⚠ `pc.cpp`, `clif.cpp`, `trade.cpp` viennent d'upstream : faire
   `git diff upstream/master -- <fichier>` avant de patcher.

## Lot 2 — char-server et base

1. `sql-files/upgrade_char_zeny_bigint.sql` :
   `ALTER TABLE \`char\` MODIFY \`zeny\` bigint(20) unsigned NOT NULL default '0';`
2. `char.cpp` : `SQLDT_INT32` → `SQLDT_INT64` (1128, 1248), `'%d'` → `PRId64` (306),
   relire la requête de la ligne 2658.
3. `CHARACTER_INFO.money` (`char.cpp:2005`) : borner à INT32_MAX (D2).
4. `mmo_charstatus` passe d'un serveur à l'autre : **recompiler le char-server et le map-server
   ensemble**, migration SQL **avant** le démarrage (même procédure que les succès
   au compte).

## Lot 3 — Paquets serveur

1. `SP_ZENY` : `clif_longpar_change` (0x00b1, int32) → `clif_longlongpar_change`
   (0x0acb) dans `clif.cpp:4087`. moonclient le gère déjà (`character.cpp`).
2. Réponses de la banque 0x09A8 / 0x09AA (`clif.cpp:15033, 15078`, champ `zeny` int32) :
   borner à INT32_MAX. ⚠ `pc_getzeny` envoie déjà le bon solde en 0x0acb
   **avant** la réponse ; moonclient doit donc ignorer ce champ (lot 4), sinon
   il écrase le bon solde par la valeur bornée.
3. Règles du serveur (0x0F38) : annoncer `MAX_ZENY` en deux moitiés, clés 11 et 12,
   comme `MOONCLIENT_RULE_BANK_ZENY_MAX_*`.

## Lot 4 — moonclient (`G:\GitHub\moonclient`)

- Lire les clés 11 et 12 ; remplacer `kPocketZenyMax` (`sim/bank.h:20`,
  `ui/bank_window.h:39`) et `kZenyCeiling` (`ui/stat_bars.h:190`) par la valeur annoncée.
- `TellTransfer` (`net/src/bank.cpp:76`) : ne plus reprendre `SP_ZENY` depuis la réponse.
- Les montants par opération restent en int32 : `kBankRequestMostZeny`,
  `kDealMostZeny`, `kMailMostZeny`, `kMostZeny` (boutique), `kVendingMostZeny`,
  `kWireMostZeny`. C'est cohérent avec D3.
- Vérifier la largeur d'affichage de 12 chiffres avec séparateurs (inventaire, banque, échange).
- Vérifier `origin/main..HEAD` avant le push (commits d'autres sessions).

## Lot 5 — Scripts `moon/` (latin-1, patch Python)

- `func.npc:203` et `huntmission.npc:399, 451, 504` comparent `BankVault` / `#BANKVAULT`
  à `MAX_ZENY` → `MAX_BANK_ZENY`. C'est déjà faux aujourd'hui, la banque ayant son propre plafond.
- `coin_exchanger.npc:171`, `loterie.npc:65`, `func.npc:201`, `huntmission.npc` (Zeny) :
  rien à faire, les entiers de script sont en int64.
- Les constantes se chargent au démarrage seulement : redémarrage complet.

## Lot 6 — Site (moonlight-web)

- `database/seeders/FakeRathena.php:45` : `unsignedInteger` → `unsignedBigInteger`.
- `GameCharacter` et `PersonalDataExport` : un int PHP fait 64 bits, rien à faire.
  Vérifier qu'aucun classement ni aucune vue ne formate en 32 bits.

## Lot 7 — Tests (base de test)

Un personnage à 5 000 000 000 zeny :

1. Sauvegarde, déconnexion et reconnexion : le solde est conservé. Sur l'écran de
   sélection, il s'affiche borné.
2. Dépôt et retrait à la banque avec un solde en poche > 2^31 ; l'affichage reste juste après la réponse.
3. Échange avec un partenaire proche du plafond, dans les deux sens.
4. Échoppe de vente : un acheteur > 2^31, un vendeur proche du plafond, et `vending_over_max`.
5. Achat et vente à un NPC, barter, styliste.
6. Pénalité de mort sur une carte à pénalité de zeny.
7. Mammonite et toute compétence qui coûte du zeny (`skill.cpp:9819`).
8. Boutique d'achat (`zenylimit`).
9. Courrier : réception d'un envoi qui ferait passer le plafond.
10. Succès `AG_GET_ZENY`.
11. Scripts : `Zeny += 3000000000`, Hunt Mission, coin exchanger.
12. `zenylog` : montants corrects (déjà en BIGINT).

## Ordre conseillé

Lot 0 (commit seul, sans effet) → lots 1, 2 et 3 ensemble (un seul déploiement,
recompilation complète + SQL) → lot 4 livré **avant ou en même temps** que ce
déploiement → lots 5 et 6 → lot 7. Sur pc-serv, c'est l'utilisateur qui lance la
migration et le redémarrage.
