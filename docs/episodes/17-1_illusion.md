# Épisode 17.1 — Illusion

| | |
|---|---|
| Nom kRO | 일루션 / *Illusion* |
| Date kRO | **2018‑07‑18** |
| Arc | suite de Terra Gloria — l'organisation Illusion |
| Niveau kRO visé | 175‑185 |
| État Moonlight | **porté (A, B1, B2), à valider en jeu** |

Le joueur enquête dans le donjon de **Rudus**, reprend le laboratoire **Regenschirm**,
traverse **OS** et atteint la cité de **Cor**. C'est le dernier épisode dont rAthena
fournit une conversion complète et testée.

## 1. Contenu principal (18 juillet 2018)

### Ville et donjons

| Carte | Nom |
|---|---|
| `sp_cor` | **Cor**, cité de l'organisation Illusion |
| `sp_rudus`, `sp_rudus2`, `sp_rudus3` | **Rudus**, étages 1 à 3 |
| `sp_rudus4` | Rudus 4F — *Champ organique* (ajouté en 17.2) |
| `sp_os` | Zone frontière spéciale — **OS** |

Mobs : Sanare, Plaga, Dolor, Venenum, Twin Caput — chacun en deux variantes.

### Instances

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Reconquête de Regenschirm** | `1@rgsr` | `Regenschirm.txt` |
| **OS scellée** (*Sealed OS*) | `1@os_b` | `SealedOs.txt` |
| **Occupation d'OS** | `1@os_a` | `OsOccupation.txt` |
| **2ᵉ fouille d'OS** | `1@os_a` | `OsOccupation.txt` |
| **Mémorial de Cor** (*EL1‑A17T*) | `1@cor` | `CorOperation.txt` |

## 2. Contenu de l'intervalle (2018‑2020)

### Donjons

| Contenu | Carte | Côté rAthena | Côté Moonlight |
|---|---|---|---|
| **Illusion of Labyrinth** | `prt_mz03_i` | PR #8506, non fusionnée | ✅ porté le 2026‑11‑08 |
| **Donjon de Magma F3** | `mag_dun03` | spawns, quêtes de Lily Blanc, échange des minerais | ✅ implanté en avril, complété le 2026‑11‑08 |
| **Donjon d'Einbech F3** | `ein_dun03` | spawns, quêtes de Sally, Elmen et son troc | ✅ porté le 2026‑11‑08 (spawns jusque-là commentés) |
| **Lac de l'Abysse F4** | `abyss_04` | mapflags seulement, aucun spawn | — |
| **Abyss Glast Heim** | — | rien | — |
| **Passé d'Odin** (*Odin's Past*) | — | rien | — |

### Instances

