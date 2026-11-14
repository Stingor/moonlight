# Épisode 18 — Direction de la prière (*Direction of Prayer*)

| | |
|---|---|
| Nom kRO | 기도의 방향 / *Direction of Prayer* |
| Date kRO | **2021‑02‑01** (annonce `seq=259` ; certaines sources donnent le 3 février) |
| Arc | Arunafeltz — les Enfants Gris |
| Niveau kRO visé | 170+ (200+ pour le mode avancé) |
| État Moonlight | **lot A porté le 2026‑11‑14 (terrain, sans accès en attendant la chaîne) ; chaîne, instances et économie absentes** |

Retour en Arunafeltz. Le scénario porte sur la tension entre missionnaires de Freyja et
natifs, et se conclut sur une fausse déesse fabriquée par magie d'illusion. Dernier
épisode que rAthena convertit — au-delà, l'amont s'arrête.

## 1. Contenu principal (1ᵉʳ février 2021)

### Ville, champs et donjon

| Carte | Nom |
|---|---|
| `wolfvill` | **Village du Loup Gris** |
| `gw_fild01`, `gw_fild02` | Forêt du Loup Gris |
| `oz_dun01`, `oz_dun02` | **Labyrinthe d'Oz**, étages 1 et 2 |
| `ra_fild10`, `ra_fild11` | Gorge d'Oz et Plaines d'Ida (Rachel) |

Mobs : Ashhopper, Ashring, Grey Wolf, Bushring, Firewind Kite, Phantom Wolf, Rake Hand,
Ash Toad, Spark, Hot Molar, Volcaring, Lava Toad, **Burning Fang** (MVP), Grey Baby Wolf,
Grey Goat.

### Instances

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Purification du Sanctuaire** | `1@nyr` | (dans `Wolves.txt` / `MazeofOz.txt`) |
| **Lieu de Rassemblement des Loups** | `1@nyr` | `Wolves.txt` |
| **Villa de la Tromperie** — normal et avancé | `1@advs` | `VillaofDeception.txt` |
| **Villa du Grand Prêtre** | `1@adv` | `VillaofHighPriest.txt` |
| **Labyrinthe d'Oz** | `1@oz` | `MazeofOz.txt` |
| **Base militaire de Thor** | `1@tcamp` | `ThorGunsuBase.txt` |

### Systèmes

- **Système d'enchantement du Loup Gris** (PNJ Emmett) ;
- **Réputation du Village du Loup Gris** — cinq paliers débloquant entrepôt,
  téléportation, boîtes de raffinage et bonus de quête ;
- monnaie : **Fragments d'Améthyste**.

## 2. Contenu de l'intervalle (2021)

### Donjons

- **Illusion of Twins** (`ant_d02_i`) — cf. [16.2](16-2_terra_gloria.md)
- **Biosphère de Varmundt**
- **Tour de l'Horloge : sous-sol inconnu**

### Instances

- **Tour Engloutie** (`1@ch_u`)
- **Tour des Constellations** mise à jour (`1@ch_t`)

### Classes

Les **sept 4ᵉ classes étendues** : Shinkiro, Shiranui, Night Watch, Sky Emperor,
Soul Ascetic, Hyper Novice, **Spirit Handler**. Hors périmètre.

> La **Forêt Profonde** (`1@exsh`, `DeepForest.txt`) est l'instance de quête de job du
> Spirit Handler : elle appartient à ce bloc et n'a pas de sens sans la classe.

## 3. État côté rAthena

