CREATE TABLE IF NOT EXISTS `alootid` (
  `id`     bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `cid`    int(11)    unsigned NOT NULL DEFAULT 0,
  `no`     tinyint(3) unsigned NOT NULL DEFAULT 0,
  `name`   varchar(255)        NOT NULL DEFAULT '',
  `auto`   tinyint(3) unsigned NOT NULL DEFAULT 0,
  `nameid` int(11)    unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY (`cid`, `no`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
