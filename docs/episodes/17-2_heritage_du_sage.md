# Épisode 17.2 — L'héritage du Sage (*Sage's Legacy*)

| | |
|---|---|
| Nom kRO | *Sage's Legacy* — le manoir de Varmundt |
| Date kRO | **2020‑10‑30** |
| Arc | l'héritage de Varmundt |
| Niveau kRO visé | 185‑200 |
| État Moonlight | **spawns du manoir actifs ; lot A (Niflheim renewal, Rudus 4F) porté le 2026‑11‑11, à valider en jeu ; quêtes, instances et économie du manoir absentes** |

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
| Jardin du Crépuscule / Hey! Sweety | `moon/instances/TwilightGarden.txt` | non référencé |
| Jardin d'Eau | `moon/instances/WaterGarden.txt` | non référencé |
| Jardin de Fleurs Caché | `moon/instances/HiddenGarden.txt` | non référencé |
| Ferme Perdue dans le Temps | `moon/instances/LostFarm.txt` | non référencé |
| Quêtes 17.2 | — | **absent** (`quests_17_2.txt`, 126 quêtes sur 139 absentes) |
| Rudus 4F | `moon/rathena/dungeons/sp_rudus.npc` | ✅ chargé le 2026‑11‑11 (lot A) |
| Quêtes d'accès et journalières de Rudus 4F et Niflheim | `moon/rathena/quests/dungeons_200.npc` | ✅ chargé le 2026‑11‑11 (lot A) ; la partie Amicitia de l'amont n'est pas reprise |

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

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — le terrain est déjà ouvert, il manque ce qui le rend jouable |
| Fait | lot A : Niflheim renewal et Rudus 4F (2026‑11‑11, non validé en jeu) |
| Reste à faire | **B1** : 4 instances (7 entrées `instance_db` à partir de 58, 29 mobs, scripts renommés en `.npc`, mapflags) · **B2** : `quests_17_2.txt` (126 quêtes et ses objets) · **B3** : l'économie du manoir (Lisa et l'équipement Automatic, modules, échanges) · la partie Amicitia de `quests_dungeons_200.txt` · hors périmètre : les 5 instances de l'intervalle, sans script nulle part |

## Sources

- [Episode XVII.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVII.II)
- [rathena/rathena PR #6799](https://github.com/rathena/rathena/pull/6799)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=245)
