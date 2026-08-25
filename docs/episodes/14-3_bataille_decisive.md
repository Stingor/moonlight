# Épisode 14.3 — Bataille décisive (*Decisive Battle*)

| | |
|---|---|
| Nom kRO | *Decisive Battle* — aussi appelé **Flame Basin** |
| Date kRO | **partie 1 : 2012‑12‑26** · **partie 2 : 2013‑03‑19** |
| Arc | fin de *Alfheim* / confrontation avec Morroc |
| Niveau kRO visé | 140‑175 |
| État Moonlight | **porté et actif**, sauf le Temple du Dieu Démon |

Épisode livré en deux temps, chacun petit. La partie 2 fait passer le plafond à 175/60,
ce qui est le vrai marqueur : à partir d'ici, le contenu kRO cesse d'être transposable
sans rééchelonnage.

## 1. Contenu

### Partie 1 (26 décembre 2012)

| Type | Contenu | Carte |
|---|---|---|
| Donjon | **Bassin de Flammes** (*Flame Basin* / Volcan de Morroc) | `moro_vol` |
| Instance | **Île de Bios** | `1@dth1` |
| Instance | **Grotte de Morse** | `1@rev` |
| Donjon | Tour de l'Horloge mode Cauchemar (2013‑03‑13) | `c_tower*` |
| Système | Système de clan — première partie | — |

### Partie 2 (19 mars 2013)

| Type | Contenu | Carte |
|---|---|---|
| Instance | **Jitterbug cauchemardesque** | `1@jtb` |
| Instance | **Temple du Dieu Démon** | `1@eom` |
| Système | Plafond **175 / 60** | — |
| Système | Nouvelles compétences de 3ᵉ classe | — |

### Systèmes de l'intervalle

- 2013‑02‑20 — comparaison d'équipement ;
- 2013‑05‑22 — prix de vente maximum en échoppe porté à 1 milliard de zeny ;
- 2013‑06‑12 — **Banque** (Ctrl+B, zeny à l'échelle du compte) ;
- 2013‑06‑26 — **système de clan** (Masse d'or, Épée, Arbalète, Bâton).

## 2. État côté rAthena

`npc/re/quests/quests_14_3.txt` et `quests_14_3_bis.txt`,
`npc/re/mobs/dungeons/moro_vol.txt`,
`npc/re/instances/{IsleOfBios,MorseCave,NightmarishJitterbug,TempleOfDemonGod}.txt`,
`npc/re/other/clans.txt`.

Le commentaire d'en-tête de `TempleOfDemonGod.txt` le rattache explicitement à
l'épisode 14.3.

## 3. État côté Moonlight (mesuré le 2026‑08‑25)

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Quêtes 14.3 | `moon/rathena/quests/quests_14_3.txt`, `quests_14_3_bis.txt` | ✅ |
| Île de Bios | `moon/instances/IsleOfBios.npc` | ✅ |
| Grotte de Morse | `moon/instances/MorseCave.npc` | ✅ |
| Jitterbug | `moon/instances/NightmarishJitterbug.npc` | ✅ |
| Tour de l'Horloge Cauchemar | `moon/rathena/dungeons/c_tower.npc`, `db/import/mobs/c_tower_n.yml` | ✅ |
| Clans | `moon/rathena/other/clans.txt` | ✅ |
| Banque | intégrée au serveur (`bank_zeny`) | ✅ |
| **Bassin de Flammes** | `moon/mobs/morocc.npc` — 11 lignes de spawn sur `moro_vol` | ✅ |

Les huit mobs du Bassin de Flammes (Fire Bug, Fire Condor, Fire Frilldora, Fire Golem,
Fire Pit, Fire Sandman, Sonia et **Incarnation of Morocc** en MVP) sont bien dans le
roster pré-renewal.

### Non porté

- **Temple du Dieu Démon** — `moon/instances/TempleofDemonGod.npc` existe **et n'est pas
  référencé** dans `moon/scripts_moon.conf`. L'entrée `Temple of the Demon God`
  (`1@eom`) est pourtant déjà dans `db/pre-re/instance_db.yml`. Il ne manque donc
  littéralement **qu'une ligne de conf** — la non-inscription est peut-être délibérée,
  à confirmer avant de la corriger.

## 4. Migration pré-renewal

Rien de structurel : l'épisode est absorbé. La seule question ouverte est le niveau de
difficulté du Bassin de Flammes, conçu côté kRO pour du 150+.

## 5. Verdict

| | |
|---|---|
| Intérêt | moyen — l'essentiel est déjà en place |
| Reste à faire | **une ligne** : décider du sort du Temple du Dieu Démon |

## Sources

- [Episode XIV.III — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIV.III)
  (les deux annonces kRO, `seq=133` et `seq=134`)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
