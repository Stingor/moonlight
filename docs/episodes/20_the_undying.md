# Épisode 20 — L'Immortel (*The Undying*)

| | |
|---|---|
| Nom kRO | *The Undying* |
| Date kRO | **2023‑03‑02** |
| Arc | fin de la traque de Bagot — le labyrinthe du serpent |
| Niveau kRO visé | **215+** |
| État Moonlight | **mobs, C++, terrain, objets, quêtes et instances portés** depuis brHades (Capuche) (2026‑11‑18, non validés en jeu) ; tables client, Dim Glacier et cartes à faire |

Suite directe d'Issgard : on descend dans le canyon de glace ancien, on démasque les
infiltrés (les faux soldats Iwin), on cherche l'entrée du repaire du serpent avant qu'un
pouvoir immortel ne soit libéré. L'Alliance du Chat Blanc remplace Issgard comme faction.

## 1. Contenu

### Cartes

| Carte | Nom |
|---|---|
| `jor_back4` à `jor_back6` | **Canyon de Glace Ancien** |
| `jor_maze` | **Labyrinthe du Serpent** |
| `jor_root1`, `jor_root2` | Racines |
| `jor_sanct` | Sanctuaire |
| `jor_twig`, `jor_twice` | Branches de l'Arbre-Monde |
| `jor_safty1`, `jor_safty2`, `jor_safy1` | Zones sûres |

### Instances (`instance_db` de brHades, Id 300 à 306)

| Instance | Carte d'entrée | Durée | Nature |
|---|---|---|---|
| **Sticky Sea** (Mer Collante) | `1@slug` | 30 min | histoire |
| **Canyon Exploration** | `1@20cn1` + `1@20cn2` | — | histoire, deux cartes |
| **Drift Ice Zone** | `1@twsd` | — | histoire |
| **Nest of Twigs** | `1@twig` | 20 min | journalière |
| **Separated Sanctuary** | `1@twas` | 30 min | journalière |
| **Immortal** / **Immortal (Hard)** | `1@twbs` | 30 min | boss Lasgand, mode difficile |

### Mécaniques

Les **Parchemins de transformation Rgan** de l'épisode 19 restent nécessaires. La chaîne
principale exige l'épisode 19 achevé et la journalière de Horr, dont le PNJ est remplacé
par le script du 20. Niveau 215 pour les instances.

### Contenu rattaché

