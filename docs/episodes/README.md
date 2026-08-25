# Épisodes de contenu kRO depuis la fin du pré-renewal

Index des fiches. Un fichier par épisode, de la bascule Renewal (2009) à l'épisode 20
(2023). Objectif : savoir, épisode par épisode, **ce que kRO a livré**, **ce que
rAthena implémente**, et **ce que Moonlight a déjà porté ou pas**.

Relevé effectué le **2026-08-25** sur les dépôts locaux
`d:\Mes documents\GitHub\rathena` et `d:\Mes documents\GitHub\moonlight`.

## Les fiches

| Fiche | Épisode | Date kRO | État Moonlight |
|---|---|---|---|
| [00_transition_renewal.md](00_transition_renewal.md) | La bascule Renewal (frontière `pre-re` / `re`) | 2009‑06‑17 → 2010 | frontière assumée : Moonlight reste `PRERE` |
| [13-3_el_dicastes.md](13-3_el_dicastes.md) | 13.3 — El Dicastes | 2009‑12‑23 | **porté** |
| [14-1_bifrost.md](14-1_bifrost.md) | 14.1 — Bifrost (Mora, Dewata, Malangdo, Port Malaya) | 2010‑06‑30 | **porté** |
| [14-2_eclage.md](14-2_eclage.md) | 14.2 — Eclage | 2011‑12‑21 | **porté** (sauf le mode Vague) |
| [14-3_bataille_decisive.md](14-3_bataille_decisive.md) | 14.3 — Decisive Battle | 2012‑12‑26 / 2013‑03‑19 | **porté** (sauf Temple du Dieu Démon) |
| [15-1_phantasmagorika.md](15-1_phantasmagorika.md) | 15.1 — Phantasmagorika (Verus) | 2013‑07 | **porté** |
| [15-2_memory_record.md](15-2_memory_record.md) | 15.2 — Memory Record | 2013‑12‑23 | **porté** (sauf Infinite Space) |
| [16-1_banquet_des_heros.md](16-1_banquet_des_heros.md) | 16.1 — Banquet of Heroes | 2015‑02‑25 | **porté en partie** |
| [16-2_terra_gloria.md](16-2_terra_gloria.md) | 16.2 — Terra Gloria (Rock Ridge, Illusions) | 2016‑03‑09 | **porté en partie** |
| [17-1_illusion.md](17-1_illusion.md) | 17.1 — Illusion (Cor, Rudus, OS) | 2018‑07‑18 | **matière présente, désactivée** |
| [17-2_heritage_du_sage.md](17-2_heritage_du_sage.md) | 17.2 — Sage's Legacy (Varmundt) | 2020‑10‑30 | **spawns actifs, quêtes absentes** |
| [18_direction_de_la_priere.md](18_direction_de_la_priere.md) | 18 — Direction of Prayer | 2021‑02‑01 | **matière présente, désactivée** |
| [19_issgard.md](19_issgard.md) | 19 — Issgard, Land of Snow Flowers | 2022‑01‑19 | **rien** (rAthena non plus) |
| [20_the_undying.md](20_the_undying.md) | 20 — The Undying | 2023‑03‑02 | **rien** (rAthena non plus) |

## Synthèse : ce qui reste à migrer, par coût croissant

Récapitulé depuis les fiches. Les chiffres sont mesurés, pas estimés.

### Niveau 1 — une ligne de configuration

Trois instances ont déjà leur entrée dans `db/import/instance_db.yml` — la base
réellement chargée — et leur script sur le disque. Il ne manque que la ligne
`npc:` dans `moon/scripts_moon.conf`.

| Contenu | Épisode | Entrée existante | Script |
|---|---|---|---|
| Mode Vague — Forêt et Ciel (`1@def01`, `1@def02`) | 14.2 | ids 22 et 23 | `moon/instances/WaveMode.txt` |
| Temple du Dieu Démon (`1@eom`) | 14.3 | id 27 | `moon/instances/TempleofDemonGod.npc` |
| Forteresse Céleste (`1@sthb`) | 16.1 | id 33 | `moon/instances/SkyFortress.txt` |

Plus une incohérence à solder : `dic_dun03` (ép. 13.3) reçoit des spawns alors que la
carte n'est pas dans le mapcache — régénérer le mapcache ou commenter le bloc.

### Niveau 2 — brancher un script et créer son entrée `instance_db`

