# Épisode 21 — L'Âge des héros (*Age of Heroes*)

| | |
|---|---|
| Nom kRO | *Age of Heroes* (영웅의 시대), extension **Yorscalp** |
| Date kRO | **2024‑04‑03** (presse coréenne ; aucune annonce kRO retrouvée, année déduite) |
| Arc | dans le passé, avant Rune-Midgard : la Résistance Gaebolg face au Serpent et à Tan |
| Niveau kRO visé | **230+**, MVP de terrain au niveau 265 |
| État Moonlight | **rien de porté** (relevé du 2026‑11‑22) ; cartes déclarées et dans les caches |

Suite directe du 20 : après la quête 18233 de *The Undying*, Diver Fapa (`jor_tail`
233,41) envoie le joueur dans le passé. Lunaforma, la base de la Résistance Gaebolg,
sert de ville ; Alberta, village de pêcheurs, de port.

## 1. Contenu

### Cartes

| Carte | Nom |
|---|---|
| `jor_mbase`, `mbase_in` | **Lunaforma** et son intérieur (base de la Résistance Gaebolg) |
| `jor_albe`, `jalbe_in` | **Alberta, village de pêcheurs**, et son intérieur |
| `jor_base` | Jornville |
| `jor_raise1`, `jor_raise2` | Rising Land nord et sud |
| `jor_crk`, `jor_crk_p` | Scale Island |
| `jor_tmple1`, `jor_tmple2` | Église de Jormungand, 1er et 2e étages |
| `luna_sf1`, `luna_sf2` | abords de Lunaforma (entrée de Final Battle sur `luna_sf2`) |
| `1@mdtem`, `2@mdtem`, `1@wtgs`, `1@ep21a`, `1@ep21b`, `1@ep21c`, `1@twbs2` | instances |

`jor_sklf1` et `jor_sklf2`, cités par une source web, sont déclarés un peu plus bas dans
`maps_athena.conf` (hors du bloc « Episode 21 ») : leur rôle dans l'épisode reste à vérifier.

### Instances

| Instance | Carte | Nature |
|---|---|---|
| **Gimli Infiltration** | `1@mdtem`, `2@mdtem` | infiltration, la plus longue (≈ 2 400 lignes chez brHades) |
| **Mysterious Ghost Ship** | `1@wtgs` | navire fantôme, aussi en quotidienne ; boss : le capitaine |
| **Secret Altar** | `1@ep21a` | boss : Lasgand |
| **Silent Sanctuary** | `1@twbs2` | courte |
| **Final Battle** | `1@ep21b`, `1@ep21c` | Facile, Normale, Difficile ; Tan possédé puis dévorant, et le Serpent |

### Mécaniques

| Élément | Détail |
|---|---|
| MVP de terrain | Ice Horn (22362, `jor_raise2`), Yortus Arbiter (22363, `jor_tmple1`), Yortus Bailiff (22364, `jor_tmple2`), niveau 265, réapparition d'une heure |
| Équipement Gaebolg | 10 pièces niveau 245 (armure/robe, bottes/souliers, manteau/cache-col, anneau/gant/boucle/collier), payées en bons Wigner ; marteaux de raffinage +9 → +12 liés à la réputation |
| Yorscalp | 12 pièces (les mêmes emplacements et deux diadèmes), transformation chez le forgeron Sansan (`Yorscalp_Scroll_A`) |
| Enchantements | mage Lobelia : orbes de forêt `Ep21_*` (POW, SPL, RES et autres stats renewal) payées en pierres magiques |
| Réputation | 7 familles, plafond 1000 ; quotidiennes de Mandel |
| Familier | Ice Seahorse (`EP21_ICESEAHORSE`, bonus RES/MRES) |
| Statut | `NPC_GROGGY_ON` / `SC_GROGGY_ON` (étourdissement des boss de Final Battle) |
| Cartes | 29 (300505 `Sea_Wind_Card` à 300533 `Encroached_Tan_Card`) |

## 2. Sources

### rAthena amont (master `dd073ca47`, 2026‑11‑15)

