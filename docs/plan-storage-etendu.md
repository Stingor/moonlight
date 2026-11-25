# Plan — le coffre de compte s'agrandit (MAX_STORAGE 600 → N)

État (2026-11-25) : **plan, rien d'engagé.** La branche `teststorage`
(ccfe4f590 puis 4bb191775, juillet 2026) sert de référence mais ne se fusionne
pas : 4 commits d'avance, 600 de retard, 37 fichiers touchés. On refait sur
`main`, par lots déployables un à un.

## Pourquoi c'est un chantier

Seul moonclient peut encore jouer : sans poignée de main, le serveur déconnecte
au bout de 15 s + 5 s (`clif_moonclient_check_handshake_timer`). La limite du
client natif ne compte donc plus. Ce qui reste bloque à quatre étages :

1. **La structure.** `s_storage` est une union : inventaire, chariot, coffre et
   coffre de guilde ont tous la taille du plus grand tableau. `map_session_data`
   en porte quatre (`storage`, `premiumStorage`, `inventory`, `cart`).
2. **L'échange entre le map-server et le char-server.** Le coffre passe en
   struct brute dans une trame dont la longueur tient sur 16 bits :
   600 × 79 = 47 400 octets aujourd'hui, et la limite se situe vers
   **829 cases**.
3. **La sauvegarde SQL.** `char_memitemdata_to_sql` compare chaque ligne de la
   base au tableau entier. Le coût est en O(lignes × cases) et tourne sur le
   char-server, qui n'a qu'un seul fil.
4. **La pile.** Deux `struct s_storage stor;` locaux (`int_storage.cpp`, lecture
   et sauvegarde) et un `map_session_data sd;` local (`map.cpp:4311`, la
   console) grossissent avec le plafond.

Ce qui tient déjà : l'index de case et le paquet `0x00f2` de compteur et de
capacité sont en `uint16` (plafond absolu **65 534**), `clif_storagelist`
découpe déjà la liste en trames, les tampons de trames sont `static`, et
moonclient dessine la liste avec un `ImGuiListClipper`.

## Décisions à prendre (lot 0)

| # | Question | Options | Proposition |
|---|---|---|---|
| D1 | Plafond de compilation `MAX_STORAGE` | 1 000 / 2 000 / 5 000 / 10 000 | **3 600, décidé (2026-11-25)** : 6 coffres de 600. Chaque ancien coffre est plafonné à 600 (y compris avec le VIP), donc le contenu fusionné tient toujours : aucun compte n'est refusé par construction. On pourra monter plus tard |
| D2 | Où vivent les cases | dans la struct, ce qui coûte le plafond en mémoire / sur le tas, à `max_amount` | **Dans la struct, décidé** : 3 600 × 79 = 284 Ko par coffre chargé |
| D3 | Favoris du coffre | F0 / F1 / F2 (voir scénarios) | **F1, décidé** : la marque est portée par la **case**, donc par l'exemplaire. Une armure favorite l'est seule, pas toutes les armures du même type. La marque suit l'objet du sac au coffre et retour (champ `favorite` des deux côtés). Elle ne dépend pas de `unique_id`, qui n'est pas toujours posé |
| D4 | Coffre de guilde | reste à 600 / s'agrandit aussi | **600, décidé** : hors du sujet pour l'instant. Il garde l'union |
| D5 | Ampleur de la séparation | minimale : on sort le coffre de compte de l'union / complète : 4 types, comme la branche | **Minimale, décidé (2026-11-25)** (voir lot 1). La complète reste possible plus tard, en lot à part (voir « Plus tard ») |
| D7 | Coffres virtuels | V1 règles / V2 par identifiant / V3 marque par case | **V3, décidé (2026-11-25)** : la plus libre, et la seule qui reprenne exactement les anciens coffres. Voir F, G et le lot 5 bis |
| D6 | Combien de coffres à la fin | 1 seul / 2 (`storage` + `storagecard`) | **1 seul** (orientation du joueur, 2026-11-25), à confirmer par le plancher que mesure le lot 0. Les 6 tables (`storage`, `storagealt1-4`, `storagecard`) se fusionnent dans `storage` au lot 8 |