| Contenu | Épisode | Carte |
|---|---|---|
| Espace Infini | 15.2 | `1@infi` |
| EDDA — Demi-lune en plein jour | 16.1 | `1@pop1` |
| Donjon du week-end / du vendredi | 16.2 | `1@md_pay`, `1@md_gef` |
| Village Poring | 16.2 | `1@begi` |
| Jardin du Crépuscule / Hey! Sweety | 17.2 | `1@bamn`, `1@bamq` |
| Jardin d'Eau (normal + difficile) | 17.2 | `1@ghg` |
| Jardin de Fleurs Caché / Zones de Sécurité | 17.2 | `1@herbs` |
| Ferme Perdue dans le Temps | 17.2 | `1@lost` |
| Niflheim renewal (spawns) | 17.2 | `nif_dun01/02` |

Soit **douze contenus de groupe** dont les scripts sont déjà sur le disque et dont les
cartes sont déjà dans le mapcache — il ne manque que la déclaration dans
`db/import/instance_db.yml` et la ligne de conf.

### Niveau 3 — ajouter des mobs puis décommenter

| Contenu | Épisode | Mobs à écrire | Lignes à décommenter |
|---|---|---:|---:|
| **Donjons Illusion** (8 donjons pré-installés) | 16.2 | **42** (`ILL_*`) | 49 |
| Rudus + OS | 17.1 | **10** (`EP17_1_*`) | 24 |
| Village du Loup Gris + Labyrinthe d'Oz | 18 | **13** (`EP18_*`) | 16 |
| Hall of Life | 20 | **32** (`SPIRIT_*`) | 36 |

**97 entrées de mob au total.** C'est le cœur du travail restant, et le gisement le plus
rentable est celui des donjons Illusion : cartes connues, monstres dérivés de créatures
pré-renewal, donc rééchelonnage mécanique.

### Niveau 4 — écrire la chaîne narrative

`quests_17_1.txt`, `quests_17_2.txt`, `quests_18.txt`,
`quests_illusion_dungeons.txt` (≈ 13 000 lignes), `enchan_illusion_dungeons.txt`,
`illusion_investigation.txt`. Ce sont des portages lourds, avec les économies associées
(Noyaux de Cor, Fragments d'Améthyste, monnaies d'enchantement Illusion) à convertir.

### Hors périmètre

Épisodes **19** et **20** : rAthena ne les convertit pas — pas de spawns, pas de scripts
d'instance, seulement des objets et des entrées `instance_db` vides. Migrer signifierait
écrire. Seul le **Hall of Life** de l'épisode 20 est à portée (niveau 3 ci-dessus).

Classes **3ᵉ et 4ᵉ**, **Doram/Summoner**, **Kagerou/Oboro**, **Rebellion**,
**Homunculus S**, plafonds 150 → 260 : structurellement incompatibles avec un build
`PRERE`.

## Les trois contraintes qui pèsent sur toute migration

Elles reviennent dans chaque fiche ; elles sont posées ici une fois pour toutes.

### 1. Moonlight est un build pré-renewal, sans 3ᵉ classe

`src/config/renewal.hpp` ne définit que `PRERE` : aucune option Renewal n'est activée.
`moon/job_master.npc:23` refuse explicitement `class >= Job_Rune_Knight && class <= Job_Mechanic`.
Le plafond reste **99 base / 70 job**, classes 2‑1/2‑2 et transcendantes.

Conséquence : tout contenu kRO conçu pour les paliers 100 / 130 / 150 / 175 / 185 / 200
doit être **rééchelonné**, pas seulement recopié. Cela touche les niveaux d'entrée
d'instance, les prérequis de quête, les récompenses d'EXP et la difficulté des mobs.

### 2. Les statistiques renewal ne se transposent pas telles quelles

`db/re/mob_db.yml` est calibré pour les formules ATK/DEF/MATK renewal et pour des
personnages de niveau 100+. Repris sans retouche en pré-renewal, un mob d'épisode 16+
devient soit intuable, soit trivial. Idem pour les équipements : la séparation
ATK/MATK, le niveau d'équipement et la table de raffinage renewal n'existent pas ici.

Ampleur du fossé **chez rAthena amont** (`db/pre-re` vs `db/re`) :

| Base | pre-re | re | écart brut |
|---|---:|---:|---:|
| `mob_db.yml` | 1 004 | 2 675 | 1 671 |
| `item_db_equip.yml` | 2 017 | 12 982 | 10 965 |
| `item_db_usable.yml` | 1 905 | 6 415 | 4 510 |
| `item_db_etc.yml` | 2 247 | 9 959 | 7 712 |
| `instance_db.yml` | 37 | 78 | 41 |

