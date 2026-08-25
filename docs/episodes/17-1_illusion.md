# Épisode 17.1 — Illusion

| | |
|---|---|
| Nom kRO | 일루션 / *Illusion* |
| Date kRO | **2018‑07‑18** |
| Arc | suite de Terra Gloria — l'organisation Illusion |
| Niveau kRO visé | 175‑185 |
| État Moonlight | **matière présente, entièrement désactivée** |

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

- **Abyss Glast Heim**
- **Donjon de Magma F3** (`mag_dun03`)
- **Illusion of Labyrinth** (`prt_mz03_i`)
- **Passé d'Odin** (*Odin's Past*)
- **Donjon d'Einbech F3** (`ein_dun03`)
- **Lac de l'Abysse F4** (`abyss_04`)

### Instances

- **EDDA — Chute de Glast Heim** (`1@gl_prq`, modes normal et avancé)
- **EDDA — Laboratoire de bio-recherche** (`1@jorlab`, *Bagot Laboratory*)
- **Glast Heim : mode défi** (`1@gl_he`)

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

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

**Tout est là, rien n'est branché.** C'est le cas le plus net du dossier.

| Élément | Fichier Moonlight | État |
|---|---|---|
| Spawns de Rudus | `moon/rathena/dungeons/sp_rudus.npc` — 24 lignes, `sp_rudus`→`sp_rudus4` | **commenté** (`scripts_moon.conf:789`) |
| Spawns de Cor | `moon/rathena/dungeons/ba_dun.npc` — 1 ligne `sp_cor` | ✅ chargé |
| Regenschirm | `moon/instances/Regenschirm.txt` | non référencé |
| OS scellée | `moon/instances/SealedOs.txt` | non référencé |
| Occupation d'OS | `moon/instances/OsOccupation.txt` | non référencé |
| Mémorial de Cor | `moon/instances/CorOperation.txt` | non référencé |
| Quêtes 17.1 | — | **absent** |

### Les mobs manquants — la liste complète

Onze identifiants, dix mobs distincts :

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

Ce sont **les seuls mobs manquants de tout le périmètre 13.3 → 17.2** : le contrôle
d'écart entre les spawns renewal de rAthena et le roster de Moonlight (2 117 entrées) ne
signale rien d'autre, hormis `sp_os`.

Note : `sp_os` n'a **aucun** fichier de spawn côté Moonlight ; il faudra reprendre
`npc/re/mobs/special_border_area.txt` de rAthena.

Toutes les cartes concernées (`sp_cor`, `sp_os`, `sp_rudus`→`sp_rudus4`, `1@rgsr`,
`1@os_a`, `1@os_b`, `1@cor`) **sont** dans le mapcache pré-renewal.

## 5. Migration pré-renewal

Le chemin est court et entièrement balisé :

1. verser les **10 mobs `EP17_1_*`** dans `db/import/mobs/` (un fichier `sp_rudus.yml`),
   avec des statistiques rééchelonnées pour du niveau 90‑99 ;
2. décommenter `scripts_moon.conf:789` ;
3. ajouter les **cinq entrées d'instance** à `db/pre-re/instance_db.yml` ;
4. renommer les quatre `.txt` d'instance en `.npc` et les référencer ;
5. porter `quests_17_1.txt` — c'est le plus gros morceau : la chaîne de quêtes est le
   fil narratif, et elle suppose l'épisode 16.2 achevé côté joueur ;
6. traiter l'économie : Noyaux de Cor, Composants Mystérieux, enchantements associés.

L'obstacle réel est le point 5 : Moonlight n'a pas non plus porté 17.2 ni 18, donc la
chaîne narrative s'arrête à 16.2. Ouvrir 17.1 sans sa chaîne revient à ouvrir des cartes
de chasse — ce qui reste une option défendable et bien moins coûteuse.

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — trois zones de chasse et cinq instances, toutes prêtes |
| Reste à faire | 10 mobs · 1 ligne de conf · 5 entrées `instance_db` · 4 scripts à brancher · (optionnel) la chaîne de quêtes |

## Sources

- [Episode XVII.I — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVII.I)
- [Episode 17.1: Illusion — ROGGH Library](https://roggh.com/episode-17-1-main-quest/)
- [Rudus — iRO Wiki](https://irowiki.org/wiki/Rudus)
- [rathena/rathena PR #6643](https://github.com/rathena/rathena/pull/6643)
- [Annonce kRO d'origine](http://ro.gnjoy.com/news/update/View.asp?seq=224)
