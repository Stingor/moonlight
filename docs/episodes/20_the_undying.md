# Épisode 20 — L'Immortel (*The Undying*)

| | |
|---|---|
| Nom kRO | *The Undying* |
| Date kRO | **2023‑03‑02** |
| Arc | fin de la traque de Bagot — le labyrinthe du serpent |
| Niveau kRO visé | **215+** |
| État Moonlight | **à porter** : une conversion complète existe (brHades, par Capuche) ; rien n'est chargé |

Suite directe d'Issgard : on descend dans le canyon de glace ancien, on démasque les
infiltrés (les faux soldats Iwin), on cherche l'entrée du repaire du serpent avant qu'un
pouvoir immortel ne soit libéré. L'Alliance du Chat Blanc remplace Issgard comme faction.

## 1. Contenu

### Cartes

| Carte | Nom |
|---|---|
| `jor_back4` à `jor_back6` | **Canyon de Glace Ancien** |
| `jor_maze` | **Labyrinthe du Serpent** |
| `jor_root1`, `jor_root2` | Racines |
| `jor_sanct` | Sanctuaire |
| `jor_twig`, `jor_twice` | Branches de l'Arbre-Monde |
| `jor_safty1`, `jor_safty2`, `jor_safy1` | Zones sûres |

### Instances (`instance_db` de brHades, Id 300 à 306)

| Instance | Carte d'entrée | Durée | Nature |
|---|---|---|---|
| **Sticky Sea** (Mer Collante) | `1@slug` | 30 min | histoire |
| **Canyon Exploration** | `1@20cn1` + `1@20cn2` | — | histoire, deux cartes |
| **Drift Ice Zone** | `1@twsd` | — | histoire |
| **Nest of Twigs** | `1@twig` | 20 min | journalière |
| **Separated Sanctuary** | `1@twas` | 30 min | journalière |
| **Immortal** / **Immortal (Hard)** | `1@twbs` | 30 min | boss Lasgand, mode difficile |

### Mécaniques

Les **Parchemins de transformation Rgan** de l'épisode 19 restent nécessaires. La chaîne
principale exige l'épisode 19 achevé et la journalière de Horr, dont le PNJ est remplacé
par le script du 20. Niveau 215 pour les instances.

### Contenu rattaché

