--
-- Album de cartes [Stingor]
--
-- Un conteneur de cartes qui vit HORS du systeme de storage de rAthena, et c'est
-- tout son interet. Un storage classique plafonne a MAX_STORAGE (600), et ce
-- plafond n'est pas negociable : `struct s_storage` voyage entre map-server et
-- char-server par memcpy brut dans un paquet dont la longueur tient sur 16 bits
-- (src/char/int_storage.cpp, mapif_storage_data_loaded). Le maximum absolu de
-- cette architecture est 850 slots. L'album ne cherche donc pas a repousser ce
-- plafond : il en sort.
--
-- Il le peut parce qu'une CARTE n'a pas d'etat d'instance : ni refine, ni
-- options, ni cartes inserees, ni attribut. Le couple (nameid, amount) la decrit
-- entierement, la ou un slot de storage traine 77 octets de `struct item`. Le
-- jeu compte 912 cartes de monstre (db/import/items/item_db_card.yml, seule
-- source chargee — cf. db/CLAUDE.md), donc un album COMPLET fait au plus 912
-- lignes. Les enchantements, qui portent aussi Type: Card mais avec
-- SubType: Enchant, sont exclus (CARD_NORMAL uniquement).
--
-- ── LA REGLE DE JEU ─────────────────────────────────────────────────────────
--
-- La PREMIERE copie d'une carte est SACRIFIEE : elle est consommee et debloque
-- definitivement l'emplacement de cette carte dans l'album. Les copies suivantes
-- s'y empilent et restent retirables. Une carte jamais sacrifiee ne peut pas
-- etre rangee.
--
-- 🔴 L'EXISTENCE DE LA LIGNE EST LE DEBLOCAGE. Il n'y a pas de colonne
-- `unlocked` : une colonne booleenne qui vaut toujours 1 ment des qu'un chemin
-- d'erreur oublie de l'ecrire. `amount` = 0 est donc un etat NORMAL et frequent
-- (emplacement debloque, reserve vide) — ne jamais DELETE une ligne a zero, ce
-- serait rendre au joueur un sacrifice qu'il a paye.
--
-- ── L'IDENTITE ──────────────────────────────────────────────────────────────
--
-- Le COMPTE MOONLIGHT (login.user_id), jamais le personnage ni le compte de jeu :
-- meme choix que user_ignore.sql et mvp_tracker.sql, et pour les memes raisons.
-- Les sacrifices deja payes profitent donc a tous les personnages et a tous les
-- comptes de jeu de la meme personne. C'est un choix de game design, pas une
-- contrainte technique : le storage classique, lui, est par account_id.
--
-- Ecrite au fil de l'eau par le map-server (une requete par operation), sur le
-- modele de user_ignore. Aucun passage par le char-server, donc aucune
-- serialisation de struct et aucun plafond a 850.
--
-- `user_id`     : le compte Moonlight proprietaire de l'album
-- `nameid`      : la carte. La ligne existe => l'emplacement est debloque.
-- `amount`      : copies en reserve, retirables. 0 est legitime.
-- `unlocked_at` : quand le sacrifice a eu lieu. Sert aux statistiques et a
--                 l'ordre « decouvertes recentes » ; jamais a une regle de jeu.
--

CREATE TABLE IF NOT EXISTS `card_album` (
  `user_id`     int(11) unsigned     NOT NULL DEFAULT 0,
  `nameid`      int(10) unsigned     NOT NULL DEFAULT 0,
  `amount`      smallint(5) unsigned NOT NULL DEFAULT 0,
  `unlocked_at` datetime             NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user_id`,`nameid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