Pour D1 et D2 : avec des tableaux dans la struct, **c'est le plafond de
compilation qui coûte la mémoire, pas le `Max` configuré**. Un `Max` à 1 000
sous un plafond à 5 000 réserve quand même 5 000 cases par coffre chargé.

---

## Scénarios chiffrés

Bases : `struct item` compacté = **79 octets**. En trame client, un objet
empilable (`NORMALITEM_INFO`) fait **36 octets** et un équipement
(`EQUIPITEM_INFO`, à notre PACKETVER) **68 octets**. Une entrée de prix
`0x0F0F` fait 15 octets. Une trame client fait au plus 32 767 octets, soit
910 objets empilables ou 481 équipements.

### A. Mémoire du map-server par joueur connecté

Avec la séparation, seuls `storage` et `premiumStorage` suivent N ;
l'inventaire et le chariot gardent l'union à 600.

| N (plafond) | 2 coffres × N × 79 | 50 connectés | 200 connectés | Si l'union était gardée (4 × N × 79) |
|---|---|---|---|---|
| 600 (actuel) | 95 Ko | 4,7 Mo | 19 Mo | 190 Ko / joueur |
| 1 000 | 158 Ko | 7,9 Mo | 32 Mo | 316 Ko |
| 2 000 | 316 Ko | 16 Mo | 63 Mo | 632 Ko |
| 5 000 | 790 Ko | 40 Mo | 158 Mo | 1,58 Mo |
| 10 000 | 1,58 Mo | 79 Mo | 316 Mo | 3,16 Mo |

À ajouter : 4 paires de tampons `static` de trames (`MAX_ITEMLIST` suit
`MAX_STORAGE`), soit 4 × N × (36 + 68) octets en tout, c'est-à-dire 2 Mo à
5 000. C'est une dépense fixe, pas par joueur.

**Avec un seul coffre (D6, l'orientation retenue)**, `premiumStorage` n'a plus
de raison d'être une fois la fusion faite, et la colonne « 2 coffres » se
divise par deux : 395 Ko par joueur à 5 000 cases, 79 Mo pour 200 connectés.
Le chiffre utile est donc N × 79 par joueur. Jusqu'au lot 8, `premiumStorage`
reste, puisque les coffres alternatifs existent encore. Il ne se retire
qu'après la fusion, dans un lot de nettoyage, et ce retrait vaut sa propre
relecture : chaque `premiumStorage`, `stor_id != 0` et `openstorage2`.

Variante de D2 : allouer `premiumStorage` seulement quand le coffre est ouvert
divise la colonne par deux. C'est un refactor de plus (`ARRAYLENGTH`, copies de
struct), à ne faire que si le tableau A devient gênant.

### B. Échange map ↔ char, coffre plein

| N | Struct brute (branche) | Format par entrées (81 o/case occupée) | Trames de 64 Ko |
|---|---|---|---|
| 600 | 47 Ko | 49 Ko | 1 |
| 1 000 | 79 Ko | 81 Ko | 2 |
| 2 000 | 158 Ko | 162 Ko | 3 |
| 5 000 | 395 Ko | 405 Ko | 7 |
| 10 000 | 790 Ko | 810 Ko | 13 |

