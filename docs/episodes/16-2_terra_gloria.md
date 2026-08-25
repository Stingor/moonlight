# Épisode 16.2 — Terra Gloria

| | |
|---|---|
| Nom kRO | 테라 그로리아 / *Terra Gloria* |
| Date kRO | **2016‑03‑09** |
| Arc | Terra Gloria — les Heart Hunters |
| Niveau kRO visé | 150‑175 |
| État Moonlight | **porté en partie** — c'est le premier gros gisement de contenu prêt à brancher |

Épisode charnière : il ouvre **Rock Ridge** et surtout la série des **donjons Illusion**,
qui recyclent d'anciens donjons pré-renewal en versions haut niveau. C'est le contenu le
plus naturellement transposable de tout le dossier, parce qu'il s'appuie sur des cartes
que le pré-renewal connaît déjà.

## 1. Contenu principal (9 mars 2016)

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Base de guerre des Heart Hunters** 1 et 2 | `1@swat` | `HeartHunterWarBase.txt` |
| **Laboratoire de Werner** (salle centrale) 1 et 2 | `1@slw` | `WernerLaboratoryCentralRoom.txt` |

## 2. Contenu de l'intervalle (2016‑2018)

### Ville et donjon

| Date kRO | Contenu | Cartes |
|---|---|---|
| 2016‑12‑07 | **Rock Ridge** (village de l'Ouest) | `rockrdg1`, `rockrdg2`, `har_in01`, `harboro1`, `harboro2` |
| 2016‑12‑07 | **Mine de Rock Ridge** | `rockmi1`, `rockmi2` |
| — | Izlude réorganisée | `izlude` |

### Les donjons Illusion

Chacun est une relecture haut niveau d'un donjon existant. Carte = la carte d'origine
suffixée `_i`.

| Date kRO | Donjon Illusion | Carte | Donjon d'origine | Niveau kRO |
|---|---|---|---|---|
| 2016‑12‑27 | **Illusion of Moonlight** | `pay_d03_i` | Donjon de Payon F3 | 100+ |
| 2017‑01‑25 | **Illusion of Vampire** | `gef_d01_i` | Donjon de Geffen F1 | 130+ |
| 2017‑02‑21 | **Illusion of Frozen** | `ice_d03_i` | Donjon de Glace F3 | 110+ |
| 2017‑04‑18 | **Illusion of Abyss** | *(non converti)* | Lac de l'Abysse | 150+ |
| 2018‑03‑09 | **Illusion of Teddy Bear** | `ein_d02_i` | Mine d'Einbech F2 | 150+ |
| 2018‑05‑04 | **Illusion of Luanda** | `com_d02_i` | Donjon de la Plage | 160+ |
| — | **Illusion of Turtle** | `tur_d03_i`, `tur_d04_i` | Île de la Tortue | — |
| — | **Illusion of Twins** | `ant_d02_i` | Fourmilière F2 | — |
| — | **Illusion of Underwater** | `iz_d04_i`, `iz_d05_i` | Byalan F4/F5 | 140+ |
| — | **Illusion of Labyrinth** | `prt_mz03_i` *(non converti)* | Forêt Labyrinthe | 170+ |

> rAthena a converti **huit** des dix : Moonlight, Vampire, Frozen, Turtle, Luanda,
> Underwater, Twins, Teddy Bear. Abyss et Labyrinth n'ont que les objets et l'enchanteur.
>
> iRO Wiki n'en recense que neuf, sans *Illusion of Turtle* ; celui-ci existe pourtant
> bien dans rAthena (`tur_d03_i`, `tur_d04_i`) et côté Moonlight en spawns commentés.

### Instances de l'intervalle

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Donjon du week-end** | `1@md_pay` | `WeekendDungeon.txt` |
| **Donjon du vendredi** | `1@md_gef` | `FridayDungeon.txt` |
| **Village Poring** | `1@begi` | `PoringVillage.txt` |
| **Old Glast Heim — mode débutant** | `1@gl_k2` | (variante d'`OldGlastHeim`) |
| Usine de Jouets d'Horreur — mode débutant | `1@xm_d` | (variante) |
| *Rêves et Ombres*, *Talons du Roi* | — | quêtes |

### Systèmes

| Système | Pertinent en pré-renewal ? |
|---|---|
| **Système Lapine** | oui |
| **Retrait de carte** (*card removal*) | oui |
| **Échoppe hors ligne** (*part-time stall*) | oui |
| **Échange d'équipement** (*gear swap*) | oui |
| Quêtes de donjon quotidiennes | oui |
| Système familial mis à jour | oui |
| Interface de groupe améliorée | oui |
| Course de Hugel revue | oui |
| Mémoire des Orcs mise à jour | oui |
| Améliorations de la carte du monde | oui |
| Refontes de compétences 3ᵉ classe #1→#4, Rebellion #2 | non |
| Quêtes Eden 100‑140 réorganisées | à arbitrer |

## 3. État côté rAthena

`npc/re/quests/quests_16_2.txt`, `npc/re/quests/quests_rockridge.txt`,
`npc/re/quests/quests_illusion_dungeons.txt` (≈ 13 000 lignes, huit donjons),
`npc/re/merchants/enchan_illusion_dungeons.txt`, `enchan_rockridge.txt`,
`npc/re/quests/illusion_investigation.txt`,
`npc/re/instances/{HeartHunterWarBase,WernerLaboratoryCentralRoom,WeekendDungeon,FridayDungeon,PoringVillage}.txt`.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

### Déjà en service

| Élément | Fichier Moonlight |
|---|---|
| Quêtes 16.2 | `moon/rathena/quests/quests_16_2.txt` |
| Rock Ridge — quêtes, guides, entrepôts, enchantements | `quests_rockridge.txt`, `guides_rockridge.txt`, `enchan_rockridge.npc`, `warps/cities/rockridge.txt` |
| Rock Ridge — mobs | `moon/mobs/rockridge.npc` (`rockrdg1/2`, `rockmi1/2`, `harboro1/2`) |
| Heart Hunters | `moon/instances/HeartHunterWarBase.npc` |
| Laboratoire de Werner | `moon/instances/WernerLaboratoryCentralRoom.npc` |

### Le gisement : les donjons Illusion sont **pré-installés mais commentés**

Les lignes de spawn Illusion sont présentes dans les fichiers de spawn de Moonlight,
**mises en commentaire**. Relevé exact :

| Carte | Donjon | Fichier Moonlight | Lignes commentées | Mobs à ajouter au roster |
|---|---|---|---|---|
| `pay_d03_i` | Moonlight | `moon/rathena/dungeons/pay_dun.npc` | 11 | 6 (`ILL_NINE_TAIL`, `ILL_MUNAK`, `ILL_BON_GUN`, `ILL_SOHEE`, `ILL_ARCHER_SKELETON`, `ILL_FURY_HERO`) |
| `gef_d01_i` | Vampire | `moon/rathena/dungeons/gef_dun.npc` | 8 | 6 (`ILL_DRAINLIAR`, `ILL_ZOMBIE`, `ILL_ZOMBIE_C`, `ILL_GHOUL`, `ILL_NIGHTMARE`, `ILL_BLACK_MUSHROOM`) |
| `ice_d03_i` | Frozen | `moon/rathena/dungeons/ice_dun.npc` | 4 | 4 (`ILL_GAZETI`, `ILL_SNOWIER`, `ILL_ICE_TITAN`, `ILL_ICEICLE`) |
| `iz_d04_i` | Underwater F1 | `moon/rathena/dungeons/iz_dun.npc` | 5 | 5 |
| `iz_d05_i` | Underwater F2 | `moon/rathena/dungeons/iz_dun.npc` | 5 | 5 |
| `tur_d03_i` | Turtle F1 | `moon/rathena/dungeons/tur_dun.npc` | 2 | 2 |
| `tur_d04_i` | Turtle F2 | `moon/rathena/dungeons/tur_dun.npc` | 5 | 5 |
| `ant_d02_i` | Twins | `moon/mobs/morocc.npc` | 9 | 9 |

**Total : 49 lignes de spawn, 42 mobs `ILL_*` à verser dans `db/import/`.**

Les deux donjons non pré-installés côté Moonlight sont **Luanda** (`com_d02_i`) et
**Teddy Bear** (`ein_d02_i`) ; leurs cartes sont bien dans le mapcache.

Manquent aussi les PNJ d'entrée et l'économie : `quests_illusion_dungeons.txt`,
`enchan_illusion_dungeons.txt`, `illusion_investigation.txt` — aucun n'est présent
côté Moonlight.

### Autres manques

| Manque | Fichier présent ? |
|---|---|
| Donjon du week-end | `moon/instances/WeekendDungeon.txt` — non référencé |
| Donjon du vendredi | `moon/instances/FridayDungeon.txt` — non référencé |
| Village Poring | `moon/instances/PoringVillage.txt` — non référencé |
| Old Glast Heim mode débutant | absent |

## 5. Migration pré-renewal

Les donjons Illusion sont **le meilleur rapport contenu/effort de tout le dossier** :

1. les cartes sont dans le mapcache ;
2. les spawns sont déjà écrits, il suffit de les décommenter ;
3. les mobs sont des variantes de monstres pré-renewal connus — leur rééchelonnage est
   mécanique (on part de la version d'origine, on monte) plutôt qu'arbitraire ;
4. ils réutilisent des donjons que les joueurs connaissent déjà.

Le point d'attention est l'**économie associée** : les enchantements Illusion supposent
des monnaies et des équipements renewal. Il faut soit les convertir, soit les remplacer
par une récompense maison.

## 6. Verdict

| | |
|---|---|
| Intérêt | **le plus élevé du dossier** |
| Reste à faire | 42 mobs `ILL_*` · décommenter 49 lignes · porter les PNJ d'entrée et l'enchanteur · 3 instances à brancher |

## Sources

- [Episode XVI.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVI.II)
- [Illusion Dungeons — iRO Wiki](https://irowiki.org/wiki/Illusion_Dungeons)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=182)
