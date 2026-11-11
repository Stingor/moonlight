--
-- La tablee de selection de moonclient [Stingor]
--
-- Ce que chaque personnage MONTRAIT a sa derniere deconnexion : ses variantes v7
-- et les teintures de calques qu'il affiche (tete, peau, suite et peau de ses
-- corps, exemplaires portes), sous les cles affichees. Le serveur de carte
-- l'ecrit a la deconnexion (src/map/style_absent.cpp, style_shown_save) ; le
-- serveur de personnages le rend aux seuls personnages du compte de la session
-- (src/char/char_style_shown.cpp, CH 0x0F65 / HC 0x0F66). Voir l'ADR 0147 et
-- docs/couleurs-du-personnage/conception.md de moonclient, lot L14.
--
-- `data` : [corps:1] puis corps x 52 octets (une entree du 0x0F27 sans son gid),
-- [nombre:1] puis nombre x une entree de calque du 0x0F5F. 3,6 Kio au plus.
-- Jamais la teinture d'un objet range. Un personnage sans rien a montrer n'a pas
-- de ligne. La ligne part avec le personnage (char_delete).
--
-- A installer une fois, sur la base du jeu :
--   mysql -u <utilisateur> -p <base> < sql-files/moonclient_style_shown.sql
--
CREATE TABLE IF NOT EXISTS `moonclient_style_shown` (
  `char_id`     int(11) unsigned    NOT NULL,
  `version`     tinyint(3) unsigned NOT NULL DEFAULT 1,
  `data`        blob                NOT NULL,
  `updated_at`  datetime            NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