La différence apparaît sur un coffre **peu rempli**. La struct brute envoie
toujours N × 79 octets. Le format par entrées n'envoie que ce qui est occupé :
un coffre de 5 000 cases rempli à 10 % pèse 40 Ko au lieu de 395 Ko, à chaque
chargement (connexion, ouverture d'un coffre alternatif) et à chaque
sauvegarde.

### C. Ouverture du coffre côté client, coffre plein

| N | Tout empilable | Tout équipement | Prix 0x0F0F | Trames (empilable / équipement) |
|---|---|---|---|---|
| 600 | 22 Ko | 41 Ko | 9 Ko | 1 / 2 |
| 1 000 | 36 Ko | 68 Ko | 15 Ko | 2 / 3 |
| 2 000 | 72 Ko | 136 Ko | 30 Ko | 3 / 5 |
| 5 000 | 180 Ko | 340 Ko | 75 Ko | 6 / 11 |
| 10 000 | 360 Ko | 680 Ko | 150 Ko | 11 / 21 |

Avec un mélange réaliste (70 % empilable), un coffre plein de 5 000 cases fait
environ 300 Ko par ouverture. Ce n'est pas un problème de débit. Il faut
seulement mesurer le temps de tri et d'empreinte (`StorageRowsFingerprint`)
dans moonclient.

### D. Sauvegarde SQL : comparaisons, coffre plein et inchangé

| N | Aujourd'hui (≈ N²/2) | Estimation char-server bloqué* | Après le lot 3 (≈ N) |
|---|---|---|---|
| 600 | 180 k | < 2 ms | 600 |
| 1 000 | 500 k | ≈ 5 ms | 1 000 |
| 2 000 | 2 M | ≈ 20 ms | 2 000 |
| 5 000 | 12,5 M | ≈ 60–125 ms | 5 000 |
| 10 000 | 50 M | ≈ 0,25–0,5 s | 10 000 |

\* Estimation à 5–10 ns par comparaison, **à mesurer** au lot 0. Elle se
répète à chaque sauvegarde d'un coffre modifié, et le char-server ne sert
personne pendant ce temps.

Sur disque, une ligne InnoDB fait environ 120 à 150 octets avec l'index (à
mesurer). Un coffre plein de 5 000 cases fait donc environ 0,7 Mo par table,
et un compte qui remplirait ses 6 coffres (`storage`, `storagealt1-4`,
`storagecard`) environ 4,5 Mo.

### E. Favoris du coffre

Aujourd'hui (F0), moonclient tient les favoris **lui-même**, par
**identifiant d'objet**, dans `coffre_favoris.yml` à la racine du client. Ce
fichier vaut pour toute l'installation : il ne suit pas le compte, se perd sur
un autre PC et est partagé par tous les comptes du même PC. Le serveur
l'ignore.

| | F1 — un drapeau par case, comme l'inventaire | F2 — une liste par compte et par identifiant |
|---|---|---|
| Sens | Une **pile** est favorite | Un **objet** est favori, dans toutes ses piles, et le reste même s'il sort puis revient (le sens actuel de moonclient) |
| Mémoire du map-server | **0** : `struct item::favorite` existe déjà (1 octet, compté dans les 79) | Un `std::set` chargé à l'ouverture, ≈ 40–50 o/favori (200 favoris ≈ 10 Ko) |
| SQL | `ALTER TABLE` sur les tables qui restent après la fusion (1 ou 2 au lieu de 6) : colonne `favorite tinyint`, +1 o/ligne, tables reconstruites (durée selon le nombre de lignes) | Table neuve `storage_favorite(account_id, nameid, PK)`, ≈ 30–40 o/ligne avec l'index, indépendante de N |
| Code du char-server | `favorite` entre dans la lecture, la comparaison, l'`UPDATE` et l'`INSERT` pour `TABLE_STORAGE` (`char.cpp` : 594, 626, 671, 680, 714, 740, 821, 853 et le contrôle de schéma à 2874) | Lecture et écriture d'une petite table, sans toucher `char_memitemdata_*` |
| Vers le client | **Déjà transmis** : `clif_item_normal` / `clif_item_equip` posent `PlaceETCTab = favorite`, et moonclient le lit (`net/src/inventory.cpp:111`, `:135`) | Une trame ZC neuve (4 o/favori) à l'ouverture |
| Du client | Une CZ neuve « basculer la case i » + l'accusé | Une CZ neuve « basculer l'objet X » (4 o) + l'accusé |
| Reprise du `.yml` | Une seule fois : le client envoie ses identifiants, le serveur marque les piles qui correspondent | Triviale : le client envoie sa liste si celle du serveur est vide |
| Dépend de N | Non | Non |

**Le piège à trancher quel que soit le choix :** aujourd'hui, un objet favori
de l'inventaire déposé au coffre garde `favorite = 1` en mémoire (la struct est
copiée). Ce drapeau part au client par `PlaceETCTab`, puis disparaît au
rechargement, puisque la table `storage` n'a pas la colonne. Avec F1, il
persisterait : il faut décider s'il suit l'objet ou s'il s'efface au dépôt. Le
paramètre `favorite` de `storage_storageget` règle le sens retour et doit être
relu dans le même mouvement. `compare_item` ignore `favorite`, donc l'empilage
au dépôt n'est pas touché.

**Proposition : F2.** Elle garde le sens que le joueur connaît, ne touche pas
la sauvegarde partagée par l'inventaire et le chariot, et son coût ne dépend
pas de N. F1 se justifie si l'on veut la même règle que dans le sac.

