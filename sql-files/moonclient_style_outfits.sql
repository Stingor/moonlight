--
-- Les tenues de moonclient : code court et garde-robe du compte [Stingor]
--
-- Une tenue est le corps (v7, un par corps habille) et les calques d'un joueur,
-- d'un seul tenant. Le serveur ne lit jamais une couleur : il range des tenues
-- qu'il a jugees sur leur structure. Voir src/map/style_outfits.hpp, et
-- docs/couleurs-du-personnage/conception.md de moonclient (section "Tenues").
--
-- Une tenue n'ecrit rien sur un personnage : le client la propose dans son
-- editeur, et le joueur la valide ensuite par les paquets ordinaires.
--

--
-- Les codes courts. Un code est IMMUABLE : il designe pour toujours la tenue
-- publiee. Le meme compte qui republie la meme tenue retrouve le meme code.
-- `char_id` : le personnage qui l'a publiee. 64 codes par compte au plus.
--
CREATE TABLE IF NOT EXISTS `moonclient_outfit_codes` (
  `code`        char(8)             NOT NULL,
  `account_id`  int(11) unsigned    NOT NULL,
  `char_id`     int(11) unsigned    NOT NULL,
  `data`        blob                NOT NULL,
  `created_at`  datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`code`),
  KEY `account_id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

--
-- La garde-robe : seize rangements nommes par compte, que tous ses personnages
-- relisent. Le nom est en octets UTF-8 (24 au plus), d'ou le varbinary.
--
CREATE TABLE IF NOT EXISTS `moonclient_wardrobe` (
  `account_id`  int(11) unsigned    NOT NULL,
  `slot`        tinyint(3) unsigned NOT NULL,
  `name`        varbinary(24)       NOT NULL,
  `data`        blob                NOT NULL,
  `updated_at`  datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
