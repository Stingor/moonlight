# Épisode 18 — Direction de la prière (*Direction of Prayer*)

| | |
|---|---|
| Nom kRO | 기도의 방향 / *Direction of Prayer* |
| Date kRO | **2021‑02‑01** (annonce `seq=259` ; certaines sources donnent le 3 février) |
| Arc | Arunafeltz — les Enfants Gris |
| Niveau kRO visé | 170+ (200+ pour le mode avancé) |
| État Moonlight | **porté (lots A, B, B1, B3) le 2026‑11‑14, à valider en jeu ; la réforme reste à faire** |

Retour en Arunafeltz. Le scénario porte sur la tension entre missionnaires de Freyja et
natifs, et se conclut sur une fausse déesse fabriquée par magie d'illusion. Dernier
épisode que rAthena convertit — au-delà, l'amont s'arrête.

## 1. Contenu principal (1ᵉʳ février 2021)

### Ville, champs et donjon

| Carte | Nom |
|---|---|
| `wolfvill` | **Village du Loup Gris** |
| `gw_fild01`, `gw_fild02` | Forêt du Loup Gris |
| `oz_dun01`, `oz_dun02` | **Labyrinthe d'Oz**, étages 1 et 2 |
| `ra_fild10`, `ra_fild11` | Gorge d'Oz et Plaines d'Ida (Rachel) |

Mobs : Ashhopper, Ashring, Grey Wolf, Bushring, Firewind Kite, Phantom Wolf, Rake Hand,
Ash Toad, Spark, Hot Molar, Volcaring, Lava Toad, **Burning Fang** (MVP), Grey Baby Wolf,
Grey Goat.

### Instances

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Purification du Sanctuaire** | `1@nyr` | (dans `Wolves.txt` / `MazeofOz.txt`) |
| **Lieu de Rassemblement des Loups** | `1@nyr` | `Wolves.txt` |
| **Villa de la Tromperie** — normal et avancé | `1@advs` | `VillaofDeception.txt` |
| **Villa du Grand Prêtre** | `1@adv` | `VillaofHighPriest.txt` |
| **Labyrinthe d'Oz** | `1@oz` | `MazeofOz.txt` |
| **Base militaire de Thor** | `1@tcamp` | `ThorGunsuBase.txt` |

### Systèmes

- **Système d'enchantement du Loup Gris** (PNJ Emmett) ;
- **Réputation du Village du Loup Gris** — cinq paliers débloquant entrepôt,
  téléportation, boîtes de raffinage et bonus de quête ;
- monnaie : **Fragments d'Améthyste**.

## 2. Contenu de l'intervalle (2021)

### Donjons

- **Illusion of Twins** (`ant_d02_i`) — cf. [16.2](16-2_terra_gloria.md)
- **Biosphère de Varmundt**
- **Tour de l'Horloge : sous-sol inconnu**

### Instances

- **Tour Engloutie** (`1@ch_u`)
- **Tour des Constellations** mise à jour (`1@ch_t`)

### Classes

Les **sept 4ᵉ classes étendues** : Shinkiro, Shiranui, Night Watch, Sky Emperor,
Soul Ascetic, Hyper Novice, **Spirit Handler**. Hors périmètre.

> La **Forêt Profonde** (`1@exsh`, `DeepForest.txt`) est l'instance de quête de job du
> Spirit Handler : elle appartient à ce bloc et n'a pas de sens sans la classe.

## 3. État côté rAthena

