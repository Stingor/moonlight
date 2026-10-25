--
-- Le classement mondial du mode survivors de moonclient [Stingor]
--
-- Le mode se joue entierement dans le client. Une partie classee part d'un
-- jeton que le serveur de carte delivre (CZ 0x0F4C -> ZC 0x0F4D), et finit par
-- ses composantes (CZ 0x0F4E -> ZC 0x0F4F) : le serveur les borne, refait le
-- score, et range ici la partie acceptee. Le tableau se lit par CZ 0x0F50 ->
-- ZC 0x0F51. Voir src/map/survivors_ranking.cpp, et le cahier du lot 8 de
-- moonclient (docs/survivors/lot-8-classement.md).
--
-- Un TABLEAU se designe par (`ruleset`, `arena`, `class_id`, `board`, `week`) :
--   `ruleset`  : version des regles du client.
--   `arena`    : arene du catalogue du client.
--   `class_id` : classe du mode, pas le job d'apparence.
--   `board`    : 0 partie classee (graine tiree par le serveur), 1 defi de la
--                semaine (graine de la semaine ISO).
--   `week`     : annee ISO x 100 + semaine de l'heure du jeton en defi, 0 en
--                partie classee.
--
-- L'identite est le compte Moonlight (`user_id`, jamais 0) ; `name` est le nom
-- du personnage, fige a la soumission. Les heures sont des secondes UTC depuis
-- 1970, a l'horloge du serveur de carte.
--
-- Le serveur ecrit les deux tables d'une seule transaction, et ne lit que
-- `survivors_best`.
--

--
-- Chaque partie acceptee. Rien ne la relit en jeu : elle garde de quoi
-- reprendre un tableau a la main, ou rejouer une partie quand le serveur saura
-- le faire (`verified`, `journal`, vides tant qu'il ne le sait pas).
--
CREATE TABLE IF NOT EXISTS `survivors_run` (
  `run_id`         bigint(20) unsigned  NOT NULL AUTO_INCREMENT,
  `user_id`        int(11) unsigned     NOT NULL,
  `account_id`     int(11) unsigned     NOT NULL DEFAULT 0,
  `char_id`        int(11) unsigned     NOT NULL DEFAULT 0,
  `name`           varchar(23)          NOT NULL DEFAULT '',
  `ruleset`        smallint(5) unsigned NOT NULL,
  `arena`          smallint(5) unsigned NOT NULL,
  `class_id`       smallint(5) unsigned NOT NULL,
  `board`          tinyint(3) unsigned  NOT NULL,
  `week`           int(11) unsigned     NOT NULL DEFAULT 0,
  `seed`           bigint(20) unsigned  NOT NULL,
  `ticket`         int(11) unsigned     NOT NULL,
  `issued_at`      bigint(20)           NOT NULL,
  `submitted_at`   bigint(20)           NOT NULL,
  `survived_steps` int(11) unsigned     NOT NULL,
  `kill_points`    int(11) unsigned     NOT NULL,
  `kills`          int(11) unsigned     NOT NULL,
  `boss_kills`     smallint(5) unsigned NOT NULL,
  `level`          smallint(5) unsigned NOT NULL,
  `outcome`        tinyint(3) unsigned  NOT NULL,
  `journal_bytes`  int(11) unsigned     NOT NULL DEFAULT 0,
  `score`          bigint(20) unsigned  NOT NULL,
  `verified`       tinyint(1) unsigned  NOT NULL DEFAULT 0,
  `journal`        mediumblob           NULL,
  PRIMARY KEY (`run_id`),
  KEY `user_id` (`user_id`),
  KEY `board` (`ruleset`, `arena`, `class_id`, `board`, `week`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Le meilleur de chaque compte sur chaque tableau : la partie `run_id` de
-- `survivors_run`. Le tableau se lit par
--   ORDER BY `score` DESC, `survived_steps` DESC, `run_id` ASC LIMIT 20
-- et le rang d'un compte est un de plus que les lignes devant lui dans cet
-- ordre. Une partie ne remplace la ligne que si son score est PLUS GRAND.
--
CREATE TABLE IF NOT EXISTS `survivors_best` (
  `user_id`        int(11) unsigned     NOT NULL,
  `ruleset`        smallint(5) unsigned NOT NULL,
  `arena`          smallint(5) unsigned NOT NULL,
  `class_id`       smallint(5) unsigned NOT NULL,
  `board`          tinyint(3) unsigned  NOT NULL,
  `week`           int(11) unsigned     NOT NULL DEFAULT 0,
  `run_id`         bigint(20) unsigned  NOT NULL,
  `char_id`        int(11) unsigned     NOT NULL DEFAULT 0,
  `name`           varchar(23)          NOT NULL DEFAULT '',
  `score`          bigint(20) unsigned  NOT NULL,
  `survived_steps` int(11) unsigned     NOT NULL,
  `level`          smallint(5) unsigned NOT NULL,
  `verified`       tinyint(1) unsigned  NOT NULL DEFAULT 0,
  PRIMARY KEY (`user_id`, `ruleset`, `arena`, `class_id`, `board`, `week`),
  KEY `ranking` (`ruleset`, `arena`, `class_id`, `board`, `week`, `score`, `survived_steps`, `run_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
