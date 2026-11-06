# Épisode 16.2 — Terra Gloria

| | |
|---|---|
| Nom kRO | 테라 그로리아 / *Terra Gloria* |
| Date kRO | **2016‑03‑09** |
| Arc | Terra Gloria — les Heart Hunters |
| Niveau kRO visé | 150‑175 |
| État Moonlight | **porté en partie** — c'est le premier gros gisement de contenu prêt à brancher |

Épisode charnière : il ouvre **Rock Ridge** et surtout la série des **donjons Illusion**,
qui recyclent d'anciens donjons pré-renewal en versions haut niveau. C'est le contenu le
plus naturellement transposable de tout le dossier, parce qu'il s'appuie sur des cartes
que le pré-renewal connaît déjà.

## 1. Contenu principal (9 mars 2016)

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Base de guerre des Heart Hunters** 1 et 2 | `1@swat` | `HeartHunterWarBase.txt` |
| **Laboratoire de Werner** (salle centrale) 1 et 2 | `1@slw` | `WernerLaboratoryCentralRoom.txt` |

## 2. Contenu de l'intervalle (2016‑2018)

### Ville et donjon

| Date kRO | Contenu | Cartes |
|---|---|---|
| 2016‑12‑07 | **Rock Ridge** (village de l'Ouest) | `rockrdg1`, `rockrdg2`, `har_in01`, `harboro1`, `harboro2` |
| 2016‑12‑07 | **Mine de Rock Ridge** | `rockmi1`, `rockmi2` |
| — | Izlude réorganisée | `izlude` |

### Les donjons Illusion

Chacun est une relecture haut niveau d'un donjon existant. Carte = la carte d'origine
suffixée `_i`.

| Date kRO | Donjon Illusion | Carte | Donjon d'origine | Niveau kRO |
|---|---|---|---|---|
| 2016‑12‑27 | **Illusion of Moonlight** | `pay_d03_i` | Donjon de Payon F3 | 100+ |
| 2017‑01‑25 | **Illusion of Vampire** | `gef_d01_i` | Donjon de Geffen F1 | 130+ |
| 2017‑02‑21 | **Illusion of Frozen** | `ice_d03_i` | Donjon de Glace F3 | 110+ |
| 2017‑04‑18 | **Illusion of Abyss** | *(non converti)* | Lac de l'Abysse | 150+ |
| 2018‑03‑09 | **Illusion of Teddy Bear** | `ein_d02_i` | Mine d'Einbech F2 | 150+ |
| 2018‑05‑04 | **Illusion of Luanda** | `com_d02_i` | Donjon de la Plage | 160+ |
| — | **Illusion of Turtle** | `tur_d03_i`, `tur_d04_i` | Île de la Tortue | — |
| — | **Illusion of Twins** | `ant_d02_i` | Fourmilière F2 | — |
| — | **Illusion of Underwater** | `iz_d04_i`, `iz_d05_i` | Byalan F4/F5 | 140+ |
| — | **Illusion of Labyrinth** | `prt_mz03_i` *(non converti)* | Forêt Labyrinthe | 170+ |

> rAthena a converti **huit** des dix : Moonlight, Vampire, Frozen, Turtle, Luanda,
> Underwater, Twins, Teddy Bear. Abyss et Labyrinth n'ont que les objets et l'enchanteur.
>
> iRO Wiki n'en recense que neuf, sans *Illusion of Turtle* ; celui-ci existe pourtant
> bien dans rAthena (`tur_d03_i`, `tur_d04_i`) et côté Moonlight en spawns commentés.

### Instances de l'intervalle

| Instance | Carte | Fichier rAthena |
|---|---|---|
| **Donjon du week-end** | `1@md_pay` | `WeekendDungeon.txt` |
| **Donjon du vendredi** | `1@md_gef` | `FridayDungeon.txt` |
| **Village Poring** | `1@begi` | `PoringVillage.txt` |
| **Old Glast Heim — mode débutant** | `1@gl_k2` | aucun script rAthena |
| Usine de Jouets d'Horreur — mode débutant | `1@xm_d2` | aucun script rAthena |
| *Rêves et Ombres*, *Talons du Roi* | — | quêtes |

### Systèmes

| Système | Pertinent en pré-renewal ? |
|---|---|
| **Système Lapine** | chantier à part, sans interface côté client custom |
| **Retrait de carte** (*card removal*) | déjà là (`moon/desloter.npc`) |
| **Échoppe hors ligne** (*part-time stall*) | oui |
| **Échange d'équipement** (*gear swap*) | déjà géré par le serveur |
| Quêtes de donjon quotidiennes | oui |
| Système familial mis à jour | oui |
| Interface de groupe améliorée | oui |
| Course de Hugel revue | oui |
| Mémoire des Orcs mise à jour | déjà chargée (`moon/instances/OrcsMemory.npc`) |
| Améliorations de la carte du monde | oui |
| Refontes de compétences 3ᵉ classe #1→#4, Rebellion #2 | non |
| Quêtes Eden 100‑140 réorganisées | déjà chargées (`moon/rathena/quests/eden/`) |

## 3. État côté rAthena

`npc/re/quests/quests_16_2.txt`, `npc/re/quests/quests_rockridge.txt`,
`npc/re/quests/quests_illusion_dungeons.txt` (≈ 13 000 lignes, huit donjons),
`npc/re/merchants/enchan_illusion_dungeons.txt`, `enchan_rockridge.txt`,
`npc/re/quests/illusion_investigation.txt`,
`npc/re/instances/{HeartHunterWarBase,WernerLaboratoryCentralRoom,WeekendDungeon,FridayDungeon,PoringVillage}.txt`.

## 4. État côté Moonlight (mesuré le 2026‑11‑06)

Relevé produit par `tools/util/port_illusion.py`, qui suit les `Footer: Imports:` de
chaque base et compare les objets par Id **et** par AegisName.

### Déjà en service

| Élément | Fichier Moonlight |
|---|---|
| Quêtes 16.2 | `moon/rathena/quests/quests_16_2.txt` |
| Rock Ridge — quêtes, guides, entrepôts, enchantements | `quests_rockridge.txt`, `guides_rockridge.txt`, `enchan_rockridge.npc`, `warps/cities/rockridge.txt` |
| Rock Ridge — mobs | `moon/mobs/rockridge.npc` (`rockrdg1/2`, `rockmi1/2`, `harboro1/2`) |
| Heart Hunters | `moon/instances/HeartHunterWarBase.npc` |
| Laboratoire de Werner | `moon/instances/WernerLaboratoryCentralRoom.npc` |
| Donjon du week-end | `moon/instances/WeekendDungeon.npc` — `1@md_pay`, id 45 |
| Donjon du vendredi | `moon/instances/FridayDungeon.npc` — `1@md_gef`, id 46 |
| Village Poring | `moon/instances/PoringVillage.npc` — `1@begi`, id 47 |

Ces trois instances sont branchées mais pas encore validées en jeu. Leur portage ajoute 35 mobs
(`db/import/mobs/memorial_dungeons.yml`), 184 lignes de skills, 10 objets, 6 quêtes et un groupe
d'objets. Les descriptions client des nouveaux objets restent à faire.

### Donjon pilote : Illusion of Moonlight

Branché, pas encore validé en jeu : `moon/rathena/quests/illusion/moonlight.npc` (quêtes, entrée,
Gemcutter), spawns décommentés dans `moon/rathena/dungeons/pay_dun.npc` et identiques à l'amont,
troc `barter_ill_moonlight` dans `moon/barters.yml`, `nomemo` et entrée `• Illusion` du warp agent
(`pay_d03_i,140,44`).

Le Gemcutter a imposé d'activer `feature.barter` et `feature.barter_extended` dans
`conf/import/battle_conf.txt` : ces trocs demandent plusieurs objets et un raffinage minimum.
Sans ces deux flags, le troc n'est pas créé et le Gemcutter ne peut rien échanger.

### Les autres donjons Illusion restent commentés

Spawns et blocs de script sont présents dans les fichiers de Moonlight, **en commentaire**.

| Carte | Donjon | Fichier Moonlight | Lignes de spawn commentées |
|---|---|---|---:|
| `pay_d03_i` | Moonlight | `moon/rathena/dungeons/pay_dun.npc` | **activé** (15) |
| `gef_d01_i` | Vampire | `moon/rathena/dungeons/gef_dun.npc` | 8 |
| `ice_d03_i` | Frozen | `moon/rathena/dungeons/ice_dun.npc` | 4 |
| `iz_d04_i` | Underwater F1 | `moon/rathena/dungeons/iz_dun.npc` | 5 |
| `iz_d05_i` | Underwater F2 | `moon/rathena/dungeons/iz_dun.npc` | 5 |
| `tur_d03_i` | Turtle F1 | `moon/rathena/dungeons/tur_dun.npc` | 2 |
| `tur_d04_i` | Turtle F2 | `moon/rathena/dungeons/tur_dun.npc` | 5 |
| `ant_d02_i` | Twins | `moon/mobs/morocc.npc` | 9 |
| `ein_d02_i` | Teddy Bear | `moon/mobs/einbroch.npc` | 8 |
| `com_d02_i` | Luanda | `moon/mobs/comodo.npc` | bloc scripté `illusion_luanda` |

**Reste : 46 lignes de spawn, plus le bloc de Luanda.**

`Crown_Of_Witch` (25897) et `IL_Kraken_Leg` (7324) manquent au troc `barter_ill_underwater` :
à porter avec Underwater.

### Données portées (non validées en jeu)

| Base | Porté | Détail |
|---|---:|---|
| Mobs | **70** | `db/import/mobs/illusion.yml` : 66 `ILL_*` et 4 invocations `G_ILL_*`. `ILL_KRAKEN` et `ILL_ABYSMAL_WITCH` étaient déjà chargés |
| `mob_skill_db` | **612 lignes** | `ILL_ICEICLE`, `ILL_FROZEN_KN` et `ILL_FROZEN_GC` n'en ont aucune |
| `skill_db` | **6** | `NPC_ALL_STAT_DOWN`, `NPC_DAMAGE_HEAL`, `NPC_IMMUNE_PROPERTY`, `NPC_MOVE_COORDINATE`, `NPC_WIDESILENCE2`, `NPC_BLEEDING2` |
| Objets ajoutés | **107** | 56 divers, 22 cartes (2 enchantements), 14 armures, 12 armes, 2 pierres de résonance, 1 soin |
| Objets renommés | **87** | même Id, nom renewal adopté, par exemple `Illusion_War_Axe` → `War_Axe_IL` |
| Quêtes | **141** | la 3465 reste écartée : commentée en `db/re` (Labyrinth, non converti), mais posée par `illusion_investigation.txt` |
| Combos | **24** | ceux dont tous les objets existent ; 16 sont écartés faute de partenaire chargé |

Les spawns restent commentés : aucun de ces mobs n'apparaît encore en jeu.

**Équipements.** Les 26 nouveaux ont été relus un par un et suivent la conversion des
`Illusion_*` déjà présents :

| Point | Conversion |
|---|---|
| `Defense` | armure 12, tête 8, bouclier 8, cape 6, chaussures 6 |
| `MagicAttack` | ignoré en pré-renewal : remplacé par `bonus bMatk,N;` en tête du script |
| `Classes` | `All_Third` + `Fourth` retirés, sinon l'objet serait inéquipable ; `Upper` seul devient `Upper`, `Third`, `Third_Upper`, `Third_Baby` |
| `bonus bDef,50` | ramené à 5 |
| `bAtkRate` | `bonus2 bAddClass,Class_All,n` |
| Nom | suffixe `[n]` selon les emplacements |

Les paliers de raffinage `>= 11` restent en place, inertes avec `MAX_REFINE` à 10. Les bonus de
compétences de 3ᵉ classe (`LG_*`, `WM_*`, `RK_*`, `AB_*`, `GC_*`) sont gardés comme sur les
objets existants, sans effet tant que ces classes restent fermées.

**Restent à décider ou à faire :**

- `ILL_Piece_A` et `ILL_Piece_B` lancent `laphine_upgrade()` : inertes tant que le système Lapine
  n'existe pas, à traiter avec `market_resonance_stone` au lot économie.
- Les albums de cartes attendent l'ouverture des donjons, pour ne pas distribuer ces cartes avant.
- Les descriptions client des nouveaux objets restent à faire.

### PNJ, économie et mapflags

- **Quêtes et accès :** `quests_illusion_dungeons.txt` (8 donjons, 303 PNJ) et
  `illusion_investigation.txt` ne sont pas présents côté Moonlight.
- **Enchantement :** `enchan_illusion_dungeons.txt` (10 PNJ) non plus.
- **Warp agent :** un accès direct aux 10 cartes reste à ajouter.
- **Mapflags :** `nomemo` manque sur les 10 cartes, et `nowarpto` sur `iz_d04_i`,
  `iz_d05_i` et `ant_d02_i`.

### Hors périmètre

**Modes débutant** (`1@gl_k2`, `1@xm_d2`) : rAthena ne fournit aucun script, seulement des
mapflags et, pour `1@gl_k2`, une entrée `instance_db`. Ils sont hors périmètre.

## 5. Migration pré-renewal

Décisions du 2026‑11‑06 :

| Sujet | Décision |
|---|---|
| Accès | Quêtes d'origine conservées, **plus** un accès direct par le warp agent |
| Découpage | Un fichier `.npc` par donjon sous `moon/rathena/quests/illusion/`, pour charger et tester un donjon à la fois |
| Équipements | Portés et contrôlés un par un |
| Stats des mobs | `tools/util/re_to_prere_mob.py` sans retouche : ATK inchangée et EXP dérivée des HP sont voulues |
| Équipement Doram | `Doram_High_Cape`, `Doram_High_Shoes`, `Doram_High_Suit` retirés des drops |
| Noms d'objets | Le local est renommé vers le nom renewal, après un grep exhaustif |
| Système Lapine | Chantier à part : le client custom n'a pas encore son interface |

Ordre de portage : Moonlight comme donjon pilote, puis Frozen, Vampire, Underwater,
Teddy Bear, Turtle, Luanda, Twins.

## 6. Verdict

| | |
|---|---|
| Intérêt | **le plus élevé du dossier** |
| Reste à faire | validation en jeu du pilote · 7 donjons à activer · enchanteur · albums · client |

## Sources

- [Episode XVI.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVI.II)
- [Illusion Dungeons — iRO Wiki](https://irowiki.org/wiki/Illusion_Dungeons)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=182)
