# Épisode 20 — L'Immortel (*The Undying*)

| | |
|---|---|
| Nom kRO | *The Undying* |
| Date kRO | **2023‑03‑02** |
| Arc | fin de la traque de Bagot — le labyrinthe du serpent |
| Niveau kRO visé | **215+** |
| État Moonlight | **rien** — et rAthena non plus |

Suite directe d'Issgard : on descend dans le canyon de glace ancien, on démasque les
infiltrés, et on cherche l'entrée du repaire du serpent avant qu'un pouvoir immortel ne
soit libéré.

## 1. Contenu

### Cartes

| Carte | Nom |
|---|---|
| `icecastle`, `icas_in`, `icas_in2` | Château de Glace (étendu) |
| `jor_back5`, `jor_back6` | **Canyon de Glace Ancien** |
| `jor_maze` | **Labyrinthe du Serpent** |
| `jor_root1`, `jor_root2` | Racines |
| `jor_sanct` | Sanctuaire |
| `jor_safty1`, `jor_safty2`, `jor_safy1` | Zones sûres |
| `jor_twice`, `jor_twig` | Zones jumelles |
| `1@20cn1`, `1@20cn2`, `1@twig`, `1@twsd` | Instances |

### Instances

| Instance | Carte |
|---|---|
| **Neutraliser l'Immortel** | `1@20cn1` / `1@20cn2` |
| **Mer Collante** (*Sticky Sea*) | `1@twsd` |

### Mécaniques

Les **Parchemins de transformation Rgan** de l'épisode 19 restent nécessaires tout du
long. Prérequis : l'épisode 19 achevé, et 30 Plumes de Tête de Copo + 20 Grandes Plumes
Grises.

### Contenu rattaché

- **Hall of Life** (`for_dun01`) et **Jardin du Temps** (`t_garden`) — sortis en kRO le
  10 juillet 2023, rattachés à l'épisode 20 par rAthena
  ([issue #8214 « Episode 20 : Garden of Time »](https://github.com/rathena/rathena/issues/8214)).
  Ce sont les **seuls morceaux de l'épisode 20 que rAthena convertit** :
  `npc/re/quests/garden_of_time.txt` et `npc/re/mobs/dungeons/for_dun.txt`
  (PR #8215, novembre 2024).

## 2. État côté rAthena

Pour l'épisode 20 proprement dit : **aucun script**, aucune entrée `instance_db` pour
`1@20cn1`, `1@20cn2`, `1@twig` ou `1@twsd`.

Pour Hall of Life / Jardin du Temps : conversion complète, 32 mobs `SPIRIT_*` répartis
sur huit familles élémentaires en quatre tailles (S / M / L / SL).

## 3. État côté Moonlight (mesuré le 2026‑08‑25)

| Élément | Fichier Moonlight | État |
|---|---|---|
| Épisode 20 principal | — | **absent** |
| **Hall of Life** | `moon/rathena/dungeons/for_dun.npc` — `for_dun01`, `t_garden` | **commenté** (`scripts_moon.conf:764`) |
| Mobs de Hall of Life | — | **32 manquants** (`SPIRIT_G_LAND_*`, `SPIRIT_B_FLAME_*`, `SPIRIT_S_WIND_*`, `SPIRIT_I_WATER_*`, `SPIRIT_H_WATER_*`, `SPIRIT_D_WIND_*`, `SPIRIT_R_FLAME_*`, `SPIRIT_F_LAND_*`) |
| Quêtes Jardin du Temps | — | absent |

Les cartes de l'épisode 20 sont **toutes** dans `db/import/map_cache.dat`, y compris
`jor_maze`, `jor_sanct`, `1@20cn1/2`, `1@twig` et `1@twsd`.

## 4. Migration pré-renewal

Même constat qu'à l'[épisode 19](19_issgard.md) : écrire, pas porter.

**Une exception intéressante** : le **Hall of Life** est déjà copié côté Moonlight,
simplement commenté. C'est un donjon d'élémentaires à quatre tailles, structure très
régulière — donc facile à rééchelonner de façon systématique. Il ne dépend d'aucune
chaîne narrative. C'est le seul morceau de l'épisode 20 réellement à portée.

Coût estimé : 32 entrées de mob dans `db/import/mobs/for_dun.yml`, une ligne de conf, et
le portage optionnel de `garden_of_time.txt` pour les PNJ.

## 5. Verdict

| | |
|---|---|
| Intérêt de l'épisode 20 principal | **très faible** — rien d'exploitable en amont |
| Intérêt du Hall of Life | **moyen** — un donjon autonome, prêt, 32 mobs à écrire |
| Recommandation | traiter Hall of Life comme un chantier isolé, ignorer le reste |

## Sources

- [Episode XX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XX)
- [Undying — iRO Wiki](https://irowiki.org/wiki/Undying)
- [Garden of Time — iRO Wiki](https://irowiki.org/wiki/Garden_of_Time)
- [rathena/rathena issue #8214](https://github.com/rathena/rathena/issues/8214)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=277)
