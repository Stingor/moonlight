--
-- Presence Moonlight [Stingor] -- le calendrier passe du YAML aux EDITIONS
-- mensuelles tirees dans un pool par le site (ADR 0156 de moonclient).
--
-- Pour une base qui a deja les tables de moonlight_attendance.sql dans leur
-- premiere forme. Puis moonlight_attendance_seed.sql.
--   mysql rathena < upgrade_moonlight_attendance_editions.sql
--

CREATE TABLE IF NOT EXISTS `moonlight_attendance_pool` (
  `id`       int(10) unsigned     NOT NULL AUTO_INCREMENT,
  `tier`     enum('day','milestone') NOT NULL,
  `item_id`  int(10) unsigned     NOT NULL,
  `amount`   smallint(5) unsigned NOT NULL DEFAULT 1,
  `weight`   smallint(5) unsigned NOT NULL DEFAULT 10,
  `enabled`  tinyint(1) unsigned  NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  KEY `tier_enabled` (`tier`, `enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `moonlight_attendance_edition` (
  `id`        int(10) unsigned NOT NULL AUTO_INCREMENT,
  `month`     date             NOT NULL,
  `drawn_at`  datetime         NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `drawn_by`  int(10) unsigned          DEFAULT NULL,
  `edited_at` datetime                  DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `month` (`month`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DELETE FROM `moonlight_attendance_calendar`;
ALTER TABLE `moonlight_attendance_calendar`
  ADD COLUMN `edition_id` int(10) unsigned NOT NULL FIRST,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`edition_id`, `day`, `slot`);

ALTER TABLE `moonlight_attendance`
  ADD COLUMN `edition_id` int(10) unsigned DEFAULT NULL AFTER `web_used`;

ALTER TABLE `moonlight_attendance_log`
  ADD COLUMN `edition_id` int(10) unsigned DEFAULT NULL AFTER `cycle_start`;