---

## Principe : séparer d'abord, monter le plafond en dernier

Les lots 1 à 3 se font **à 600 cases**. Chacun se déploie et se valide seul,
sans aucun changement visible : s'ils cassent quelque chose, on le voit à
taille constante. Le plafond ne monte qu'au lot 4, puis le `Max` monte par
paliers.

### F. Coffres virtuels (D7)

Le coffre unique se découpe **à l'écran**, comme un disque en partitions. La
barre d'onglets de coffres de moonclient (renommage, icône, remise à zéro)
affiche les partitions au lieu des coffres du serveur. Ranger un objet ne le
déplace pas dans l'inventaire du serveur : aucun risque de perte ou de
duplication, et la capacité reste globale (N).

Rien ne repère un objet de façon stable pour l'instant : au chargement, le
serveur range les objets à la suite (`char.cpp`, boucle sur `NextRow`), donc
les numéros de case changent à chaque connexion, et `unique_id` ne part pas
dans la liste du coffre.

| | V1 — Règles (filtres) | V2 — Par identifiant d'objet | V3 — Marque par case, gardée par le serveur |
|---|---|---|---|
| Serveur | Rien | Rien | Colonne `partition tinyint` sur `storage`, `uint8 partition[N]` **à côté** des cases (et non dans `struct item`), CZ « ranger la case i », ZC des marques |
| Deux exemplaires du même objet dans deux partitions | Non | Non | Oui |
| Reprise des 6 anciens coffres (lot 8) | Non | Approximative | **Exacte** : `partition = ancien stor_id` |
| Favoris | — | F2 en est un cas particulier | F1 en est un cas particulier (une marque de plus) |

