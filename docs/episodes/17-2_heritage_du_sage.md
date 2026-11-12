# Épisode 17.2 — L'héritage du Sage (*Sage's Legacy*)

| | |
|---|---|
| Nom kRO | *Sage's Legacy* — le manoir de Varmundt |
| Date kRO | **2020‑10‑30** |
| Arc | l'héritage de Varmundt |
| Niveau kRO visé | 185‑200 |
| État Moonlight | **porté (lots A, A2, B1, B2, B3), du 2026‑11‑11 au 2026‑11‑12, à valider en jeu** |

Le plus gros apport de cartes depuis l'épisode 14.1 : tout le manoir de Varmundt et ses
dépendances, soit une douzaine de cartes. C'est aussi l'épisode qui introduit les
**4ᵉ classes** — définitivement hors périmètre pour un serveur pré-renewal.

## 1. Contenu principal (30 octobre 2020)

### Ville et donjons

| Carte | Nom |
|---|---|
| `ba_maison` | **Jardin du manoir de Varmundt** |
| `ba_in01` | Intérieurs du manoir |
| `ba_pw01`, `ba_pw02`, `ba_pw03` | **1ʳᵉ et 2ᵉ centrales électriques** |
| `ba_lib` | **Bibliothèque — Couloir de la Mémoire** |
| `ba_2whs01`, `ba_2whs02` | **Réserve de Tartaros** |
| `ba_bath` | **Grands bains de Meditatio** |
| `ba_lost` | **Zoo — Vallée Perdue** |
| `ba_chess`, `ba_go` | Salles annexes |
| `ver_eju`, `ver_tunn` | Station d'épuration (rattachée à Verus) |

### Instances

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Hey! Sweety!** | `1@bamq` | `TwilightGarden.txt` |
| **Jardin du Crépuscule** (*Twilight Garden*) | `1@bamn` | `TwilightGarden.txt` |
| **Jardin d'Eau** — normal et difficile | `1@ghg` | `WaterGarden.txt` |
| **Jardin de Fleurs Caché** | `1@herbs` | `HiddenGarden.txt` |
| **Zones de Sécurité 1 et 2** | `1@herbs` | `HiddenGarden.txt` |
| **Ferme Perdue dans le Temps** | `1@lost` | `LostFarm.txt` |

## 2. Contenu de l'intervalle (2020‑2021)

### Donjons

- **Illusion of Underwater** (`iz_d04_i`, `iz_d05_i`) — cf. [16.2](16-2_terra_gloria.md)
- **Niflheim : Salle de Banquet des Morts** et **Opéra Effondré** (`nif_dun01`, `nif_dun02`)
- **Laboratoire abandonné Amicitia F1 et F2** (`amicitia1`, `amicitia2`)
- **Rudus 4F — Champ organique** (`sp_rudus4`)

### Instances

| Instance | Carte |
|---|---|
| **Tour de Thanatos** | `1@thts` |
| **Arène Nocturne de Geffen** | `1@ge_sn` |
| **Crash d'aéronef** | `1@mjo1` |
| **Tombeau du Remords** | `1@spa2` |
| **Tour des Constellations** | `1@ch_t` |

### Classes

**Les treize 4ᵉ classes** (Dragon Knight, Arch Mage, Cardinal, Meister, Shadow Cross,
Imperial Guard, Elemental Master, Inquisitor, Biolo, Abyss Chaser, Wind Hawk, Troubadour,
Trouvère). Hors périmètre.

## 3. État côté rAthena