- **Hall of Life** (`for_dun01`) et **Jardin du Temps** (`t_garden`), sortis en kRO le
  10 juillet 2023 et rattachés à l'épisode 20 par rAthena
  ([rathena/rathena#8214](https://github.com/rathena/rathena/issues/8214)) : convertis en
  amont (`npc/re/quests/garden_of_time.txt`, `npc/re/mobs/dungeons/for_dun.txt`). Chantier
  séparé, voir § 5.

## 2. Sources

### brHades — la conversion de référence

[glitzhXEC/brHades](https://github.com/glitzhXEC/brHades) (GPL-3.0), commit
[`c2196f9be`](https://github.com/glitzhXEC/brHades/commit/c2196f9be) « ep20 » du
2024‑11‑04, 37 fichiers, environ 21 000 lignes. Les scripts sont signés **Capuche**
(développeur rAthena), au format « Walkthrough Conversion » de la PR de l'épisode 19
([rathena/rathena#8527](https://github.com/rathena/rathena/pull/8527)), dont Issgard est
issu. Aucune PR amont pour le 20 au 2026‑11‑17. Le même dépôt a aussi l'épisode 21
(`a9ebedab8`, 2025‑12‑09).

| Élément | Contenu |
|---|---|
| Instances | 6 scripts dans `npc/re/ep20/instances/` (5 300 lignes) |
| Quêtes | `quests_20_main.txt` (7 700 lignes, variable `ep20_main` jusqu'à 100), `quests_20_daily.txt` (4 900), `quests_20_side_quests.txt` (1 700), `quests_20_misc.txt` (570) |
| Terrain | `ep20_spawn.txt` (`jor_back4` à `jor_root`, MVP Snow Angel), `ep20_warps.txt`, `mapflags.txt` |
| Économie | 2 fichiers de barters, `map_drops.yml` (+207 : minerais, cristaux et extraits EP19/EP20), `laphine_upgrade.yml`, prix d'objets |
| `mob_db` | **40 mobs** écrits en entier ; `db/re` amont n'a que des lignes commentées pour ces Id (21965 à 22007, 23060) |
| `mob_skill_db` | 163 lignes |
| `quest_db` | **159 quêtes** (Id 8839 à 23118), absentes de `db/re` amont |
| Réputation | `REPU_EP20` = 4, **White cat alliance** (`RepPointsWhiteCatAlliance`) |
| C++ | `specialeffect3`, `UMOB_BODYSIZE` / `UNPC_BODYSIZE` (`clif_body_size`), `UMOB_MATK` (renewal seul) |
| Manque connu | `NPC_SEEDTRAP` non implémentée (noté dans `Immortal.txt`) |

### patnawa/rathena_pn — repli

`npc/custom/episode20/` : `Fields.txt`, `Instances.txt`, `NavigationConnections.txt`,
`Progression.txt`, `SidesAndDailies.txt`. Réécriture « clean-room », beaucoup plus courte
(le Canyon tient en trois vagues). À ne consulter que pour un trou de brHades.

## 3. État côté Moonlight (mesuré le 2026‑11‑17)

| Élément | État |
|---|---|
| Cartes | les 17 sont dans le map_cache, instances comprises (`1@slug`, `1@20cn1/2`, `1@twsd`, `1@twig`, `1@twas`, `1@twbs`) |
| Épisode 19 | porté (`moon/rathena/quests/quests_19.npc`, `ep19_main`, Horr) : c'est le prérequis |
| Mobs EP20 | **lot A fait** (2026‑11‑17) : voir ci-dessous |
| Quêtes, instances, spawns | **rien** |
| C++ | ni `specialeffect3` ni `UMOB_BODYSIZE` |
| Hall of Life | `moon/rathena/dungeons/for_dun.npc` toujours **commenté** (`scripts_moon.conf:836`), 32 mobs `SPIRIT_*` à écrire |

### Lot A — mobs et compétences (porté le 2026‑11‑17, non validé en jeu)

| Élément | Détail |
|---|---|
| Fichier | `db/import/mobs/ep20.yml` (40 mobs, Id 21965 à 22007 et 23060), dans le Footer de `db/import/mob_db.yml` |
| Base | brHades : stats, vitesses, IA, modes, classe, drops |
| Conversion | `re_to_prere_mob.py` sans retouche, ATK inchangée ; `Resistance`, `MagicResistance` et `DamageTaken` retirés |
| Ordres de grandeur | mob normal ≈ 510 000 à 590 000 PV et 820 000 à 940 000 EXP ; MVP 4,5 à 6,3 M PV ; Lasgand d'instance 21,6 M |
| Compléments Divine Pride | EXP de Snowstorm Angel (21996) et du Chef Nettoyeur (21980), DEF de Snowstorm Angel, drops des quatre MVP et des deux pierres de Lasgand (22004, 22005), `BaseExp` de Jormungand Guardian-Y absente de brHades |
| Drops écartés | les 20 cartes EP20 (bonus renewal à convertir, comme au 19), les 8 armes Dim Glacier, `Bs_Making_S` (absent chez nous) |
| Matériaux | 26 objets `Ep20_*` / `EP20_*` (1001226 à 1001251) dans `db/import/items/item_db_etc.yml` : entrée `db/re`, prix de brHades, nom anglais du client (« Irwin » corrigé en Iwin, grades alignés sur le 19) |
| Compétences | 163 lignes de brHades en fin de `db/import/mob_skill_db.txt` (+ 2 commentées de la source) |
| `skill_db` | `NPC_LEASH` (747) et `NPC_WIDELEASH` (748) copiés de `db/re` (Fallen Angel Slug) : implémentés en C++ (`skills/npc/leash.cpp`, `wideleash.cpp`), absents du pré-re |
| Audits | `audit_mob_skills.py` et `audit_status.py` : 0 problème |
| Sprites | `EP20_NPC_NYAR`, `EP20_NPC_LASGAND` : sprites de PNJ |

**Reporté aux lots suivants.** Immortal retouche Lasgand par `setunitdata` : DEF 527 et 700,
RES, MATK. En pré-renewal, une DEF au-dessus de 100 rend le boss quasi invulnérable, à adapter
au lot F. `NPC_SEEDTRAP`, `NPC_RAINOFMETEOR` et `NPC_GROUNDDRIVE` y sont lancés par script.
Les branches (Dead / Bloody) n'ont rien reçu : la liste est à valider, et l'épisode 19 n'y est
pas non plus.

## 4. Plan de portage

Mêmes règles que pour le 19 (checklist de portage de contenu renewal → pre-re). Chaque
lot se valide au boot du serveur avant le suivant.

| Lot | Contenu | Points d'attention |
|---|---|---|
| **A — Mobs** | 40 mobs dans `db/import/mobs/ep20.yml` via `re_to_prere_mob.py`, 163 lignes de `mob_skill_db` | audit des compétences pre-re (`audit_mob_skills.py`) ; `NPC_SEEDTRAP` à remplacer ou à écrire ; `UMOB_MATK` sans effet en pre-re ; Snow Angel et Lasgand rattachés aux albums |
| **B — C++** | `specialeffect3`, `UMOB_BODYSIZE`, `UNPC_BODYSIZE` | `git diff upstream/master` sur chaque fichier avant de patcher ; recompilation |
| **C — Terrain** | spawns, portails, mapflags | mapflags relus contre les pièges connus (`nobranch` inversé) ; Warp Agent |
| **D — Objets et économie** | objets EP20 de `db/re` vers `db/import`, `map_drops`, barters, Laphine, réputation White cat alliance | constante `REPU_EP20` au boot ; table `reputation` de moonclient ; prix de boutique ≥ 1,65 × revente |
| **E — Quêtes** | 159 entrées `quest_db` ; `quests_20_main`, `_daily`, `_side_quests`, `_misc` | remplacement du PNJ Horr de `quests_19.npc` ; 586 `cutin` (images à vérifier côté client) ; latin-1 |
| **F — Instances** | 7 entrées `instance_db`, 6 scripts | Id libres côté Moonlight ; Warp Agent (enum généré depuis `instance_db.yml`) ; hard mode |
| **G — Client** | tables moonclient (quêtes, réputation, objets) | — |

## 5. Hall of Life

Toujours à part : le donjon d'élémentaires est copié et commenté, il manque ses 32 mobs
`SPIRIT_*` (huit familles, quatre tailles). Indépendant de la chaîne narrative, il peut
passer avant ou après les lots ci-dessus.

## 6. Verdict

| | |
|---|---|
| Intérêt | **élevé** — la suite directe d'Issgard, complète, du même auteur que la conversion du 19 |
| Reste à faire | tout : lots A à G, puis le Hall of Life |

## Sources

- [glitzhXEC/brHades, commit c2196f9be « ep20 »](https://github.com/glitzhXEC/brHades/commit/c2196f9be)
- [patnawa/rathena_pn, `npc/custom/episode20`](https://github.com/patnawa/rathena_pn/tree/main/npc/custom/episode20)
- [Episode XX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XX)
- [Undying — iRO Wiki](https://irowiki.org/wiki/Undying)
- [Garden of Time — iRO Wiki](https://irowiki.org/wiki/Garden_of_Time)
- [rathena/rathena issue #8214](https://github.com/rathena/rathena/issues/8214)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=277)
