# Épisode 14.1 — Bifrost

| | |
|---|---|
| Nom kRO | 비프로스트 / *Bifrost* |
| Date kRO | **2010‑06‑30** |
| Arc | *Alfheim, The Fairy Land* |
| Niveau kRO visé | 100‑130 |
| État Moonlight | **porté et actif**, sauf les classes étendues |

Le plus gros épisode de la période. La sortie initiale est mince (Mora + la Forêt
Brumeuse), mais les dix-huit mois d'intervalle avant Eclage ont apporté **trois
localisations** (Dewata, Malangdo, Port Malaya), **six instances** et une pluie de
systèmes qui structurent encore le jeu aujourd'hui.

## 1. Contenu principal (30 juin 2010)

| Carte | Nom |
|---|---|
| `mora` | Mora, village des Laphine |
| `bif_fild01`, `bif_fild02` | Bifrost — Forêt d'Alfheim |
| `1@mist` | **Forêt Brumeuse** (*Hazy Forest*), instance |

Mobs : Miming, Bomberling, Wootan (Fighter/Shooter/Defender), Pom Spider, Angra Mantis,
Parus, Little Fatum, **Bacsojin/Baksojin** en écho.

> ⚠ La **Tour de Bifrost** (`ecl_tdun01`→`04`) porte le nom de l'épisode mais appartient
> à l'épisode **14.2 (Eclage)**. L'encyclopédie la range à tort sous 14.1.

## 2. Contenu de l'intervalle (2010‑2011)

### Localisations « Global Project »

| Sortie kRO | Lieu | Cartes |
|---|---|---|
| 2010‑09‑29 | **Dewata** (Indonésie) | `dewata`, `dew_fild01`, `dew_dun01`, `dew_dun02`, `dew_in01` |
| 2010‑11‑24 | **Malangdo** (île des chats) | `malangdo`, `mal_in01/02`, `mal_dun01` |
| 2011‑09‑28 | **Port Malaya** (Philippines) | `malaya`, `ma_fild01/02`, `ma_dun01`, `ma_in01`, `ma_scene01`, `ma_zif01`→`09` |
| 2011‑11‑16 | **Nouvelle Alberta** (port étendu) | `alberta` |

### Instances

| Instance | Carte | Rattachement |
|---|---|---|
| Forêt Brumeuse | `1@mist` | 14.1 principal |
| Grotte de Buwaya | `1@ma_c` | Port Malaya |
| Hôpital Bangungot 2F | `1@ma_h` | Port Malaya |
| Lac Bakonawa | `1@ma_b` | Port Malaya |
| Grotte du Poulpe | `1@cash` | Malangdo |
| Égouts de Malangdo | `1@pump` | Malangdo |
| Laboratoire de Wolfchev | `1@lhz` | 2011 |

### Donjons rouverts ou étendus

- **Byalan F6** (`iz_dun05`) — 2010‑11‑24, réservé aux Gold Netcafé à l'origine ;
- **Laboratoire de Lighthalzen F4** (`lhz_dun04`) — 2011‑03‑30 ;
- **Pyramides mode Cauchemar** (`moc_prydn1`, `moc_prydn2`) — 2011‑12‑07 ;
- **Hall des Scarabées mode Cauchemar** ;
- **Hall of Abyss** — donjon de guilde exclusif WoE, 2011‑06‑29.

### Systèmes

| Sortie | Système | Pertinent en pré-renewal ? |
|---|---|---|
| 2010‑07‑28 | **Système de costumes** | oui |
| 2010‑07‑28 | Synthèse d'équipement | oui |
| 2010‑08‑18 | **Recherche d'échoppes** (*stall search*) | oui |
| 2010‑11‑24 | **Pierres tombales de MVP** (*MVP tombstone*) | oui |
| 2010‑11‑24 | Nouvelles montures | oui |
| 2010‑12‑29 | Extension Super Novice | oui |
| 2011‑03‑09 | Système de rediffusion (*replay*) | oui, côté client |
| 2011‑06‑29 | **WoE 1 renouvelé** : 5 → 4 forts, investissement de guilde | à arbitrer |
| 2011‑08‑31 | **Homunculus S** | non (Genetic uniquement) |
| 2011‑08‑31 | Grande passe d'équilibrage de classes | non (classes 3ᵉ) |
| 2011‑11‑02 | **Kagerou / Oboro** | non (extension de Ninja niveau 99+) |
| 2011‑12‑14 | Améliorations du système de groupe | oui |

