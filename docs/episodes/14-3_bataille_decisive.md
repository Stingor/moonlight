# Épisode 14.3 — Bataille décisive (*Decisive Battle*)

| | |
|---|---|
| Nom kRO | *Decisive Battle* — aussi appelé **Flame Basin** |
| Date kRO | **partie 1 : 2012‑12‑26** · **partie 2 : 2013‑03‑19** |
| Arc | fin de *Alfheim* / confrontation avec Morroc |
| Niveau kRO visé | 140‑175 |
| État Moonlight | **porté et actif**, Temple du Dieu Démon compris (depuis le 2026‑09‑05) |

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

## 3. État côté Moonlight (mesuré le 2026‑10‑07)

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Quêtes 14.3 | `moon/rathena/quests/quests_14_3.npc`, `quests_14_3_bis.txt` | ✅ |
| Île de Bios | `moon/instances/IsleOfBios.npc` | ✅ |
| Grotte de Morse | `moon/instances/MorseCave.npc` | ✅ |
| Jitterbug | `moon/instances/NightmarishJitterbug.npc` | ✅ |
| Tour de l'Horloge Cauchemar | `moon/rathena/dungeons/c_tower.npc`, `db/import/mobs/c_tower_n.yml` | ✅ |
| Clans | `moon/rathena/other/clans.npc` | ✅ |
| Banque | intégrée au serveur (`bank_zeny`) | ✅ |
| **Bassin de Flammes** | `moon/mobs/morocc.npc` — 11 lignes de spawn sur `moro_vol` (Incarnations réduites à 20) | ✅ |
| **Temple du Dieu Démon** | `moon/instances/TempleofDemonGod.npc` — `1@eom`, id 27 | ✅ |

Les huit mobs du Bassin de Flammes (Fire Bug, Fire Condor, Fire Frilldora, Fire Golem,
Fire Pit, Fire Sandman, Sonia et **Incarnation of Morocc** en MVP) sont bien dans le
roster pré-renewal.

### Temple du Dieu Démon — porté le 2026‑09‑05

Le script était présent mais non référencé. Le brancher n'a pas suffi :

- **Quêtes** (`aad78faa0`) : les six quêtes 7596 et 7601‑7605 ne vivaient que dans
  `db/re/quest_db.yml`, jamais chargé en pré-renewal — dont la 7605 qui porte le
  cooldown de 3 h. Portées dans `db/import/`, avec les deux apôtres 3105 / 3106 que
  le générateur de `templedemon.yml` avait oubliés.
- **Accès direct** (`cb1239043`) : par le Warp Agent, le joueur arrivait sans la
  quête 7593 que pose la chaîne 14.3, et Ahat refusait le passage. La quête est
  désormais posée par `Process_Instance` à l'entrée.
- **Boss** (`965fd53e6`) : MM_BRINARANEA (3091) et MM_MUSPELLSKOLL (3092) avaient
  perdu leur `Class: Boss` au portage. Trois mobs (3088‑3090) ont aussi reçu leurs
  lignes de `mob_skill_db` (`59ed6f066`).
- **Cartes** : Ahat, Shnaim, Brinaranea, Muspellskoll et Despair God Morocc entrent
  dans les albums 12246 / 616 (`a45521aad`).

## 4. Migration pré-renewal

Rien de structurel : l'épisode est absorbé.

**Difficulté du Bassin de Flammes (2026‑11‑17).** Les mobs de feu (niveau 141 à 148,
37 000 à 50 000 PV, environ 60 000 d'EXP) sont dans la norme des zones 140‑150 du serveur
(Eclage, Devil Tower) et restent tels quels. Le point dur venait des **Incarnations de
Morocc** (1918‑1921) : classe Boss, ATK pre-re de 3 000 à 8 600, soit environ six fois les
autres mobs de la carte, et 60 exemplaires sans délai de réapparition. Elles passent à
**4 × 5 = 20** sur `moro_vol`. Leurs stats ne bougent pas, car la Dimensional Gorge
(`moc_fild22`) utilise les mêmes mobs.

## 5. Verdict

| | |
|---|---|
| Intérêt | moyen — l'essentiel est déjà en place |
| Reste à faire | **rien** — Bassin de Flammes ajusté (Incarnations de Morocc 60 → 20), à vérifier en jeu |

## Sources

- [Episode XIV.III — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIV.III)
  (les deux annonces kRO, `seq=133` et `seq=134`)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
