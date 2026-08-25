# Épisode 14.2 — Eclage

| | |
|---|---|
| Nom kRO | 에클라주 / *Eclage* |
| Date kRO | **2011‑12‑21** |
| Arc | *Alfheim, The Fairy Land* |
| Niveau kRO visé | 130‑160 |
| État Moonlight | **porté et actif**, sauf le mode Vague |

La capitale des Laphine. La sortie initiale se résume à la ville ; comme pour 14.1, c'est
l'intervalle (2012) qui apporte le gros du contenu — dont **Old Glast Heim**, sans doute
l'instance la plus jouée de tout le renewal.

## 1. Contenu principal (21 décembre 2011)

| Carte | Nom |
|---|---|
| `eclage` | Eclage, cité des fées |
| `ecl_in01`→`04` | Intérieurs |
| `ecl_hub01` | Place centrale |
| `ecl_fild01` | Champ d'Eclage |
| `ecl_tdun01`→`04` | **Tour de Bifrost**, étages 1 à 4 |
| `1@ecl` | Intérieur d'Eclage, instance |

Mobs : Cenere, Antique Book, Lichtern (bleu/vert/jaune/rouge), Faceworm, Naga,
Draco, Pinguicula, **Kraken** et **Bacsojin** en écho.

> La Tour de Bifrost est ici, pas à l'épisode 14.1 malgré son nom.

## 2. Contenu de l'intervalle (2012)

### Instances

| Date kRO | Instance | Carte | Fichier rAthena |
|---|---|---|---|
| 2012‑05‑30 | **Old Glast Heim** | `1@gl_k` | `OldGlastHeim.txt` |
| 2012‑08‑22 | Nid du Faceworm | `1@face` | `FacewormsNest.txt` |
| 2012‑08‑22 | Mémoire de Sarah | `1@sara` | `SaraMemory.txt` |
| 2012‑08‑22 | **Mode Vague** (*Wave Mode*) | `1@def01`, `1@def02` | `WaveMode.txt` |
| 2012‑10‑17 | Tour du Diable | `1@tnm1` | `DevilTower.txt` |
| 2012‑10‑17 | Tournoi de Magie de Geffen | `1@gef` | (dans `GeffenMagicTournament`) |
| 2012‑10‑17 | Chevalier Maudit | — | (variante du Tournoi) |
| 2012‑12‑18 | **Usine de Jouets d'Horreur** | `1@xm_d` | `HorrorToyFactory.txt` |

Les trois premières forment les **Traces des Héros — Épisode I**, les trois suivantes
l'**Épisode II**.

### Donjons

- **Glast Heim mode Cauchemar** (`gl_chyard_`, `gl_cas02_`).

### Systèmes

| Date kRO | Système | Pertinent en pré-renewal ? |
|---|---|---|
| 2011‑12‑27 | File d'attente des Champs de bataille | oui |
| 2012‑02‑08 | Nouveaux chariots Mechanic/Genetic | non |
| 2012‑03‑21 | **Flûte du faucon** (*Falcon Flute*) | oui |
| 2012‑03‑28 | **Système de navigation** | oui — déjà en place côté Bourgeon |
| 2012‑03‑28 | **Nouvelle Izlude + Académie Criatura** | à arbitrer |
| 2012‑04‑04 | **Barres de vie sur les monstres** | oui, côté client |
| 2012‑04‑25 | **WoE : Édition d'entraînement** (WoE:TE) | à arbitrer |
| 2012‑04‑25 | Quête de transcendance sans coût | oui |
| 2012‑06‑13 | Ombres des monstres | oui, côté client |
| 2012‑09‑19 | **Monstres Champions** | oui |
| 2012‑12‑18 | **Système d'ombres** (*Shadow gear*) | à arbitrer |
| — | Synthèse de cristaux du Groupe Eden | oui |
| — | Plafond de niveau 150 → 160/50 | non |

## 3. État côté rAthena

`npc/re/cities/eclage.txt`, `npc/re/quests/quests_eclage.txt`,
`npc/re/mobs/dungeons/ecl_tdun.txt` (relevé jRO via Auriga, corrigé sur les données de
navigation), et les huit scripts d'instance cités plus haut.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

**Porté et actif**, à une exception près.

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Eclage, ville et quêtes | `moon/rathena/cities/eclage.txt`, `quests_eclage.txt`, `guides_eclage.txt` | ✅ |
| Tour de Bifrost | `moon/mobs/eclage.npc` — `ecl_tdun01`→`04`, `ecl_fild01` | ✅ |
| Intérieur d'Eclage | `moon/instances/EclageInterior.npc` | ✅ |
| Old Glast Heim | `moon/instances/OldGlastHeim.npc` + `OldGlastHeim_merchants.txt` | ✅ |
| Nid du Faceworm | `moon/instances/FacewormsNest.npc` | ✅ |
| Mémoire de Sarah | `moon/instances/SaraMemory.npc` | ✅ |
| Tour du Diable | `moon/instances/DevilTower.npc` | ✅ |
| Tournoi de Geffen | `moon/instances/GeffenMagicTournament.npc` | ✅ |
| Usine de Jouets d'Horreur | `moon/instances/HorrorToyFactory.npc` + marchands | ✅ |
| Glast Heim Cauchemar | `moon/rathena/dungeons/glastheim.npc` (`gl_chyard_`, `gl_cas02_`) | ✅ |
| Monstres Champions | `moon/mobs/championmobs.npc` | ✅ |
| Navigation | `moon/rathena/guides/navigation.txt` | ✅ |

### Non porté

- **Mode Vague** — `moon/instances/WaveMode.txt` est présent mais **non référencé** dans
  `moon/scripts_moon.conf`. Les deux entrées d'instance existent déjà
  (`Wave Mode - Forest` id 22 et `Wave Mode - Sky` id 23 dans
  `db/import/instance_db.yml`), et les cartes `1@def01`/`1@def02` sont dans le
  mapcache : **il ne manque littéralement qu'une ligne de configuration**.
  C'est le seul manque PvE de l'épisode.
- **Académie Criatura / nouvelle Izlude** — `moon/rathena/jobs/novice/academy.txt` est
  **commenté** (`scripts_moon.conf:115`). Choix assumé : Moonlight garde son propre
  parcours de départ.
- **WoE:TE** et **système d'ombres** — non portés ; ce sont des chantiers d'équilibrage,
  pas du contenu.

## 5. Verdict

| | |
|---|---|
| Intérêt | très élevé — Old Glast Heim et l'Usine de Jouets sont du contenu de groupe durable |
| Reste à faire | **une ligne** : référencer `WaveMode` dans `scripts_moon.conf` |

## Sources

- [Episode XIV.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIV.II)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=104)