Règle à trancher pour V3 : un objet déposé alors que le même objet a déjà une
pile dans une autre partition rejoint-il cette pile, ou en ouvre-t-il une
nouvelle (marque ajoutée à la clé d'empilage) ?

### G. Lien avec la sauvegarde en ligne des réglages (projet plus large)

Le joueur prévoit de garder **côté serveur** les réglages, les profils de
réglage et les dispositions de fenêtres. C'est un chantier à part, mais il
fixe une frontière pour celui-ci :

- **La présentation va dans les réglages en ligne** : noms, icônes et ordre des
  partitions, règles V1, liste de favoris par identifiant (F2), correspondance
  identifiant → partition (V2). Avec ce stockage, F2 et V2 ne demandent plus de
  table à eux : ils deviennent des réglages comme les autres, et ne se perdent
  plus d'un PC à l'autre.
- **Le rattachement d'une case reste avec l'objet** (V3, F1) : dans la table
  `storage`, enregistré dans la même sauvegarde que la case. Une marque gardée
  dans un profil se désynchroniserait à la première coupure entre deux
  sauvegardes, et les numéros de case ne survivent pas au rechargement.
- **La clé commune est le numéro de partition.** Il n'est **jamais réattribué** :
  supprimer une partition dans le profil renvoie ses cases à 0 (« non
  classé ») côté client, et une partition créée plus tard reçoit un numéro
  neuf. Sans cette règle, des objets oubliés réapparaîtraient dans une
  partition sans rapport.
- **Portée** : le coffre est au compte, donc les partitions sont au compte. Les
  réglages poussés aujourd'hui (`clif_moonclient_settings`) sont par
  personnage, dans le registre, et en paires `{id, valeur}` numériques. Le
  stockage en ligne devra porter au moins les portées compte, personnage et
  profil nommé.

Conséquence sur l'ordre : **D3 et D7 se tranchent après le schéma du stockage
en ligne**, ou au moins après sa frontière. Le lot 5 en dépend. Le lot 8 n'en
dépend que si V3 est retenue (la colonne `partition` doit exister avant la
fusion pour reprendre les anciens coffres).

## Lot 0 — Mesures et décisions (aucun code)

- Répartition du remplissage des 6 tables :
  `SELECT account_id, COUNT(*) c FROM storage GROUP BY 1 ORDER BY c DESC LIMIT 20`
  (et pareil pour les cinq autres tables).
- **Ce que la fusion demandera (D6).** Pour chaque compte, le nombre de cases
  occupées des tables qui fusionnent (somme des `COUNT(*)`), puis ce nombre
  après empilage (`COUNT(DISTINCT nameid, card0..3, refine, attribute,
  identify, expire_time, bound, unique_id)` sur l'union des tables, pour les
  seuls objets empilables). Le maximum sur tous les comptes est le **plancher
  de N** : un plafond plus bas rendrait la fusion impossible pour ce compte.
- Pic de connectés, mémoire du map-server aujourd'hui.
- Temps de `char_memitemdata_to_sql` mesuré sur un coffre synthétique de
  600, 5 000 et 10 000 cases, pour remplacer l'estimation du tableau D.
- Trancher D1 à D5.

## Lot 1 — Sortir le coffre de compte de l'union (plafond 600)

- Nouveau type `s_account_storage` : l'en-tête d'aujourd'hui plus
  `items[MAX_STORAGE]`. `items_storage` sort de l'union, qui garde inventaire,
  chariot et guilde (taille 600, comme aujourd'hui).
- `storage` et `premiumStorage` prennent ce type. Il faut suivre les
  44 `u.items_storage` et les signatures qui ne voient que le coffre de compte
  (`storage_delitem`, `storage_storageadd`, `storage_storageget`,
  `storage_storageaddfromcart`, `storage_storagegettocart`).
- Les fonctions **génériques** se dédoublent : `intif_storage_save`
  (inventaire, chariot et coffre), `char_memitemdata_from_sql`,
  `mapif_storage_data_loaded`, `storage_fromsql`.
- **Le compilateur sert de filet** : chaque confusion entre les deux types
  devient une erreur de compilation. Ce filet ne vaut que s'il n'y a pas de
  transtypage pour le contourner. Points de relecture : chaque
  `(struct s_storage*)`, `reinterpret_cast`, `memcpy` et `sizeof(struct s_storage)`,
  et toute lecture de `u.items_storage` à travers une union qui n'est pas un
  coffre de compte (accès par l'union, « punning »).
- Pas de `memset` sur les nouveaux agrégats : `x = T{}`, en gardant `stor_id`
  (la branche en ajoutait).
- `static_assert(sizeof(struct item) == 79)`, et un `static_assert` qui lie la
  taille de chaque trame à sa formule.

Ordre de grandeur : une centaine de lignes, contre plus de 800 pour la
séparation complète (781 `u.items_inventory/cart/guild` à renommer en plus).
Le lot est donc bien plus robuste aux conflits avec un `main` qui bouge.

## Lot 2 — Échange map ↔ char par entrées (plafond 600)

- Le chargement et la sauvegarde du coffre de compte passent par des trames qui
  ne portent que les cases occupées : `(index uint16, item)` × n. Chaque trame
  porte des **entrées entières**, jamais un morceau d'objet. Opcodes à prendre
  libres sur le `main` du moment (la branche prenait 0x308c et 0x388d : à
  revérifier).
- L'en-tête porte : `account_id`, `stor_id`, un `transfer_id` (compteur sur
  32 bits), le total d'entrées, et un drapeau « dernière trame ».
- Le récepteur assemble par `(account_id, stor_id)` et **n'applique qu'à la
  fin**, après avoir vérifié que le nombre reçu égale le total annoncé, que
  chaque index est inférieur à `max_amount` et qu'aucun index n'apparaît deux
  fois. Un assemblage partiel ne touche ni la mémoire ni la base.
- L'accusé de sauvegarde part **après** l'écriture SQL, jamais à la réception
  de la première trame.
- Nettoyage des assemblages en cours : à la déconnexion du map-server (côté
  char), à la sortie du joueur (côté map), et après un délai.
- Une trame d'un autre `transfer_id` pendant un assemblage ne peut pas arriver
  (un seul fil, un seul flux TCP). Si elle arrive quand même, c'est un défaut :
  on l'écrit au journal et on jette les deux transferts, jamais un mélange des
  deux.
- L'inventaire et le chariot gardent `0x3019` / `0x388a` : ils tiennent dans
  une trame.

Bugs à écarter, en scénarios de test : perte ou duplication d'objet si un
assemblage partiel était appliqué ; sauvegarde automatique pendant que le
coffre est ouvert ; fermeture puis réouverture immédiate pendant que la
sauvegarde est en vol ; changement de map-server ; coupure du joueur au milieu
d'un transfert ; garde `storage_flag` (voir le correctif de `@searchid`).

## Lot 3 — Sauvegarde SQL linéaire

- `char_memitemdata_to_sql` indexe le tableau par la **clé qu'il compare déjà**
  (`nameid`, `card[0..3]`, `unique_id`) dans une table de listes d'indices
  rangées dans l'ordre du tableau. Chaque ligne de la base prend le premier
  indice libre de sa clé, comme aujourd'hui.
- ⚠ **Ne pas apparier par `item.id`.** Un objet venu de l'inventaire porte
  l'`id` de la ligne d'**inventaire**, qui ne désigne rien dans la table
  `storage`. Apparier par `id` mettrait à jour ou effacerait la mauvaise ligne.
- La fonction est partagée par l'inventaire, le chariot et la guilde : le gain
  vaut pour tous, et le risque aussi. D'où la preuve d'équivalence : on
  journalise les requêtes produites par l'ancienne et la nouvelle version sur
  un corpus de coffres aléatoires (doublons de même `nameid` et mêmes cartes,
  objets neufs à `id = 0`, objets retirés, quantités changées), puis on compare
  les **listes triées de requêtes** champ par champ, pas des empreintes.

## Lot 4 — Monter le plafond

- `MAX_STORAGE` → plafond de D1.
- Les locaux sur la pile passent sur le tas : `int_storage.cpp` (lecture et
  sauvegarde), `map.cpp:4311` (le `map_session_data` de la console), et tout
  autre local que révèle
  `grep -rn "s_storage [a-z_]*;\|map_session_data [a-z_]*;"`. À 5 000, un seul
  `s_storage` local fait 395 Ko, alors que la pile principale de Windows fait
  1 Mo.
- Relire chaque usage de `MAX_STORAGE` (28 aujourd'hui) : `vip_storage_increase`
  (`battle.cpp:8900`), `script.cpp:8501`, les plafonnements de
  `inter.cpp:949` et `intif.cpp:3698`, le passage VIP → non-VIP
  (`chrif.cpp:1629`, et `storage.cpp:283` pour les cases au-delà de la
  taille), `@searchid` / `storagecountitem`, « tout déposer »
  (`storage.cpp:1331`), le mail, la récupération des objets liés, et
  `storagecard`.
- Monter `Max` dans `conf/import/inter_server.yml` par paliers : 1 000, puis la
  cible.

## Lot 5 — Favoris (selon D3, indépendant des lots 1 à 4)

Voir le tableau E. Côté moonclient : retirer `coffre_favoris.yml` **après** la
reprise (un fichier lu une dernière fois, envoyé, puis renommé en `.migre` et
non supprimé), et ajouter les chaînes dans `lang/*.txt`.

## Lot 5 bis — Partitions V3 (D7), avant la fusion

- **Serveur, map.** `uint8 partition[MAX_STORAGE]` dans `s_account_storage`, à
  côté de `items` : `struct item` ne change pas, ni l'inventaire, ni le
  chariot, ni la guilde. 0 = non classé, 1 à 255 = partitions.
- **Invariant :** la marque suit la case. Chaque écriture ou effacement d'une
  case (`storage_additem`, `storage_delitem`, empilage, « tout déposer », tri,
  retrait vers le sac ou le chariot) écrit ou efface `partition[i]` dans la
  même fonction. Une case vidée retombe à 0, sinon l'objet suivant hériterait
  d'une partition qui n'est pas la sienne. C'est le premier point de la
  relecture.
- **Dépôt :** la CZ de dépôt porte la partition visée (celle de l'onglet
  ouvert). L'empilage ne fusionne qu'avec une pile de **la même partition**
  (la marque entre dans la clé, à côté de `compare_item`), comme pour la
  fusion du lot 8.
- **Ranger :** CZ « case i → partition p » et l'accusé. Pas de déplacement
  d'objet, seulement la marque et `dirty`.
- **Échange map ↔ char (lot 2) :** l'entrée devient `(index, partition, item)`,
  soit 82 o au lieu de 81. Sauvegarde SQL : `partition` entre dans la
  comparaison et l'`UPDATE` pour `TABLE_STORAGE` seulement.
- **Vers le client :** une ZC des marques (index et partition, 3 o par case
  occupée, soit 15 Ko à 5 000 cases), après la liste, sur le modèle de 0x0F0F.
  Sessions vérifiées seulement.
- **Favoris (D3) :** avec V3, F1 devient le choix cohérent. C'est une marque
  par case, indépendante de la partition (une case peut être favorite **et**
  rangée), portée par `struct item::favorite`, qui existe déjà.
- **Ce qui va dans le profil en ligne (G) :** le nom, l'icône et l'ordre de
  chaque partition, et le prochain numéro libre (jamais réattribué). Le
  serveur ne lit pas ces définitions.
- **Plafond : 255 partitions par compte.** Passer en `uint16` coûte 5 Ko de
  plus à 5 000 cases, si un jour il le faut.

## Lot 6 — moonclient

- Essai `tests/ui` : une liste synthétique de N cases (600, 5 000, 10 000) ;
  vérifier le compteur et la capacité (`tally.capacity`), la recherche, le tri,
  le cache de cellules (`storage_cells_cache`), et que l'empreinte ne recalcule
  pas tout à chaque image.
- Analyseur `storage_list` : aucune hypothèse de taille ; essai d'une liste
  répartie sur 11 trames.

## Lot 7 — Banc de test (base de test)

- Script SQL qui remplit un coffre de N objets mélangés : empilables,
  équipements avec cartes et options, liés, à date d'expiration.
- Scénarios : ouvrir, fermer, se reconnecter, changer de map-server, sauvegarde
  automatique coffre ouvert, coffre plein qui refuse un dépôt, empilage au
  dépôt, « tout déposer », coffres alternatifs, VIP qui expire avec des cases
  au-delà, coupure pendant un transfert.
- Preuve : export **complet** des lignes avant et après, trié, puis comparé
  ligne à ligne. Pas d'empreinte.

## Lot 8 — Fusionner les coffres (D6 : 6 → 2, ou 6 → 1)

Ce lot est le seul **irréversible** du chantier : il déplace des objets
réels. Il se fait serveur **arrêté**, par un script SQL rejouable, après une
sauvegarde complète.

- **Destination.** Les cinq tables `storagealt1-4` et `storagecard` entrent dans
  `storage` (D6 : un seul coffre). La commande qui envoie toutes les cartes
  vers `storagecard` est retirée ou redirigée vers le coffre unique, à décider.
  Les cartes y retrouvent leur onglet de type d'objet dans moonclient.
- **Empilage.** Deux piles du même objet fusionnent selon la règle de
  `compare_item` (`nameid`, `identify`, `refine`, `attribute`, `expire_time`,
  `bound`, `unique_id`, cartes), et **seulement pour un objet empilable**. Un
  équipement garde sa case. Une pile fusionnée ne dépasse pas `MAX_AMOUNT`
  (99 999) : le surplus reste dans une seconde pile.
- **Capacité.** À 3 600 (D1), la somme des 6 coffres tient par construction.
  Le contrôle reste quand même dans le script, comme garde-fou : un compte dont
  le contenu fusionné dépasserait `Max` **ne se fusionne pas**, le script le
  liste et le laisse intact. On ne tronque jamais.
- **Preuve.** Avant et après, par compte, la somme des quantités par clé
  d'empilage doit être identique, et le nombre d'équipements aussi. On compare
  deux exports triés ligne à ligne, pas des empreintes.
- **Favoris.** Avec F1, une pile fusionnée est favorite si l'une des piles
  d'origine l'était. Avec F2, rien à faire : la liste ne dépend pas du coffre.
- **Partitions (V3).** Chaque case reçoit `partition = ancien stor_id`, et deux
  piles de partitions différentes ne fusionnent pas. Le profil en ligne du
  compte reçoit une partition par ancien coffre, avec son nom (le `Name` de
  `inter_server.yml`, ou celui que le joueur lui avait donné dans moonclient)
  et son icône.
- **Ce qui disparaît** une fois les tables vidées :
  - les commandes `storagealt1-4` (`conf/import/atcommands.yml`) et leurs droits
    (`conf/import/groups.yml:88`) ;
  - avec un seul coffre, aussi `storagealt5` / `storagecard` et l'envoi de
    toutes les cartes vers `storagecard` ;
  - les entrées de `conf/import/inter_server.yml` ;
  - les `openstorage2` des scripts qui visent ces numéros (à relever) ;
  - les tables elles-mêmes, renommées en `_ancien` et pas supprimées tant que
    R3 n'est pas passée.
- **moonclient.** Les onglets de coffre viennent du serveur (`StorageShelf`),
  donc ils disparaissent d'eux-mêmes. En revanche, les noms et icônes que le
  joueur a posés sur un onglet sont rangés par numéro de coffre côté client :
  les entrées des numéros disparus se nettoient, et ne se recollent pas sur un
  autre onglet. Avec un seul coffre, la barre d'onglets peut aussi
  disparaître.

Ce lot vient **après** le lot 4 et après la montée de `Max`. On ne fusionne
que dans un coffre qui peut déjà tout recevoir.

---

## Relectures

Chaque relecture est faite par un agent **indépendant** qui reçoit le diff et
la liste ci-dessous, pas les conclusions de l'auteur. Ses constats sont
vérifiés, corrigés, puis consignés ici, comme dans `plan-zeny-int64.md`.

| # | Quand | Ce qu'elle cherche |
|---|---|---|
| R1 | Après le lot 1 | Transtypages qui contournent les types, lectures par l'union, `sizeof` et `ARRAYLENGTH` du mauvais type, `memset`, fonctions génériques oubliées |
| R2 | Après les lots 2 et 3 | Lecture adversariale « peut-on perdre ou dupliquer un objet ? » : assemblage partiel, ordre accusé / SQL, nettoyage, appariement de la sauvegarde, équivalence des requêtes |
| R3 | Avant de monter `Max` en production | Le chantier entier et les résultats du banc ; locaux sur la pile ; usages de `MAX_STORAGE` ; favoris |
| R5 | Après le lot 5 bis | Chaque écriture ou effacement de case met-il sa marque à jour ? Une case vidée retombe-t-elle à 0 ? L'empilage respecte-t-il la partition ? La marque passe-t-elle partout dans l'échange et la sauvegarde ? |
| R4 | Avant de lancer la fusion en production | Le script du lot 8 et sa preuve, rejoués sur une copie de la base de production : empilage, comptes refusés, aucun objet perdu ni créé |

## Déploiement

1. Sauvegarde SQL des 6 tables avant chaque étape.
2. Lots 1 à 3 à 600 cases, en production pendant au moins une semaine.
3. Lot 4 avec `Max` à 1 000, puis à la cible.
4. **Retour en arrière** : on ne descend jamais `Max` sous le remplissage le
   plus élevé (requête du lot 0). Au-delà, des cases deviendraient
   inaccessibles. Le rollback du code reste possible tant que `Max` n'a pas
   dépassé 600.

## Plus tard (hors de ce chantier)

- **Séparation complète** (inventaire, chariot et guilde en trois types
  distincts). C'est un pur renommage (~780 `u.items_*`) que le compilateur
  vérifie, sans changement de comportement. À faire quand `main` est calme, ou
  quand le coffre de guilde (D4) devient le sujet. Gains : environ 63 Ko par
  joueur (inventaire et chariot dimensionnés à 200 au lieu de 600), des
  échanges d'inventaire et de chariot trois fois plus légers entre serveurs,
  et des confusions entre ces trois types impossibles à compiler. La branche
  `teststorage` (ccfe4f590) en donne un modèle.
- **Liste du coffre envoyée une fois par session**, puis seulement ce qui
  change : c'est le poste qui pèse sur la connexion internet d'un serveur
  hébergé à la maison (environ 200 Ko par ouverture d'un coffre plein de
  3 600 cases).

## Ordre conseillé

Lot 0 → lot 1 → R1 → lots 2 et 3 → R2 → déploiement à 600 → lot 6 → lot 4 →
lot 7 → R3 → montée par paliers → lot 5 bis (partitions) → R5 → lot 8 → R4 →
fusion. La colonne `partition` existe donc **avant** la fusion, qui s'en sert
pour reprendre les anciens coffres. Le lot 5 (favoris F1) se fait avec le
lot 5 bis, sur la seule table `storage` : inutile d'ajouter la colonne à des
tables qui vont disparaître. Les définitions de partitions attendent le
stockage en ligne. D'ici là, moonclient les garde dans un fichier local qui
sera repris tel quel.
