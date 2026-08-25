# Épisode 19 — Issgard, Terre des Fleurs de Neige

| | |
|---|---|
| Nom kRO | 이스가드 / *Issgard, Land of Snow Flowers* |
| Date kRO | **2022‑01‑19** |
| Arc | poursuite de Bagot — le culte de Jörmungandr |
| Niveau kRO visé | **200+** |
| État Moonlight | **rien** — et rAthena non plus |

Premier épisode que l'amont ne convertit pas. À partir d'ici, migrer signifie **écrire**,
pas porter.

## 1. Contenu principal (19 janvier 2022)

### Ville, champs et donjons

| Carte | Nom |
|---|---|
| `icecastle`, `icas_in` | **Château de Glace d'Issgard** |
| `jor_tail` | Queue Gelée |
| `jor_back1`, `jor_back2`, `jor_back3` | Collines, Plaines et Glaciers des Écailles Gelées |
| `jor_nest`, `jor_dun02`, `jor_dun03` | Nid des Rgan |
| `jor_ab01`, `jor_ab02` | Fosse Abandonnée |

### Instances

| Instance | Carte |
|---|---|
| **Patrouille Iwin** | `1@iwp` |
| **Destruction de l'aéronef** | `1@whl` |
| **Bataille de simulation** | `1@jorlab` |
| **Nid du Serpent Confus** | `1@jorchs` |
| **Laboratoire de Bagot** | `1@jorlab` |

### Mécaniques

- **Parchemins de transformation Rgan** — le joueur se déguise pour infiltrer la société
  Rgan ; toute la progression du scénario en dépend.
- Monnaie : **Pétale de Fleur de Neige**, servant aussi à la réputation du Château de Glace.

## 2. Contenu de l'intervalle (2022)

| Date kRO | Contenu |
|---|---|
| 2022‑09‑26 | **Herosria** — nouvel environnement de siège, centré sur les 4ᵉ classes, le dimanche de 20 h à 22 h |
| 2022‑12‑21 | Plafond de niveau porté à **260** |
| — | **Livres VR** (*Virtual Reading Books*) — mini-instances de lore donnant de l'EXP |
| — | Refontes de compétences de 4ᵉ classe #2 et #3, avec nouvelles compétences pour Dragon Knight, Meister et Biolo |
| — | Donjons : Plaines Tordues du Pouvoir, Grotte Souterraine de Mjolnir |

## 3. État côté rAthena

**Aucun script.** Vérifié : `npc/re/mobs/dungeons/` ne contient aucun fichier `jor_*`,
`icas_*` ou `iss_*`, et aucun script d'instance ne référence `1@jorchs`, `1@iwp` ou
`1@whl`.

Ce que rAthena possède quand même :

- les **entrées d'instance** dans `db/re/instance_db.yml` (`Iwin Patrol`,
  `Airship Destruction`, `Simulation Battle`, `Confused Snake's Nest`,
  `Bagot Laboratory`) — des coquilles, sans script derrière ;
- les **objets** de l'épisode (commit `Episode 19 items (kRO 2022-01-19 patch)`, PR #6563,
  février 2022) ;
- les **constantes** (`Episode 19 constants`, janvier 2022).

Autrement dit : le catalogue existe, le contenu jouable non.

## 4. État côté Moonlight

**Rien.** Aucun fichier, aucune référence.

En revanche — et c'est le fait notable — **les cartes sont là** :
`icecastle`, `icas_in`, `jor_tail`, `jor_back1/2/3`, `jor_nest`, `jor_dun02`, `jor_dun03`,
`jor_ab01`, `jor_ab02`, `1@jorchs` sont toutes présentes dans
`db/import/map_cache.dat`. Le client de Moonlight sait afficher Issgard.

## 5. Migration pré-renewal

Ce n'est plus une migration, c'est un **développement**. Il faudrait :

1. créer le roster de mobs d'Issgard depuis zéro (aucune donnée `mob_db` amont utilisable
   telle quelle, et rien de rééchelonné) ;
2. écrire les spawns ;
3. écrire cinq scripts d'instance ;
4. écrire la chaîne de quêtes, qui repose entièrement sur le déguisement Rgan — donc
   sur une mécanique de transformation à implémenter ;
5. tout cela pour du contenu conçu au niveau 200+, à ramener à 99.

Le prérequis narratif est l'épisode 18, lui-même non porté.

## 6. Verdict

| | |
|---|---|
| Intérêt | **faible à court terme** — coût d'écriture disproportionné |
| Ce qui est acquis | les cartes ; c'est tout |
| Recommandation | ne pas ouvrir ce chantier tant que 16.2 → 18 ne sont pas soldés |

## Sources

- [Episode XIX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIX)
- [Episode 19: Issgard, Land of Snow Flowers — Hazy Forest](https://hazyforest.com/episodes:19_issgard_land_of_snow_flower)
- [Issgard Land of Snow Flowers — iRO Wiki](https://irowiki.org/wiki/Issgard_Land_of_Snow_Flowers)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=268) · [Herosria](https://ro.gnjoy.com/news/update/View.asp?seq=271) · [niveau 260](https://ro.gnjoy.com/news/update/View.asp?seq=275)
