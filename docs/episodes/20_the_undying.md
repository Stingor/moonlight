# Épisode 20 — L'Immortel (*The Undying*)

| | |
|---|---|
| Nom kRO | *The Undying* |
| Date kRO | **2023‑03‑02** |
| Arc | fin de la traque de Bagot — le labyrinthe du serpent |
| Niveau kRO visé | **215+** |
| État Moonlight | **mobs, C++, terrain, objets, quêtes et instances portés** depuis brHades (Capuche) (2026‑11‑18, non validés en jeu) ; tables client, Dim Glacier (D2a), cartes EP19/EP20 (D2b) et familiers Icewind (D2c) faits ; Hall of Life porté |

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
| Drops écartés | `Bs_Making_S` (absent chez nous). Les 20 cartes EP20 et les 8 armes Dim Glacier, écartées au lot A, sont revenues aux sous-lots D2b et D2a |
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
| Familier | `pet_db.yml`, `item_db_pet.yml` | le Copo (œuf `Copo_Egg`), `bAtkRate` en `bAddClass`. Évolutions vers les deux Icewind ajoutées au sous-lot D2c, avec les cartes Icewind |
| Laphine | `laphine_upgrade.yml` | `Glacier_Armor_9Refine` : +9 sur les 7 armures Glacier, raffinement 8 au plus, sans carte |
| Barters | `moon/barters.yml` | `barter_ep20_shop_1_A`, `_1_B` (costumes, cube +9), `barter_ep20_glacier_eq` (11 pièces Glacier contre 100 moustaches), `barter_ep20_shop_2` (pierres et extraits de magie pure) ; `EP20_DGA_Refine` (Dim Glacier) retiré. Les PNJ qui les ouvrent arrivent avec les quêtes (lot E) |
| Contrôle | | toutes les références (barters, Laphine, familier) pointent vers un objet ou un mob chargé ; chargement sans erreur |

**Reporté (sous-lot D2) : le Dim Glacier et les cartes.** Le Dim Glacier est porté au sous-lot
D2a (plus bas). Les 44 cartes EP19 et EP20 sont portées au sous-lot D2b, les deux familiers Icewind au D2c. Les `map_drops` de
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

### Sous-lot D2c — familiers Icewind (porté le 2026‑11‑18, non validé en jeu)

| Élément | Fichier | Détail |
|---|---|---|
| Familiers | `pet_db.yml` | Gray Icewind (`EP20_ICEWIND1`) et Icewind (`EP20_ICEWIND2`), de brHades : `bAtkRate` en `bAddClass`, POW en STR, SPL en INT ; non apprivoisables (`CaptureRate: 0`), ils ne s'obtiennent que par évolution |
| Évolution du Copo | idem | vers l'un ou l'autre : 200 grandes plumes (grises ou blanches), 150 coquilles d'œuf, 150 têtes de Copo et la carte Icewind correspondante |
| Œufs | `item_db_pet.yml` | `Grey_Icewind_Egg` (9146), `Icewind_Egg` (9147) |
| Combos des cartes | `item_combos.yml` | 15 combos des cartes EP19/EP20, oubliés au D2b (Copo + Icewind, Rgan, Hallucigenia…) ; `bAtkRate` converti. Écartés : les deux de `Lore_Seeker`, absent |
| Client | moonclient | `pets.yml` (évolutions), fiches des trois œufs alignées (celle du Copo avait gardé « ATK +n% » au lot D) |

### Sous-lot D2b — cartes EP19 et EP20 (porté le 2026‑11‑18, non validé en jeu)