| Instance | Carte | Côté rAthena |
|---|---|---|
| **EDDA — Chute de Glast Heim** (normal et avancé) | `1@gl_prq` | entrées `instance_db` 51 et 52, **aucun script** |
| **EDDA — Laboratoire de bio-recherche** (*Bagot Laboratory*) | `1@jorlab` | entrée `instance_db` 74, **aucun script** |
| **Glast Heim : mode défi** | `1@gl_he` | entrée `instance_db` 49, **aucun script** (absent d'`OldGlastHeim.txt`) |

Toutes ces cartes sont dans le mapcache. Attention à l'Id 51 : chez Moonlight, c'est déjà la
*Bloody Branch Cave*.

### Systèmes et classes

| Contenu | Pertinent en pré-renewal ? |
|---|---|
| Plafond 185, puis **200** | non |
| Tenues alternatives des 3ᵉ classes (sprites jRO) | non |
| Refontes de compétences 3ᵉ classe #5→#8 | non |
| Refonte des compétences des classes étendues #1 | non |
| Améliorations du système de cuisine | oui |
| Équipement de montée exclusif 16.1/16.2/17.1 | à convertir |
| **Potions Sirop** (soins améliorés) | oui |

## 3. État côté rAthena

Conversion livrée par la PR [#6643 « Episode 17.1 — Illusion »](https://github.com/rathena/rathena/pull/6643)
(avril 2022), complétée depuis.

`npc/re/quests/quests_17_1.txt`, `npc/re/mobs/dungeons/sp_rudus.txt`,
`npc/re/mobs/special_border_area.txt`,
`npc/re/instances/{Regenschirm,SealedOs,OsOccupation,CorOperation}.txt`.

## 4. État côté Moonlight (mesuré le 2026‑11‑08)

**Porté le 2026‑11‑08, non validé en jeu.** Le relevé d'avant portage suit, pour mémoire.

| Lot | Contenu | Fichiers |
|---|---|---|
| **A** | Rudus 1 à 3 et OS en zones de chasse : 10 mobs `EP17_1_*` et `G_TWIN_CAPUT2`, 91 lignes de skills ; Bellare 1/2 et R48-85-Bestia (gabarits) remplacés ; 7 cartes renommées ; Rudus 4 (17.2) commenté ; warp agent « Rudus / OS » | `db/import/mobs/sp_rudus.yml`, `moon/rathena/dungeons/sp_rudus.npc`, `sp_os.npc` |
| **B1** | Chaîne de quêtes et 4 instances : 14 mobs `MD_*` (dont le gabarit `MD_E_13EN0`), 69 quêtes, `instance_db` 39 à 43, mapflags ; on entre par la chaîne, pas par le warp agent | `moon/rathena/quests/quests_17_1.npc`, `moon/instances/{Regenschirm,SealedOs,OsOccupation,CorOperation}.npc`, `db/import/mobs/ep17_1_instances.yml` |
| **B2** | Marchands et enchanteurs de Cor : 40 armures Grace, 26 orbes, 11 coffrets Lapine, groupes `EP17_1_SPC01`→`04`, 18 boutiques de troc ; l'opérateur Grace aiguille sur la **classe 2** (Chevalier → boutique Rune Knight, etc.) ; 51 objets renommés (armes OS, modules) | `moon/rathena/merchants/enchan_illusion_17_1.npc`, `moon/barters.yml` |

Écarts : le stuff Doram (Grace Carrot et Picky Robe, ElectricFox OS, boutique Summoner) ; le
paquet de forge `Bs_Making_S` ; le **système Reform** (`rgsr_in.txt`, mécanique renewal de 2022).
Trois compétences ont été ajoutées à la table des classes fermées, validées le 2026‑11‑08
([skills_classes_fermees.md](skills_classes_fermees.md)).

Relevé du 2026‑11‑08 avant portage :

| Élément | Fichier Moonlight | État |
|---|---|---|
| Spawns de Rudus | `moon/rathena/dungeons/sp_rudus.npc`, `sp_rudus`→`sp_rudus4` | **commenté** (`scripts_moon.conf:810`) |
| Spawns d'OS | — | **absent** ; l'amont tient en 12 lignes (`npc/re/mobs/special_border_area.txt`) |
| Spawns de Cor | `moon/rathena/dungeons/ba_dun.npc`, 1 ligne `sp_cor` | ✅ chargé |
| Regenschirm | `moon/instances/Regenschirm.txt` | non référencé |
| OS scellée | `moon/instances/SealedOs.txt` | non référencé |
| Occupation d'OS | `moon/instances/OsOccupation.txt` | non référencé |
| Mémorial de Cor | `moon/instances/CorOperation.txt` | non référencé |
| Entrées d'instance (Id 39 à 43) | `db/import/instance_db.yml` | **absentes** ; les Id sont libres |
| Quêtes 17.1 | `npc/re/quests/quests_17_1.txt`, 8 962 lignes | **non porté** ; 69 quêtes sur 88 absentes, toutes en `db/re` |
| Enchanteur, échanges | `npc/re/merchants/enchan_illusion_17_1.txt` (618 lignes), `rgsr_in.txt` (451), `barters/quests_17_1.yml` (1 300) | **non porté** |
| **Illusion of Labyrinth** | `moon/rathena/quests/illusion/labyrinth.npc` | branché le 2026‑11‑08, non validé en jeu |

### Les objets attendus

Les scripts amont (quêtes, instances, enchanteur, échanges) et les drops des mobs citent
**157 objets** : 87 sont chargés, dont 37 sous un autre nom (à renommer vers le nom renewal),
et **70 manquent** :

| Type | Absents |
|---|---:|
| Armures `Grace_*` | **42** |
| Coffrets (`DelayConsume`) | 20 |
| Matériaux | 3 |
| Cartes | 2 |
| Consommables | 2 |
| Cash | 1 |

Les `Grace_*` sont le gros morceau : leurs bonus portent sur des compétences de 3ᵉ classe,
donc chacun passe par la table des classes fermées (`skills_classes_fermees.md`).

### Illusion of Labyrinth (porté le 2026‑11‑08)

rAthena ne l'a pas fusionné : il vient de la PR
[#8506 « Initial release of Illusion of Labyrinth »](https://github.com/rathena/rathena/pull/8506),
ouverte et approuvée deux fois. Ses 12 fichiers sont appliqués à `db/re/` et `npc/re/` (tête
`be8d3e56f`), puis portés comme les donjons de [16.2](16-2_terra_gloria.md) :

| Élément | Moonlight |
|---|---|
| Quête (Irene, Esmeralda, Andrea, Anes, Silvano, Cecilia ; `ill_laby`) | `moon/rathena/quests/illusion/labyrinth.npc`, entrée par la *Twisted Crack* de `prt_maze01`, niveau 170 |
| Enquête : Terrian, quête 3465 | `investigation.npc`, version de la PR (Terrian déplacé dans `prt_mz03_i`) |
| Échangeuse Esmeralda, troc `barter_ill_labyrinth` | `enchan_illusion.npc`, `moon/barters.yml` |
| Warps (58), spawns, apparition du MVP | `moon/rathena/dungeons/prt_maze.npc` |
| 14 mobs, 155 lignes de skills, `NPC_GRADUAL_GRAVITY` | `db/import/mobs/illusion.yml`, `mob_skill_db.txt`, `skill_db.yml` |
| 13 matériaux, 2 coffrets Lapine, 24 quêtes | `item_db_etc.yml`, `item_db_usable.yml`, `laphine_upgrade.yml`, `quest_db.yml` |
| `F_queststatus` | ajoutée à `moon/rathena/other/Global_Functions.txt` (absente du pré-renewal) |
| Drops de carte | `db/import/map_drops.yml`, à l'échelle de 16.2 |

Décisions et écarts :

- **MVP.** Chaos Baphomet (`ILL_BAPHOMET`) apparaît quand les quatre sous-boss, qui
  reviennent toutes les heures, sont morts ; 2 h de repos après sa mort. La PR oublie son
  `Modes: Mvp` alors qu'il a des `MvpDrops` : il est ajouté. Il entre au carnet (case
  `prt_mz03_i` dans `mvp_tracker.cpp`, recompilation du map-server), à 8 points dans `$mvps`,
  et ses labels valident les missions de chasse.
- **Cartes.** Les dix cartes `Chaotic_*` locales prennent leur nom renewal (`Chaos_*`, même Id),
  combos compris ; leurs scripts locaux ne changent pas.
- **Accès.** Le warp agent ouvre `prt_mz03_i` dès `ill_laby >= 2`, débloqué au compte Moonlight
  par `##ill_open_prt_mz03_i`, sinon il mène à Irene.
- **Branches et albums.** Chaos Baphomet en Bloody Branch, les 12 autres mobs en Dead Branch
  (l'invocation `G_ILL_BAPHOMET_J` exclue). 9 cartes en `CARDALBUM`, celle du MVP en
  `MAGICCARDALBUM` et `MVPCARDENCHANT`.
- **Mapflags.** `nomemo` et `nowarpto`, comme les autres cartes Illusion.

### Les mobs manquants — la liste complète

Treize mobs `EP17_1_*` en `db/re` ; `BELLARE1`, `BELLARE2` et `R4885_BESTIA` sont déjà
chargés. Restent dix identifiants, toujours absents le 2026‑11‑08 :

| Id | AegisName | Cartes |
|---|---|---|
| 20357 | `EP17_1_SANARE1` | `sp_os` |
| 20358 | `EP17_1_SANARE2` | `sp_rudus*` |
| 20359 | `EP17_1_PLAGA1` | `sp_rudus*` |
| 20360 | `EP17_1_PLAGA2` | `sp_rudus*` |
| 20361 | `EP17_1_DOLOR1` | `sp_rudus*` |
| 20362 | `EP17_1_DOLOR2` | `sp_rudus*` |
| 20363 | `EP17_1_VENENUM1` | `sp_rudus*`, `sp_os` |
| 20364 | `EP17_1_VENENUM2` | `sp_rudus*` |
| 20365 | `EP17_1_TWIN_CAPUT1` | `sp_rudus*` |
| 20366 | `EP17_1_TWIN_CAPUT2` | `sp_rudus*` |

Au 2026‑08‑25, c'étaient **les seuls mobs manquants de tout le périmètre 13.3 → 17.2** : le
contrôle d'écart entre les spawns renewal de rAthena et le roster de Moonlight (2 117 entrées)
ne signalait rien d'autre, hormis `sp_os`. Ce contrôle ne couvre pas les mobs posés par les
scripts d'instance.

Note : `sp_os` n'a **aucun** fichier de spawn côté Moonlight ; il faudra reprendre
`npc/re/mobs/special_border_area.txt` de rAthena.

Toutes les cartes concernées (`sp_cor`, `sp_os`, `sp_rudus`→`sp_rudus4`, `1@rgsr`,
`1@os_a`, `1@os_b`, `1@cor`) **sont** dans le mapcache (vérifié le 2026‑11‑08).

## 5. Migration pré-renewal

Mêmes règles que pour 16.2 (checklist de portage, `tools/util/port_illusion.py` comme modèle
d'inventaire). Deux niveaux possibles.

**A. Zones de chasse seules** — court :

1. verser les **10 mobs `EP17_1_*`** dans `db/import/mobs/` (un fichier `sp_rudus.yml`), par
   `tools/util/re_to_prere_mob.py` **sans retouche** : l'ATK inchangée et l'EXP dérivée des HP
   sont voulues (le rééchelonnement pour du niveau 90‑99 proposé ici le 2026‑08‑25 est
   abandonné) ; leurs lignes de `mob_skill_db` ;
2. décommenter `scripts_moon.conf:810` et reprendre les 12 lignes de spawn de `sp_os` ;
3. drops de carte (`db/re/map_drops.yml` : `sp_rudus4` relève de 17.2), branches Dead et
   Bloody, albums de cartes, mapflags, warp agent.

**B. Portage complet** — de l'ordre de 16.2 :

4. les **cinq entrées d'instance** (Id 39 à 43) dans `db/import/instance_db.yml`, les quatre
   `.txt` d'instance renommés en `.npc` et référencés, avec leurs mapflags (§10b) ;
5. `quests_17_1.txt` et ses **69 quêtes** : c'est le fil narratif, il suppose 16.2 achevé côté
   joueur ;
6. l'économie : enchanteur, `rgsr_in.txt`, échanges, coffrets ;
7. les **70 objets** absents, dont les **42 `Grace_*`** à convertir un par un (paliers au-dessus
   de +10, compétences de classes fermées), et les 37 renommages ;
8. carnet MVP, missions de chasse, client (toutes les tables, fiches `item_info`).

17.2 et 18 ne sont pas portés : la chaîne narrative s'arrête après 17.1. Le niveau A reste une
option défendable et bien moins coûteuse.

Le contenu de l'intervalle (Chute de Glast Heim, Laboratoire Bagot, mode défi, Lac de
l'Abysse F4, Passé d'Odin, Abyss Glast Heim) n'a aucun script dans rAthena : hors périmètre
tant qu'il faudrait l'écrire.

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — trois zones de chasse et cinq instances fournies par rAthena |
| Fait | Illusion of Labyrinth, Magma F3, Einbech F3, A, B1, B2 (2026‑11‑08, non validés en jeu) |
| Reste à faire | validation en jeu · décision : système Reform |
| Décidé | Magma F3 et Einbech F3 restent sans mapflags `nomemo`/`nowarpto`, accès libre ; les quêtes de Lily (175) et de Sally (180) gardent le seuil de l'amont ; les trois ajouts à la table des compétences sont validés (2026‑11‑08) |

## Sources

- [Episode XVII.I — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVII.I)
- [Episode 17.1: Illusion — ROGGH Library](https://roggh.com/episode-17-1-main-quest/)
- [Rudus — iRO Wiki](https://irowiki.org/wiki/Rudus)
- [rathena/rathena PR #6643](https://github.com/rathena/rathena/pull/6643)
- [Annonce kRO d'origine](http://ro.gnjoy.com/news/update/View.asp?seq=224)
