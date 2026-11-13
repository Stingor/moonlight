--
-- Les teintures des effets de chapeau de moonclient [Stingor]
--
-- Un joueur teint l'effet de chapeau qu'il affiche (calque 8 du CZ 0x0F5E).
-- La teinture appartient a l'EXEMPLAIRE de l'objet qui pose l'effet, par son
-- `unique_id`, et au NUMERO d'effet de chapeau (`e_hat_effects`) : un objet dont
-- le script pose deux effets en a deux. Elle suit l'objet a l'echange, a la
-- vente, au storage et au courrier, et ne s'applique qu'a l'objet qui porte a la
-- fois ce `unique_id` ET cet identifiant `nameid`. Voir src/map/style_layers.hpp,
-- et docs/couleurs-du-personnage/effets-de-chapeau.md de moonclient.
--
-- `data` : une entree de calque, telle qu'elle part dans un ZC 0x0F5F (15
-- octets : un seul reglage). `char_id` : le dernier qui l'a ecrite.
--
-- Une teinture dont l'objet a disparu reste orpheline. Le nettoyage, serveur de
-- carte ARRETE (il garde les teintures en memoire) :
--   DELETE d FROM `moonclient_item_effect_dyes` d
--   WHERE NOT EXISTS (SELECT 1 FROM `inventory` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `cart_inventory` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `storage` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `guild_storage` WHERE `unique_id` = d.`unique_id`)
--     AND NOT EXISTS (SELECT 1 FROM `mail_attachments` WHERE `unique_id` = d.`unique_id`);
--
CREATE TABLE IF NOT EXISTS `moonclient_item_effect_dyes` (
  `unique_id`   bigint(20) unsigned   NOT NULL,
  `hat_effect`  smallint(5) unsigned  NOT NULL,
  `nameid`      int(10) unsigned      NOT NULL,
  `version`     tinyint(3) unsigned   NOT NULL DEFAULT 1,
  `data`        blob                  NOT NULL,
  `char_id`     int(11) unsigned      NOT NULL DEFAULT 0,
  `updated_at`  datetime              NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`unique_id`, `hat_effect`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Le journal de chaque teinture d'effet de chapeau ecrite ou retiree : qui, quel
-- exemplaire, quel effet, avant et apres. `source` : 'joueur' ou 'staff'
-- (@moonstyle). Rien ne le relit en jeu.
--
CREATE TABLE IF NOT EXISTS `moonclient_item_effect_dye_log` (
  `id`          bigint(20) unsigned   NOT NULL AUTO_INCREMENT,
  `logged_at`   datetime              NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `account_id`  int(11) unsigned      NOT NULL DEFAULT 0,
  `char_id`     int(11) unsigned      NOT NULL DEFAULT 0,
  `unique_id`   bigint(20) unsigned   NOT NULL,
  `hat_effect`  smallint(5) unsigned  NOT NULL,
  `nameid`      int(10) unsigned      NOT NULL,
  `source`      varchar(8)            NOT NULL DEFAULT '',
  `before_data` blob                  NOT NULL,
  `after_data`  blob                  NOT NULL,
  PRIMARY KEY (`id`),
  KEY `unique_id` (`unique_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
