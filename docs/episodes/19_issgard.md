# Épisode 19 — Issgard, Terre des Fleurs de Neige

| | |
|---|---|
| Nom kRO | 이스가드 / *Issgard, Land of Snow Flowers* |
| Date kRO | **2022‑01‑19** |
| Arc | poursuite de Bagot — le culte de Jörmungandr |
| Niveau kRO visé | **200+** |
| État Moonlight | **mobs, terrain, chaîne de quêtes, économie, instances et enchantement Snowflower portés** (2026‑11‑15, non validés en jeu) ; enchantement Glacier et compétences à faire |

Premier épisode que l'amont n'a pas fusionné. Une conversion complète attend pourtant
dans la PR rAthena [#8527](https://github.com/rathena/rathena/pull/8527), jamais fusionnée :
le chantier redevient un **portage**, sauf pour les compétences des mobs.

## 1. Contenu principal (19 janvier 2022)

### Ville, champs et donjons

| Carte | Nom |
|---|---|
| `icecastle`, `icas_in` | **Château de Glace d'Issgard** |
| `jor_tail` | Queue Gelée |
| `jor_back1`, `jor_back2`, `jor_back3` | Collines, Plaines et Glaciers des Écailles Gelées |
| `jor_nest`, `jor_dun02`, `jor_dun03` | Nid des Rgan |
| `jor_ab01`, `jor_ab02` | Fosse Abandonnée |

### Instances

| Instance | Carte |
|---|---|
| **Patrouille Iwin** | `1@iwp` |
| **Destruction de l'aéronef** | `1@whl` |
| **Bataille de simulation** | `1@jorlab` |
| **Nid du Serpent Confus** | `1@jorchs` |
| **Laboratoire de Bagot** | `1@jorlab` |

### Mécaniques

- **Parchemins de transformation Rgan** — le joueur se déguise pour infiltrer la société
  Rgan ; toute la progression du scénario en dépend.
- Monnaie : **Pétale de Fleur de Neige**, servant aussi à la réputation du Château de Glace.

## 2. Contenu de l'intervalle (2022)

| Date kRO | Contenu |
|---|---|
| 2022‑09‑26 | **Herosria** — nouvel environnement de siège, centré sur les 4ᵉ classes, le dimanche de 20 h à 22 h |
| 2022‑12‑21 | Plafond de niveau porté à **260** |
| — | **Livres VR** (*Virtual Reading Books*) — mini-instances de lore donnant de l'EXP |
| — | Refontes de compétences de 4ᵉ classe #2 et #3, avec nouvelles compétences pour Dragon Knight, Meister et Biolo |
| — | Donjons : Plaines Tordues du Pouvoir, Grotte Souterraine de Mjolnir |

## 3. État côté rAthena

**Branche principale : aucun script.** `npc/re/` ne contient rien pour `jor_*`, `icas_*` ou
les instances `1@jorchs`, `1@iwp`, `1@whl`. Ce qu'elle possède quand même :

- les **entrées d'instance** dans `db/re/instance_db.yml` (Id 74 à 78) ;
- les **objets** de l'épisode (PR #6563, février 2022) et les **constantes** ;
- **40 mobs `EP19_*`** dans `db/re/mob_db.yml`, mais **commentés** (Id et AegisName seuls).

**PR [#8527](https://github.com/rathena/rathena/pull/8527)** (*Initial release of episode 19 -
Walkthrough Conversion*, Atemo, ouverte depuis le 2024‑07‑30, mise à jour le 2026‑02‑25,
+25 566 lignes, 35 fichiers, renewal). Étudiée le 2026‑11‑15 :

| Contenu | Détail |
|---|---|
| Quêtes | `quests_19.txt` (17 351 lignes, 766 PNJ), 214 quêtes nouvelles |
| Instances | Destruction de l'aéronef, Laboratoire de Bagot, Nid du Serpent Confus, Patrouille Iwin. la Bataille de simulation (`1@jorlab`) est dans le script du laboratoire |
| Terrain | spawns `jor_back`, `jor_dun`, `jor_ab` ; warps `icecastle`, `jor_fild`, `jor_dun` |
| Économie | `episode_19.txt`, barters, groupe `D_GW_EXTRACTOR`, Lapine |
| `mob_db` | les 40 mobs avec vitesses, IA, modes et classe ; PNJ et pierres de stockage en coquilles, trois mobs à stats nulles |
| C++ (≈ 60 lignes) | mapflags `nolootnormalmob` et `no_npc_selfdestruction_on_all`, commande `unitisforcewalk`, statut `SC_MTP_W_POTION_100`, groupe `IG_D_GW_EXTRACTOR`, compétence visuelle `DE_BERSERKAIZER`, correctif `SCB_RANGE` |

Ses défauts : **aucun `mob_skill_db`** (signalé deux fois en commentaire), une erreur YAML dans
sa `quest_db` (`Title: Standby: Neutralizing Aircraft` sans guillemets), quelques octets de
coréen mal encodés, et des TODO ouverts (`D_GW_EXTRACTOR`, `DE_BERSERKAIZER`, mapflags et
commandes maison).

Le **déguisement Rgan** n'est pas une mécanique à écrire : la PR utilise `transform 21530,…`,
commande déjà présente sur Moonlight.

### Divine Pride

Les fiches de monstres de [Divine Pride](https://www.divine-pride.net) (API, serveur kROM)
donnent stats, drops et spawns. Elles concordent avec la PR sur 37 mobs sur 40 (stats et
drops identiques, ATK à un point près). Elles ne publient **ni IA ni compétences** pour ces
mobs, alors qu'elles en ont pour d'autres (13 pour Schulang, comme `db/re`).

Outils : `tools/util/fetch_divinepride_mobs.py` (clé dans `~/.divine_pride_key`, une requête
toutes les 1,5 s, arrêt net sur 429, cache hors git) et `tools/util/divinepride_to_mobdb.py`.

**Spawns : les deux sources divergent.** Divine Pride donne plus de mobs (par exemple 70/70/60/60/10
sur `jor_ab01` contre 65/65/35/35/10), des spawns sur `jor_tail` et `jor_back4` à `jor_back6`
absents de la PR, et 6 h de respawn pour Ultra Limacina contre 2 h. **Divine Pride retenu**
le 2026‑11‑15.

## 4. État côté Moonlight

**Les cartes sont là** : `icecastle`, `icas_in`, `icas_in2`, `jor_tail`, `jor_back1` à
`jor_back5`, `jor_nest`, `jor_dun01` à `jor_dun03`, `jor_ab01`, `jor_ab02`, `jor_que` et les
quatre cartes d'instance `1@iwp`, `1@whl`, `1@jorlab`, `1@jorchs`, dans
`db/import/map_cache.dat` et `conf/maps_athena.conf`. Le **système de grades**, de la même
période, est porté (commit `20cf36893`, armes seulement).

### Mobs (importés le 2026‑11‑15, non validés en jeu)

| Élément | Détail |
|---|---|
| Fichier | `db/import/mobs/ep19.yml`, dans le Footer de `db/import/mob_db.yml` |
| Base | la PR #8527 : vitesses, IA, modes, classe |
| Compléments Divine Pride | stats des trois mobs à 0 (Aquila, Aquila B, Rgan ES), drops normaux d'Aquila, pierres de stockage entières, noms anglais |
| Conversion | `re_to_prere_mob.py` sans retouche, ATK inchangée, `DamageTaken` et `Resistance` retirés |
| PNJ `EP19_NPC_*` | sept coquilles (Id, AegisName, Name) : ce ne sont que des sprites de PNJ, l'entrée sert à déclarer la constante |
| Pierres de stockage | `Ai: 06` (immobile, n'attaque pas, comme `TREASURE_BOX*`), `Class: Boss`, 1 PV |
| Ordres de grandeur | mob normal ≈ 500 000 à 600 000 PV ; MVP 4,6 à 8,3 M ; ATK 4 700 à 16 300 |
| Matériaux | 22 objets EP19 ajoutés à `db/import/items/item_db_etc.yml` |
| Drops de carte | **écartés** : les 24 cartes EP19 ont des bonus renewal (`bPAtk`, `bSMatk`, `bRes`, paliers +11/+12, `getequiparmorlv()`) à convertir |

**Noms.** Seize mobs n'ont pas de nom anglais (kROM seulement). Onze se déduisent des mobs frères
ou des cartes ; les grades Iwin sont traduits : Trainee, Private, Private First Class, Corporal,
Captain (le PNJ `Captain#iws` de la PR porte le sprite `_SR`). iRO nomme tous les faux Iwin de
l'épisode 20 « Fake Iwin Soldier ».

### Terrain (porté le 2026‑11‑15, non validé en jeu)

| Élément | Détail |
|---|---|
| Spawns | `moon/mobs/issgard.npc` : 36 lignes de Divine Pride sur `jor_tail`, `jor_back1` à `jor_back6`, `jor_dun01`, `jor_dun02`, `jor_ab01`, `jor_ab02` |
| MVP | Ultra Limacina sur `jor_back3` dans `moon/mobs/mvps.npc`, 6 h de respawn, 6 points de classement |
| Warps de ville | `moon/rathena/warps/cities/icecastle.npc` (PR) : maisons, caserne et salles du château ; la maison 1 reste fermée tant que `ep19_side_saint` < 18 |
| Warps de field et de donjon | fin de `moon/mobs/issgard.npc` (PR), sans le doublon `#to_jor_back2_D` |
| Warp agent | Donjons > Issgard, sous verrou de quête au compte (voir la chaîne ci-dessous), et la navigation de ses douze cartes |
| Noms de carte | 16 noms dans `db/import/map_index.yml`, repris de `map_names.yml` de moonclient |
| Mapflags | **aucun** : ni l'amont ni la PR n'en posent sur ces cartes |

Les passages `jor_dun01` → `jor_dun02`, `jor_back2` → `jor_ab01` et `jor_ab01` → `jor_ab02`
sont des scripts de la chaîne (ci-dessous). Les spawns de `jor_que`, carte de la quête des
Juncea, restent à faire.

`jor_back4` à `jor_back6` sont des cartes de l'épisode 20 : Divine Pride y place des mobs de
l'épisode 19, aucun warp n'y mène encore, le warp agent si.

### Chaîne de quêtes (portée le 2026‑11‑15, non validée en jeu)

| Élément | Détail |
|---|---|
| Script | `moon/rathena/quests/quests_19.npc`, copie de `quests_19.txt` de la PR (17 000 lignes, 500 PNJ) |
| Quêtes | 211 dans `db/import/quest_db.yml` : 208 nouvelles, et les 18119 à 18121 de la PR renumérotées **18145 à 18147** (script compris), Moscovia gardant Gusli et Shafka. 7894, 7898 et 12490 : la PR n'en change que le titre, la version locale reste |
| YAML de la PR | les titres à deux-points nus (« Standby: … ») mis entre guillemets |
| Objets | 11 objets de quête Ep19 (`db/re`) ; `Ep19_Rgan_Scroll` dans la version de la PR (`transform 21530`), **sans** `SC_MTP_W_POTION_100`, que la PR n'ajoute qu'à l'enum ; `Brown_Rat_Tail` existait déjà sous ce nom |
| Barter | `barter_Ep19_Rgan_Scroll` dans `moon/barters.yml` |
| Réputation | Isgard (Id 4, groupe 3) et `REPUTATION_EP19` dans `db/import` |
| Fonction | `F_queststatus_between` dans `Global_Functions.txt` |
| Texte | ponctuation cp949 convertie en cp1252 (`·`, espace, `*`) : moonclient lit le cp1252 |

**Déblocage au compte Moonlight**, comme l'épisode 18, dès le niveau 200 (celui du début de la
chaîne) :

| Drapeau | Posé par | Ouvre |
|---|---|---|
| `##dun_open_issgard` | l'étape 7, l'arrivée par l'aéronef | l'aéronef de Ginger (manoir de Varmundt), et au warp agent le château, les fields et les donjons du serpent |
| `##dun_open_jor_ab01` | `ep19_side_sea` 11 | le passage de `jor_back2` vers la Fosse F1 |
| `##dun_open_jor_ab02` | `ep19_side_sea` au-delà de 15 | le passage vers la Fosse F2 |

Le warp agent pose aussi ces drapeaux à qui a passé ces étapes, et envoie les autres au villageois
loup gris de `wolfvill` (début de la chaîne, `ep18_main` 57), ou à Lehar pour la Fosse.

### Économie (portée le 2026‑11‑15, non validée en jeu)

| Élément | Détail |
|---|---|
| Armes Glacier | 25 dans `item_db_weapon.yml` : WeaponLevel 4, `MagicAttack` en `bonus bMatk`, `Classes` retiré, Rebellion → Gunslinger, KagerouOboro → Ninja. **Glacier_Foxtail** (Doram) écartée, et retirée de la boutique |
| Snowflower | 10 pièces dans `item_db_armor.yml` : DEF 12 (armure, robe) et 6 (capes, chaussures), ArmorLevel 1, `[1]`, `bAtkRate` en `bAddClass`. Les traits (`bPAtk`, `bSMatk`, `bPow`, `bSpl`) restent, comme à l'épisode 18. Les versions `_JP` ne sont pas portées |
| Consommables | cubes +9 Snowflower et Glacier (`laphine_upgrade`, 6 et 25 cibles), aile de transport du château, Ice Gangu séché. Les deux ailes de mouche de la boutique existaient sous d'autres noms (`N_Fly_Wing_`, `Compressed_Wing_Of_Fly`) |
| Marchands | Hoyoyo (pierres de fleur de neige, réputation) et Maram (fournitures, Snowflower, armes Glacier) dans `quests_19.npc` ; quatre barters dans `moon/barters.yml` |
| Prix des armes | **200 000 z** et non 150 000 : la formule de prix de Moonlight (`itemdb.cpp`, armes de niveau 4) les revend 80 000 + 100 par ATK, jusqu'à 148 800 z avec Surcharge, au-dessus des 112 500 z d'un achat avec Remise. Un `Buy:` dans l'objet n'y change rien, la formule l'écrase |
| Combos | les 12 combos Snowflower de `db/re`, `bAtkRate` en `bAddClass` ; ceux des `_JP`, d'`Ice_Pendant` et des pièces Glacier non portées attendent |
| Coffre | `moon/rathena/merchants/episode_19.npc` : le stockage d'Hoyoyo contre 500 z |

### Instances (portées le 2026‑11‑15, non validées en jeu)

| Instance | Carte | Entrée | Délai | Au warp agent |
|---|---|---|---|---|
| Iwin Patrol (76) | `1@iwp` | le capitaine, `icecastle` (`ep19_main` 10) | 5973 | oui |
| Airship Destruction (77) | `1@whl` | la corde, `jor_nest` (`ep19_main` 33) | 12560 | oui |
| Simulation Battle (78) | `1@jorlab` | Arolong, `jor_nest` (`ep19_main` 100) | 16663 | oui |
| Bagot Laboratory (74) | `1@jorlab` | Juncea, `jor_dun03` (chaîne, étape 58) | — | non |
| Confused Snake's Nest (75) | `1@jorchs` | Lehar, `jor_que` (chaîne) | — | non |

La Bataille de simulation, qu'on croyait absente, est dans `BaggotLaboratory.txt` de la PR.

| Élément | Détail |
|---|---|
| Scripts | `moon/instances/IwinPatrol.npc`, `AirshipDestruction.npc`, `BaggotLaboratory.npc`, `ConfusedSnakesNest.npc`, copies de la PR (ASCII) |
| `instance_db` | 74 à 78, de `db/re` ; constantes `INST_*` régénérées |
| C++ | `unitisforcewalk` dans `script.cpp` (boss d'Airship Destruction) ; les autres ajouts de la PR (mapflags `nolootnormalmob`, `no_npc_selfdestruction_on_all`, `SC_MTP_W_POTION_100`, `IG_D_GW_EXTRACTOR`, correctif `SCB_RANGE`) ne servent à aucun script porté |
| Compétence | `DE_BERSERKAIZER` (578), effet visuel seul, dans `db/import/skill_db.yml` |
| Mob | `MD_BETA_SCISSORE_NG` (20631, épisode 17.2) dans `db/import/mobs/ep19_instances.yml`, ses 4 compétences de `db/re`, sans sa carte |
| Coffres | `Aquila_Antiquity` et `Aquila_Antiquity2`, et leurs groupes dans `item_group_db/ep19.yml` : Dim Glacier et extraits écartés, raffinements au-delà de 10 ramenés à 10 ; `Black_Dia` ajouté |
| Mapflags | jeu standard de l'épisode 18 sur les quatre cartes : `nomemo`, `noteleport`, `monster_noteleport`, `nowarpto`, `partylock`, `restricted 6` (sans `nobranch`) |
| Warp agent | Instance > Iwin Patrol, Airship Destruction, Simulation Battle, qui envoient près de leur PNJ d'entrée |

**Contrôle de chargement** (2026‑11‑15) : `map-server-generator --generate-navi` sur une conf
factice charge toutes les bases et les 21 679 PNJ, **0 erreur, 0 avertissement** après correction du
prix des armes Glacier. Ce n'est pas un boot (pas de base, pas de joueurs), mais tout le parsing
y passe. Les tables serveur de moonclient (objets, monstres, groupes, recettes, Lapine, propriétés,
navigation) sont régénérées.

### Enchantement Snowflower (porté le 2026‑11‑15, non validé en jeu)

| Élément | Détail |
|---|---|
| Groupes | `item_enchant` 26 à 30 de `db/re` dans `db/import/item_enchant.yml` : trois cases (3, 2, 1), **choix sûrs seulement**, une pierre par orbe, ni zeny ni tirage |
| Orbes | 97 `Ice_F_Orb_*` dans `item_db_enchant.yml` ; les trois orbes Doram (`Skill_49` à `51`) écartés et retirés des groupes |
| Pierres | 97 `Ice_F_Stone_*` (`Etc`) dans `item_db_etc.yml` |
| Obtention | Vellgunde : 35 fleurs de neige contre un tirage du groupe `ICE_F_STONE_BOX`, ou trois pierres contre une par la synthèse Laphine `Ice_F_Stone_Box2` (bloc commenté de `db/re`, rendu actif). Le groupe reprend le contenu et les taux de l'amont, pierres Doram retirées. Le PNJ et la synthèse de la PR appelaient `ICE_F_STONE_BOX2`, qui n'a pas de constante `IG_` dans le C++ : ils appellent `ICE_F_STONE_BOX`, sans recompilation |
| Conversions | paliers +11 en `getallrefine() >= 22` ; `bAtkRate` en `bAddClass` ; orbe de DEF 250/75/100/100 en **5/1/2/2** et de MDEF 25/7/10/10 en **5/1/2/2** (÷50 et ÷5, comme l'orbe Automatic à 3/1/1) ; traits gardés ; compétences fermées par la table, doublons fusionnés ; recharges en `bDelayrate` (1 s = 5 %, cumul plafonné à -25 %) |
| Compétences de 4ᵉ classe | ajoutées au palier +11 de chaque orbe de compétence. Les lignes déjà validées de la table s'appliquent (`DK_STORMSLASH`, `IG_SHIELD_SHOOTING`, `MT_AXE_STOMP`, `NW_*`, `SS_SEKIENHOU`…) ; **les 57 autres sont retirées**, le même palier renforçant déjà la compétence jouable de l'orbe |
| Retiré aussi | la recharge de `WL_TELEKINESIS_INTENSE` (-30 s, orbe `Skill_21`) : en `bDelayrate` elle vaudrait -25 % sur tout |

Conversions validées par l'utilisateur le 2026‑11‑15, inscrites dans
`skills_classes_fermees.md` : `LG_HESPERUSLIT` → `CR_GRANDCROSS`, `GN_HELLS_PLANT` →
`AM_DEMONSTRATION`, `SO_CLOUD_KILL` → `WZ_HEAVENDRIVE` (fusionnée avec `SO_POISON_BUSTER`), retrait
des 4ᵉ classes non tabulées, DEF et MDEF à 5/1/2/2.

**Client** : les fiches des 97 orbes et des 97 pierres (`item_info.yml` de moonclient) sont
réécrites d'après les scripts convertis ; la fenêtre d'enchantement prend d'office le choix sûr
quand une case n'offre pas de tirage.

Reste à faire :

- **l'enchantement des armes Glacier** (`item_enchant` 31, que Vellgunde appelle aussi) ;
- **le Dim Glacier** (26 armes `D_Glacier_*`, grade, enchantement 47, Forr, le four
  d'incinération, groupe `D_GW_EXTRACTOR`, `barter_ep19_new_content`) : aucun mob de
  l'épisode ne les fait tomber, ils relèvent de l'épisode 20 ;
- les combos Gray Wolf de l'épisode 18, oubliés au même titre (mêmes entrées de `db/re`).

**Côté client** : les 211 quêtes sont dans `quests.yml` de moonclient (2026‑11‑15), prises de
`OngoingQuests.lub` (ROenglishRE, Renewal) et insérées sans redumper le fichier ; 18145 à 18147
y portent le texte de 18119 à 18121 de la table native. 19 quêtes d'attente de l'épisode y sont
marquées `cooldown`. Les nouveaux objets attendent encore la régénération des tables.

## 5. Migration pré-renewal

1. **Compétences des mobs** : aucune source. Les écrire à la main, ou les relever sur vidéo
   comme pour le mode défi d'OGH.
2. **Spawns et terrain** : faits (Divine Pride et la PR, voir §4).
3. **Cartes EP19** (24) et **armes Glacier** (27) : conversion une par une, comme Illusion.
4. **Quêtes**, **économie** et **instances** : faites (voir §4). **Enchantement Snowflower** et
   **Dim Glacier** : à faire.
   **Client** : les tables d'objets (les titres de quête sont faits).
5. **C++** de la PR, après `git diff upstream/master` sur chaque fichier.
6. **Bataille de simulation** : faite, elle est dans `BaggotLaboratory.txt` de la PR.

Le prérequis narratif, l'épisode 18, est porté et déployé en production le 2026‑11‑15.

## 6. Verdict

| | |
|---|---|
| Intérêt | **réévalué à la hausse** : la PR #8527 fournit l'essentiel du contenu |
| Ce qui est acquis | les cartes, les entrées d'instance, les objets amont, les grades, les 40 mobs, les spawns, les warps, le warp agent, la chaîne de quêtes |
| Manque | les compétences des mobs, l'enchantement Snowflower, le Dim Glacier |
| Recommandation | le test au boot et en jeu, puis l'enchantement Snowflower ; les compétences en lot séparé |

## Sources

- [PR rAthena #8527 — Episode 19 walkthrough conversion](https://github.com/rathena/rathena/pull/8527)
- [Divine Pride](https://www.divine-pride.net) — API des monstres, serveur kROM
- [Episode XIX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIX)
- [Episode 19: Issgard, Land of Snow Flowers — Hazy Forest](https://hazyforest.com/episodes:19_issgard_land_of_snow_flower)
- [Issgard Land of Snow Flowers — iRO Wiki](https://irowiki.org/wiki/Issgard_Land_of_Snow_Flowers)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=268) · [Herosria](https://ro.gnjoy.com/news/update/View.asp?seq=271) · [niveau 260](https://ro.gnjoy.com/news/update/View.asp?seq=275)