Conversion par la PR [#7917 « Initial release of episode 18.1 »](https://github.com/rathena/rathena/pull/7917)
(avril 2024), corrigée en 2024‑2025 (dialogues, quête quotidienne de Mejai, points de
réputation de Budan).

`npc/re/quests/quests_18.txt`, `npc/re/mobs/dungeons/oz_dun.txt`,
`npc/re/instances/{MazeofOz,ThorGunsuBase,VillaofDeception,VillaofHighPriest,Wolves,DeepForest}.txt`,
`db/re/reputation.yml`.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

**Tout est copié, rien n'est actif.**

| Élément | Fichier Moonlight | État |
|---|---|---|
| Village et forêt du Loup Gris | `moon/rathena/fields/gw_fild.npc` — 8 lignes sur `wolfvill`, `gw_fild01/02` | **commenté** (`scripts_moon.conf:747`) |
| Labyrinthe d'Oz | `moon/rathena/dungeons/oz_dun.npc` — 8 lignes sur `oz_dun01/02` | **commenté** (`scripts_moon.conf:782`) |
| Labyrinthe d'Oz (instance) | `moon/instances/MazeofOz.txt` | non référencé |
| Base militaire de Thor | `moon/instances/ThorGunsuBase.txt` | non référencé |
| Villa de la Tromperie | `moon/instances/VillaofDeception.txt` | non référencé |
| Villa du Grand Prêtre | `moon/instances/VillaofHighPriest.txt` | non référencé |
| Rassemblement des Loups | `moon/instances/Wolves.txt` | non référencé |
| Forêt Profonde | `moon/instances/DeepForest.txt` | non référencé |
| Quêtes 18 | — | **absent** |

### Les mobs manquants — la liste complète

Treize identifiants, tous absents du roster de 2 117 entrées :

| Fichier de spawn | Mobs manquants |
|---|---|
| `gw_fild.npc` | `EP18_ASHHOPPER`, `EP18_ASHRING`, `EP18_GREY_WOLF`, `EP18_TUMBLE_RING`, `EP18_FIREWIND_KITE`, `EP18_PHANTOM_WOLF` |
| `oz_dun.npc` | `EP18_ASH_TOAD`, `EP18_RAKEHAND`, `EP18_SPARK`, `EP18_HOT_MOLAR`, `EP18_VOLCARING`, `EP18_LAVA_TOAD`, `EP18_BURNING_FANG` (MVP) |

Toutes les cartes (`wolfvill`, `gw_fild01/02`, `oz_dun01/02`, `ra_fild10/11`, `1@nyr`,
`1@adv`, `1@advs`, `1@oz`, `1@tcamp`, `1@exsh`) **sont** dans le mapcache.

## 5. Migration pré-renewal

Chemin identique à celui de [17.1](17-1_illusion.md), avec un volume légèrement supérieur :

1. verser les **13 mobs `EP18_*`** dans `db/import/mobs/` (`gw_fild.yml`, `oz_dun.yml`) ;
2. décommenter deux lignes de `scripts_moon.conf` (747 et 782) ;
3. ajouter **six entrées** à `db/import/instance_db.yml` (base réellement chargée) ;
4. brancher cinq scripts d'instance (laisser `DeepForest`, lié au Spirit Handler) ;
5. arbitrer le **système de réputation** du Loup Gris : Moonlight n'utilise pas
   `reputation.yml` en pré-renewal, il faut soit l'activer, soit remplacer les paliers
   par une progression maison ;
6. convertir l'économie des **Fragments d'Améthyste**.

Le contenu de terrain (un village, deux champs, un donjon à deux étages, un MVP) est
autonome : il peut être ouvert **sans** la chaîne de quêtes, contrairement aux instances
qui en dépendent pour leur déverrouillage.

## 5b. Relevé du 2026‑11‑13 et lot A (2026‑11‑14)

Relevé avant portage : 38 mobs absents (15 de terrain, 23 d'instance et de quête), 41 objets
(15 cartes, 11 pièces `Gray_W_*`, 3 tickets ou coffrets, 2 coiffes, 11 Etc), 171 quêtes sur 172,
7 entrées `instance_db` (67 à 73 chez rAthena). La réputation (`db/re/reputation.yml`, Grey Wolf
Village) n'est pas chargée en pré-renewal (`Mode: Renewal`) mais le C++ et moonclient la gèrent :
il suffira de recopier l'entrée. L'enchanteur Emmett n'a de script nulle part.

**Le terrain n'a aucun warp dans le monde** : on entre au village et au labyrinthe par les PNJ de
la chaîne, ou en sortant de certaines instances.

| Partie | Contenu | Fichiers |
|---|---|---|
| Mobs | 6 de la Forêt du Loup Gris et 7 du Labyrinthe d'Oz (niveau 177 à 212), `re_to_prere_mob.py` sans retouche, `DamageTaken` retiré ; Burning Fang 4,8 M de PV ; 80 lignes de skills | `db/import/mobs/ep18_fields.yml` |
| Cartes | 13 cartes ; Ashring : palier `.@r>=14` en `getallrefine() >= 28` ; Rakehand : `bDef,50` en 5 | `db/import/items/item_db_card.yml` |
| Spawns | `gw_fild.npc` et `oz_dun.npc` décommentés ; aucun drop de carte chez l'amont | `moon/scripts_moon.conf` |
| Mapflags | `nomemo` et `nowarpto` sur `oz_dun01` et `oz_dun02` ; ceux du village viendront avec la chaîne | `moon/mapflag/` |
| Branches, albums | 12 mobs en Dead Branch, Burning Fang en Bloody Branch, cartes à l'album, carte MVP en `mca` et `mvpenchant` ; Burning Fang à 6 points MVP | |
| Accès | **aucune entrée au warp agent** tant que la chaîne n'est pas portée (choix du 2026‑11‑14) | |

## 5c. Lots B et B1 : la chaîne, la réputation et les instances (2026‑11‑14, non validés en jeu)

La chaîne et les instances vont ensemble : la chaîne prend la suite de celle du 17.2
(`ep18_main` 1, posé à la fin du 17.2), entre elle-même dans la Villa du Grand Prêtre, et les cinq
scripts d'instance lisent `ep18_main`.

| Partie | Contenu | Fichiers |
|---|---|---|
| Chaîne | copie de l'amont, 13 109 lignes ; elle commence au manoir de Varmundt (Est, `ba_in01`) | `moon/rathena/quests/quests_18.npc` |
| Instances | Rassemblement des Loups, Purification du Sanctuaire, Labyrinthe d'Oz, Base militaire de Thor, Villa du Grand Prêtre, Villa de la Tromperie (normale et avancée) : `instance_db` 66 à 72 (67 à 73 chez rAthena), constantes régénérées, scripts de l'amont en `.npc` ; la Forêt Profonde (Spirit Handler) reste écartée | `moon/instances/{Wolves,MazeofOz,ThorGunsuBase,VillaofHighPriest,VillaofDeception}.npc` |
| Mobs | 25 mobs de la chaîne et des instances (gardes, Heart Hunters, villageois armés, Schulang et Demi Freyja en trois versions, Grey Goat, Grey Baby Wolf), `re_to_prere_mob.py` sans retouche, `DamageTaken` retiré ; les boss de la Villa de la Tromperie à 2 Md de PV renewal deviennent 21,6 M ; 149 lignes de skills ; `NPC_WIDESTONE2`, `NPC_WIDECONFUSE2`, `NPC_DEADLYCURSE2` et `NPC_CANE_OF_EVIL_EYE` ajoutés au skill_db, statut `Deadly_Defeasance` repris de `db/re` (sans son `DurationLookup` de 4ᵉ classe) | `db/import/mobs/ep18_instances.yml` |
| Objets | 14 : 2 cartes (Schulang, Demi Freyja), 2 coiffes, 10 objets de quête dont les Fragments d'Améthyste de l'épisode ; `Pontifex_Courage` (options des armes Adulter Fides) attend le lot B3 et sort des drops | `db/import/items/` |
| Quêtes | 170 ; aucune collision avec un Id pre-re de contenu différent | `db/import/quest_db.yml` |
| Réputation | Grey Wolf Village (Id 3, `RepPointsWolf`, -5000 à 5000) et le groupe Arunafeltz, importés : `db/re/reputation.yml` n'est pas lu en pre-renewal | `db/import/reputation.yml`, `reputation_group.yml` |
| Accès au compte | les cordes de la gorge (`ra_fild10`), d'Oz et de la forêt s'ouvrent à l'étape 31 et posent `##dun_open_oz_dun01` ; le campeur vers le village s'ouvre à l'étape 35 et pose `##dun_open_wolfvill` ; les autres personnages du compte passent dès le niveau 170 | |
| Warp agent | Donjons > **Grey Wolf / Oz** (village, forêt F1 et F2, Oz F1 et F2), verrou de quête au compte et niveau 170, sur le principe des donjons Illusion ; Instance > **Sanctuary Purification** et **Villa of Deception**, les deux instances rejouables, vers leur PNJ d'entrée | `moon/warp_agent.npc`, `db/import/map_index.yml` |
| Mapflags | village (`town`, `nopenalty`, `reset`, `nomemo`, `nowarpto`, `restricted 7`, et `noattackskill`, `noduel` comme Rachel), cinq cartes d'instance, `que_thr` ; `nobranch` écarté | `moon/mapflag/` |
| Nettoyage | les PNJ bouche-trous `Est#ep181_est01` et `Rookie#ep181_rk01` de `quests_17_2.npc` (9ddef7a79) sont retirés : `quests_18.npc` les définit | |

## 5d. Lot B3 : l'économie (2026‑11‑14, non validé en jeu)

| Partie | Contenu |
|---|---|
| Loup Gris | 10 pièces `Gray_W_*` (DEF 12 pour l'armure, 6 pour la cape et les chaussures, `bAtkRate` en `bAddClass`), le troc contre des Fragments d'Améthyste (`barter_ep18_1` à `3`, appelés par la chaîne) et les tickets +7 et +9 (recettes Lapine) ; les variantes `_JP` ne sont pas reprises |
| Villa de la Tromperie | **74 armes** : 37 Adulter Fides (mode normal) et 37 Vivatus Fides (mode avancé, qui les fait tomber directement), règles habituelles : `Classes` retiré, Rebellion au Gunslinger, Kagerou et Oboro au Ninja, `MagicAttack` en `bMatk`, paliers +11 et +12 en `getallrefine() >= 22` et `>= 24`, `bAtkRate` en `bAddClass`, délais en `bDelayrate` plafonnés à -25 %, compétences fermées par la table (`SO_POISON_BUSTER` → `WZ_HEAVENDRIVE`) ; les 4 armes Doram sont écartées |
| Pontifex | Courage, Wisdom, Belief, Tenacity : options aléatoires sur les armes Fides (recettes Lapine, groupes `PONTIFEX_*` déjà en local) |
| Drops de carte | `1@advs` : MVP inchangés (armes Fides à 1,5 %, Pontifex Belief et Tenacity) ; Fragments d'Améthyste des mobs normaux à 2 % |

**Réforme : écartée de ce lot, portée au lot B4 (§5e).** Restent écartés le Blue Prints Scroll
(plans `Conse_F_*`) et le coffret de Demi Freyja, que rien ne distribue.

## 5e. Lot B4 : la réforme Adulter → Vivatus (2026‑11‑14, non validé en jeu)

Bamdat (`wolfvill` 170,131, `quests_18.npc`, étape 38 de la chaîne) ouvre la fenêtre par
`item_reform("Consecrate_Weapon_1")` (rituel de base) ou `"Consecrate_Weapon_2"` (avancé).

| Partie | Contenu | Fichiers |
|---|---|---|
| Serveur | rien à recompiler : PACKETVER 20250716 ouvre les paquets 0x0B8F à 0x0B92, `item_reform` existe en script, et `db/item_reform.yml` lit `db/import/item_reform.yml` sans condition de mode (seul `db/re` est en `Mode: Renewal`) | |
| Déclencheurs | `Consecrate_Weapon_1` (100648) et `_2` (100649), `DelayConsume` + `item_reform()` comme l'amont ; non distribués, ils servent de clé de recette | `db/import/items/item_db_usable.yml` |
| Matériaux | `Purificatory_Holy_O` (1000501), `Purificatory_Holy_W` (1000502), `Sanctuary_Sudarium` (1000503) | `db/import/items/item_db_etc.yml` |
| Drops de carte | `1@advs`, mode avancé : les trois matériaux à 50 % sur Schulang et Demi Freyja (MVP, taux amont) et à **2 %** sur Sanare et Heart Hunter (monnaie de carte ; 1,5 % en renewal) | `db/import/map_drops.yml` |
| Recettes | 2 × 37 : Adulter Fides **+9** au moins → Vivatus Fides, cartes et options gardées. Base : 50 O + 50 W + 50 Sudarium + 200 Fragments d'Améthyste, raffinement -5 (+9 → +4). Avancé : 200 + 200 + 200 + 400, raffinement -1. **Écart Moonlight : sans les plans `Conse_F_*`** (10 ou 30 en amont), le Blue Prints Scroll n'étant pas porté ; armes Doram absentes | `db/import/item_reform.yml` |
| Client | fenêtre de réforme (liste des pièces admises, pièce et résultat, matériaux possédés / requis, confirmation), table `item_reform.yml` générée par `scripts/yaml_tables/item_reform.py` | moonclient |

⚠ Le serveur refuse **sans répondre** une demande invalide (`clif_parse_item_reform_start`) : le
client juge tout avant l'envoi, y compris la règle « seule la première pile d'un matériau compte »
(`pc_search_inventory`).

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — un village complet, un MVP, six instances |
| Fait | lot A : le terrain ; lots B et B1 : la chaîne, la réputation, les instances et l'accès (2026‑11‑14, non validés en jeu) |
| Fait | lot B3 : l'économie (2026‑11‑14) |
| Fait | lot B4 : la réforme Adulter → Vivatus, serveur et fenêtre moonclient (2026‑11‑14) |
| Reste à faire | validation en jeu · Blue Prints Scroll (plans `Conse_F_*`) et coffret de Demi Freyja · hors périmètre : la Forêt Profonde (Spirit Handler), l'enchanteur Emmett (sans script) |
| Épisode | **clos** côté portage le 2026‑11‑14 |

## Sources

- [Episode XVIII — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVIII)
- [Episode 18: Direction of Prayer — Hazy Forest](https://hazyforest.com/episodes:18_direction_of_prayer)
- [Direction of Prayer — iRO Wiki](https://irowiki.org/wiki/Direction_of_Prayer)
- [rathena/rathena PR #7917](https://github.com/rathena/rathena/pull/7917)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=259)