| Élément | Fichier | Détail |
|---|---|---|
| Cartes | `item_db_card.yml` | 44 : 24 de l'épisode 19 (300359 à 300382) et 20 de l'épisode 20 (300455 à 300474). Règles habituelles : traits, `bAtkRate` en `bAddClass`, `readparam(bSpl)` en `readparam(bInt)`, paliers `>= 11` / `>= 12` et `< 12` en `getallrefine() >= 22` / `>= 24` et `< 24` (règle 2N, somme du raffinage porté), niveau d'arme 5 → 4, DEF ÷50 |
| Niveau d'armure 2 | idem | refusé en pre-re : traduit par « la pièce qui porte la carte est un Dim Glacier » (armure/robe, cape/cache-col, bottes/souliers, `getequipid`). Sur casque (Icewind's Egg) et bouclier (Modified Senior Rgan), sans Dim Glacier, le bloc est retiré |
| Jormungandr Sanctuary Guardian | idem | DEF par raffinage retirée (règle Glacier), RES par raffinage convertie : de 0 à -8 % de dégâts physiques à +10 |
| Drops | `mobs/ep19.yml`, `mobs/ep20.yml` | 45 drops (0,01 %, protégés du vol) : 23 de la PR #8527, 22 de brHades. **Sans source** : `Aquila_Card`, `Snow_Angel_Card`, `Rgan_Chief_Clean_Card` |
| Client | `item_info.yml` | 30 fiches alignées (traits, paliers en « Refine Total Stuff », « When compounded on a Dim Glacier piece », niveau d'arme 4) |

Albums (feuille de route §12, validés le 2026‑11‑18) : 37 cartes à l'album commun
(`CARDALBUM`, `Rate: 1`), 4 cartes de boss (Lasgand, ange déchu, Juncea, Ultra Limacina) à
l'album magique et à l'enchanteur MVP (`Rate: 1`).

### Sous-lot D2a — Dim Glacier (porté le 2026‑11‑18, non validé en jeu)

La chaîne entière : les armes tombent des mobs, le **four d'incinération** les fond en
matériaux (synthèse Laphine `D_Gw_Extractor`, coffre `D_GW_EXTRACTOR`), **Forr** les échange
(`barter_ep19_new_content`) et enchante les armes (47), **Pipiring** enchante les armures
Glacier et Dim Glacier (89 à 97). Le lot E appelait déjà ces enchantements et `IG_D_GW_EXTRACTOR`
sans qu'ils existent : le chargement ne le voyait pas (une constante inconnue y est lue comme
une variable), les menus auraient échoué en jeu.

