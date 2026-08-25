# Épisode 17.2 — L'héritage du Sage (*Sage's Legacy*)

| | |
|---|---|
| Nom kRO | *Sage's Legacy* — le manoir de Varmundt |
| Date kRO | **2020‑10‑30** |
| Arc | l'héritage de Varmundt |
| Niveau kRO visé | 185‑200 |
| État Moonlight | **spawns actifs, quêtes et instances absentes** |

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

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

Situation inattendue et intéressante : **les spawns sont déjà en service**, alors que
l'épisode 17.1 qui les précède est éteint.

| Élément | Fichier Moonlight | État |
|---|---|---|
| Manoir de Varmundt et dépendances | `moon/rathena/dungeons/ba_dun.npc` — `ba_2whs01/02`, `ba_bath`, `ba_in01`, `ba_lib`, `ba_lost`, `ba_maison`, `ba_pw01/02/03` | ✅ chargé |
| Laboratoire Amicitia | `moon/rathena/dungeons/amicitia.npc` — `amicitia1`, `amicitia2` | ✅ chargé |
| Mobs correspondants | `db/import/mobs/ba_dun.yml`, `db/import/mobs/amicitia.yml` | ✅ |
| Niflheim renewal | `moon/rathena/dungeons/nif_dun.npc` — `nif_dun01`, `nif_dun02` | **commenté** (`scripts_moon.conf:780`) |
| Jardin du Crépuscule / Hey! Sweety | `moon/instances/TwilightGarden.txt` | non référencé |
| Jardin d'Eau | `moon/instances/WaterGarden.txt` | non référencé |
| Jardin de Fleurs Caché | `moon/instances/HiddenGarden.txt` | non référencé |
| Ferme Perdue dans le Temps | `moon/instances/LostFarm.txt` | non référencé |
| Quêtes 17.2 | — | **absent** |
| Rudus 4F | `moon/rathena/dungeons/sp_rudus.npc` | commenté, cf. [17.1](17-1_illusion.md) |

Le contrôle d'écart mob ne signale **aucun manque** sur `ba_*` ni sur `amicitia` : le
roster est complet pour ces cartes.

## 5. Migration pré-renewal

C'est l'épisode où le déséquilibre est le plus visible : les joueurs peuvent déjà chasser
dans le manoir de Varmundt, mais **aucune instance, aucune quête, aucune économie** n'y
est attachée. Deux façons de refermer l'écart :

- **voie courte** — décommenter `nif_dun` et brancher les quatre instances
  (`1@bamn`, `1@bamq`, `1@ghg`, `1@herbs`, `1@lost` sont toutes dans le mapcache) ;
  cela donne cinq contenus de groupe sans rien écrire ;
- **voie longue** — porter `quests_17_2.txt` et la chaîne narrative, ce qui suppose
  d'avoir d'abord porté 17.1.

Dans les deux cas, le rééchelonnage 185‑200 → 90‑99 est le vrai travail.

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — le terrain est déjà ouvert, il manque ce qui le rend jouable |
| Reste à faire | 6 entrées `instance_db` · 4 scripts à brancher · décommenter `nif_dun` · (optionnel) la chaîne de quêtes |

## Sources

- [Episode XVII.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVII.II)
- [rathena/rathena PR #6799](https://github.com/rathena/rathena/pull/6799)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=245)
