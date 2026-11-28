--
-- Presence Moonlight [Stingor] -- les bonus de date (ADR 0156 de moonclient).
-- Pour une base qui a deja les editions ; puis le bonus de Noel 2026.
--   mysql rathena < upgrade_moonlight_attendance_bonus.sql
--

--
-- Les BONUS DE DATE (Noel, fetes...) : une reclamation faite ce jour-la, en jeu
-- ou sur le site, recoit en plus les blocs du bonus, quel que soit le jour du
-- cycle. Tenus par l'equipe sur le site ; le map-server les lit et les livre
-- avec la recompense du jour (la reclamation du site les recoit a la
-- connexion). Dans 0x0F6F, les blocs du bonus d'aujourd'hui ont `day` = 0.
--
-- `on_date` : le jour du bonus. `label` : son nom ("Noel"), dit dans le chat
-- et sur le site ; latin1 comme le reste du chat du jeu.
-- Blocs : `item_id` 0 = des points d'event, comme dans le calendrier.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance_bonus_day` (
  `on_date`    date             NOT NULL,
  `label`      varchar(40)      NOT NULL DEFAULT '',
  `created_by` int(10) unsigned          DEFAULT NULL,
  PRIMARY KEY (`on_date`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `moonlight_attendance_bonus` (
  `on_date` date                 NOT NULL,
  `slot`    tinyint(3) unsigned  NOT NULL,
  `item_id` int(10) unsigned     NOT NULL,
  `amount`  smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`on_date`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Noel 2026 : une proposition, que l'equipe retouche sur le site.
INSERT IGNORE INTO `moonlight_attendance_bonus_day` (`on_date`, `label`) VALUES ('2026-12-25', 'Noël');
INSERT IGNORE INTO `moonlight_attendance_bonus` (`on_date`, `slot`, `item_id`, `amount`) VALUES
  ('2026-12-25', 0, 0, 10),
  ('2026-12-25', 1, 12246, 1);
