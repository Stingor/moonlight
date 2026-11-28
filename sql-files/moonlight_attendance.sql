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
-- La regle (ecrite dans pc_attendance.cpp et dans App\Services\Attendance) :
--   - un cycle est A OUVRIR si cycle_start est NULL, si
--     DATEDIFF(jour, cycle_start) >= 28, ou si claimed >= 28 ;
--   - refus si last_claim = jour ;
--   - un cycle a ouvrir repart : cycle_start = jour, claimed = 0, web_used = 0 ;
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
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance` (
  `user_id`     int(11) unsigned    NOT NULL,
  `cycle_start` date                         DEFAULT NULL,
  `claimed`     tinyint(3) unsigned NOT NULL DEFAULT 0,
  `last_claim`  date                         DEFAULT NULL,
  `web_used`    tinyint(3) unsigned NOT NULL DEFAULT 0,
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
-- `claimed_on`   : le jour reclame (CURDATE()).
-- `source`       : 'game' ou 'web'.
-- `char_id`      : le personnage qui a recu, une fois livree.
-- `by_mail`      : 1 si elle est partie par RODEX faute de place.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance_log` (
  `id`           int(11) unsigned    NOT NULL AUTO_INCREMENT,
  `user_id`      int(11) unsigned    NOT NULL,
  `cycle_start`  date                NOT NULL,
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
-- Le calendrier, PUBLIE par le map-server depuis
-- db/import/moonlight_attendance.yml a chaque chargement (@reloadattendancedb
-- compris), pour que le site l'affiche. Le YAML fait foi : cette table est
-- videe puis reecrite, jamais editee a la main.
--
-- `day`       : 1 a 28. `slot` : rang de l'item dans le jour (0, 1, ...).
-- `milestone` : 1 pour un jour-palier, repete sur chaque item du jour.
--
CREATE TABLE IF NOT EXISTS `moonlight_attendance_calendar` (
  `day`       tinyint(3) unsigned  NOT NULL,
  `slot`      tinyint(3) unsigned  NOT NULL,
  `item_id`   int(10) unsigned     NOT NULL,
  `amount`    smallint(5) unsigned NOT NULL,
  `milestone` tinyint(1) unsigned  NOT NULL DEFAULT 0,
  PRIMARY KEY (`day`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
