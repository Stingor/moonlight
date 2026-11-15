# Épisode 19 — Issgard, Terre des Fleurs de Neige

| | |
|---|---|
| Nom kRO | 이스가드 / *Issgard, Land of Snow Flowers* |
| Date kRO | **2022‑01‑19** |
| Arc | poursuite de Bagot — le culte de Jörmungandr |
| Niveau kRO visé | **200+** |
| État Moonlight | **mobs importés** (2026‑11‑15, non validé en jeu) ; le reste à faire |

Premier épisode que l'amont n'a pas fusionné. Une conversion complète attend pourtant
dans la PR rAthena [#8527](https://github.com/rathena/rathena/pull/8527), jamais fusionnée :
le chantier redevient un **portage**, sauf pour les compétences des mobs.

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

**Branche principale : aucun script.** `npc/re/` ne contient rien pour `jor_*`, `icas_*` ou
les instances `1@jorchs`, `1@iwp`, `1@whl`. Ce qu'elle possède quand même :

- les **entrées d'instance** dans `db/re/instance_db.yml` (Id 74 à 78) ;
- les **objets** de l'épisode (PR #6563, février 2022) et les **constantes** ;
- **40 mobs `EP19_*`** dans `db/re/mob_db.yml`, mais **commentés** (Id et AegisName seuls).

**PR [#8527](https://github.com/rathena/rathena/pull/8527)** (*Initial release of episode 19 -
Walkthrough Conversion*, Atemo, ouverte depuis le 2024‑07‑30, mise à jour le 2026‑02‑25,
+25 566 lignes, 35 fichiers, renewal). Étudiée le 2026‑11‑15 :

| Contenu | Détail |
|---|---|
| Quêtes | `quests_19.txt` (17 351 lignes, 766 PNJ), 214 quêtes nouvelles |
| Instances | Destruction de l'aéronef, Laboratoire de Bagot, Nid du Serpent Confus, Patrouille Iwin. **La Bataille de simulation (`1@jorlab`) manque** |
| Terrain | spawns `jor_back`, `jor_dun`, `jor_ab` ; warps `icecastle`, `jor_fild`, `jor_dun` |
| Économie | `episode_19.txt`, barters, groupe `D_GW_EXTRACTOR`, Lapine |
| `mob_db` | les 40 mobs avec vitesses, IA, modes et classe ; PNJ et pierres de stockage en coquilles, trois mobs à stats nulles |
| C++ (≈ 60 lignes) | mapflags `nolootnormalmob` et `no_npc_selfdestruction_on_all`, commande `unitisforcewalk`, statut `SC_MTP_W_POTION_100`, groupe `IG_D_GW_EXTRACTOR`, compétence visuelle `DE_BERSERKAIZER`, correctif `SCB_RANGE` |

Ses défauts : **aucun `mob_skill_db`** (signalé deux fois en commentaire), une erreur YAML dans
sa `quest_db` (`Title: Standby: Neutralizing Aircraft` sans guillemets), quelques octets de
coréen mal encodés, et des TODO ouverts (`D_GW_EXTRACTOR`, `DE_BERSERKAIZER`, mapflags et
commandes maison).

Le **déguisement Rgan** n'est pas une mécanique à écrire : la PR utilise `transform 21530,…`,
commande déjà présente sur Moonlight.

### Divine Pride

Les fiches de monstres de [Divine Pride](https://www.divine-pride.net) (API, serveur kROM)
donnent stats, drops et spawns. Elles concordent avec la PR sur 37 mobs sur 40 (stats et
drops identiques, ATK à un point près). Elles ne publient **ni IA ni compétences** pour ces
mobs, alors qu'elles en ont pour d'autres (13 pour Schulang, comme `db/re`).

Outils : `tools/util/fetch_divinepride_mobs.py` (clé dans `~/.divine_pride_key`, une requête
toutes les 1,5 s, arrêt net sur 429, cache hors git) et `tools/util/divinepride_to_mobdb.py`.

**Spawns : les deux sources divergent.** Divine Pride donne plus de mobs (par exemple 70/70/60/60/10
sur `jor_ab01` contre 65/65/35/35/10), des spawns sur `jor_tail` et `jor_back4` à `jor_back6`
absents de la PR, et 6 h de respawn pour Ultra Limacina contre 2 h. À arbitrer.

## 4. État côté Moonlight

**Les cartes sont là** : `icecastle`, `icas_in`, `icas_in2`, `jor_tail`, `jor_back1` à
`jor_back5`, `jor_nest`, `jor_dun01` à `jor_dun03`, `jor_ab01`, `jor_ab02`, `jor_que` et les
quatre cartes d'instance `1@iwp`, `1@whl`, `1@jorlab`, `1@jorchs`, dans
`db/import/map_cache.dat` et `conf/maps_athena.conf`. Le **système de grades**, de la même
période, est porté (commit `20cf36893`, armes seulement).

### Mobs (importés le 2026‑11‑15, non validés en jeu)

| Élément | Détail |
|---|---|
| Fichier | `db/import/mobs/ep19.yml`, dans le Footer de `db/import/mob_db.yml` |
| Base | la PR #8527 : vitesses, IA, modes, classe |
| Compléments Divine Pride | stats des trois mobs à 0 (Aquila, Aquila B, Rgan ES), drops normaux d'Aquila, pierres de stockage entières, noms anglais |
| Conversion | `re_to_prere_mob.py` sans retouche, ATK inchangée, `DamageTaken` et `Resistance` retirés |
| PNJ `EP19_NPC_*` | sept coquilles (Id, AegisName, Name) : ce ne sont que des sprites de PNJ, l'entrée sert à déclarer la constante |
| Pierres de stockage | `Ai: 06` (immobile, n'attaque pas, comme `TREASURE_BOX*`), `Class: Boss`, 1 PV |
| Ordres de grandeur | mob normal ≈ 500 000 à 600 000 PV ; MVP 4,6 à 8,3 M ; ATK 4 700 à 16 300 |
| Matériaux | 22 objets EP19 ajoutés à `db/import/items/item_db_etc.yml` |
| Drops de carte | **écartés** : les 24 cartes EP19 ont des bonus renewal (`bPAtk`, `bSMatk`, `bRes`, paliers +11/+12, `getequiparmorlv()`) à convertir |

**Noms.** Seize mobs n'ont pas de nom anglais (kROM seulement). Onze se déduisent des mobs frères
ou des cartes ; les grades Iwin sont traduits : Trainee, Private, Private First Class, Corporal,
Captain (le PNJ `Captain#iws` de la PR porte le sprite `_SR`). iRO nomme tous les faux Iwin de
l'épisode 20 « Fake Iwin Soldier ».

## 5. Migration pré-renewal

1. **Compétences des mobs** : aucune source. Les écrire à la main, ou les relever sur vidéo
   comme pour le mode défi d'OGH.
2. **Spawns** : choisir entre la PR et Divine Pride.
3. **Cartes EP19** (24) et **armes Glacier** (27) : conversion une par une, comme Illusion.
4. **Quêtes et instances** depuis la PR : 45 objets cités absents, les Id **18119 à 18121**
   déjà pris en pré-renewal (Gusli, Shafka) à copier dans `db/import`, le YAML de leur
   `quest_db` à corriger.
5. **C++** de la PR, après `git diff upstream/master` sur chaque fichier.
6. **Bataille de simulation** : aucune source.

Le prérequis narratif, l'épisode 18, est porté et déployé en production le 2026‑11‑15.

## 6. Verdict

| | |
|---|---|
| Intérêt | **réévalué à la hausse** : la PR #8527 fournit l'essentiel du contenu |
| Ce qui est acquis | les cartes, les entrées d'instance, les objets amont, les grades, les 40 mobs |
| Manque | les compétences des mobs, la Bataille de simulation |
| Recommandation | arbitrer les spawns, puis porter terrain et quêtes depuis la PR ; les compétences en lot séparé |

## Sources

- [PR rAthena #8527 — Episode 19 walkthrough conversion](https://github.com/rathena/rathena/pull/8527)
- [Divine Pride](https://www.divine-pride.net) — API des monstres, serveur kROM
- [Episode XIX — Ragnarok Online Encyclopedia](https://ragnarok-online-encyclopedia.fandom.com/wiki/Episode_XIX)
- [Episode 19: Issgard, Land of Snow Flowers — Hazy Forest](https://hazyforest.com/episodes:19_issgard_land_of_snow_flower)
- [Issgard Land of Snow Flowers — iRO Wiki](https://irowiki.org/wiki/Issgard_Land_of_Snow_Flowers)
- [Annonce kRO d'origine](https://ro.gnjoy.com/news/update/View.asp?seq=268) · [Herosria](https://ro.gnjoy.com/news/update/View.asp?seq=271) · [niveau 260](https://ro.gnjoy.com/news/update/View.asp?seq=275)