| Élément | Fichier | Détail |
|---|---|---|
| Armes | `item_db_weapon.yml` | 25 `D_Glacier_*` sur 26 (Foxtail, Doram, écarté) : `Jobs` et `Gender` des armes Glacier du 19, WeaponLevel 4, `MagicAttack` en `bMatk`, `[1]` |
| Armures | `item_db_armor.yml` | 6 pièces : DEF 14 (armure, robe) et 7 (cape, cache-col, bottes, souliers), soit les 12/6 du Glacier relevés d'un cran ; ArmorLevel 1 (2 est refusé en pre-re ; les conditions `getequiparmorlv()==2` des cartes EP19/EP20 seront à revoir au D2b) ; grade retiré comme au Glacier ; traits, `bAtkRate` et paliers +11 convertis |
| Orbes | `item_db_enchant.yml` | 47 `Ice_C_Orb_*` (89 à 94) et 93 `Snake_*` (95 à 97, paliers 1 à 10) : traits convertis, DEF ÷50 et MDEF ÷5 (règle de l'orbe), grade retiré, +11 en `getallrefine() >= 22` |
| Matériaux | `item_db_etc.yml`, `item_db_usable.yml` | `EP19_S_F_1/2/3_Extract`, `EP19_Gla_Extract`, `EP19_D_P_Stone` ; `D_Gw_Extractor`, `EP19_DGW_Refine`, `EP20_DGA_Refine` |
| Enchantements | `item_enchant.yml` | 47 : 121 orbes Glacier sur 280 (les 81 écartés au 19, quatre cases en trois, et les 10 orbes de grade renewal : la case 1 disparaît). 89 à 97 entiers |
| Reforms | `item_reform.yml` | renforcement des armes et des armures : **+9 → +10 seulement** (+9..+11 → +10..+12 en renewal), Foxtail retiré |
| Four, coffre | `laphine_synthesis.yml`, `item_group_db/ep19.yml`, `src/map/itemdb.hpp`, `script_constants.hpp` | trois armes Dim Glacier contre un tirage `D_GW_EXTRACTOR` (taux custom de la PR #8527), constante `IG_D_GW_EXTRACTOR` ajoutée en C++ comme dans la PR (un groupe sans valeur d'enum est refusé) ; le même coffre sert au cache mystérieux de Pipiring |
| Forr | `moon/rathena/merchants/episode_19.npc`, `moon/barters.yml` | Forr et le four (`icas_in` 188,60 et 192,57), 8 échanges ; `laphine_synthesis` appelé par l'AegisName. `EP20_DGA_Refine` revient chez Pipiring (`barter_ep20_shop_2`) |
| Combos | `item_combos.yml` | 62 : les deux sets (armure/bottes/cape et robe/souliers/cache-col), seuls et avec chacune des 24 armes, et 9 combos d'orbes serpent ; grade de l'arme gardé (le fork en donne un aux armes). Écartés : Foxtail et les combos des pièces Isgard, absentes |
| Drops | `mobs/ep20.yml`, `map_drops.yml` | les 8 armes des mobs EP20 (1 %) ; `1@slug` et `1@twbs` entiers (extraits et six pièces Dim Glacier de Lasgand et de ses coffres, 10 %) |
| Coffres du 19 | `item_group_db/ep19.yml` | Aquila (×2) et Ultra Limacina reprennent leurs lignes écartées faute d'objets : armes Dim Glacier, extraits, sort miasmatique, matériaux EP20. Raffinements au-delà de +10 ramenés à +10, comme au 19 |

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

## 5. Hall of Life (porté le 2026‑11‑18, non validé en jeu)

Le donjon (`for_dun01`, `for_dun02`) et le **Jardin du Temps** (`t_garden`), qui y donne
accès par la barrière dimensionnelle de `xmas_fild01` (niveau 250). Source : `npc/re`
(Atemo), `db/re`.

| Élément | Fichier | Détail |
|---|---|---|
| Mobs | `db/import/mobs/hall_of_life.yml` | les 48 `SPIRIT_*` (32 de terrain, 16 des vagues de contamination de `for_dun.npc`), `re_to_prere_mob.py` ; aucune compétence amont. `HEROS_IN_ORB_1` à `_4` et `HEROS_OUT_ORB` : sprites des distributeurs et de l'orbe |
| Objets | `db/import/items/` | 53 : 39 matériaux (gemmes et colères des esprits, énergies, matériaux de saison, fruits), 4 Capuches des saisons (DEF 7, grade retiré, traits et +11 convertis), 4 Signets (sans DEF, comme les anneaux Glacier), Protection et Barrière Constellation, cristaux de retour, parchemins |
| Quêtes | `db/import/quest_db.yml` | 36 (12592 à 12602, 12618, 12620 à 12643), journal du client compris, 5 d'attente |
| Scripts | `moon/rathena/quests/garden_of_time.npc`, `moon/rathena/dungeons/for_dun.npc` | le Jardin activé, le donjon décommenté. Les distributeurs #3 et #4 appellent les enchantements 117 à 124 et 142, la réforme `Season_H_Upgrade` (Capuche → cape Circulation of Life) le distributeur des équipements |
| Enchantements | `db/import/item_enchant.yml` | 117 à 124 et 142 **d'après `db/re`**, où rAthena les commente (« some fields are not supported ») : kRO y fait monter les pierres au hasard. Ici, montées **à coup sûr** (`Upgrades`) : coûts du printemps relevés sur MuhRO, interpolés ailleurs, une étape au hasard coûte son prix kRO divisé par sa chance. Impairs = Capuches (graine 1-10, fruit 1-15, vie 1-15), pairs = capes Circulation of Life (jusqu'à 20, feuille 1-5 et fleur 1-5). Énergies élémentaires : printemps Vent+Terre (MuhRO), été Feu+Terre, automne Vent+Eau, hiver Eau+Feu (choix Moonlight). 142 = Signets : grade 0 seul, montées 1 à 5 Moonlight (Circulation of Life, Énergie élémentaire, 0,5 à 2 M). Remplace la version sur mesure du même jour (orbes Glacier) |
| Échanges | `moon/barters.yml` | `barter_t_garden_1` à `_5` (énergies, Signets, Capuches, clés) |
| Raccordement | `map_drops.yml`, `noteleport.txt`, `map_index.yml` | `map_drops` de `for_dun01` (pierres Etel), `noteleport` sur les deux zones, noms des trois cartes, `naviregisterwarp` sur la barrière, les entrées et les sondes de retour |
| Circulation of Life | `item_db_armor/enchant/etc/usable`, `item_reform.yml`, `item_combos.yml` | 4 capes (DEF 8, grade retiré, traits et +11 convertis), 191 pierres (Physical/Magical Seed, Leaf, Flower, Fruit ; Life of saison 1-20 ; Signets 1-5 ; Token of Life ; traits convertis, **un seul bonus de dégâts par pierre de Signet**), matériaux Circulation, Vérité et Lumière de la vie, parchemin de réforme ; 27 combos (Signet + cape, pierres + Star Cluster). Écartées : les 12 variantes « Life of saison (stat) », identiques au niveau 20 |
| Matériaux | `db/import/mobs/hall_of_life.yml` | rAthena n'a pas le donjon-instance du Hall of Life, seule source kRO. **Choix Moonlight** : les 32 esprits de terrain lâchent Circulation of Life (2 % petits, 4 % autres) et un fragment de météorite de leur élément (2-3 %) ; géants : Fruit de la naissance 1 %, Lumière de la vie 0,2 % ; mutants : Fruit de l'extinction 1 %, Vérité de la vie 0,3 %. Une cape montée au maximum : ~580 Circulation, 285 Fruits de chaque, 131 Vérités, 50 Lumières (taux à revoir en jeu) |
| Écartés | | les combos Signet + bottes Dimension (épisode 21), les coffres d'événement `T_GARDEN_EV_*` |

Branches (§12), écrites le 2026‑11‑18 : les 32 `SPIRIT_*` de terrain dans
`Branch_Of_Dead_Tree` (Rate 32000) ; les 16 des vagues scriptées écartés.

## 6. Verdict

| | |
|---|---|
| Intérêt | **élevé** — la suite directe d'Issgard, complète, du même auteur que la conversion du 19 |
| Porté | lots A à G, sous-lots D2a à D2c, Hall of Life (2026‑11‑17 et 18) ; chargement sans erreur ni avertissement, 74 cutin présentes dans les GRF |
| Choix Moonlight | `Aquila_Card`, `Snow_Angel_Card`, `Rgan_Chief_Clean_Card` n'ont aucun drop amont (ni db/re, ni brHades) : 0,01 % protégé du vol sur Aquila (`EP19_MD_AQUILA`, Crash d'aéronef), le Chef Nettoyeur (`jor_root3`) et Snowstorm Angel (`jor_back4`), et rangées aux albums magique et MVP (2026‑11‑18) |
| Reste à faire | validation en jeu (rien n'est testé) ; taux des matériaux du Hall of Life |

## Sources

- [glitzhXEC/brHades, commit c2196f9be « ep20 »](https://github.com/glitzhXEC/brHades/commit/c2196f9be)
- [patnawa/rathena_pn, `npc/custom/episode20`](https://github.com/patnawa/rathena_pn/tree/main/npc/custom/episode20)
- [Episode XX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XX)
- [Undying — iRO Wiki](https://irowiki.org/wiki/Undying)
- [Garden of Time — iRO Wiki](https://irowiki.org/wiki/Garden_of_Time)
- [rathena/rathena issue #8214](https://github.com/rathena/rathena/issues/8214)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=277)
