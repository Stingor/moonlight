-- [Stingor] Rapports de bug in-game (client Bourgeon CZ 0x0F13).
-- Alimentée par clif_parse_bourgeon_bug_report ; lue via le site moonlight.
-- utf8mb4 : les messages joueurs contiennent des accents (et parfois des emoji).
CREATE TABLE IF NOT EXISTS `bug_reports` (
  `id`         bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int(11)    unsigned NOT NULL DEFAULT 0,
  `char_id`    int(11)    unsigned NOT NULL DEFAULT 0,
  `char_name`  varchar(30)         NOT NULL DEFAULT '',
  `category`   varchar(16)         NOT NULL DEFAULT 'generic',  -- generic/item/skill/npc/quest
  `context`    text                         DEFAULT NULL,        -- JSON machine, ex. {"item_id":501}
  `message`    varchar(512)        NOT NULL DEFAULT '',          -- texte libre du joueur
  `map_name`   varchar(24)         NOT NULL DEFAULT '',
  `x`          smallint(6)         NOT NULL DEFAULT 0,
  `y`          smallint(6)         NOT NULL DEFAULT 0,
  `status`     enum('new','triaged','resolved','wontfix') NOT NULL DEFAULT 'new',
  `created_at` datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `k_status`   (`status`),
  KEY `k_category` (`category`),
  KEY `k_account`  (`account_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;
