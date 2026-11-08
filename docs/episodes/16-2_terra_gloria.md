# Épisode 16.2 — Terra Gloria

| | |
|---|---|
| Nom kRO | 테라 그로리아 / *Terra Gloria* |
| Date kRO | **2016‑03‑09** |
| Arc | Terra Gloria — les Heart Hunters |
| Niveau kRO visé | 150‑175 |
| État Moonlight | **porté, à valider en jeu** |

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
| 2017‑04‑18 | **Illusion of Abyss Turtle Island** (*Illusion of Turtle* chez rAthena) | `tur_d03_i`, `tur_d04_i` | Île de la Tortue | 150+ |
| 2018‑03‑09 | **Illusion of Teddy Bear** | `ein_d02_i` | Mine d'Einbech F2 | 150+ |
| 2018‑05‑04 | **Illusion of Luanda** | `com_d02_i` | Donjon de la Plage | 160+ |
| — | **Illusion of Twins** | `ant_d02_i` | Fourmilière F2 | — |
| — | **Illusion of Underwater** | `iz_d04_i`, `iz_d05_i` | Byalan F4/F5 | 140+ |

> rAthena a converti les **huit** : Moonlight, Vampire, Frozen, Turtle, Luanda,
> Underwater, Twins, Teddy Bear. Le neuvième de la série, *Illusion of Labyrinth*
> (`prt_mz03_i`), relève de l'intervalle de [17.1](17-1_illusion.md), où il est décrit ;
> il est porté depuis le 2026‑11‑08.
>
> *Illusion of Abyss* et *Illusion of Turtle* sont un seul donjon : le titre kRO est
> *Illusion of Abyss Turtle Island*, et rAthena a fermé son issue
> [#6458](https://github.com/rathena/rathena/issues/6458) (« Episode 16.2 : Illusion of
> Abyss Turtle Island ») avec Illusion of Turtle. C'est aussi pourquoi iRO Wiki en compte
> neuf sans nommer Turtle.

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

## 4. État côté Moonlight (mesuré le 2026‑11‑06, complété le 2026‑11‑08)

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
| Village Poring | `moon/instances/PoringVillage.npc` — `1@begi`, id 47, niveau 30 et plus : le plafond de 60 est retiré |

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

**Accès par le warp agent.** `IllusionMenu` (`moon/warp_agent.npc`) ouvre une carte quand le
personnage a le niveau requis et que l'avancement de sa quête atteint le seuil de l'entrée
normale. Sinon, il mène au PNJ qui démarre la chaîne. L'avancement est débloqué **pour tout le
compte Moonlight** : au seuil, chaque quête pose `##ill_open_<carte>`, et le menu pose le drapeau
pour les personnages déjà arrivés au seuil avant ce système. Le niveau, lui, reste par
personnage. Kena (Underwater) laisse passer dès que `##ill_open_iz_d05_i` est posé.

Les variables `##` suivent désormais le compte Moonlight (`login.user_id`) et non plus le compte
de jeu (`src/login/account.cpp`). Elles ne servent qu'aux drapeaux : elles sont lues à la
connexion, et la dernière écriture l'emporte. Une migration est nécessaire avant le démarrage du
login-server recompilé : `tools/sql/global_acc_reg_user_bound.sql`. La fusion de comptes de
moonlight-web regroupe aussi ces variables (`AccountMerge.php`, grants dans
`deploy/server/merge.sql`).

**Carnet MVP et missions de chasse.** Les MVP Illusion sont invoqués par script et meurent sur
un label. Ils n'atteignent donc jamais `OnNPCKillEvent`, et le registre du carnet ne les voyait
pas. Huit cases `MVP_SLOT_SCRIPT_KILLCOUNT` les ajoutent dans `src/map/mvp_tracker.cpp`, et le
client connaît ce type (`kScriptKillCount = 4`). Chaque label de mort appelle
`F_HuntMissionKill` (`moon/func.npc`) pour les mobs, ou `MissionMVP` pour les MVP : les missions
de chasse se terminent ainsi sur les cartes de 16.2. Les 9 MVP Illusion rapportent 8 points
dans `$mvps`.