## 3. État côté rAthena

Contenu ancien et stable : `npc/re/cities/{mora,dewata,malangdo,malaya}.txt`,
`npc/re/quests/quests_{mora,dewata,malangdo,malaya}.txt`,
`npc/re/instances/{HazyForest,BuwayaCave,BangungotHospital,BakonawaLake,OctopusCave,MalangdoCulvert,WolfchevLaboratory}.txt`,
`npc/re/merchants/enchan_mora.txt`.

## 4. État côté Moonlight (mesuré le 2026‑08‑25)

**Porté et actif dans sa quasi-totalité.**

| Élément | Fichier Moonlight | Chargé |
|---|---|---|
| Mora, Bifrost | `moon/rathena/cities/mora.txt`, `moon/mobs/bifrost.npc` | ✅ |
| Dewata | `moon/rathena/cities/dewata.txt`, `moon/mobs/dewata.npc`, `quests_dewata.txt` | ✅ |
| Malangdo | `moon/rathena/cities/malangdo.txt`, `moon/mobs/malangdo.npc`, `quests_malangdo.txt` | ✅ |
| Port Malaya | `moon/rathena/cities/malaya.txt`, `moon/mobs/malaya.npc`, `quests_malaya.txt` | ✅ |
| Forêt Brumeuse | `moon/instances/HazyForest.npc` | ✅ |
| Buwaya / Bangungot / Bakonawa | `moon/instances/BuwayaCave.npc`, `BangungotHospital.npc`, `BakonawaLake.npc` | ✅ |
| Poulpe / Égouts | `moon/instances/OctopusCave.npc`, `MalangdoCulvert.npc` | ✅ |
| Laboratoire de Wolfchev | `moon/instances/WolfchevLaboratory.npc` | ✅ |
| Byalan F6 | `moon/rathena/dungeons/iz_dun.npc` (`iz_dun05`) | ✅ |
| Biolab F4 | `moon/rathena/dungeons/lhz_dun.npc` (`lhz_dun04`) | ✅ |
| Pyramides Cauchemar | `moon/rathena/dungeons/moc_pryd.npc` (`moc_prydn1/n2`) | ✅ |
| Enchantement de Mora | `moon/rathena/merchants/enchan_mora.txt` | ✅ |

Le contrôle d'écart mob ne signale **aucun manque** sur `dew_dun`, `mal_dun`, `ma_dun`,
`iz_dun`, `lhz_dun`, `moc_pryd`.

### Ce qui n'est pas porté, et ne le sera pas

- **Kagerou / Oboro** — extension de Ninja au-delà du niveau 99 ; hors périmètre du build
  `PRERE`.
- **Homunculus S** — attaché au Genetic. `npc/re/quests/homun_s.txt` est absent de
  Moonlight, ce qui est cohérent.
- **Montures de classe** — dépendent des 3ᵉ classes.

### WoE 1 renouvelé : laissé tel quel

**Décision du 2026‑11‑17 : on n'y touche pas.** Il s'agit d'une refonte de siège : 4 forts,
investissement de guilde, Hall of Abyss. Ce n'est pas du contenu PvE. Moonlight garde ses
sièges actuels, dont `moon/rathena/guild2/*` (WoE:SE).

### Alberta : la carte du serveur n'est pas celle du client (enquête du 2026‑11‑17)

`moon/rathena/cities/alberta.txt` est la version commune de rAthena (`npc/cities/`), avec
deux retouches locales. Les PNJ des autres villes placés à Alberta (passeurs d'Amatsu,
d'Ayothaya, de Brasilis, de Kunlun, de Louyang, de Moscovia, le marin de l'île aux
tortues, Gershaun) sont aux **positions de 2011**, celles de `npc/re/cities/alberta.txt`.

Il y a **deux versions de la carte** :

| Source | Version d'`alberta.gat` |
|---|---|
| serveur : `db/import/map_cache.dat` (= `db/pre-re`) | ancienne |
| client : `data.grf` | ancienne, identique au serveur |
| client : `data-2025-07-16.grf`, distribué par le miroir de fichiers du patcheur et monté **avant** `data.grf` | **2011**, identique à `db/re/map_cache.dat` |