Aucune PR ni issue sur l'épisode. Seuls liens : [#9524](https://github.com/rathena/rathena/issues/9524)
(`NPC_LOCKON_LASER` de Tan dévorant), [#9631](https://github.com/rathena/rathena/pull/9631)
(groupes du navire fantôme et des antiquités de Tan), #10111 (clés de raffinage du cache-col Gaebolg).

| Présent dans `db/re` | Absent |
|---|---|
| 62 objets `Ep21_*` dans `item_db_etc` (orbes, pierres, cartes), 1 dans `item_db_usable` | les mobs : 59 entrées **commentées**, Id et AegisName seuls, sans stats (les mêmes 63 noms en commentaire dans notre `hall_of_life.yml`) |
| les 22 pièces Gaebolg et Yorscalp | `mob_skill_db` : aucune ligne |
| enchantements **143 à 146** (Gaebolg) et **152 à 155** (Yorscalp) | `instance_db`, `quest_db` : aucune entrée |
| 4 réformes : `Gaebolg_A_Hammer_1` à `_3`, `Yorscalp_Scroll_A` | `npc/re` : aucun script |
| groupes `EP21_COSTUME_BOX1/2`, `YOR_CARD_P_BOX`, 30 `RT_EP21_*` (tablettes de runes) | `IG_YOR_CARD`, `IG_TAN_BOX`, `SC_GROGGY_ON` |
| sprites `JT_4_EP21_*`, `EFST_GROGGY`, les 20 cartes dans `map_index` / `maps_athena` | familier : `EP21_ICESEAHORSE` commenté |

### hayakawakaki — l'original

`hayakawakaki/rathena-script-collection`, dossier `Episode 21/`, commit `04b95b29e`
(2026‑06‑23), signé **crazyarashi**. La source la plus complète, avec les dialogues
officiels :

| Élément | Contenu |
|---|---|
| Quêtes | `21_Heroes_Age.c` (681 Ko) |
| Instances | `Gimli_Infiltration.c` (109 Ko), `Mysterious_Ghostship.c` (77 Ko), `Final_Battle.c` (63 Ko), `Secret_Altar.c` (32 Ko), `Silent_Sanctuary.c` |
| Terrain, économie | `Mapflags.c`, `Shops.yml`, `YORSCALP_SHOP.yml`, `21_MAP_DROP` (7 cartes) |
| Données | 50 mobs, `mob_skill_db` (27 Ko), 24 objets, familier, `21_INSTANCE` (Id 147 à 152, **151 en double**), `21_QUEST`, `21_REPUTATION` (Id 13, « Gaebolg Resistance »), `21_SKILL` (`NPC_GROGGY_ON`, 792), `21_STATUS` |
| Yorscalp | `DB_Yorscalp/` : objets, groupes, quêtes, réforme, drops, mobs et compétences |
| C++ | `ItemGroup.patch` : `IG_YOR_CARD`, `IG_TAN_BOX` |

Dépend de `Function.c`, déjà chez nous depuis l'EDDA
(`moon/rathena/other/hayakawakaki_functions.npc`). Licence : GPL-3.0 affichée, mais le
README dit « don't redistribute » — même réserve que pour l'EDDA.

### brHades — copie de hayakawakaki

[glitzhXEC/brHades](https://github.com/glitzhXEC/brHades) (GPL-3.0) : `a9ebedab8`
« initial release ep21 » (2025‑12‑09), puis `eae85d074`, `c0b848dca`, `c60d2bdf7`,
`d3d23fd5f`, `7ae526852` (+26 201 lignes), `316c33f91` et `a5f8240f7` (2026‑03‑24).

- `21_Heroes_Age.txt` est celui de hayakawakaki à ≈ 200 lignes près, **crédit de
  crazyarashi retiré** : attribution douteuse, à ne pas prendre comme source.
- Les premières versions de Final Battle et du navire fantôme (signées Capuche et
  AoShinHo) sont des scripts simplifiés, aux dialogues traduits automatiquement.
- Mobs : 39 puis 64 entrées, stats estimées (Seawind niveau 236, 28,8 M PV).
- Cartes renommées (`Seawind_Card` au lieu de l'amont `Sea_Wind_Card`) ; deux combos
  cassés en doublons, en partie réparés par `c60d2bdf7`.
- C++ : `IG_YOR_CARD`, `IG_TAN_BOX`, `SC_GROGGY_ON` (identiques au patch de hayakawakaki).
- `instance_db` aux Id 100 à 106 ; ≈ 212 quêtes.

Copies sans apport : AoShinRO/brHades, WillianErthal/brHades4th-version,
sammeepay/serverTHClassic22.

### patnawa/rathena_pn — reconstruction libre

`npc/custom/episode21/` (2026‑09‑03 au 09‑13), GPL-3.0-or-later, chaque fichier en
« clean-room reconstruction » avec ses sources citées (le même dépôt nous avait servi pour
une première version du Crash d'aéronef).
14 fichiers (≈ 150 Ko) : progression, mobs de terrain, les 5 instances (Final Battle en
trois difficultés), Black Haired Beast, services d'équipement, réputation et
bénédictions des familles, quotidiennes, raccordement de la navigation, barters ; 64 mobs,
`mob_skill`, `instance_db`, groupe de réputation. Plus compact : les systèmes et les
verrous, pas les dialogues officiels. Redistribution sans réserve.

### Web

- [Episode 21: Age of Heroes — MuhRO wiki](https://wiki.muhro.eu/21_Age_of_Heroes)
- [Yorscalp — Rate My Ragnarok](https://ratemyragnarok.com/episodes/21/yorscalp/)
- [Episode 21 — Rate My Ragnarok](https://ratemyragnarok.com/episodes/21/) (contredit les autres, peu fiable)
- [Article coréen de la sortie](https://m.hungryapp.co.kr/news_en/news_view.php?bcode=news&pid=121484)

## 3. État côté Moonlight (mesuré le 2026‑11‑22)

| Élément | État |
|---|---|
| Cartes | les 20 dans `conf/maps_athena.conf` et dans `db/pre-re/map_cache.dat` / `db/import/map_cache.dat` |
| Client | ROenglishRE a 108 objets EP21 dans `ItemDBNameTbl.lub`, les musiques des cartes et les intérieurs ; présence des `.gat`/`.rsw` dans nos GRF **non vérifiée** |
| Objets, mobs, quêtes, instances, scripts | aucun ; `hall_of_life.yml` ne garde que les 63 noms commentés de l'amont |
| Déjà en place | `EP17_2_EP21` (objet de transition du 17.2), `Function.c` de hayakawakaki |

## 4. Points d'attention pour un portage

- **Tout est renewal** : niveaux 230 à 265, RES/MRES, POW/SPL dans les orbes, grades,
  raffinage à +12. Même travail de conversion qu'au 19 et au 20 (`re_to_prere_mob.py`,
  règles des orbes et des paliers), mais sans conversion de référence propre.
- **Niveau d'accès** à trancher : l'EDDA est à 170, le Hall of Life à 250.
- **Raffinage +12** : nos reforms Dim Glacier s'arrêtent à +10 ; les marteaux Gaebolg
  (+9 → +12) sont à ramener.
- **Id** : `instance_db` de brHades (100 à 106) et de hayakawakaki (147 à 152) à reloger
  dans nos plages libres ; constante `REPU_*` (redémarrage complet).
- **Noms de cartes** : garder ceux de l'amont (`Sea_Wind_Card`), pas ceux de brHades.
- **C++** : `IG_YOR_CARD`, `IG_TAN_BOX`, `SC_GROGGY_ON`, `NPC_GROGGY_ON` ;
  `git diff upstream/master` avant chaque patch.
- **Choix de source** : patnawa pour la licence nette, hayakawakaki pour les dialogues
  et les mobs (réserve « don't redistribute », comme pour l'EDDA). brHades écarté.

## 5. Verdict

| | |
|---|---|
| Intérêt | **élevé** — suite directe du 20, nouvelle ville, cinq instances, trois MVP |
| Coût | **le plus lourd depuis le 17.2** : aucun mob jouable ni script en amont, tout à rééquilibrer |
| Non vérifié | stats des mobs de brHades et de patnawa face à kRO, changement éventuel du plafond de niveau, fichiers des cartes dans nos GRF, rôle de `jor_sklf1` / `jor_sklf2` |
