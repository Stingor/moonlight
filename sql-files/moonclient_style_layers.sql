--
-- Les teintures de calques de moonclient [Stingor]
--
-- A cote de la recette v7 du corps (char_reg_str `bourgeon_style$`, intacte), un
-- joueur teint sa tete, sa peau, ses coiffes et sa cape. Le serveur ne lit jamais
-- une couleur : il range des entrees qu'il a jugees sur leur structure, et les
-- rediffuse. Voir src/map/style_layers.hpp, et l'ADR 0146 de moonclient.
--
-- Une entree : [taille:1][calque:1][cle:4][profil:1][drapeaux:1][masque:2] puis
-- 5 octets par reglage non neutre. `version` dit la disposition du BLOB.
-- Depuis le profil mixte (2026-11-10), une entree peut porter une queue (drapeau
-- 0x02) : reste et retouches, 246 octets par entree au plus. Le BLOB (64 Kio)
-- garde une ligne pleine de personnage (16 entrees, moins de 4 Kio) : aucune
-- mise a jour de colonne n'est necessaire, et les lignes d'avant se relisent
-- telles quelles.
--

--
-- La tete et la peau, une ligne par personnage : la suite de ses entrees, la tete
-- sous sa coupe et sa couleur. Supprimee avec le personnage (char_delete).
--
CREATE TABLE IF NOT EXISTS `moonclient_style_layers` (
  `char_id`     int(11) unsigned    NOT NULL,
  `version`     tinyint(3) unsigned NOT NULL DEFAULT 1,
  `data`        blob                NOT NULL,
  `updated_at`  datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- La teinture d'un EXEMPLAIRE d'objet, une ligne par `unique_id`. Elle suit
-- l'objet d'un inventaire a l'autre, et ne s'applique qu'a l'objet qui porte a la
-- fois ce `unique_id` ET cet identifiant `nameid`. `char_id` : le dernier qui l'a
-- ecrite.
--
-- Une teinture dont l'objet a disparu reste orpheline. Le nettoyage :
--   DELETE d FROM `moonclient_item_dyes` d
--   WHERE NOT EXISTS (SELECT 1 FROM `inventory` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `cart_inventory` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `storage` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `guild_storage` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `mail_attachments` WHERE `unique_id` = d.`unique_id`);
-- serveur de carte ARRETE : il garde les teintures en memoire.
--
CREATE TABLE IF NOT EXISTS `moonclient_item_dyes` (
  `unique_id`   bigint(20) unsigned NOT NULL,
  `nameid`      int(10) unsigned    NOT NULL,
  `version`     tinyint(3) unsigned NOT NULL DEFAULT 1,
  `data`        blob                NOT NULL,
  `char_id`     int(11) unsigned    NOT NULL DEFAULT 0,
  `updated_at`  datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`unique_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Le journal de chaque teinture d'objet ecrite ou retiree : qui, quel exemplaire,
-- avant et apres. `source` : 'joueur' (atelier du styliste) ou 'staff'
-- (@moonstyle). Rien ne le relit en jeu.
--
CREATE TABLE IF NOT EXISTS `moonclient_item_dye_log` (
  `id`          bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `logged_at`   datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `account_id`  int(11) unsigned    NOT NULL DEFAULT 0,
  `char_id`     int(11) unsigned    NOT NULL DEFAULT 0,
  `unique_id`   bigint(20) unsigned NOT NULL,
  `nameid`      int(10) unsigned    NOT NULL,
  `source`      varchar(8)          NOT NULL DEFAULT '',
  `before_data` blob                NOT NULL,
  `after_data`  blob                NOT NULL,
  PRIMARY KEY (`id`),
  KEY `unique_id` (`unique_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
