-- Tables de support pour le chatbot Sting (service Groq)
-- À exécuter dans la base utilisée par rAthena (query_sql) + le service Python.

-- File d'attente des requêtes/réponses
CREATE TABLE IF NOT EXISTS `chatbot_queue` (
  `id`         int(11)      NOT NULL AUTO_INCREMENT,
  `reqid`      varchar(40)  NOT NULL,
  `player`     varchar(30)  NOT NULL DEFAULT '',
  `message`    varchar(300) NOT NULL DEFAULT '',
  `player_ctx` varchar(200)          DEFAULT NULL,
  `response`   varchar(500)          DEFAULT NULL,
  `status`     enum('pending','processing','done','error') NOT NULL DEFAULT 'pending',
  `created_at` timestamp    NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`),
  KEY `reqid` (`reqid`),
  KEY `status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Statut online/offline du bot (déco quand quota tokens épuisé)
CREATE TABLE IF NOT EXISTS `chatbot_status` (
  `id`        tinyint(1)   NOT NULL DEFAULT 1,
  `online`    tinyint(1)   NOT NULL DEFAULT 1,
  `resume_at` datetime              DEFAULT NULL,
  `note`      varchar(500)          DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `chatbot_status` (`id`, `online`) VALUES (1, 1)
  ON DUPLICATE KEY UPDATE `online` = `online`;

-- Répliques à dire à voix haute (tools/sting_voice_service.py) : le PNJ y
-- dépose chaque ligne qu'il affiche, avec sa position ; le service la lit, la
-- synthétise et l'efface. La réplique écrite part toujours, service ou non.
CREATE TABLE IF NOT EXISTS `chatbot_voice` (
  `id`         int(11)      NOT NULL AUTO_INCREMENT,
  `line`       varchar(300) NOT NULL DEFAULT '',
  `map`        varchar(16)  NOT NULL DEFAULT '',
  `x`          smallint(6)  NOT NULL DEFAULT 0,
  `y`          smallint(6)  NOT NULL DEFAULT 0,
  `created_at` timestamp    NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
