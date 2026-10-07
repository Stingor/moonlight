# Épisode 16.1 — Banquet des Héros (*Banquet of Heroes* / *Royal Banquet*)

| | |
|---|---|
| Nom kRO | 영웅들의 연회 / *Banquet of Heroes* |
| Date kRO | **2015‑02‑25** |
| Arc | Terra Gloria — première partie |
| Niveau kRO visé | 140‑175 |
| État Moonlight | **porté**, hors classe Summoner (depuis le 2026‑09‑05) |

Deux instances à la sortie, puis un intervalle chargé : la race **Doram** et sa ville
**Lasagna**, la refonte de Prontera et du Groupe Eden, les **hauts faits et titres**.

## 1. Contenu principal (25 février 2015)

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Salle de la Conscience** | `1@mir` | `RoomOfConsciousness.txt` |
| **Forteresse Céleste** (*Sky Fortress*) | `1@sthb` | `SkyFortress.txt` |
| **Rituel de Bénédiction** | `2@mir` | `RitualOfBlessing.txt` |

Ajouts joints : jetons d'honneur, nouvel objet d'enchantement, préparatifs du banquet.

## 2. Contenu de l'intervalle (2015‑2016)

### Classes et races

- **Summoner**, fondé sur la nouvelle race **Doram** — classe renewal, hors périmètre
  pré-renewal.
- Compétence *Change Cart 2* ajoutée à la branche Merchant.

### Villes et zones

| Contenu | Cartes |
|---|---|
| **Lasagna**, village Doram | `lasagna`, `lasa_fild01/02`, `lasa_in01`, `lasa_sea` |
| **Nid du Basilic** (donjon de Lasagna) | `lasa_dun01`, `lasa_dun02`, `lasa_dun03`, `lasa_dun_q` |
| Refonte du **Groupe Eden** | — |
| **Prontera rénovée** | `prontera` |

### Instances de l'intervalle

| Instance | Carte | Note |
|---|---|---|
| **Espace Infini — mode difficile** | `1@infi` | cf. [15‑2](15-2_memory_record.md) |
| **EDDA — Demi-lune en plein jour** | `1@pop1` | rattaché au 16.1 par l'en-tête rAthena |

### Systèmes

| Système | Pertinent en pré-renewal ? |
|---|---|
| **Hauts faits et titres** (puis leur refonte) | oui |
| **Enchantements de costume** | oui |
| **Lien d'objet** (*item link* en discussion) | oui — déjà côté client Bourgeon |
| **Salon de coiffure** (*styling shop*) | oui |
| Système de réputation | oui |
| Biolab Cauchemar mis à jour | oui |

## 3. État côté rAthena

`npc/re/quests/quests_16_1.txt`, `npc/re/custom/lasagna/*` (Lasagna est rangée dans
`custom` parce que la conversion est communautaire), `npc/re/other/achievements.txt`,
`npc/re/merchants/Extended_Stylist.txt`,
`npc/re/instances/{RoomOfConsciousness,SkyFortress,RitualOfBlessing,EddaHalfMoonInTheDaylight,InfiniteSpace}.txt`.

## 4. État côté Moonlight (mesuré le 2026‑10‑07)

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Quêtes 16.1 | `moon/rathena/quests/quests_16_1.txt` | ✅ |
| Salle de la Conscience | `moon/instances/RoomOfConsciousness.npc` | ✅ |
| Rituel de Bénédiction | `moon/instances/RitualOfBlessing.npc` | ✅ |
| Lasagna + Nid du Basilic | `moon/customs/lasagna/{lasagna_npcs,lasa_dun,lasa_fild,warps}.txt` | ✅ |
| Hauts faits | `moon/rathena/other/achievements.txt` + `db/import/achievement_db.yml` | ✅ |
| Salon de coiffure | `moon/stylist.npc` | ✅ *(version maison)* |
| **Forteresse Céleste** | `moon/instances/SkyFortress.npc` — `1@sthb`, id 33 | ✅ |
| **EDDA Demi-lune** | `moon/instances/EddaHalfMoonInTheDaylight.npc` + `moon/rathena/merchants/enchan_edda_half_moon.npc` — `1@pop1/2/3`, id 44 | ✅ |
| **Espace Infini** | `moon/instances/InfiniteSpace.npc` — cf. [15‑2](15-2_memory_record.md) | ✅ |

### Portés depuis le premier relevé

- **Forteresse Céleste** (`59ed6f066`) — onze mobs manquants (3475‑3485), deux
  autres (3473 boss final, 3474) qui n'étaient que des gabarits ; les treize passés
  au convertisseur pré-renewal (boss final : HP 20 M → 4,1 M). Aucun n'avait ses
  compétences : 44 lignes de `mob_skill_db` ajoutées.
- **EDDA Demi-lune** (`d22a214d0`) — dix-sept mobs 3510‑3526 cachés derrière des
  variables de mode, convertis vers `db/import/mobs/edda_halfmoon.yml` (98 lignes
  de `mob_skill`) ; cinq objets, quatre quêtes, entrée `instance_db` créée (id 44),
  mapflags des trois cartes `1@pop1/2/3` et enchanteur du Bishop Necklace.
  Les escortes du mode Hard ne marchaient pas faute de `mercenary_db` : vingt
  entrées portées depuis `db/re` (`e0c0e51e9`), ce qui répare aussi dix-sept
  parchemins de mercenaire.
- **Espace Infini** — cf. [15‑2](15-2_memory_record.md).

### Non porté

- **Summoner / Doram** — hors périmètre : classe renewal.

## 5. Migration pré-renewal

Lasagna est déjà en service, mais **sans la classe Doram** : c'est une ville et un donjon
utilisables par les classes pré-renewal, ce qui est le bon compromis. Les trois instances
de l'épisode ont été rééchelonnées par `tools/util/re_to_prere_mob.py` au portage ; les
niveaux d'entrée (140‑175 côté kRO) ne bloquent pas, le plafond réel étant
`MaxBaseLevel: 999`.

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — trois instances de groupe et un donjon complet, tous en service |
| Reste à faire | **rien** — Summoner / Doram restent hors périmètre |

## Sources

- [Episode XVI.I — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVI.I)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=164)
