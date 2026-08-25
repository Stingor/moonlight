# Épisode 15.1 — Phantasmagorika

| | |
|---|---|
| Nom kRO | 판타스마고리카 / *Phantasmagorika* |
| Date kRO | **2013‑07‑03** (annonce kRO `seq=137` ; l'encyclopédie donne le 13 juillet) |
| Arc | Verus — le laboratoire souterrain |
| Niveau kRO visé | 140‑175 |
| État Moonlight | **porté et actif** |

Ouverture de **Verus**, la ville-laboratoire, et de tout le fil narratif qui alimentera
les épisodes 15.2 et 16.x.

## 1. Contenu principal

| Carte | Nom |
|---|---|
| `verus01` | Verus — zone extérieure |
| `verus02`, `verus02_a`, `verus02_b` | Verus — quartiers |
| `verus03`, `verus04` | Verus — laboratoire |
| `un_bunker`, `un_bk_q`, `un_myst` | Bunker souterrain |
| `ver_eju`, `ver_tunn` | Tunnels de Verus |
| `1@mcd` | **Charleston en détresse** (*Charleston Crisis*), instance |

Mobs marquants : Charleston 3, Venom Bug, Chimera, R48‑Aggressive, Alnoldi, Petal,
**Amdarais** en écho, Repair Robot Turbo.

## 2. Contenu de l'intervalle (2013)

| Date kRO | Contenu | Type |
|---|---|---|
| 2013‑07‑31 | **Rebellion** (classe étendue Gunslinger) | classe |
| 2013‑08‑02 | Armes de Rebellion | objets |
| 2013‑08‑21 | **Nouveaux plafonds de HP** (99 → 330 k, 150 → 660 k, 175 → 1,1 M) | système |
| 2013‑09‑25 | **Traces des Héros — Épisode III** : *Fenrir et Sarah* (`1@glast`) et *Assaut sur l'aéronef* (`1@air1`) | instances |
| 2013‑12‑17 | **Biolab mode Cauchemar** (Tombe des Déchus / Bio 5) | donjon |

## 3. État côté rAthena

`npc/re/mobs/verus.txt`, `npc/re/quests/quests_15_1.txt`,
`npc/re/instances/{CharlestonCrisis,SarahAndFenrir,AirshipAssault}.txt`,
`npc/re/merchants/enchan_verus.txt`.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

**Porté et actif intégralement.**

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Verus | `moon/mobs/verus.npc` — `verus01/02/03`, `un_bunker`, `ver_eju`, `ver_tunn` | ✅ |
| Quêtes 15.1 | `moon/rathena/quests/quests_15_1.txt` | ✅ |
| Charleston | `moon/instances/CharlestonCrisis.npc` | ✅ |
| Fenrir et Sarah | `moon/instances/SarahAndFenrir.npc` | ✅ |
| Assaut sur l'aéronef | `moon/instances/AirshipAssault.npc` | ✅ |
| Enchantement de Verus | `moon/rathena/merchants/enchan_verus.txt` | ✅ |
| Biolab Cauchemar | `moon/customs/nightmare_biolab.npc` + `bio4_reward.npc` | ✅ *(version maison)* |
| Mobs de Verus | `db/import/mob_db.yml` | ✅ |

> Le Biolab Cauchemar de Moonlight est une **implémentation maison**
> (`moon/customs/nightmare_biolab.npc`), pas la conversion rAthena. C'est une divergence
> volontaire à connaître avant toute resynchronisation avec l'amont.

### Non porté, et ne le sera pas

- **Rebellion** — classe étendue de niveau 99+, hors périmètre du build `PRERE`.
- **Nouveaux plafonds de HP** — attachés aux niveaux 150/175.

## 5. Verdict

| | |
|---|---|
| Intérêt | élevé — Verus est une zone de haut niveau complète, déjà en service |
| Reste à faire | **rien** |

## Sources

- [Episode XV.I — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XV.I)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=137)