- **Hall of Life** (`for_dun01`) et **Jardin du Temps** (`t_garden`), sortis en kRO le
  10 juillet 2023 et rattachés à l'épisode 20 par rAthena
  ([rathena/rathena#8214](https://github.com/rathena/rathena/issues/8214)) : convertis en
  amont (`npc/re/quests/garden_of_time.txt`, `npc/re/mobs/dungeons/for_dun.txt`). Chantier
  séparé, voir § 5.

## 2. Sources

### brHades — la conversion de référence

[glitzhXEC/brHades](https://github.com/glitzhXEC/brHades) (GPL-3.0), commit
[`c2196f9be`](https://github.com/glitzhXEC/brHades/commit/c2196f9be) « ep20 » du
2024‑11‑04, 37 fichiers, environ 21 000 lignes. Les scripts sont signés **Capuche**
(développeur rAthena), au format « Walkthrough Conversion » de la PR de l'épisode 19
([rathena/rathena#8527](https://github.com/rathena/rathena/pull/8527)), dont Issgard est
issu. Aucune PR amont pour le 20 au 2026‑11‑17. Le même dépôt a aussi l'épisode 21
(`a9ebedab8`, 2025‑12‑09).

| Élément | Contenu |
|---|---|
| Instances | 6 scripts dans `npc/re/ep20/instances/` (5 300 lignes) |
| Quêtes | `quests_20_main.txt` (7 700 lignes, variable `ep20_main` jusqu'à 100), `quests_20_daily.txt` (4 900), `quests_20_side_quests.txt` (1 700), `quests_20_misc.txt` (570) |
| Terrain | `ep20_spawn.txt` (`jor_back4` à `jor_root`, MVP Snow Angel), `ep20_warps.txt`, `mapflags.txt` |
| Économie | 2 fichiers de barters, `map_drops.yml` (+207 : minerais, cristaux et extraits EP19/EP20), `laphine_upgrade.yml`, prix d'objets |
| `mob_db` | **40 mobs** écrits en entier ; `db/re` amont n'a que des lignes commentées pour ces Id (21965 à 22007, 23060) |
| `mob_skill_db` | 163 lignes |
| `quest_db` | **159 quêtes** (Id 8839 à 23118), absentes de `db/re` amont |
| Réputation | `REPU_EP20` = 4, **White cat alliance** (`RepPointsWhiteCatAlliance`) |
| C++ | `specialeffect3`, `UMOB_BODYSIZE` / `UNPC_BODYSIZE` (`clif_body_size`), `UMOB_MATK` (renewal seul) |
| Manque connu | `NPC_SEEDTRAP` non implémentée (noté dans `Immortal.txt`) |

### patnawa/rathena_pn — repli

`npc/custom/episode20/` : `Fields.txt`, `Instances.txt`, `NavigationConnections.txt`,
`Progression.txt`, `SidesAndDailies.txt`. Réécriture « clean-room », beaucoup plus courte
(le Canyon tient en trois vagues). À ne consulter que pour un trou de brHades.

## 3. État côté Moonlight (mesuré le 2026‑11‑17)

| Élément | État |
|---|---|
| Cartes | les 17 sont dans le map_cache, instances comprises (`1@slug`, `1@20cn1/2`, `1@twsd`, `1@twig`, `1@twas`, `1@twbs`) |
| Épisode 19 | porté (`moon/rathena/quests/quests_19.npc`, `ep19_main`, Horr) : c'est le prérequis |
| Mobs EP20 | **lot A fait** (2026‑11‑17) : voir ci-dessous |
| Spawns, portails, mapflags | **lot C fait** (2026‑11‑17) : voir ci-dessous |
| Objets, réputation, barters | **lot D fait** (2026‑11‑17), sauf le Dim Glacier et les cartes (D2) |
| Quêtes | **lot E fait** (2026‑11‑18) : voir ci-dessous |
| Instances | **lot F fait** (2026‑11‑18) : voir ci-dessous |
| C++ | **lot B fait** (2026‑11‑17) : voir ci-dessous |
| Hall of Life | `moon/rathena/dungeons/for_dun.npc` toujours **commenté** (`scripts_moon.conf:836`), 32 mobs `SPIRIT_*` à écrire |

### Lot A — mobs et compétences (porté le 2026‑11‑17, non validé en jeu)

| Élément | Détail |
|---|---|
| Fichier | `db/import/mobs/ep20.yml` (40 mobs, Id 21965 à 22007 et 23060), dans le Footer de `db/import/mob_db.yml` |
| Base | brHades : stats, vitesses, IA, modes, classe, drops |
| Conversion | `re_to_prere_mob.py` sans retouche, ATK inchangée ; `Resistance`, `MagicResistance` et `DamageTaken` retirés |
| Ordres de grandeur | mob normal ≈ 510 000 à 590 000 PV et 820 000 à 940 000 EXP ; MVP 4,5 à 6,3 M PV ; Lasgand d'instance 21,6 M |
| Compléments Divine Pride | EXP de Snowstorm Angel (21996) et du Chef Nettoyeur (21980), DEF de Snowstorm Angel, drops des quatre MVP et des deux pierres de Lasgand (22004, 22005), `BaseExp` de Jormungand Guardian-Y absente de brHades |
| Drops écartés | les 20 cartes EP20 (bonus renewal à convertir, comme au 19), les 8 armes Dim Glacier, `Bs_Making_S` (absent chez nous) |
| Matériaux | 26 objets `Ep20_*` / `EP20_*` (1001226 à 1001251) dans `db/import/items/item_db_etc.yml` : entrée `db/re`, prix de brHades, nom anglais du client (« Irwin » corrigé en Iwin, grades alignés sur le 19) |
| Compétences | 163 lignes de brHades en fin de `db/import/mob_skill_db.txt` (+ 2 commentées de la source) |
| `skill_db` | `NPC_LEASH` (747) et `NPC_WIDELEASH` (748) copiés de `db/re` (Fallen Angel Slug) : implémentés en C++ (`skills/npc/leash.cpp`, `wideleash.cpp`), absents du pré-re |
| Audits | `audit_mob_skills.py` et `audit_status.py` : 0 problème |
| Sprites | `EP20_NPC_NYAR`, `EP20_NPC_LASGAND` : sprites de PNJ |

**Reporté aux lots suivants.** Immortal retouche Lasgand par `setunitdata` : DEF 527 et 700,
RES, MATK. En pré-renewal, une DEF au-dessus de 100 rend le boss quasi invulnérable, à adapter
au lot F. `NPC_SEEDTRAP`, `NPC_RAINOFMETEOR` et `NPC_GROUNDDRIVE` y sont lancés par script.
### Branches (2026‑11‑17, épisodes 19 et 20)

Liste validée par l'utilisateur, dans `db/import/mob_summon.yml` :

| Branche | Ajouts | Taux |
|---|---|---|
| `Branch_Of_Dead_Tree` | 21 mobs de terrain du 19 (ceux que pose `moon/mobs/issgard.npc`) et 22 du 20 | 32000 |
| `Bloody_Dead_Branch` | Ultra Limacina (19), Snowstorm Angel et Chef Nettoyeur (20) | 25000 |

Écartés : copies d'instance `MD_*`, invocations `G_*`, PNJ, pierres de stockage, Rgan E (forme
de transformation, jamais posé), Ice Egg (immobile), boss d'instance.

### Lot B — C++ (porté le 2026‑11‑17, non validé en jeu)

Rien de tout cela n'existe en amont (`upstream/master` du 2026‑11‑15) : pas de conflit sémantique.

| Élément | Fichier | Détail |
|---|---|---|
| `specialeffect3 <effet>,<cible>,<unité>` | `script.cpp` | effet visuel sur n'importe quelle unité |
| `UMOB_BODYSIZE`, `UNPC_BODYSIZE` | `script.hpp`, `script_constants.hpp`, `script.cpp` | `setunitdata` et `getunitdata` ; 100 = taille normale |
| `unit_data::body_size` | `unit.hpp` | 0 = non posée |
| `clif_body_size` | `clif.cpp`, `clif.hpp` | statut client 1421, renvoyé à l'apparition, au déplacement et à la découverte de l'unité. Écart à brHades : la découverte n'envoie qu'au joueur concerné, pas à toute la zone |
| `UMOB_MATK` | `script.hpp`, `script_constants.hpp` | constante seule, pour que les scripts se chargent ; sans effet en pré-renewal (le code de brHades est sous `#ifdef RENEWAL`). Les appels d'Immortal sont à réécrire au lot F |
| Écarté | `status.cpp` | retouches de calcul d'ATK et de MATK propres au renewal |

Compilé en Release sous Windows sans erreur ; les bibliothèques du dépôt sont en Release (le
Debug échoue à l'édition de liens). Pas encore compilé sur pc-serv.

### Lot C — terrain (porté le 2026‑11‑17, non validé en jeu)

| Élément | Détail |
|---|---|
| Spawns | `moon/mobs/ep20.npc` : 42 lignes de brHades sur `jor_back4` à `jor_back6`, `jor_maze`, `jor_root1` à `jor_root3`, `jor_twice`, `jor_twig` ; délai de 5 s comme au 19. Les 4 lignes de l'épisode 19 sur `jor_back4` à `jor_back6` quittent `issgard.npc` (mêmes densités) |
| MVP | `moon/mobs/mvps.npc` : Snowstorm Angel sur `jor_back4` et le Chef Nettoyeur sur `jor_root3`, 6 h (Divine Pride ; brHades disait 2 h, « custom »), 6 et 8 points de classement |
| Portails | fin de `moon/mobs/ep20.npc`, tels que brHades : `jor_back3` ↔ `jor_back4` ↔ `jor_back5` ↔ `jor_back6` ↔ `jor_root3`, `jor_root1` → `jor_nest`, racines, sanctuaire, 24 portails du labyrinthe, zones sûres. Verrous de chaîne : `jor_maze` → `jor_twice` (`ep20_main` 34), la chambre 5 du château (`icas_in2`, 61), le chat blanc vers `jor_twig` (34) |
| Contrôle | 112 coordonnées (positions, destinations, `warp` de script) toutes sur une cellule marchable |
| Mapflags | `nomemo` sur le labyrinthe, les racines, `jor_twice`, le sanctuaire et les zones sûres ; `noteleport`, `noicewall` sur les zones sûres et `icas_in2` ; `nowarp`, `nowarpto` sur ces trois-là et le sanctuaire ; `privateairship_source` et `_destination` sur Issgard, `jor_tail`, `jor_back1` à `jor_back6` |
| Écartés | `nobranch` (inversé sur ce fork) ; `restricted 7` de `jor_sanct` (le bit 2048 sert ici au marqueur `noattackskill`) ; mapflags des cartes d'instance, au lot F |
| Noms de carte | 10 noms dans `db/import/map_index.yml`, repris de `map_names.yml` de moonclient |
| Noms de mob | `EP20_SNOW_ANGEL` « Snowstorm Angel », `EP20_BOSS_CLANER` « Jormund Chief Cleaner » (iRO), au lieu des noms tronqués de brHades |

`jor_root1` n'a pas d'entrée depuis `jor_nest` : elle passe par la chaîne (lot E, porte
`#20ms01`), comme la zone de glace dérivante (`jor_twice`, transport de Cororo).

### Lot D — objets et économie (porté le 2026‑11‑17, non validé en jeu)

| Élément | Fichier | Détail |
|---|---|---|
| Réputation | `db/import/reputation.yml`, `reputation_group.yml`, `const.yml` | **White Cat Alliance, Id 5**, `RepPointsWhiteCatAlliance`, -3000 à 3000, dans le groupe Isgard. brHades la mettait à l'Id 4 **à la place d'Isgard** : chez nous la 4 reste Isgard (épisode 19). `REPU_EP20` vaut 5 (constante : redémarrage complet) |
| Objets de quête | `item_db_etc.yml` | `Ep20_Jor_Berry`, `Ep20_Cat_Whiskers` (monnaie de l'Alliance), `Ep20_Jor_Device`, `Ep20_Ymir_Heart` ; entrées `db/re`, noms du client |
| Consommables | `item_db_usable.yml` | `Ep20_Fatty_Icegangu` (apprivoise le Copo, `pet;`), `Glacier_Armor_9Refine` (cube +9 des armures Glacier) |
| Costumes | `item_db_costumes.yml` | `C_Armored_Copo`, `C_Egg_Copo` |
| Familier | `pet_db.yml`, `item_db_pet.yml` | le Copo (œuf `Copo_Egg`), `bAtkRate` en `bAddClass`. Évolutions vers les deux Icewind **écartées** : elles exigent les cartes Icewind, non portées |
| Laphine | `laphine_upgrade.yml` | `Glacier_Armor_9Refine` : +9 sur les 7 armures Glacier, raffinement 8 au plus, sans carte |
| Barters | `moon/barters.yml` | `barter_ep20_shop_1_A`, `_1_B` (costumes, cube +9), `barter_ep20_glacier_eq` (11 pièces Glacier contre 100 moustaches), `barter_ep20_shop_2` (pierres et extraits de magie pure) ; `EP20_DGA_Refine` (Dim Glacier) retiré. Les PNJ qui les ouvrent arrivent avec les quêtes (lot E) |
| Contrôle | | toutes les références (barters, Laphine, familier) pointent vers un objet ou un mob chargé ; chargement sans erreur |

**Reporté (sous-lot D2) : le Dim Glacier et les cartes.** Le Dim Glacier compte 26 armes et
6 armures `D_Glacier_*`, le grade, l'enchantement 47, Forr, le four d'incinération,
`EP20_DGA_Refine` et `barter_ep19_new_content`. Il y a aussi les 44 cartes EP19 et EP20 (bonus
renewal à convertir), puis les deux familiers Icewind qui en dépendent. Les `map_drops` de
brHades ne couvrent que les cartes d'instance : ils iront au lot F.

### Lot E — quêtes (porté le 2026‑11‑18, non validé en jeu)

| Élément | Fichier | Détail |
|---|---|---|
| Quêtes | `db/import/quest_db.yml` | les **159** de brHades (Id 8839 à 23118), aucune collision avec l'import ni `db/pre-re` ; trois titres à deux-points nus mis entre guillemets (« Standby: … », bug déjà vu dans la PR du 19) ; objectifs et drops pointent vers des mobs et objets chargés |
| Scripts | `moon/rathena/quests/quests_20_main.npc`, `_daily`, `_side_quests`, `_misc` | copies de brHades (7 700, 4 900, 1 700 et 570 lignes ; 345 PNJ), latin-1, chargés après `quests_19.npc` |
| Texte | | ponctuation cp949 convertie en cp1252 (`●` → `•`, `…`, `―` → `—`, `※` → `*`, émoticônes `(≥▽≤)` → `(>v<)`), ligne coréenne d'en-tête retirée |
| Horuru | `quests_20_main.npc`, `quests_19.npc` | brHades remplace le PNJ de la quotidienne du 19 (`icecastle,27,126`) par une version qui garde la quotidienne et ouvre la chaîne du 20, et décharge l'ancien par `unloadnpc("ep19_if19msdq")`. Nos PNJ n'ont pas ce nom unique : `#e19ms00` et `Horuru#if19msdq` sont **commentés** dans `quests_19.npc`, `ep20_temporary_cleaning` retiré. « Horr » devient **Horuru** partout, le nom de notre 19 |
| Noms de PNJ du 19 | | brHades vise les PNJ du 19 par des noms uniques `ep19_<carte>_<x>_<y>` que notre 19 n'a pas : les cinq cités (Toralung, Horyara, Zoryara, Lazy, Juncea) renvoient à nos noms |
| Réputation | | la quotidienne de Horuru créditait la réputation **3** (Village des Loups Gris), bug de brHades : remise sur `REPUTATION_EP19` (Isgard), comme notre 19. Le reste de l'épisode crédite `REPU_EP20` |
| Fonctions | `moon/rathena/other/Global_Functions.txt` | `F_ep20_checkTransform` (renvoie au nid qui n'est pas déguisé en Rgan) ; `queststatus_between` appelé `F_queststatus_between`, notre nom |
| Bugs de brHades | | `Lehar#20ms41` sans son nom unique `ep20_jor_safty2_310_43` (trois `npctalk` le visent) ; une espace finale dans un nom de PNJ visé par `npctalk` |
| Boutiques | | les quatre barters du lot D, ouverts par les PNJ de l'Alliance (`callshop`) |
| Images | | 66 `cutin`, dont 13 nouvelles `ep20_*` (Cocopo, Lasgand, Nyar) : à confirmer côté client ; les autres servent déjà aux épisodes 18 et 19 |

**Déblocage au compte Moonlight**, comme les épisodes 18 et 19, dès le niveau 215 (celui du
début de la chaîne) :

| Drapeau | Posé par | Ouvre |
|---|---|---|
| `##dun_open_jor_root` | `ep20_main` 16 | la porte des racines (`jor_nest` → `jor_root1`), et au warp agent `jor_root1`, `jor_root2`, `jor_sanct`, `jor_maze` |
| `##dun_open_jor_twice` | `ep20_main` au-delà de 40 | le transport de Cororo (`icecastle` → `jor_twice`), et `jor_twice` au warp agent |

Le warp agent (Donjons > Issgard) gagne six cartes au niveau 215 : les cinq ci-dessus et
`jor_root3`, ouverte avec Issgard puisque les portails de `jor_back6` y mènent. Il pose aussi
les drapeaux à qui a passé ces étapes, et renvoie les autres à Horuru.

**Contrôle de chargement** (2026‑11‑18) : `map-server-generator --generate-navi`, 1 305 quêtes
dans l'import (+159), 22 390 PNJ (+436), aucune erreur nouvelle.

### Lot F — instances (porté le 2026‑11‑18, non validé en jeu)

| Instance | Id | Carte | Entrée | Délai | Au warp agent |
|---|---|---|---|---|---|
| Sticky Sea | 84 | `1@slug` | Cocopo, `jor_back4` (`ep20_sidequest_1` 18, rejouable à 20) | 11950 | oui |
| Canyon Exploration | 85 | `1@20cn1`, `1@20cn2` | Lehar, `icecastle` (`ep20_main` 9) | — | non |
| Drift Ice Zone | 86 | `1@twsd` | le mur branlant, `jor_maze` (`ep20_main` 32) | — | non |
| Nest of Twigs | 87 | `1@twig` | Lehar, `jor_twice` (`ep20_main` 42) | — | non |
| Separated Sanctuary | 88 | `1@twas` | Aurelie, `jor_twig` (`ep20_main` 60) | — | non |
| Immortal / Immortal (Hard) | 89, 90 | `1@twbs` | les Branches de l'Arbre-Monde, `jor_twig` (`ep20_main` 100, niveau 215) | 12588, 12589 | oui |

brHades les numérote 300 à 306 ; chez nous, les Id libres suivants (84 à 90). Les noms ne
changent pas, les scripts les citent. Constantes `INST_*` régénérées.

| Élément | Détail |
|---|---|
| Scripts | `moon/instances/StickySea.npc`, `CanyonExploration.npc`, `DriftIceZone.npc`, `NestOfTwigs.npc`, `SeparateSanctuary.npc`, `Immortal.npc` : copies de brHades, latin-1, ponctuation cp949 en cp1252 |
| Lasgand (Immortal) | brHades retouche ses stats par `setunitdata` : `UMOB_MATK`, que le pre-re n'a pas (le `setunitdata` échouerait et couperait le script d'apparition), DEF 527/700 et MDEF 286/400, qui en pre-re annuleraient tout dégât physique, `UMOB_RES` et `UMOB_MRES` (traits renewal). **Facile** : les stats de `re_to_prere_mob.py` (DEF 65, MDEF 44, MATK naturelle 3 296–6 156, l'ordre des 4 000 de brHades). **Difficile** : DEF 77 (plafond du convertisseur) et MATK ×1,75 (5 768–10 773), le rapport 7 000/4 000 de brHades. **Vision** : MATK ×7,5 (24 720–46 170), le rapport 30 000/4 000 |
| Mapflags | ceux de brHades **sans `nobranch`** (inversé sur Moonlight) : `nomemo`, `nosave`, `nowarp`, `nowarpto`, `noteleport`, `monster_noteleport`, `noicewall`, `partylock`, `restricted 6` sur les sept cartes, `nomobloot` sur `1@twig` |
| `map_drops` | Fallen Angel (`1@slug`), Lasgand et ses deux coffres (`1@twbs`) : minerais, cristaux et extrait EP19/EP20, `Ep20_Angels_Tail`, `C_Serpent_Shadow`. Retirés : `EP19_S_F_1_Extract` et `EP19_Gla_Extract` (déjà écartés des coffres du 19) et les six `D_Glacier_*` (D2) |
| `NPC_SEEDTRAP` | non implémentée chez brHades non plus : le piège de la Vision ne fait rien |
| Warp agent | Instance > Immortal et Sticky Sea, qui envoient près de leur PNJ d'entrée ; les quatre autres ne se jouent que dans la chaîne |

**Contrôle de chargement** (2026‑11‑18) : 87 instances, 22 536 PNJ, **0 erreur, 0
avertissement**. Le premier passage signalait `specialeffect3` inconnu : `map-server-generator.exe`
datait du 15/11, avant le lot B. Recompilé en Release, il charge tout. `audit_instance.py` ne
relève rien d'autre que l'absence voulue des quatre instances de la chaîne au warp agent.

### Lot G — client (porté le 2026‑11‑18, non validé en jeu)

Tables de `G:\GitHub\moonclient\dist\data\yaml_files`, régénérées depuis le serveur après
les lots A à F (procédure du §11 de la feuille de route).

| Table | Changement |
|---|---|
| `reputation.yml` | faction 5 White Cat Alliance, rangée dans le groupe Isgard avec la faction 4 |
| `quests.yml` | les 159 quêtes du bloc « Episode 20 » de `quest_db`, prises de `SystemEN\OngoingQuests.lub` (ROenglishRE, Renewal) et insérées à leur place sans redumper le fichier ; 18 quêtes d'attente marquées `cooldown: true` (`quest_cooldowns.py` restreint à l'épisode) |
| `item_db`, `item_properties`, `item_groups`, `map_drops` | les objets EP20, leurs poids, la Glacier Armor +9 Refinement Cube et les drops de `1@slug` / `1@twbs` |
| `mob_db`, `mob_db_context`, `monster_list` | les 40 mobs du lot A |
| `pets.yml` | Copo (21965, œuf 9145, Chubby Ice Gangu 102202) |
| `laphine.yml` | Glacier Armor +9 Refinement Cube (102227), sept armures cibles |
| `item_info.yml` | 0 objet sans fiche ; 34 faits serveur (type, poids) retirés de 33 fiches EP20 |
| `navigation.yml` | relue depuis la génération du contrôle du lot F (cartes, PNJ et spawns de l'épisode) |

`warp_agents.yml` ne change pas. Restent ouverts : les 15 `cutin "ep20_*"` des scripts, à
chercher dans les GRF du client ; 52 autres quêtes du serveur inconnues du journal
(Crash d'aéronef, Tombeau du Remords, Tour de Thanatos, Tour des Constellations, Arène
Nocturne de Geffen), hors de l'épisode 20.

## 4. Plan de portage

Mêmes règles que pour le 19 (checklist de portage de contenu renewal → pre-re). Chaque
lot se valide au boot du serveur avant le suivant.

| Lot | Contenu | Points d'attention |
|---|---|---|
| **A — Mobs** | 40 mobs dans `db/import/mobs/ep20.yml` via `re_to_prere_mob.py`, 163 lignes de `mob_skill_db` | audit des compétences pre-re (`audit_mob_skills.py`) ; `NPC_SEEDTRAP` à remplacer ou à écrire ; `UMOB_MATK` sans effet en pre-re ; Snow Angel et Lasgand rattachés aux albums |
| **B — C++** | `specialeffect3`, `UMOB_BODYSIZE`, `UNPC_BODYSIZE` | `git diff upstream/master` sur chaque fichier avant de patcher ; recompilation |
| **C — Terrain** | spawns, portails, mapflags | mapflags relus contre les pièges connus (`nobranch` inversé) ; Warp Agent |
| **D — Objets et économie** | objets EP20 de `db/re` vers `db/import`, `map_drops`, barters, Laphine, réputation White cat alliance | constante `REPU_EP20` au boot ; table `reputation` de moonclient ; prix de boutique ≥ 1,65 × revente |
| **E — Quêtes** | 159 entrées `quest_db` ; `quests_20_main`, `_daily`, `_side_quests`, `_misc` | remplacement du PNJ Horr de `quests_19.npc` ; 586 `cutin` (images à vérifier côté client) ; latin-1 |
| **F — Instances** | 7 entrées `instance_db`, 6 scripts | Id libres côté Moonlight ; Warp Agent (enum généré depuis `instance_db.yml`) ; hard mode |
| **G — Client** | tables moonclient (quêtes, réputation, objets) | fait le 2026‑11‑18 ; cutins à vérifier dans les GRF |

## 5. Hall of Life

Toujours à part : le donjon d'élémentaires est copié et commenté, il manque ses 32 mobs
`SPIRIT_*` (huit familles, quatre tailles). Indépendant de la chaîne narrative, il peut
passer avant ou après les lots ci-dessus.

## 6. Verdict

| | |
|---|---|
| Intérêt | **élevé** — la suite directe d'Issgard, complète, du même auteur que la conversion du 19 |
| Reste à faire | tout : lots A à G, puis le Hall of Life |

## Sources

- [glitzhXEC/brHades, commit c2196f9be « ep20 »](https://github.com/glitzhXEC/brHades/commit/c2196f9be)
- [patnawa/rathena_pn, `npc/custom/episode20`](https://github.com/patnawa/rathena_pn/tree/main/npc/custom/episode20)
- [Episode XX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XX)
- [Undying — iRO Wiki](https://irowiki.org/wiki/Undying)
- [Garden of Time — iRO Wiki](https://irowiki.org/wiki/Garden_of_Time)
- [rathena/rathena issue #8214](https://github.com/rathena/rathena/issues/8214)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=277)