Conversion par la PR [#7917 « Initial release of episode 18.1 »](https://github.com/rathena/rathena/pull/7917)
(avril 2024), corrigée en 2024‑2025 (dialogues, quête quotidienne de Mejai, points de
réputation de Budan).

`npc/re/quests/quests_18.txt`, `npc/re/mobs/dungeons/oz_dun.txt`,
`npc/re/instances/{MazeofOz,ThorGunsuBase,VillaofDeception,VillaofHighPriest,Wolves,DeepForest}.txt`,
`db/re/reputation.yml`.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

**Tout est copié, rien n'est actif.**

| Élément | Fichier Moonlight | État |
|---|---|---|
| Village et forêt du Loup Gris | `moon/rathena/fields/gw_fild.npc` — 8 lignes sur `wolfvill`, `gw_fild01/02` | **commenté** (`scripts_moon.conf:747`) |
| Labyrinthe d'Oz | `moon/rathena/dungeons/oz_dun.npc` — 8 lignes sur `oz_dun01/02` | **commenté** (`scripts_moon.conf:782`) |
| Labyrinthe d'Oz (instance) | `moon/instances/MazeofOz.txt` | non référencé |
| Base militaire de Thor | `moon/instances/ThorGunsuBase.txt` | non référencé |
| Villa de la Tromperie | `moon/instances/VillaofDeception.txt` | non référencé |
| Villa du Grand Prêtre | `moon/instances/VillaofHighPriest.txt` | non référencé |
| Rassemblement des Loups | `moon/instances/Wolves.txt` | non référencé |
| Forêt Profonde | `moon/instances/DeepForest.txt` | non référencé |
| Quêtes 18 | — | **absent** |

### Les mobs manquants — la liste complète

Treize identifiants, tous absents du roster de 2 117 entrées :

| Fichier de spawn | Mobs manquants |
|---|---|
| `gw_fild.npc` | `EP18_ASHHOPPER`, `EP18_ASHRING`, `EP18_GREY_WOLF`, `EP18_TUMBLE_RING`, `EP18_FIREWIND_KITE`, `EP18_PHANTOM_WOLF` |
| `oz_dun.npc` | `EP18_ASH_TOAD`, `EP18_RAKEHAND`, `EP18_SPARK`, `EP18_HOT_MOLAR`, `EP18_VOLCARING`, `EP18_LAVA_TOAD`, `EP18_BURNING_FANG` (MVP) |

Toutes les cartes (`wolfvill`, `gw_fild01/02`, `oz_dun01/02`, `ra_fild10/11`, `1@nyr`,
`1@adv`, `1@advs`, `1@oz`, `1@tcamp`, `1@exsh`) **sont** dans le mapcache.

## 5. Migration pré-renewal

Chemin identique à celui de [17.1](17-1_illusion.md), avec un volume légèrement supérieur :

1. verser les **13 mobs `EP18_*`** dans `db/import/mobs/` (`gw_fild.yml`, `oz_dun.yml`) ;
2. décommenter deux lignes de `scripts_moon.conf` (747 et 782) ;
3. ajouter **six entrées** à `db/import/instance_db.yml` (base réellement chargée) ;
4. brancher cinq scripts d'instance (laisser `DeepForest`, lié au Spirit Handler) ;
5. arbitrer le **système de réputation** du Loup Gris : Moonlight n'utilise pas
   `reputation.yml` en pré-renewal, il faut soit l'activer, soit remplacer les paliers
   par une progression maison ;
6. convertir l'économie des **Fragments d'Améthyste**.

Le contenu de terrain (un village, deux champs, un donjon à deux étages, un MVP) est
autonome : il peut être ouvert **sans** la chaîne de quêtes, contrairement aux instances
qui en dépendent pour leur déverrouillage.

## 5b. Relevé du 2026‑11‑13 et lot A (2026‑11‑14)

Relevé avant portage : 38 mobs absents (15 de terrain, 23 d'instance et de quête), 41 objets
(15 cartes, 11 pièces `Gray_W_*`, 3 tickets ou coffrets, 2 coiffes, 11 Etc), 171 quêtes sur 172,
7 entrées `instance_db` (67 à 73 chez rAthena). La réputation (`db/re/reputation.yml`, Grey Wolf
Village) n'est pas chargée en pré-renewal (`Mode: Renewal`) mais le C++ et moonclient la gèrent :
il suffira de recopier l'entrée. L'enchanteur Emmett n'a de script nulle part.

**Le terrain n'a aucun warp dans le monde** : on entre au village et au labyrinthe par les PNJ de
la chaîne, ou en sortant de certaines instances.

| Partie | Contenu | Fichiers |
|---|---|---|
| Mobs | 6 de la Forêt du Loup Gris et 7 du Labyrinthe d'Oz (niveau 177 à 212), `re_to_prere_mob.py` sans retouche, `DamageTaken` retiré ; Burning Fang 4,8 M de PV ; 80 lignes de skills | `db/import/mobs/ep18_fields.yml` |
| Cartes | 13 cartes ; Ashring : palier `.@r>=14` en `getallrefine() >= 28` ; Rakehand : `bDef,50` en 5 | `db/import/items/item_db_card.yml` |
| Spawns | `gw_fild.npc` et `oz_dun.npc` décommentés ; aucun drop de carte chez l'amont | `moon/scripts_moon.conf` |
| Mapflags | `nomemo` et `nowarpto` sur `oz_dun01` et `oz_dun02` ; ceux du village viendront avec la chaîne | `moon/mapflag/` |
| Branches, albums | 12 mobs en Dead Branch, Burning Fang en Bloody Branch, cartes à l'album, carte MVP en `mca` et `mvpenchant` ; Burning Fang à 6 points MVP | |
| Accès | **aucune entrée au warp agent** tant que la chaîne n'est pas portée (choix du 2026‑11‑14) | |

## 6. Verdict

| | |
|---|---|
| Intérêt | élevé — un village complet, un MVP, six instances |
| Fait | lot A : le terrain (2026‑11‑14, non validé en jeu, sans accès) |
| Reste à faire | **B** : la chaîne `quests_18.txt` (171 quêtes) et la réputation, puis l'accès au warp agent · **B1** : 6 instances et leurs 23 mobs · **B3** : l'équipement du Loup Gris et ses trocs · hors périmètre : la Forêt Profonde (Spirit Handler), l'enchanteur Emmett (sans script) |

## Sources

- [Episode XVIII — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVIII)
- [Episode 18: Direction of Prayer — Hazy Forest](https://hazyforest.com/episodes:18_direction_of_prayer)
- [Direction of Prayer — iRO Wiki](https://irowiki.org/wiki/Direction_of_Prayer)
- [rathena/rathena PR #7917](https://github.com/rathena/rathena/pull/7917)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=259)