**Drops de carte.** `db/import/map_drops.yml` reprend les 10 cartes Illusion de l'amont et ajoute
16 coffrets d'équipement dans `item_db_usable.yml`. Les coffrets `S_` passent par Lapine
(`laphine_upgrade.yml`) : un équipement du donjon raffiné de +4 à +9 en ressort à +9 ou +10. Les `item_rate_*` du serveur ne s'appliquent pas
aux map_drops : les taux ont donc été remis à l'échelle à la main, sur 100 000.

| Drop (mobs normaux) | kRO | Moonlight |
|---|---:|---:|
| Illusion Stone | 0,01 % | 2 % |
| Équipements | 0,005 à 0,025 % | 0,5 à 2,5 % |
| Coffret | 0,005 % | 0,5 % |
| Coffret `S_` | 0,15 % | 5 % |

Les autres drops, surtout ceux des MVP, gardent leur taux d'origine.

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
| Quêtes | **141** | la 3465 (Labyrinth) est arrivée avec le portage de [17.1](17-1_illusion.md) |
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

Les paliers au-dessus de `MAX_REFINE` passent par `getallrefine()` : `.@r >= 11` devient `>= 22`,
et une somme `>= 22` sur deux pièces devient `>= 44`. Les compétences de classes fermées sont
remplacées par leur équivalent jouable (`LG_RAYOFGENESIS` par `CR_GRANDCROSS`, etc.), les bonus de
`bSkillCooldown` sans équivalent par `bDelayrate`. Les valeurs de bonus ne changent pas.

**Stockage.** `NoStorage` est retiré des matériaux de troc `Sticky_Blood`, `Mushroom_Sap`,
`Swaying_Mane`, `Dry_Branch` et `Shabby_Metal_Chip`, à la suite d'un signalement de joueur.

**Restent à décider ou à faire :**

- **Déploiement**, dans l'ordre :
  1. Sauvegarder les tables `global_acc_reg_*`.
  2. Arrêter le login-server et lancer `tools/sql/global_acc_reg_user_bound.sql`.
  3. Recompiler le login-server et le map-server, puis redémarrer.
  4. Lancer `deploy/server/merge.sql` et déployer moonlight-web.
- **Client :** `moonclient` connaît les 121 nouveaux objets. Son `item_db.yml` est à jour avec le serveur, et ses
  faits (DEF, poids, prérequis) viennent de là. `item_info.yml` a perdu ses lignes de faits périmées
  (`--retirer-faits`) et `DEF +5` est corrigé. La présence des sprites n'est pas vérifiée dans les GRF.
- **Les autres cartes de `db/re/map_drops.yml`** ne relèvent pas de 16.2. `1@xm_d` (Usine de
  Jouets d'Horreur, [14.2](14-2_eclage.md)) est portée le 2026‑11‑08, avec la robe `Celine_Dress`
  et ses trois combos. Les sept restantes iront avec leur épisode :

  | Cartes | Épisode |
  |---|---|
  | `amicitia1`, `amicitia2`, `sp_rudus4`, `nif_dun01`, `nif_dun02` | [17.2](17-2_heritage_du_sage.md) |
  | `1@advs` | [18](18_direction_de_la_priere.md) |
  | `for_dun01` | [20](20_the_undying.md) |

### Économie

- **Enquête :** `moon/rathena/quests/illusion/investigation.npc`, repris tel quel. Terrian et sa
  quête 3465 y sont actifs depuis le portage de Labyrinth ([17.1](17-1_illusion.md)).
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
| Accès | Quêtes d'origine conservées ; le warp agent ouvre une carte selon le niveau et l'avancement, sinon il mène au PNJ de départ |
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
| Reste à faire | déploiement (migration SQL, recompilation) · validation en jeu · livrer `moonclient` |

## Sources

- [Episode XVI.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XVI.II)
- [Illusion Dungeons — iRO Wiki](https://irowiki.org/wiki/Illusion_Dungeons)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=182)
