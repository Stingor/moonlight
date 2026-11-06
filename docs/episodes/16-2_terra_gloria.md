# Épisode 16.2 — Terra Gloria

| | |
|---|---|
| Nom kRO | 테라 그로리아 / *Terra Gloria* |
| Date kRO | **2016‑03‑09** |
| Arc | Terra Gloria — les Heart Hunters |
| Niveau kRO visé | 150‑175 |
| État Moonlight | **porté, à valider en jeu** — reste le `.thor` client |

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
| *Rêves et Ombres*, *Talons du Roi* | — | quêtes, aucun script trouvé dans rAthena par mots-clés |

### Systèmes

| Système | Pertinent en pré-renewal ? |
|---|---|
| **Système Lapine** | partiel : les deux pierres d'Illusion seulement |
| **Retrait de carte** (*card removal*) | déjà là (`moon/desloter.npc`) |
| **Échoppe hors ligne** (*part-time stall*) | équivalent : commande `@autotrade` |
| **Échange d'équipement** (*gear swap*) | déjà géré par le serveur |
| Quêtes de donjon quotidiennes | portées avec les donjons Illusion et l'enquête |
| Système familial mis à jour | système maison : `moon/adoption.npc`, `moon/marriage.npc` |
| Interface de groupe améliorée | côté client, non évalué ici |
| Course de Hugel revue | `moon/rathena/other/hugel_bingo.txt` chargé ; la refonte kRO n'est pas comparée |
| Mémoire des Orcs mise à jour | déjà chargée (`moon/instances/OrcsMemory.npc`) |
| Améliorations de la carte du monde | côté client, non évalué ici |
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
d'objets.

### Les huit donjons Illusion sont branchés (non validés en jeu)

| Carte | Donjon | Script | Spawns |
|---|---|---|---|
| `pay_d03_i` | Moonlight | `moon/rathena/quests/illusion/moonlight.npc` | `moon/rathena/dungeons/pay_dun.npc` |
| `ice_d03_i` | Frozen | `frozen.npc` | `moon/rathena/dungeons/ice_dun.npc` |
| `gef_d01_i` | Vampire | `vampire.npc` | `moon/rathena/dungeons/gef_dun.npc` |
| `iz_d04_i`, `iz_d05_i` | Underwater | `underwater.npc` | `moon/rathena/dungeons/iz_dun.npc` |
| `ein_d02_i` | Teddy Bear | `teddy_bear.npc` | `moon/mobs/einbroch.npc` |
| `tur_d03_i`, `tur_d04_i` | Turtle | `turtle.npc` | `moon/rathena/dungeons/tur_dun.npc` |
| `com_d02_i` | Luanda | `luanda.npc` | `moon/mobs/comodo.npc` |
| `ant_d02_i` | Twins | `twins.npc` | `moon/mobs/morocc.npc` |

Les blocs de spawn sont repris de l'amont après comparaison ligne à ligne avec les versions
commentées. Le warp agent mène à chaque carte par `• Illusion`, dans le menu des donjons, et
chaque échangeur a son troc dans `moon/barters.yml`.

`feature.barter` et `feature.barter_extended` sont activés dans `conf/import/battle_conf.txt` :
ces trocs demandent plusieurs objets et un raffinage minimum.

`ILL_KRAKEN` et `ILL_ABYSMAL_WITCH` n'étaient que des gabarits sans drop ni skill : ils sont
remplacés par leurs valeurs converties. Les cartes Illusion ont rejoint les albums : 49 dans
`CARDALBUM`, 10 dans `MAGICCARDALBUM`, 9 dans `MVPCARDENCHANT`.

### Données portées (non validées en jeu)

| Base | Porté | Détail |
|---|---:|---|
| Mobs | **72** | `db/import/mobs/illusion.yml` : 68 `ILL_*` et 4 invocations `G_ILL_*` |
| `mob_skill_db` | **677 lignes** | `ILL_ICEICLE`, `ILL_FROZEN_KN` et `ILL_FROZEN_GC` n'en ont aucune |
| `skill_db` | **7** | `NPC_ALL_STAT_DOWN`, `NPC_DAMAGE_HEAL`, `NPC_IMMUNE_PROPERTY`, `NPC_MOVE_COORDINATE`, `NPC_WIDESILENCE2`, `NPC_BLEEDING2`, `NPC_HELLJUDGEMENT2` |
| Objets ajoutés | **111** | 58 divers, 24 cartes (2 enchantements), 14 armures, 12 armes, 2 pierres de résonance, 1 soin |
| Objets renommés | **87** | même Id, nom renewal adopté, par exemple `Illusion_War_Axe` → `War_Axe_IL` |
| Quêtes | **141** | la 3465 reste écartée : Labyrinth n'est pas converti, et son PNJ est commenté dans l'amont |
| Combos | **24** | ceux dont tous les objets existent ; 16 sont écartés faute de partenaire chargé |

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

- **Client :** `itemInfokro.lua` décrit déjà les 121 nouveaux objets. `itemInfomoon.lua` surcharge les
  11 dont le texte mentait (`Defense`, `DEF +50`, Minstrel et Wanderer). Le `.thor` reste à générer,
  et la présence des sprites n'a pas pu être vérifiée dans les GRF.

### Économie

- **Enquête :** `moon/rathena/quests/illusion/investigation.npc`, repris tel quel. Terrian et sa
  quête 3465 sont déjà en commentaire dans l'amont, faute de Labyrinth.
- **Illusion Enchanter :** `moon/rathena/merchants/enchan_illusion.npc`, à `prontera,90,115`.
- **Illusion Merchant :** dans le même fichier, à `prontera,88,113`. Il vend `ILL_Piece_A` et `ILL_Piece_B`
  (2 000 000 zeny ou 10 Illusion Stones). Leurs recettes sont dans `db/import/laphine_upgrade.yml` :
  36 armes `*_IL`, options `ILL_PIECE_A` et `ILL_PIECE_B`. Rien d'autre n'est portable aujourd'hui : les
  déclencheurs des 102 autres recettes et des 179 de `laphine_synthesis` n'existent pas dans les objets chargés.

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
| Système Lapine | Réduit aux deux pierres d'Illusion, le client custom ayant désormais son interface |

## 6. Verdict

| | |
|---|---|
| Intérêt | **le plus élevé du dossier** |
| Reste à faire | validation en jeu · publier la mise à jour client |

## Sources

- [Episode XVI.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVI.II)
- [Illusion Dungeons — iRO Wiki](https://irowiki.org/wiki/Illusion_Dungeons)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=182)
