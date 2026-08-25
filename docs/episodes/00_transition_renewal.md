# La bascule Renewal — la frontière `pre-re` / `re`

> Ce n'est pas un épisode. C'est la rupture de règles qui sépare le pré-renewal de tout
> ce que documentent les fiches suivantes, et c'est elle qui rend une migration
> non triviale.

| | |
|---|---|
| Sakray (serveur de test kRO) | août 2008 |
| Serveur principal kRO, classes 3‑1 | **2009‑06‑17** |
| Classes 3‑2 | 2009‑10‑14 |
| Classes 3ᵉ « bébé » | 2010‑03‑31 |
| Plafond de niveau | 99/50 → **150/50** |
| Position dans la chronologie | entre l'ép. 13.2 (2008‑12‑17) et l'ép. 13.3 (2009‑12‑23) |

## 1. Où passe exactement la frontière

Le dernier état « pré-renewal » du jeu est celui de la **fin de l'épisode 13.2,
*Encounter with the Unknown*** (kRO, 2008‑12‑17) : Manuk, Splendide, la Racine
d'Yggdrasil, le Nid de Nidhoggur.

Le découpage de rAthena colle à cette frontière, et cela se vérifie dans l'arborescence :

- `npc/pre-re/quests/quests_13_1.txt` existe, et `npc/quests/quests_13_2.txt` est dans le
  dossier **partagé**, donc chargé par les deux modes ;
- `npc/re/quests/quests_dicastes.txt` (ép. 13.3) n'a **aucun** équivalent `pre-re`.

Tout ce qui vient après le 17 décembre 2008 est donc, par construction, du contenu
« renewal » du point de vue de l'émulateur — y compris quand il n'a rien de mécaniquement
renewal (une ville, des PNJ, des cartes).

**C'est le point central de tout ce dossier** : la plupart du contenu des épisodes 13.3 à
18 est du contenu *scénarisé* qui ne dépend pas des formules renewal. Ce qui en dépend,
ce sont les **statistiques** (mobs, objets) et les **paliers de niveau**.

## 2. Ce que Renewal change réellement

Classé par impact sur une migration.

### Impact fort — à retraiter systématiquement

| Sujet | Pré-renewal | Renewal |
|---|---|---|
| Plafond | 99 base / 50 job (70 pour les transcendantes) | 150 puis 160, 175, 185, 200, 260 |
| Classes | 1ʳᵉ, 2ᵉ, transcendantes, étendues | + 3ᵉ classes, puis 4ᵉ |
| Statistiques de mob | calibrées pour du niveau 1‑99 | calibrées pour du niveau 100+ |
| Table d'EXP | `db/pre-re/job_exp.yml` | courbes étendues jusqu'au plafond |
| Pénalité de niveau | absente | `level_penalty.yml` (EXP et drop selon l'écart) |

### Impact moyen — visible sur les objets

- **ATK et MATK séparés** sur les armes ; en pré-renewal, l'INT porte la magie.
- **DEF** : en renewal, DEF plate (réduction) + DEF de soft (esquive de dégâts) ;
  en pré-renewal, DEF en pourcentage + VIT.
- **Raffinage** : tables et bonus différents (`db/*/refine.yml`).
- **Niveau d'équipement** (`EquipLevelMin`) : utilisé partout en renewal, souvent 100+.
- **Options aléatoires**, **grades d'enchantement**, **réforme**, **Laphine** : mécaniques
  renewal sans équivalent pré-renewal (`item_randomopt_db.yml`, `enchantgrade.yml`,
  `item_reform.yml`, `laphine_*.yml` n'existent que dans `db/re`).

### Impact faible pour Moonlight

Formules d'ASPD, temps d'incantation variable/fixe, tableau élémentaire, HP/SP par
niveau : ces sujets sont tranchés par le build (`PRERE`) et n'ont pas à être arbitrés
épisode par épisode.

## 3. Position de Moonlight

Mesuré le 2026‑08‑25 :

- `src/config/renewal.hpp` ne définit que `#define PRERE`. **Aucune** option renewal.
- `moon/job_master.npc:23` refuse la promotion vers les classes 3ᵉ :
  `class >= Job_Rune_Knight && class <= Job_Mechanic`.
- Le roster de mobs vit dans `db/pre-re/mob_db.yml` (1 004 entrées) **plus**
  `db/import/mob_db.yml` et `db/import/mobs/*.yml` — **2 117 entrées au total**.
  C'est le mécanisme par lequel tout le contenu renewal déjà porté a été absorbé.
- Les objets suivent le même schéma via `db/import/items/`.

Autrement dit : **la voie de migration est déjà tracée et éprouvée**. Elle consiste à
déposer le contenu dans `db/import/` et `moon/`, puis à le référencer dans
`moon/scripts_moon.conf`. Les fiches suivantes disent, pour chaque épisode, où en est ce
travail.

## 4. Le seul vrai arbitrage récurrent

Un serveur pré-renewal plafonné à 99/70 ne peut pas reprendre les paliers de kRO. Pour
chaque épisode, il faut trancher :

1. **Niveau d'entrée** — un donjon kRO « 130+ » devient quoi sur une échelle 1‑99 ?
2. **Statistiques des mobs** — recalibrer HP/ATK/DEF, ou reprendre tel quel et assumer
   un contenu « fin de jeu » très dur ?
3. **Récompenses** — les EXP kRO sont dimensionnées pour des courbes 150/175 ; reprises
   telles quelles en pré-renewal elles font exploser la progression.
4. **Objets** — convertir les équipements renewal (ATK/MATK, `EquipLevelMin`) ou les
   remplacer par des équivalents pré-renewal.

Ces quatre questions se reposent à l'identique dans chaque fiche ; elles n'y sont pas
répétées, seules les particularités de l'épisode y figurent.

## Sources

- [Ragnarok Episode Timeline — Hercules Board](https://board.herc.ws/threads/ragnarok-episode-timeline.3554/)
  (`2009.06.17 : Renewal Release (3-1 Jobs)`, `2009.10.14 : 3-2 Jobs`, `2010.03.31 : Baby 3rd Jobs`)
- [Episode XIII.II — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIII.II)
- Arborescence `db/pre-re` et `db/re` de rathena/rathena
