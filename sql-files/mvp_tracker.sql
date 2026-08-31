-- [Stingor] MVP tracker : le groupe de chasse, ses invitations, ses favoris.
-- Conception : docs/mvp_tracker.md, implementation : docs/mvp_tracker_blueprint.md
-- (depot Bourgeon), lot 3.
--
-- Ce qui est ICI est ce qui doit SURVIVRE a un redemarrage : la composition du
-- groupe et les favoris. Ce qui n'y est PAS l'est volontairement : les
-- OBSERVATIONS (qui a vu quoi, quand, ou) vivent en RAM et meurent avec le
-- map-server, parce qu'un carnet de chasse rouvert sur des heures de mort d'avant
-- la coupure mentirait sans le dire.
--
-- L'identite est le COMPTE MOONLIGHT (login.user_id), jamais le personnage ni le
-- compte de jeu : meme choix que user_ignore.sql, et pour les memes raisons. Un
-- favori suit donc son proprietaire sur tous ses personnages et tous ses postes.
--
-- Aucun nom de personne n'est stocke : un libelle fige ment des le premier
-- renommage. Les noms affiches sont recalcules a la connexion.
--
-- utf8mb4 : un nom de groupe est saisi par un joueur, donc accentue.
-- InnoDB : ecritures concurrentes et cles composites.

--
-- Un groupe de chasse. `owner_user_id` peut changer : quand le proprietaire
-- part, la succession va au membre le plus ancien (ORDER BY joined_at LIMIT 1),
-- sans vote et sans colonne supplementaire.
--
CREATE TABLE IF NOT EXISTS `mvp_group` (
  `group_id`       int(11) unsigned NOT NULL AUTO_INCREMENT,
  `owner_user_id`  int(11) unsigned NOT NULL DEFAULT 0,
  `name`           varchar(32)      NOT NULL DEFAULT '',
  `created_at`     datetime         NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`group_id`),
  KEY `k_owner` (`owner_user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Les membres. PRIMARY KEY (user_id) et non (user_id, group_id) : la regle
-- « un compte = AU PLUS un groupe » devient une contrainte de STOCKAGE, pas une
-- verification applicative qu'un chemin d'erreur peut sauter. L'INSERT echoue,
-- point.
--
CREATE TABLE IF NOT EXISTS `mvp_group_member` (
  `user_id`   int(11) unsigned NOT NULL DEFAULT 0,
  `group_id`  int(11) unsigned NOT NULL DEFAULT 0,
  `joined_at` datetime         NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user_id`),
  KEY `k_group` (`group_id`,`joined_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Les invitations en attente. `expires_at` evite que la table devienne un
-- cimetiere : 7 jours, assez pour un joueur qui ne se connecte que le week-end.
--
CREATE TABLE IF NOT EXISTS `mvp_group_invite` (
  `group_id`     int(11) unsigned NOT NULL DEFAULT 0,
  `user_id`      int(11) unsigned NOT NULL DEFAULT 0,
  `from_user_id` int(11) unsigned NOT NULL DEFAULT 0,
  `expires_at`   datetime         NOT NULL,
  PRIMARY KEY (`group_id`,`user_id`),
  KEY `k_user` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Les favoris, en SQL et non dans SaveData\ : un favori est une DONNEE qui suit
-- le compte, pas une preference d'interface. Ne restent locaux que le seuil
-- d'alerte et le son.
--
-- La cle est (mob_id, map_name) et surtout PAS le slot_id : celui-ci vaut le rang
-- dans le registre construit au boot, donc il bouge des qu'un boss_monster est
-- ajoute. `mob_id` = 0 designe un creneau scripte (Bio Lab, Lord of Death,
-- Thanatos), ou la carte suffit a identifier.
--
CREATE TABLE IF NOT EXISTS `mvp_favorite` (
  `user_id`  int(11) unsigned     NOT NULL DEFAULT 0,
  `mob_id`   smallint(5) unsigned NOT NULL DEFAULT 0,
  `map_name` varchar(24)          NOT NULL DEFAULT '',
  PRIMARY KEY (`user_id`,`mob_id`,`map_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
