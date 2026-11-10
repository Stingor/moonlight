# Old Glast Heim : mode défi — fiche de conception

| | |
|---|---|
| Nom officiel | *Old Glast Heim: Challenge Mode* (OGH:CM), « Contaminated Dimension » |
| Sortie | kRO puis serveurs Gravity occidentaux, annonce du **2023‑12‑05** |
| Carte | `1@gl_he` (« Contaminated Dimension », déjà dans le mapcache et le client) ; le wiki iRO cite `4@ghc0`, absent ici |
| Côté rAthena | `instance_db` 49, gabarits de mobs commentés (`MD_C_*`, 20572‑20585), objets et cartes présents, **aucun script** |
| Côté Moonlight | rien — ce document précède l'écriture du script |
| État | décisions prises le 2026‑11‑09 (section 8) ; **lot C1 fait le 2026‑11‑09** (section 9) ; C2 et C3 à écrire |

Le script n'existe nulle part : ni chez rAthena, ni chez hayakawakaki, ni chez AsurielRO. Ce
qui suit est une **reconstitution**. Les sections 1 à 5 relèvent ce que les sources disent ;
les sections 6 et 7 disent ce que Moonlight en ferait ; la section 8 liste ce qu'il reste à
trancher.

## Sources

| Source | Ce qu'elle apporte | Fiabilité |
|---|---|---|
| Annonce officielle, [renewal.playragnarok.com id=355](https://renewal.playragnarok.com/news/updatedetail.aspx?id=355) (deux images, lues le 2026‑11‑09) | entrée, niveau, cooldown, déroulé en trois temps, tables de coffre, échanges, enchantements, effets des Circlet et des Temporal Jewel | **fait foi** |
| [Divine Pride](https://www.divine-pride.net/database/monster/20574), fiches 20572 à 20585 | stats, compétences (niveaux, déclencheurs, taux) et drops des 14 mobs | données client, fiables |
| [iRO Wiki](https://irowiki.org/wiki/Old_Glast_Heim:_Challenge_Mode) | niveaux de contamination, PV des boss, chances de Himmelmez, mécaniques | communautaire, « needs to be updated » |
| [ROGGH](https://roggh.com/old-glast-heim-challenge-mode/) | mécaniques du combat de boss, conditions de récompense | communautaire, diverge du wiki sur plusieurs points |
| `db/re` (rAthena) | Id, AegisName, objets, cartes | fiable |

## 1. Entrée et cadre (annonce officielle)

- **Oscar** à `glast_01` **143,288** (le wiki dit 139,285). Niveau **170**. Cooldown **3 jours**.
  Récompenses selon le niveau de difficulté.
- ROGGH ajoute un prérequis : avoir fini une fois Old Glast Heim Advanced (porté le
  2026‑11‑09, instance 55).
- Groupe d'un joueur ou plus. Limite de temps 1 h 30 (wiki).
- Quête de cooldown « Aftermath of the challenge » (wiki).

## 2. Niveaux de contamination

L'annonce : « un système de difficulté sélectionnable où l'on débloque des versions plus
fortes après avoir fini une version plus facile ». Le chef de groupe choisit le niveau auprès
d'Oscar. Le compteur de victoires est **par personnage**, quel que soit le niveau joué, et
s'affiche dans l'achievement **129021 « Glastheim Challenge Mode »** (100 victoires, titre
1045), déjà présent dans `db/import/achievement_db.yml` sans cible.

| Niveau | Victoires requises | PV du fantôme (renewal) | Chance de Himmelmez (wiki) |
|---|---|---|---|
| 1 | 0 | 600 M | 4,59 % |
| 2 | 5 | 750 M | 4,72 % |
| 3 | 11 | 900 M | 4,95 % |
| 4 | 18 | 1,05 Md | 5,32 % |
| 5 | 26 | 1,2 Md | 5,88 % |
| 6 | 35 | 1,35 Md | 6,76 % |
| 7 | 45 | 1,5 Md | 8,20 % |
| 8 | 60 | 1,65 Md | 10,87 % |
| 9 | 80 | 1,8 Md | 17,24 % |
| 10 | 100 | 2 Md | 50 % |

Ce qui monte avec le niveau (ROGGH) : les dégâts de tous les monstres, les PV du boss, sa
régénération (300 000 × niveau toutes les 2 s), les PV des épines, le nombre de Prime
Corruption Root, les mécaniques supplémentaires (niveau 3+ : Max Pain, Grim Reaper ;
niveau 9+ : Gradual Gravity, Circle Arena), et les récompenses.

## 3. Déroulé (annonce officielle, complété par les wikis)

1. **Entrée.** Le chef choisit le niveau auprès d'Oscar.
2. **Deux ailes.** Le groupe est réparti au hasard entre l'aile ouest et l'aile est,
   **50 monstres** chacune. L'annonce : « nettoyer les monstres des deux côtés ». Une aile
   nettoyée (moins de 5 restants selon le wiki, 47 tués sur 50 selon ROGGH) ouvre un portail.
3. **Prime Corruption Root.** Quand une aile s'ouvre, des racines apparaissent dans le couloir
   central pour **30 secondes** (1 au niveau 1, jusqu'à 4 ou 5 au niveau 10). Non tuées, elles
   explosent (zone 15×15) et Oscar les détruit lui-même. Elles lâchent des Temporal Gemstone.
4. **Couloir central.** Le chef parle à Oscar au bout du couloir, qui ouvre la salle du boss.
5. **Boss.** Oscar invoque le **Fantôme d'Amdarais**, ou le **Fantôme de Himmelmez** selon la
   chance du niveau. L'annonce : « si vous avez de la chance, ce sera Amdarais ; sinon,
   Himmelmez. Suivez les instructions d'Oscar pour vous placer. »
6. **Coffre.** Le boss tué, un coffre apparaît au centre. Oscar donne ensuite des **Sealed
   Temporal Circlet** à chaque membre, selon la performance du groupe.

**Règles du combat (ROGGH, à reprendre en partie, voir section 6) :** dégâts ramenés à 1 tant
qu'un membre est mort, absent ou trop loin ; boss qui disparaît après 5 minutes de groupe
hors combat ; arène dont il ne doit pas sortir (30 s) ; soin différé de 60 % des dégâts ;
téléportations aléatoires ; grille d'épines suivie d'un Grand Darkness (sous 90 % de PV) ;
immunité d'élément 30 s ; invocation de mutants (Khalitzburg sous 70 %, White Knight sous
50 %, Raydric Archer sous 30 %).

## 4. Monstres (Divine Pride, stats renewal avant conversion)

| Id | AegisName | Nom | Niv. | PV | Race / élément | Rôle |
|---|---|---|---|---|---|---|
| 20572 | `MD_C_HEMEL` | Phantom of Himmelmez | 195 | 2 Md | Angel / Holy 3 | boss (rare) |
| 20573 | `MD_C_AMDARAIS` | Phantom of Amdarais | 192 | 2 Md | Demon / Dark 3 | boss |
| 20574 | `MD_C_WHITEKNIGHT` | Mutating White Knight | 188 | 2 450 483 | Undead / Dark 3 | ailes |
| 20575 | `MD_C_CORRUPTION_ROOT` | Prime Corruption Root | 190 | 36 760 359 | Demon / Earth 3 | couloir, minuté |
| 20576 | `MD_C_KHALITZBURG` | Mutating Khalitzburg | 185 | 2 401 358 | Undead / Dark 3 | ailes |
| 20577 | `MD_C_RAYDRIC` | Cursed Raydric | 182 | 2 362 114 | Human / Dark 2 | ailes |
| 20578 | `MD_C_RAYDRIC_ARCHER` | Cursed Raydric Archer | 186 | 2 394 654 | Human / Dark 2 | ailes |
| 20579 | `MD_C_ZOMBIE` | Cursed Butler | 180 | 2 306 607 | Undead / Undead 2 | ailes |
| 20580 | `MD_C_GHOUL` | Cursed Servant | 184 | 2 360 625 | Undead / Undead 2 | ailes |
| 20581 | `MD_C_THORN` | Thorn | 185 | 50 | Formless / Fire 3 | piège (1 dégât par coup) |
| 20582 | `MD_MANHOLE4` | Manhole | 175 | 10 000 | Formless | piège au sol, invisible |
| 20583 | `MD_C_WHITEKNIGHT_G` | Mutated White Knight | 188 | 2 450 483 | | invoqué par le boss, sans drop |
| 20584 | `MD_C_KHALITZBURG_G` | Mutated Khalitzburg | 185 | 2 401 358 | | invoqué, sans drop |
| 20585 | `MD_C_RAY_ARCHER_G` | Cursed Raydric Archer | 186 | 2 394 654 | | invoqué, sans drop |

Les 14 ont leur sprite dans le client (`c_himel`, `c_amdarais`, `c_whiteknight`…). Les fiches
complètes (caractéristiques, compétences avec niveau, déclencheur et taux, drops) sont sur
Divine Pride, `https://www.divine-pride.net/database/monster/<Id>` pour 20572 à 20585 : le
lot C1 les relève, les met au format `mob_db` et `mob_skill_db`, puis les passe au
convertisseur `tools/util/re_to_prere_mob.py`. Deux points relevés le 2026‑11‑09 : Cursed
Butler et Cursed Servant lâchent la même carte (27388), et les trois invoqués (`_G`) n'ont ni
EXP ni drop.

## 5. Récompenses et monnaies (annonce officielle)

**Coffre du boss**, quantité tirée selon le niveau :

| Niveau | Temporal Fragment | Sealed Temporal Circlet | Temporal Gemstone | Temporal Spell |
|---|---|---|---|---|
| 1 | 1‑15 | 1‑5 | — | — |
| 2 | 1‑20 | 1‑6 | — | — |
| 3 | 1‑25 | 2‑7 | 1‑3 | — |
| 4 | 1‑30 | 2‑8 | 1‑4 | — |
| 5 | 2‑30 | 3‑9 | 1‑4 | 1‑3 |
| 6 | 2‑30 | 3‑10 | 1‑5 | 3‑7 |
| 7 | 3‑30 | 4‑10 | 2‑5 | 5‑11 |
| 8 | 3‑30 | 4‑10 | 2‑6 | 8‑16 |
| 9 | 4‑30 | 5‑10 | 2‑6 | 11‑22 |
| 10 | 4‑30 | 5‑10 | 2‑7 | 14‑28 |

**Les quatre monnaies**, toutes dans `db/re` : Sealed Temporal Circlet (25864), Temporal
Gemstone (25865), Temporal Spell (25866), Temporal Fragment (25867).

**PNJ près d'Oscar :**
- **HUGINN2001** (distributeur) : 5 Sealed Temporal Circlet → un **Circlet of Time** de sa
  classe ; démontage d'un Sealed Circlet → Fragments, Gemstone ou Spell au hasard ; cartes
  **Mutated Khalitzburg** et **Mutated White Knight** à **50 Temporal Spell** chacune (le
  wiki dit 70 pour la Khalitzburg ; l'annonce fait foi).
- **OSC0005** (machine) : enchante le Circlet of Time en trois étapes, plus une remise à zéro
  (500 000 zeny à 70 %, sinon l'objet est détruit ; ou un Nyangvine Fruit à 100 %).
  - 4ᵉ emplacement, 10 Fragments : INT/STR/VIT/LUK +1 à +4.
  - 3ᵉ emplacement, 20 Fragments (mêmes stats et Temporal Jewel) ou 20 Gemstone (Temporal
    Jewel seuls) : Temporal Jewel STR/AGI/VIT/INT/DEX/LUK Lv1 à Lv3.
  - 2ᵉ emplacement, 2 Temporal Spell : Sharp, Expert Archer, ASPD, Spell 1 à 4, ou Mettle,
    Magic Essence, Acute, Master Archer, Adamantine, Affection, Unyielding Lv1 ; ces derniers
    montent jusqu'au niveau 10 contre 4 à 40 Temporal Spell (échec : −1 niveau, jamais de
    destruction).
- **MUNNIN2003** (ROGGH seulement) : échange d'armes Royal / Guardian Knight contre des
  Temporal Spell. À vérifier avant d'en tenir compte.

**Circlet of Time** : 19 versions de classe (`S_Circlet_Of_Time_*`, `db/re/item_db_equip.yml`),
DEF 10, niveau 170, un emplacement, bonus par raffinement et bonus de compétences de 3ᵉ
classe.

## 6. Ce que Moonlight en ferait

**Repris tel quel, une fois converti en pré-renewal :**
- les 14 mobs, par le convertisseur, avec leurs compétences et leurs drops ;
- les 4 monnaies, les 8 cartes (27381‑27388), les tables de coffre ;
- Oscar, HUGINN2001 et OSC0005 à `glast_01` ;
- le compteur de victoires par personnage, branché sur l'achievement 129021 (cible à ajouter).

**Adapté :**
- **Les Circlet of Time** : comme les armures Grace, on retire le bloc `Classes` (`All_Third`
  + `Fourth` les rend inéquipables ici) et la classe de base (`Jobs: Knight`) les porte ; les
  compétences de 3ᵉ classe passent par la table des classes fermées
  (`docs/episodes/skills_classes_fermees.md`), les paliers au‑delà de +10 par
  `getallrefine() >= 2N`. Les versions Kagerou, Oboro, Rebellion et Summoner tombent avec
  leurs classes.
- **Les PV des fantômes** : 600 M à 2 Md n'ont pas de sens en pré-renewal. Proposition :
  prendre le PV converti du niveau 1 comme base et appliquer le rapport officiel
  (×1,25 par palier de 150 M, soit ×3,33 au niveau 10). Même logique pour les dégâts des
  mobs, que l'officiel augmente sans chiffre publié.
- **Les mécaniques du boss** : ne garder que celles qu'on peut écrire proprement avec les
  compétences NPC du fork (grille d'épines + Grand Darkness, immunité d'élément, invocation
  des mutants, arène, dégâts à 1 si un membre est mort ou absent). Max Pain, Grim Reaper,
  Gradual Gravity et Circle Arena seraient une seconde passe.

**Écrit pour Moonlight (rien n'existe) :**
- tout le script d'instance : placement des 50 mobs par aile, compteurs, portails, minuterie
  des racines, couloir, invocation du boss, coffre, distribution des Circlet ;
- les dialogues d'Oscar, à partir du « Dimensional Guard Report » de l'annonce.

## 7. Lots proposés

| Lot | Contenu |
|---|---|
| **C1** | mobs, compétences, monnaies, cartes, instance (Id libre suivant), mapflags de `1@gl_he`, quête de cooldown, achievement 129021 avec sa cible |
| **C2** | script d'instance : niveaux 1 à 10, deux ailes, racines, boss et ses mécaniques de base, coffre, Sealed Circlet |
| **C3** | HUGINN2001 (Circlet, démontage, cartes) et les 19 Circlet convertis |
| **C4** | OSC0005 et les enchantements : 18 Temporal Jewel, les familles Mettle / Magic Essence / Acute / Master Archer / Adamantine / Affection / Unyielding (Lv1‑10), Nyangvine Fruit — rien de tout cela n'est en local |
| **C5** | mécaniques avancées du boss (Max Pain, Grim Reaper, Gradual Gravity, Circle Arena) |

## 8. Décisions (utilisateur, 2026‑11‑09)

1. **Prérequis** : avoir fini Old Glast Heim Advanced, en plus du niveau 170.
2. **Échelle des PV et dégâts** : celle de la section 6 (PV converti du niveau 1, puis le
   rapport officiel, ×3,33 au niveau 10).
3. **HM maison** : la contamination le **remplace** pour cette instance ; pas de cumul.
4. **Cooldown** : **23 h**, comme l'OGH Advanced, et non les 3 jours officiels.
5. **Divergences des sources** : le wiki iRO tranche (ailes ouvertes à moins de 5 restants,
   table des chances de Himmelmez, nombre de racines).
6. **Circlet** : les 14 Circlet de 3ᵉ classe vont à leur classe de base ; Rebellion va au
   Gunslinger, Super Novice au Novice et Super Novice, Kagerou **et** Oboro au Ninja (deux
   variantes au choix, par défaut, à revoir si l'utilisateur préfère n'en garder qu'une) ;
   **pas de Circlet pour l'Invocateur** : il n'y a pas de Doram jouable.
7. **Périmètre** : lots **C1 à C3** d'abord ; les enchantements (C4) et les mécaniques
   avancées (C5) plus tard.

## 9. Lot C1 (fait le 2026‑11‑09)

- **Mobs** : `db/import/mobs/ogh_challenge.yml`, 14 entrées reconstituées depuis Divine Pride.
  Étalonnage vérifié sur quatre mobs présents à la fois dans `db/re` et sur Divine Pride
  (20367, 20368, 20420, 20543), à une unité près : `Attack` = 2,5 × (ATK max − ATK min),
  `Attack2` = (MATK max − MATK min) / 0,6, `AttackMotion` = 1000 / Aspd, `WalkSpeed` =
  1000 / cases par seconde (plafonné à 1000 pour le Manhole). AttackDelay, DamageMotion, Ai et
  modes viennent d'un mob analogue (`MG_AMDARAIS`, `MG_KNIGHT_OF_ABYSS`, `MG_KHALITZBURG`,
  `RAYDRIC_H`, `RAYDRIC_ARCHER_H`, `MG_ZOMBIE`, `MG_GHOUL`, `MG_CORRUPTION_ROOT`,
  `MD_MANHOLE2`) ; le Fantôme de Himmelmez emprunte ceux d'Amdarais, faute d'analogue.
- **PV après conversion** : mobs des ailes environ 460 000, Prime Corruption Root 2,1 M,
  fantômes 9,2 M au niveau 1 (base de 600 M renewal ; le script C2 appliquera ×1,25 par palier,
  jusqu'à ×3,33 au niveau 10).
- **Compétences** : 90 lignes dans `db/import/mob_skill_db.txt`, cible (`self` ou `target`)
  reprise de l'usage le plus fréquent de chaque compétence ; les `NPC_EMOTION` sont omis
  (Divine Pride ne donne pas l'émotion), la condition « Skilluse » d'Amdarais devient `always`.
- **Objets** : les 4 monnaies (25864‑25867) et les 8 cartes (27381‑27388) ; cartes dans `oca`,
  `mca` (Prime Corruption Root au taux 30) et `mvpenchant` (les deux fantômes).
- **Instance 57** « Old Glast Heim Challenge Mode » sur `1@gl_he` (1 h 30), constante
  `INST_OLD_GLAST_HEIM_CHALLENGE_MODE` ; mapflags calqués sur `1@gl_k2`.
- **Quête de cooldown 12494** « Aftermath of the challenge », +23h, ajoutée aussi au journal du
  client (`quests.yml`).
- **Achievement 129021** : objectif de 100 victoires, côté serveur et client ; le script C2 le
  fera avancer par `achievementupdate`.
