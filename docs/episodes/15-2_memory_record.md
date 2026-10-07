# Épisode 15.2 — Memory Record

| | |
|---|---|
| Nom kRO | 메모리 레코드 / *Memory Record* |
| Date kRO | **2013‑12‑23** |
| Arc | suite de Verus |
| Niveau kRO visé | 150‑175 |
| État Moonlight | **porté et actif**, Infinite Space compris (depuis le 2026‑09‑05) |

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

## 4. État côté Moonlight (mesuré le 2026‑10‑07)

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Quêtes 15.2 | `moon/rathena/quests/quests_15_2.txt` | ✅ |
| Laboratoire central | `moon/instances/CentralLaboratory.npc` | ✅ |
| Dernière salle | `moon/instances/LastRoom.npc` | ✅ |
| **Espace Infini** | `moon/instances/InfiniteSpace.npc` + `moon/rathena/merchants/InfiniteSpace_merchants.npc` | ✅ |
| **Para Market** | `moon/rathena/merchants/eden_market.npc` | ✅ |
| **Évolution des familiers** | `db/import/pet_db.yml` (27 évolutions) | ✅ |
| Navigation | `moon/rathena/guides/navigation.txt` | ✅ |
| Entrepôt de guilde | `moon/archive_gstorage.npc` | ✅ |
| RODEX | intégrée au serveur | ✅ |
| Clans | `moon/rathena/other/clans.txt` | ✅ |

### Espace Infini — porté le 2026‑09‑05

Le script dormait depuis le sync upstream, identique à `upstream/master` et non
déclaré. Branché par `793d8d952`, puis corrigé par `68f980664` et `8552f26a1` :

- **Instance** : entrée `Infinite Space` (Id 38, `1@infi`, `TimeLimit: 3600`) dans
  `db/import/instance_db.yml` (41 entrées désormais), constante `INST_*` régénérée,
  accès par le Warp Agent (« Infinite Space (HM sur place) »).
- **58 mobs** (3384‑3440 + Shining Poring 3494) extraits de `db/re` vers
  `db/import/mobs/infinitespace.yml` : `Attack2` recalculé (MATK renewal → max ATK),
  puis passage au convertisseur `tools/util/re_to_prere_mob.py` (DEF ≤ 37,
  MDEF ≤ 25, HP ÷ ~2). 241 lignes ajoutées à `db/import/mob_skill_db.txt`.
  ⚠️ 11 skills dépassent leur MaxLevel (MG_FIREBALL lv43, AL_DECAGI lv48…) :
  conservés tels quels, à surveiller.
- **Objets** : les 10 armes Infinity, 10 cartes et 4 combos étaient déjà en base ;
  les cartes Infinite rejoignent les albums 12246 / 616 (`a45521aad`).
- **Script** : compteur de mobs restants façon Endless Tower, warp et coffre
  immédiats ; le Shining Poring ne referme plus la sortie (label `OnFloorClear`
  + filet `OnTimer20000`).
- **Mapflags** : les huit flags de `1@infi` portés dans `moon/mapflag/` (ils ne
  vivaient que dans `npc/re/mapflag/`, non lu). `nobranch` volontairement écarté
  (inversé sur ce fork), `nodynamicnpc` aussi (casserait `addtempnpc`).
- **En suspens** : la navigation client vers `cmd_fild07` n'est enregistrée qu'en
  (0, 0) — elle mène sur la carte, pas devant l'Exploratrice en 53,270.

### Para Market — porté le 2026‑09‑05

`npc/re/merchants/eden_market.txt` (en réalité le marché du Paradise Group) porté
par `ceb5d4936` : 68 PNJ, 22 `marketshop` à **stock fini partagé par le serveur**,
réapprovisionné à minuit avec rotation par jour de semaine. Vend notamment les
parchemins MS / SG / LoV niveau 10 et les monnaies de farm renewal (Mora, Sapha,
Splendide, Manuk) — raccourci assumé ; retirer les entrées de `para_coin10`
suffit à le couper. Accès direct par le Warp Agent, menu « Autre »
(`54be04994`) ; retour par `@load`.

### Évolution des familiers — activée le 2026‑09‑05

`db/import/pet_db.yml` n'était pas chargé (absent du Footer de `db/pet_db.yml`).
Activé par `a27ad992e`, contenu repris de `db/pre-re`, et six évolutions
décommentées : Aliot, Alicel, Nightmare Terror H, Wander Man H, Desert Wolf et
Fire Golem (œufs 9129 / 9131 ajoutés). `feature.petevolution: on`.

### Non porté

- **Roulette de la chance** — `feature.roulette: off` dans
  `conf/import/battle_conf.txt` (la table `db_roulette` est pourtant déclarée
  dans `conf/import/inter_conf.txt`). Choix de configuration, pas un manque.

## 5. Verdict

| | |
|---|---|
| Intérêt | moyen pour les cartes, **élevé** pour les systèmes (déjà acquis) |
| Reste à faire | **rien** de bloquant — surveiller les skills hors MaxLevel d'Infinite Space ; la Roulette reste désactivée par choix |

## Sources

- [Episode XV.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XV.II)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=143)
