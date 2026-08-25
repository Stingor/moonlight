# Épisode 15.2 — Memory Record

| | |
|---|---|
| Nom kRO | 메모리 레코드 / *Memory Record* |
| Date kRO | **2013‑12‑23** |
| Arc | suite de Verus |
| Niveau kRO visé | 150‑175 |
| État Moonlight | **porté et actif**, sauf Infinite Space |

Épisode léger en cartes mais **très lourd en systèmes** : c'est ici que kRO livre la
navigation moderne, la RODEX, l'entrepôt de guilde et l'évolution des familiers.

## 1. Contenu principal (23 décembre 2013)

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Laboratoire central** | `1@lab` | `CentralLaboratory.txt` |
| **Dernière salle** (*Last Room*) | `1@uns` | `LastRoom.txt` |

## 2. Contenu de l'intervalle (2014)

### Instance

| Date kRO | Instance | Carte |
|---|---|---|
| 2014‑10‑28 | **Espace Infini** (*Infinite Space*) | `1@infi` |

### Systèmes — le vrai contenu de l'épisode

| Date kRO | Système | Pertinent en pré-renewal ? |
|---|---|---|
| 2014‑01‑08 | **Nouvelle carte du monde** | oui |
| 2014‑01‑22 | Historique d'échoppe (journal achat/vente) | oui |
| 2014‑03‑12 | **Para Market** (halle du Groupe Eden) | oui |
| 2014‑04‑16 | Refonte des invocations et de l'Homunculus S | non |
| 2014‑08‑06 | **EXP des monstres augmentée** (base +75 %, job +100 %) | non — rééquilibrage renewal |
| 2014‑08‑06 | Ajustement HP/ATK des monstres | non |
| 2014‑09‑16 | Objets WoE:TE | à arbitrer |
| 2014‑10‑07 | **Roulette de la chance** | oui |
| 2014‑10‑07 | **Évolution des familiers** | oui |
| 2014‑11‑05 | Système de clan (2ᵉ partie) | oui |
| 2014‑11‑11 | **RODEX** (refonte du courrier) | oui |
| — | **Entrepôt de guilde** | oui |
| — | **Système de navigation** | oui |
| — | Refonte de l'EXP de job | non |

## 3. État côté rAthena

`npc/re/quests/quests_15_2.txt`,
`npc/re/instances/{CentralLaboratory,LastRoom,InfiniteSpace}.txt`.
`InfiniteSpace.txt` porte la mention `[Walkthrough Conversion] — Infinite Space with hard
mode (Episode 16.1)` : rAthena le rattache donc plutôt au 16.1, alors que kRO l'a livré
dans l'intervalle 15.2. Les deux lectures se défendent ; il est traité ici.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Quêtes 15.2 | `moon/rathena/quests/quests_15_2.txt` | ✅ |
| Laboratoire central | `moon/instances/CentralLaboratory.npc` | ✅ |
| Dernière salle | `moon/instances/LastRoom.npc` | ✅ |
| Navigation | `moon/rathena/guides/navigation.txt` | ✅ |
| Entrepôt de guilde | `moon/archive_gstorage.npc` | ✅ |
| RODEX | intégrée au serveur | ✅ |
| Clans | `moon/rathena/other/clans.txt` | ✅ |

### Non porté

- **Espace Infini** — `moon/instances/InfiniteSpace.txt` est présent mais **non
  référencé**. L'entrée `Infinite Space` (`1@infi`) manque à
  `db/pre-re/instance_db.yml`. La carte `1@infi` **est** dans le mapcache pré-renewal.
  Reste à faire : ajouter l'entrée `instance_db`, référencer le script, vérifier le
  roster de mobs (rAthena a ajouté les monstres d'Infinite Space en 2022, PR #6491).
- **Roulette de la chance**, **Para Market**, **évolution des familiers** — à vérifier au
  cas par cas ; ce sont des systèmes serveur, pas du contenu de carte.

## 5. Verdict

| | |
|---|---|
| Intérêt | moyen pour les cartes, **élevé** pour les systèmes (déjà acquis) |
| Reste à faire | brancher **Espace Infini** — une entrée `instance_db` + une ligne de conf + contrôle du roster |

## Sources

- [Episode XV.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XV.II)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=143)
