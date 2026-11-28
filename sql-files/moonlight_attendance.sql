--
-- Presence (attendance) Moonlight [Stingor] -- ADR 0156 de moonclient
--
-- Un cycle GLISSANT de 28 jours, par COMPTE MOONLIGHT (login.user_id) : meme
-- identite que card_album.sql et mvp_tracker.sql. Il s'ouvre a la premiere
-- reclamation et dure 28 jours de calendrier ; chaque reclamation donne la
-- recompense suivante ; un jour manque est perdu.
--
-- Le map-server ET le site (moonlight-web) lisent et ecrivent ces tables, en
-- transaction (SELECT ... FOR UPDATE sur la ligne du compte). C'est pourquoi
-- l'etat n'est pas dans des variables ## : elles sont copiees dans chaque
-- session ouverte, et deux sessions de la meme personne se doubleraient une
-- reclamation.
--
-- Le JOUR est CURDATE() de ce serveur MySQL, pour le map-server comme pour le
-- site : une seule horloge.
--
-- Le CALENDRIER est une EDITION par mois, que le site tire dans un pool et que
-- l'equipe peut retoucher avant le premier du mois. Un cycle garde l'edition du
-- mois ou il s'est ouvert jusqu'a sa fin : `edition_id` de moonlight_attendance,
-- pose a l'ouverture. L'edition d'un cycle est celle du plus grand `month`
-- inferieur ou egal a son `cycle_start` (une ligne sans edition_id, ou une
-- reclamation sans edition_id, s'y resout).
--
-- La regle (ecrite dans pc_attendance.cpp et dans App\Services\Attendance) :
--   - un cycle est A OUVRIR si cycle_start est NULL, si
--     DATEDIFF(jour, cycle_start) >= 28, ou si claimed >= 28 ;
--   - refus si last_claim = jour ;
--   - un cycle a ouvrir repart : cycle_start = jour, claimed = 0, web_used = 0,
--     edition_id = l'edition du jour ;
--   - par le site : refus si web_used >= 3 ;
--   - sinon claimed += 1, last_claim = jour, et web_used += 1 par le site.
--

--
-- L'etat du cycle d'un compte. Pas de ligne = jamais reclame.
--
-- `cycle_start` : premier jour de la fenetre du cycle en cours.
-- `claimed`     : recompenses obtenues dans ce cycle (0 a 28).
-- `last_claim`  : dernier jour reclame, par le jeu ou le site.
-- `web_used`    : charges du site consommees dans ce cycle (0 a 3).
-- `edition_id`  : l'edition du calendrier que ce cycle suit.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance` (
  `user_id`     int(11) unsigned    NOT NULL,
  `cycle_start` date                         DEFAULT NULL,
  `claimed`     tinyint(3) unsigned NOT NULL DEFAULT 0,
  `last_claim`  date                         DEFAULT NULL,
  `web_used`    tinyint(3) unsigned NOT NULL DEFAULT 0,
  `edition_id`  int(10) unsigned             DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Chaque reclamation. C'est l'historique, et la FILE des recompenses du site :
-- une ligne `source` = 'web' a `delivered_at` NULL attend la prochaine
-- connexion d'un personnage du compte, qui la livre (UPDATE ... WHERE
-- delivered_at IS NULL : une seule session gagne).
--
-- `day`          : rang de la recompense dans le cycle (1 a 28).
-- `cycle_start`  : le cycle dont elle fait partie.
-- `edition_id`   : l'edition de ce cycle : la livraison en donne le jour `day`.
-- `claimed_on`   : le jour reclame (CURDATE()).
-- `source`       : 'game' ou 'web'.
-- `char_id`      : le personnage qui a recu, une fois livree.
-- `by_mail`      : 1 si elle est partie par RODEX faute de place.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance_log` (
  `id`           int(11) unsigned    NOT NULL AUTO_INCREMENT,
  `user_id`      int(11) unsigned    NOT NULL,
  `cycle_start`  date                NOT NULL,
  `edition_id`   int(10) unsigned             DEFAULT NULL,
  `day`          tinyint(3) unsigned NOT NULL,
  `claimed_on`   date                NOT NULL,
  `source`       enum('game','web')  NOT NULL,
  `claimed_at`   datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `delivered_at` datetime                     DEFAULT NULL,
  `char_id`      int(11) unsigned             DEFAULT NULL,
  `by_mail`      tinyint(1) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_day` (`user_id`, `claimed_on`),
  KEY `pending` (`user_id`, `delivered_at`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Le POOL des recompenses, que l'equipe tient sur le site. Un lot = un item et
-- sa quantite. `tier` : 'day' pour les jours ordinaires (un lot par jour),
-- 'milestone' pour les paliers 7, 14, 21 et 28 (deux lots par palier).
-- `weight` : la chance relative d'etre tire. `enabled` = 0 le retire du tirage
-- sans l'effacer.
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

--
-- Une EDITION du calendrier : celle d'un mois (`month` = le premier du mois).
-- Tiree par le site (`drawn_by` NULL : le tirage automatique ; sinon le
-- user_id de l'equipier), retouchable jusqu'au premier du mois.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance_edition` (
  `id`        int(10) unsigned NOT NULL AUTO_INCREMENT,
  `month`     date             NOT NULL,
  `drawn_at`  datetime         NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `drawn_by`  int(10) unsigned          DEFAULT NULL,
  `edited_at` datetime                  DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `month` (`month`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Les 28 jours d'une edition. Ecrit par le site seul ; le map-server le lit.
--
-- `day`       : 1 a 28. `slot` : rang du bloc dans le jour (0, 1, ...).
-- `item_id`   : l'item ; 0 pour des points d'event (#KAFRAPOINTS), `amount`
--               points, credites au compte de jeu qui recoit.
-- `milestone` : 1 pour un jour-palier, repete sur chaque bloc du jour.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance_calendar` (
  `edition_id` int(10) unsigned     NOT NULL,
  `day`        tinyint(3) unsigned  NOT NULL,
  `slot`       tinyint(3) unsigned  NOT NULL,
  `item_id`    int(10) unsigned     NOT NULL,
  `amount`     smallint(5) unsigned NOT NULL,
  `milestone`  tinyint(1) unsigned  NOT NULL DEFAULT 0,
  PRIMARY KEY (`edition_id`, `day`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