Conversion par la PR [#6799 « Episode 17.2 — Sage's Legacy »](https://github.com/rathena/rathena/pull/6799)
(avril 2023).

`npc/re/quests/quests_17_2.txt`, `npc/re/mobs/dungeons/{ba_2whs,ba_bath,ba_lib,ba_pw,amicitia,nif_dun}.txt`,
`npc/re/instances/{TwilightGarden,WaterGarden,HiddenGarden,LostFarm}.txt`.

## 4. État côté Moonlight (relevé le 2026‑08‑25, mis à jour le 2026‑11‑11)

Les spawns du manoir sont en service depuis le premier relevé ; l'épisode 17.1 qui les
précède est porté depuis le 2026‑11‑08.

| Élément | Fichier Moonlight | État |
|---|---|---|
| Manoir de Varmundt et dépendances | `moon/rathena/dungeons/ba_dun.npc` — `ba_2whs01/02`, `ba_bath`, `ba_in01`, `ba_lib`, `ba_lost`, `ba_maison`, `ba_pw01/02/03` | ✅ chargé |
| Laboratoire Amicitia | `moon/rathena/dungeons/amicitia.npc` — `amicitia1`, `amicitia2` | ✅ chargé |
| Mobs correspondants | `db/import/mobs/ba_dun.yml`, `db/import/mobs/amicitia.yml` | ✅ |
| Niflheim renewal | `moon/rathena/dungeons/nif_dun.npc` — `nif_dun01`, `nif_dun02` | ✅ chargé le 2026‑11‑11 (lot A) |
| Jardin du Crépuscule / Hey! Sweety | `moon/instances/TwilightGarden.npc` | ✅ chargé le 2026‑11‑11 (lot B1) |
| Jardin d'Eau | `moon/instances/WaterGarden.npc` | ✅ chargé le 2026‑11‑11 (lot B1) |
| Jardin de Fleurs Caché | `moon/instances/HiddenGarden.npc` | ✅ chargé le 2026‑11‑11 (lot B1) |
| Ferme Perdue dans le Temps | `moon/instances/LostFarm.npc` | ✅ chargé le 2026‑11‑11 (lot B1) |
| Quêtes 17.2 | `moon/rathena/quests/quests_17_2.npc` | ✅ chargé le 2026‑11‑11 (lot B2) |
| Rudus 4F | `moon/rathena/dungeons/sp_rudus.npc` | ✅ chargé le 2026‑11‑11 (lot A) |
| Quêtes d'accès et journalières de Rudus 4F, Niflheim et Amicitia | `moon/rathena/quests/dungeons_200.npc` | ✅ chargé le 2026‑11‑11 (lot A), Amicitia le 2026‑11‑12 (lot A2) |

Le contrôle d'écart mob ne signale **aucun manque** sur `ba_*` ni sur `amicitia` : le
roster est complet pour ces cartes.

## 5. Migration pré-renewal

C'est l'épisode où le déséquilibre est le plus visible : les joueurs peuvent déjà chasser
dans le manoir de Varmundt, mais **aucune instance, aucune quête, aucune économie** n'y
est attachée. Deux façons de refermer l'écart :

- **voie courte** — décommenter `nif_dun`, déclarer les six entrées dans
  `db/import/instance_db.yml` (aucune n'y figure aujourd'hui) et référencer les quatre
  scripts ; `1@bamn`, `1@bamq`, `1@ghg`, `1@herbs` et `1@lost` sont toutes dans le
  mapcache. Cela donne six contenus de groupe sans écrire une ligne de script ;
- **voie longue** — porter `quests_17_2.txt` et la chaîne narrative, ce qui suppose
  d'avoir d'abord porté 17.1.

Dans les deux cas, le rééchelonnage 185‑200 → 90‑99 est le vrai travail.

## 5b. Lot A (porté le 2026‑11‑11, non validé en jeu)

Relevé du 2026‑11‑11 avant portage (`tools/util/port_illusion.py`, pointé sur les scripts du
17.2) : 4 instances (7 entrées `instance_db`, amont 53 à 60, nos Id 53 à 57 déjà pris),
29 mobs d'instance, 126 quêtes sur 139, 140 objets ; les 5 instances de l'intervalle n'ont de
script ni chez rAthena ni ailleurs.

| Partie | Contenu | Fichiers |
|---|---|---|
| Mobs | 9 de Niflheim renewal et 6 de Rudus 4F (niveau 213 à 255), par `re_to_prere_mob.py` sans retouche, `DamageTaken` retiré : environ 530 000 PV, R001-Bestia 5,1 M, Death Witch 7,4 M ; 156 lignes de skills ; `NPC_WIDEBLEEDING2` (756) ajouté au skill_db | `db/import/mobs/nif_dun.yml`, `db/import/mobs/sp_rudus.yml` |
| Objets | 11 matériaux, Etel Dust et Etel Stone, 15 cartes (paliers +11 en `getallrefine() >= 22`, `bAtkRate` de Ghost Cube en `bAddClass`) ; les conditions `BaseLevel >= 200` restent, le plafond réel est 999 | `db/import/items/` |
| Armes Fourth_* | 17 armes de 4ᵉ classe adaptées comme le stuff de 3ᵉ : `Classes` retiré, classe de base, `WeaponLevel` 4, ATK gardée, `MagicAttack` en `bMatk`, compétences par la table du 2026‑11‑11 ([skills_classes_fermees.md](skills_classes_fermees.md)) ; Relapse Plant (Doram) écartée | `db/import/items/item_db_weapon.yml` |
| Drops de carte | `db/re/map_drops.yml` des trois cartes : armes Fourth_* et Etel ; mobs normaux à l'échelle du 2026‑11‑07 (armes 1 à 2,5 %, Etel 2 %), MVP inchangés | `db/import/map_drops.yml` |
| Accès | quêtes officielles : Pion à Rudus 3 (16521) pour Rudus 4F, Milla à Niflheim (8639, 8640) pour Niflheim F1, niveau 240 pour F2 ; 28 quêtes avec les journalières ; **déblocage au compte Moonlight** : `##dun_open_sp_rudus4` et `##dun_open_nif_dun01`, posés par la quête, laissent passer les autres personnages du compte dès le niveau 200 ; pas d'entrée au warp agent pour l'instant | `moon/rathena/quests/dungeons_200.npc`, `moon/rathena/dungeons/nif_dun.npc` |
| Mapflags | `nomemo` et `nowarpto` de l'amont sur `nif_dun01` et `nif_dun02` | `moon/mapflag/etc.npc`, `nowarpto.txt` |
| Branches et albums | 13 mobs en Dead Branch, Death Witch et R001-Bestia en Bloody Branch, 13 cartes à l'album, 2 cartes MVP en `mca` et `mvpenchant` ; points MVP : R001-Bestia 6, Death Witch 8 | `db/import/mob_summon.yml`, `db/import/item_group_db/`, `moon/mobs/mvps.npc` |
| Client | tables serveur régénérées, 28 quêtes au journal, fiches des armes et cartes alignées (noms de compétences, « Refine Total Stuff +22 ») | `moonclient/dist/data/yaml_files/` |

## 5b bis. Lot A2 : Amicitia (2026‑11‑12, non validé en jeu)

Amicitia est une implantation manuelle de Moonlight (avril 2026) : spawns montés à 80 et 90
par étage (55 et 65 chez l'amont), PV et EXP rééquilibrés le 2026‑04‑20, The One dans
`moon/mobs/mvps.npc` (10 points), cartes déjà dans les albums, mobs déjà dans les branches.
Rien de cela n'est retouché.

| Ajout | Contenu |
|---|---|
| Quêtes | la partie Amicitia de `quests_dungeons_200.txt` : Lana (`ein_fild08`, accès rapide, niveau 215), Ansuriya et Gerbena (`pub_cat`, journalières à 215 et 230), les traces de Werner à Amicitia F1 ; 12 quêtes |
| Drops de carte | `db/re/map_drops.yml` d'`amicitia1` et `amicitia2` à l'échelle de Rudus 4F et Niflheim ; The One (MVP) inchangé |
| Armes | les 8 dernières armes Fourth_* (Fortified Book, Edge, Dagger, Rod, Spear, Wand, Staff, Claymore), mêmes règles que le lot A, table du 2026‑11‑12 |
| Choix | le warp vers Amicitia F2 reste libre : l'amont le ferme sous le niveau 230 |

## 5c. Lots B2 et B1 (portés le 2026‑11‑11, non validés en jeu)

La chaîne et les instances vont ensemble : deux instances sont créées depuis la chaîne, les deux
autres vérifient ses variables (`ep17_2_main`, `ep172_watergarden`) et ses quêtes. La chaîne
démarre à la fin de celle du 17.1 (`ep17_2_main` 1 et 2, déjà porté) ; les warps du manoir
(`ba_dun.npc`) suivaient déjà `ep17_2_main`, qu'aucun script ne faisait avancer au-delà de 2.

| Partie | Contenu | Fichiers |
|---|---|---|
| Chaîne | copie de l'amont, 10 609 lignes | `moon/rathena/quests/quests_17_2.npc` |
| Instances | Jardin du Crépuscule, Hey! Sweety!, Jardin d'Eau (normal et difficile), Jardin de Fleurs Caché, Zones de Sécurité 1 et 2, Ferme Perdue : `instance_db` 58 à 65 (53 à 60 chez rAthena), constantes `INST_*` régénérées ; scripts identiques à l'amont, en `.npc`. On y entre par la chaîne, pas par le warp agent | `moon/instances/{TwilightGarden,WaterGarden,HiddenGarden,LostFarm}.npc`, `db/import/instance_db.yml` |
| Mobs | 38 mobs d'instance (`MD_*`, `G_*`, `EP17_2_BETA_BASIC`, et `MD_GRAN_PAPILIA`, cible de la quête 16433), `re_to_prere_mob.py` sans retouche, `DamageTaken` retiré ; MVP de 2,2 M (Sweety) à 12,8 M (Red Pepper difficile, 1 Md en renewal) ; 193 lignes de skills, `NPC_RAINOFMETEOR` (769) ajouté au skill_db | `db/import/mobs/ep17_2_instances.yml` |
| Objets | 48 : 21 cartes, matériaux et objets de quête, 4 boissons, 2 mousses de bain (statuts `Bath_foam_a` à `_c` repris de `db/re`), 4 coiffes | `db/import/items/` |
| Conversions | paliers au-delà de +10 en `getallrefine()` (Senior Special Alnoldi, Senior Papila Cae), `bAtkRate` en `bAddClass`, compétences fermées par la table (Sweety : Rapid Shower ; Senior Broken Gardener : Counter Kick, Turn Kick ; Autumn Headband : Arrow Shower ; Black Ribbon : Spiral Pierce, Bowling Bash, délai en `bDelayrate`), DEF 10 des coiffes ramenée à 8 | |
| Écarts | baton Doram `Ep172_1h_Foxtail` (retiré des drops) ; **Epic Module Box** : son groupe tire les modules Automatic du lot B3, le tirage à 5 % du Jardin de Fleurs Caché donne 5 pierres d'Igni en attendant | |
| Quêtes | 126 quêtes (chaîne et instances) | `db/import/quest_db.yml` |
| Mapflags | les cinq cartes d'instance (`nomemo`, `noteleport`, `monster_noteleport`, `nowarpto`, `restricted 6`) et `ba_in01` (`nomemo`, `noteleport`, `nowarpto`) ; `nobranch` écarté (inversé ici) | `moon/mapflag/` |
| Client | 126 quêtes au journal, tables régénérées, fiches des objets convertis alignées | `moonclient/dist/data/yaml_files/` |

## 5d. Lot B3 : l'économie du manoir (2026‑11‑12, non validé en jeu)

`moon/rathena/merchants/enchan_sage_legacy_17_2.npc`, copie de l'amont, dans `ba_in01` :

| PNJ | Rôle | Ce qui est porté |
|---|---|---|
| Lisa | pose un module dans l'équipement Automatic : l'enchantement correspondant (orbe) prend un emplacement de carte | 95 modules (`item_db_etc`) et 95 orbes (`item_db_enchant`) ; les 3 orbes Doram et leurs modules sont écartés, et retirés des listes de Lisa |
| Yeoncheong | échange l'équipement Illusion du 16.2 contre l'équipement Automatic | 10 pièces Automatic ; les 10 pièces Illusion locales prennent leur nom renewal (même Id) pour que l'échange les trouve |
| Yecheon | vend les modules et les dispositifs d'amélioration contre des Barmil Tickets | dispositifs `Auto_Imp_*` (options aléatoires `AUTO_IMP_*` sur les armes du manoir, recettes Lapine), boîte de modules Epic (groupe `EPIC_MODULE_MIX`, `db/import/item_group_db/ep17_2.yml`) |
| Cube Lane | vend les cubes d'amélioration (Illusion et Automatic, raffinage à +7, recettes Lapine) | 2 cubes |
| Spiera | échange les pierres d'Igni contre les modules Epic | troc `barter_EP17_2_EP` |
| Child Manager | familiers Little Manager et Cloud Cotton, accessoires | 2 familiers (`pet_db`, mobs `EP17_2_CHILD_ADMIN1` et `2`), 2 accessoires, Cloud Cotton |
| Marchand de papillons | aile vers le manoir (à partir de l'étape 8 de la chaîne) | `Teleport_Ep17_02` |

Conversions, validées le 2026‑11‑12 :
- **orbes** : paliers `.@r>=11` en `getallrefine() >= 22`, `bAtkRate` en `bAddClass`, compétences
  fermées par la table (dix ajouts, [skills_classes_fermees.md](skills_classes_fermees.md)), règle
  des doublons, délais en `bDelayrate` cumulés par palier et plafonnés à -25 % ; **orbe de DEF à
  3 / 1 / 1 et de MDEF à 3 / 1 / 1**, comme l'orbe Grace du 17.1 (150 / 75 / 75 et 15 / 7 / 7 en
  renewal) ;
- **équipement Automatic** : DEF 12 pour l'armure, 6 pour la cape et les chaussures,
  `bAtkRate` en `bAddClass` ;
- l'Epic Module Box revient dans le Jardin de Fleurs Caché (5 pierres d'Igni l'avaient remplacée) ;
- Lisa a pour sprite le mob `G_ASSISTANT`, ajouté à `ep17_2_instances.yml` ;
- côté client, les fiches des orbes nomment les compétences de remplacement, et leurs délais
  s'affichent en « After Skill Delay ».

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — le terrain est déjà ouvert, il manque ce qui le rend jouable |
| Fait | lot A : Niflheim renewal et Rudus 4F ; lots B2 et B1 : la chaîne de quêtes et les quatre instances (2026‑11‑11) ; lot A2 : Amicitia et lot B3 : l'économie du manoir (2026‑11‑12) ; les cartes des instances sont dans les albums ; rien n'est validé en jeu |
| Épisode | **clos** côté portage le 2026‑11‑12 |
| Reste à faire | validation en jeu · hors périmètre : les 5 instances de l'intervalle, sans script nulle part |

## Sources

- [Episode XVII.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVII.II)
- [rathena/rathena PR #6799](https://github.com/rathena/rathena/pull/6799)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=245)
