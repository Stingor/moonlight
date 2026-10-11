-- [Stingor] Signalements de joueurs (CZ 0x0BE2, la fenêtre « Report a User »).
-- Alimentée par clif_parse_report_user ; consultée par le staff, dans la base.
-- Rien ne part sur Discord.
--
-- Le compte et le personnage de l'auteur, la carte et sa position viennent de la
-- session, jamais du paquet ; la cible est relue en ligne. Les noms et le texte
-- arrivent dans la page de codes du jeu, comme pour `bug_reports`.
--
-- `reason`  : 0 programme illégal soupçonné · 1 bug ou abus du système
-- `message` : le texte libre du joueur, cent caractères au plus
-- `status`  : à la main du staff
--
-- Les limites (une cible par auteur toutes les trente minutes, vingt
-- signalements par auteur et par jour) se lisent dans cette table : l'index
-- `k_reporter` les sert.
CREATE TABLE IF NOT EXISTS `player_reports` (
  `id`                  bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `created_at`          datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `reporter_account_id` int(11)    unsigned NOT NULL DEFAULT 0,
  `reporter_char_id`    int(11)    unsigned NOT NULL DEFAULT 0,
  `reporter_name`       varchar(30)         NOT NULL DEFAULT '',
  `target_account_id`   int(11)    unsigned NOT NULL DEFAULT 0,
  `target_char_id`      int(11)    unsigned NOT NULL DEFAULT 0,
  `target_name`         varchar(30)         NOT NULL DEFAULT '',
  `map_name`            varchar(24)         NOT NULL DEFAULT '',
  `x`                   smallint(6)         NOT NULL DEFAULT 0,
  `y`                   smallint(6)         NOT NULL DEFAULT 0,
  `reason`              smallint(6)         NOT NULL DEFAULT 0,
  `message`             varchar(101)        NOT NULL DEFAULT '',
  `status`              enum('new','reviewed','dismissed') NOT NULL DEFAULT 'new',
  PRIMARY KEY (`id`),
  KEY `k_reporter` (`reporter_account_id`, `created_at`),
  KEY `k_target`   (`target_account_id`, `created_at`),
  KEY `k_status`   (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