Un client qui monte `data-2025-07-16.grf` affiche donc l'Alberta de 2011, alors que le
serveur calcule les déplacements sur l'ancienne. **5 091 cellules** n'ont pas le même
statut (marchable ou non) d'une carte à l'autre.

| Grille | PNJ et portails dans un mur (sur 77) |
|---|---|
| ancienne (serveur) | 8. Aibakthing (passeur d'Ayothaya, 246,29) est à 9 cellules du sol, le passeur de Brasilis (246,82) à 6. **Pier le pêcheur** (`moon/quests/hats/229.npc`, 170,225) est à **45 cellules** : le clic exige au plus 19 (`area_size` 18 + 1), il est donc **inaccessible**. |
| 2011 (client) | 2, à 1 ou 2 cellules du sol (Akagi, Sign#alberta), comme partout |

Les placements de Moonlight visent donc l'Alberta de 2011. Seule la grille du serveur
est en retard.

**Le problème ne se limite pas à Alberta.** `data-2025-07-16.grf` contient 98 `.gat`.
7 sont identiques au serveur et 53 désignent des cartes que le serveur n'a pas. **38
divergent**, dont `lighthalzen` (14 084 cellules), `morocc` (5 871), `alberta` (5 091),
`airplane` (3 370), `yuno` (2 728), `einbroch` (2 093), `gefenia04` (785), `prt_mz03_i`
(399), `splendide` (188), `aldeg_cas01` (129). Les autres divergent de moins de 50 cellules.

#### Alignement du 2026‑11‑17 : 35 cartes sur 38

`db/import/map_cache.dat` reprend désormais la grille du client pour **35 cartes**, dont
`alberta`, `morocc`, `airplane`, `prontera`, `gefenia04` et `prt_mz03_i`. La grille
appliquée est la première `.gat` trouvée dans l'ordre des archives du client
(`moonlight.grf`, `data-2025-07-16.grf`, `data.grf`).
- **L'eau du serveur est conservée** : une cellule que le client garde marchable reste
  aquatique (type 3) si elle l'était. Le niveau d'eau des `.rsw` récents (0x206, 0x207) n'est
  pas à l'offset que lit `src/tool/mapcache.cpp`. 52 autres cartes ne différaient que par
  l'eau : elles sont restées telles quelles.
- **Contrôle** des 1 382 coordonnées des scripts chargés sur ces 35 cartes (PNJ, portails,
  destinations, apparitions, coordonnées « carte »,x,y) :
  - 17 sont réparées, dont Pier le pêcheur, les passeurs d'Ayothaya et de Brasilis et le
    portail `alb09` ;
  - 2 tombent dans un mur à une cellule du sol, sans effet : le soldat de Morroc
    (150,120) et le portail `prt08` de Prontera, dont la zone 5×5 touche encore le sol.
- Le serveur charge les 1 301 cartes sans erreur ni avertissement.

**`yuno`, `einbroch` et `lighthalzen` gardent l'ancienne grille.** Leur version 2025
(`data-2025-07-16.grf`) condamne les aéroports : la zone ouest de Juno et tout l'est de
Lighthalzen. Le décor de Lighthalzen y est pourtant le même que dans `data.grf`, seule la
collision change. Juno a aussi un nouveau décor (`.rsw` 0x207). Les aligner casserait 19
coordonnées : quais, portails et destinations des dirigeables (`moon/rathena/airports/*`,
`warps/other/airplane.txt`). Pour un client qui monte `data-2025-07-16.grf`, ces trois
villes restent désaccordées : ce client refuse d'y marcher vers les aéroports. **À trancher
côté client** : remettre les anciennes `.gat` de ces trois villes dans `moonlight.grf`.

Les 205 cartes du serveur absentes des GRF du client (copies d'instances, `pvp_y_*`…)
passent par la `resnametable` et n'ont pas été touchées.

## 5. Verdict

| | |
|---|---|
| Intérêt | très élevé — trois villes, six instances, le système de costumes |
| Reste à faire | rien de PvE ; WoE 1 renouvelé laissé tel quel ; grilles alignées sur le client (35 cartes) ; Juno, Einbroch et Lighthalzen à régler côté client (aéroports) |

## Sources

- [Episode XIV.I — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIV.I)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Bifrost Tower — Ragnarok Wiki](https://ragnarok.fandom.com/wiki/Bifrost_Tower) (rattachement à l'ép. 14.2)