**Ces chiffres ne décrivent pas Moonlight** : `db/pre-re/` y est débranché — voir
l'encadré ci-dessous.

### 2 bis. Où vivent réellement les données de Moonlight

Le chargement d'une base rAthena suit la chaîne `Footer: Imports:` depuis le fichier
racine `db/<nom>.yml` (`YamlDatabase::parseImports`, `src/common/database.cpp:176`) —
il n'y a **aucun** import implicite.

Or Moonlight a mis en commentaire la ligne `- Path: db/pre-re/…` dans neuf fichiers
racine : `mob_db`, `item_db`, `instance_db`, `item_combos`, `skill_tree`, `statpoint`,
`job_stats`, `mob_summon`, `achievement_db`. **Tout passe par `db/import/`.**

| Base | Ce qui est réellement chargé | Entrées |
|---|---|---:|
| mobs | `db/import/mob_db.yml` (1 454) + 46 fichiers de `db/import/mobs/` | **2 117** |
| objets | `db/import/items/*.yml` (12 fichiers) | **14 325** |
| instances | `db/import/instance_db.yml` | **39** |
| quêtes | `db/pre-re/quest_db.yml` (3 691) + `db/import/quest_db.yml` (156) | **3 768** |

Deux conséquences pratiques :

- `db/pre-re/mob_db.yml`, `db/pre-re/item_db_*.yml` et `db/pre-re/instance_db.yml` sont
  des **fichiers morts** : les éditer n'a aucun effet. Les 1 004 ids du premier sont un
  sous-ensemble strict du roster réel, ce qui explique que rien ne manque à l'usage ;
- **`quest_db` fait exception** : `db/pre-re/quest_db.yml` est bien chargé, c'est donc là
  qu'une entrée de quête manquante doit être ajoutée.

Toute la suite de ce dossier raisonne sur les bases `db/import/`.

### 3. Le client n'est pas le facteur limitant

Contrôle sur `db/pre-re/map_cache.dat` de Moonlight : **1 103 cartes**, soit *plus* que
le mapcache renewal de rAthena (1 025).

Deux vérifications exhaustives ont été faites :

1. les cartes de terrain des épisodes 13.3 à 20 (villes, champs, donjons, y compris
   Issgard et le labyrinthe du serpent de l'épisode 20) ;
2. les 31 cartes d'entrée des instances renewal que Moonlight n'a **pas** portées
   (`1@infi`, `1@rgsr`, `1@os_a/b`, `1@cor`, `1@ghg`, `1@herbs`, `1@oz`, `1@nyr`,
   `1@advs`, `1@jorlab`, `1@jorchs`, `1@iwp`, `1@whl`…).

Une seule carte manque à l'appel sur l'ensemble : **`dic_dun03`** (3ᵉ étage du Hall des
Scarabées, ajouté après coup en renewal).

Autrement dit : **le blocage est côté serveur, pas côté ressources graphiques**. Aucune
fiche de ce dossier ne peut invoquer « le client ne sait pas afficher la carte » comme
motif de blocage, sauf `dic_dun03`.

## Convention de lecture de l'état Moonlight

Moonlight distingue par l'extension :

- `.npc` — script adopté, référencé dans `moon/scripts_moon.conf`, **actif** ;
- `.txt` — copie brute de rAthena déposée dans l'arborescence mais **non référencée**,
  donc inerte ;
- ligne `// npc: …` dans `scripts_moon.conf` — script présent mais **volontairement
  désactivé**.

Cette convention est ce qui permet de dire, dans chaque fiche, si le travail restant
est « écrire » ou seulement « brancher et rééquilibrer ».

## Sources

- [Episodes — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episodes)
  (une page par épisode, contenu principal et contenu intercalaire)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
  (chronologie datée des patchs kRO serveur principal, tenue par des développeurs d'émulateur)
- [Episodes — Hazy Forest](https://hazyforest.com/episodes:list)
- [iRO Wiki](https://irowiki.org/) — pages d'épisode et de donjon
- [ROGGH Library](https://roggh.com/roggh-content/) — guides détaillés à partir de l'épisode 14.2
- Journal git de [rathena/rathena](https://github.com/rathena/rathena) pour l'état d'implémentation
