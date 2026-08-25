# Épisode 13.3 — El Dicastes

| | |
|---|---|
| Nom kRO | 엘 디카스타스 / *El Dicastes* |
| Date kRO | **2009‑12‑23** |
| Arc | *Another World* — suite de l'ép. 13.1/13.2 (Ash Vacuum) |
| Niveau kRO visé | ~110‑130 |
| État Moonlight | **porté et actif** |

Premier épisode entièrement postérieur à la bascule Renewal. Le joueur, déjà passé par
Manuk et Splendide à l'épisode 13.2, atteint enfin la capitale des Sapha.

## 1. Contenu principal

### Ville et champs

| Carte | Nom |
|---|---|
| `dicastes01` | El Dicastes (niveau bas) |
| `dicastes02` | El Dicastes (niveau haut) |
| `dic_in01` | Intérieurs (Salle du Diel, etc.) |
| `dic_fild01` | Terres de Kamidal |
| `dic_fild02` | Terres de El Dicastes |

### Donjons

| Carte | Nom |
|---|---|
| `dic_dun01`, `dic_dun02` | Montagne de Kamidal / Hall des Scarabées F1‑F2 |
| `dic_dun03` | Hall des Scarabées F3 *(ajout ultérieur)* |

Mobs marquants : Scaraba (doré/vert), Rake Scaraba, Antler Scaraba, **Queen Scaraba**
(MVP), Centipede, Cornus, Naga.

### Systèmes livrés dans l'intervalle 13.3 → 14.1

D'après la chronologie Hercules :

- 2010‑03‑17 — système de **recrutement de groupe** ;
- 2010‑03‑31 — **classes 3ᵉ « bébé »** ;
- 2010‑05‑12 — **échoppes d'achat** (*Buy Shop*) et **système d'esprit du Sorcerer**.

Aucun de ces trois éléments n'a de sens sur un serveur pré-renewal sans 3ᵉ classe, sauf
le recrutement de groupe et les échoppes d'achat, qui sont des mécaniques client/serveur
génériques.

## 2. État côté rAthena

| Élément | Fichier |
|---|---|
| Ville et PNJ | `npc/re/cities/dicastes.txt` |
| Quêtes | `npc/re/quests/quests_dicastes.txt` |
| Guides | `npc/re/guides/guides_dicastes.txt` |
| Spawns | `npc/re/mobs/dungeons/dic_dun.txt`, `npc/re/mobs/fields/dicastes.txt` |
| Réputation Sapha | `db/re/reputation.yml` |

Contenu mûr, converti depuis longtemps, corrigé au fil des années.

## 3. État côté Moonlight (mesuré le 2026‑08‑25)

**Porté et actif.** Toutes les pièces sont référencées dans `moon/scripts_moon.conf` :

- `moon/rathena/cities/dicastes.txt`
- `moon/rathena/quests/quests_dicastes.txt`
- `moon/rathena/guides/guides_dicastes.txt`
- `moon/mobs/dicaste.npc` — couvre `dicastes01`, `dic_fild01/02`, `dic_dun01/02/03`
- `db/import/mobs/dicaste.yml` — les mobs sont dans le roster pré-renewal

Le contrôle d'écart mob (`db/re/mob_db.yml` des spawns rAthena vs roster Moonlight) ne
signale **aucun mob manquant** sur `dic_dun`.

`dic_dun03` — le 3ᵉ étage du Hall des Scarabées, ajouté après coup en renewal — est
bien peuplé par `moon/mobs/dicaste.npc`, et la carte est présente dans
`db/import/map_cache.dat` comme dans `db/map_index.txt`.

## 4. Migration pré-renewal

Rien à faire : c'est fait. Reste l'arbitrage d'équilibrage habituel (cf.
[00_transition_renewal.md](00_transition_renewal.md) §4), déjà tranché par les
`db/import/` existants.

## 5. Verdict

| | |
|---|---|
| Intérêt pour un serveur pré-renewal | élevé — une ville complète, deux donjons, un MVP |
| Reste à faire | **rien** |

## Sources

- [Episode XIII.III — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIII.III)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](http://ro.gnjoy.com/news/notice/View.asp?BBSMode=10001&seq=5421&curpage=138)
