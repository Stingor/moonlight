--
-- Les teintures des compagnons de moonclient [Stingor]
--
-- Un joueur teint son homoncule et son familier (calque 7 du CZ 0x0F5E). La
-- teinture appartient au COMPAGNON, par son identifiant durable : `hom_id` de la
-- table `homunculus` pour un homoncule, `pet_id` de la table `pet` pour un
-- familier (il suit l'oeuf a l'echange). Elle est faite pour une CLASSE : sous
-- une autre (une evolution), elle dort et ne se diffuse pas. Voir
-- src/map/style_layers.hpp, et docs/couleurs-du-personnage/compagnons.md de
-- moonclient.
--
-- `kind` : 1 homoncule, 2 familier. `data` : une entree de calque, telle qu'elle
-- part dans un ZC 0x0F5F (246 octets au plus). `char_id` : le dernier qui l'a
-- ecrite.
--
-- Supprimee avec le compagnon (le serveur de carte, a la suppression de
-- l'homoncule ou du familier ; le serveur de personnages, a celle du personnage).
-- Le nettoyage des orphelines, serveur de carte ARRETE :
--   DELETE d FROM `moonclient_companion_dyes` d
--   WHERE (d.`kind` = 1 AND NOT EXISTS (SELECT 1 FROM `homunculus` h WHERE h.`homun_id` = d.`companion_id`))
--      OR (d.`kind` = 2 AND NOT EXISTS (SELECT 1 FROM `pet` p WHERE p.`pet_id` = d.`companion_id`));
--
CREATE TABLE IF NOT EXISTS `moonclient_companion_dyes` (
  `kind`          tinyint(3) unsigned NOT NULL,
  `companion_id`  int(11) unsigned    NOT NULL,
  `class`         int(10) unsigned    NOT NULL,
  `version`       tinyint(3) unsigned NOT NULL DEFAULT 1,
  `data`          blob                NOT NULL,
  `char_id`       int(11) unsigned    NOT NULL DEFAULT 0,
  `updated_at`    datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`kind`, `companion_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Le journal de chaque teinture de compagnon ecrite ou retiree : qui, quel
-- compagnon, pour quelle classe, avant et apres. `source` : 'joueur'. Rien ne le
-- relit en jeu.
--
CREATE TABLE IF NOT EXISTS `moonclient_companion_dye_log` (
  `id`            bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `logged_at`     datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `account_id`    int(11) unsigned    NOT NULL DEFAULT 0,
  `char_id`       int(11) unsigned    NOT NULL DEFAULT 0,
  `kind`          tinyint(3) unsigned NOT NULL,
  `companion_id`  int(11) unsigned    NOT NULL,
  `class`         int(10) unsigned    NOT NULL,
  `source`        varchar(8)          NOT NULL DEFAULT '',
  `before_data`   blob                NOT NULL,
  `after_data`    blob                NOT NULL,
  PRIMARY KEY (`id`),
  KEY `companion` (`kind`, `companion_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
