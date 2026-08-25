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

### Reste à arbitrer

- **WoE 1 renouvelé** (4 forts, investissement de guilde, Hall of Abyss) : c'est une
  refonte de siège, pas du contenu PvE. Moonlight charge
  `moon/rathena/guild2/*` (WoE:SE) mais pas la variante renouvelée de WoE 1.
- **Alberta étendue** : `moon/rathena/cities/alberta.txt` est chargé, mais il faudrait
  vérifier s'il s'agit de la version d'origine ou de la version 2011.

## 5. Verdict

| | |
|---|---|
| Intérêt | très élevé — trois villes, six instances, le système de costumes |
| Reste à faire | quasi rien de PvE ; deux vérifications (Alberta, WoE 1 renouvelé) |

## Sources

- [Episode XIV.I — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIV.I)
- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
- [Bifrost Tower — Ragnarok Wiki](https://ragnarok.fandom.com/wiki/Bifrost_Tower) (rattachement à l'ép. 14.2)
