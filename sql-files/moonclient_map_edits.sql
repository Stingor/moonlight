--
-- Les editions de carte de moonclient [Stingor]
--
-- Le serveur tient UN document d'edition par carte : le YAML qu'ecrit l'editeur
-- de carte de moonclient, que le serveur ne lit pas. Il le range compresse par
-- zlib, tel que le client l'a envoye, avec une REVISION qui croit de 1 a chaque
-- enregistrement accepte. Voir src/map/map_edits.hpp, et l'ADR 0124 de
-- moonclient.
--
-- Le serveur de carte lit `moonclient_map_edit` entiere au demarrage et la garde
-- en memoire. Seul un enregistrement ecrit ici, en une transaction sur les deux
-- tables.
--

--
-- La derniere revision de chaque carte.
--
-- `text_size` : la taille du texte une fois decompresse. Le client la verifie a
-- l'octet pres.
--
CREATE TABLE IF NOT EXISTS `moonclient_map_edit` (
  `map`          varchar(15)      NOT NULL,
  `revision`     int(11) unsigned NOT NULL DEFAULT 0,
  `text_size`    int(11) unsigned NOT NULL DEFAULT 0,
  `account_id`   int(11) unsigned NOT NULL DEFAULT 0,
  `char_id`      int(11) unsigned NOT NULL DEFAULT 0,
  `author_name`  varchar(23)      NOT NULL DEFAULT '',
  `document`     mediumblob       NOT NULL,
  `updated_at`   datetime         NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`map`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Chaque revision acceptee, pour revenir a la main sur un enregistrement
-- malheureux. Rien ne la relit en jeu.
--
-- Revenir a la revision N d'une carte : la recopier dans `moonclient_map_edit`
-- sous une revision PLUS GRANDE que la courante, puis redemarrer le serveur de
-- carte. Un client compare les revisions par difference : il redemandera.
--
CREATE TABLE IF NOT EXISTS `moonclient_map_edit_history` (
  `map`          varchar(15)      NOT NULL,
  `revision`     int(11) unsigned NOT NULL DEFAULT 0,
  `text_size`    int(11) unsigned NOT NULL DEFAULT 0,
  `account_id`   int(11) unsigned NOT NULL DEFAULT 0,
  `char_id`      int(11) unsigned NOT NULL DEFAULT 0,
  `author_name`  varchar(23)      NOT NULL DEFAULT '',
  `document`     mediumblob       NOT NULL,
  `updated_at`   datetime         NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`map`, `revision`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
