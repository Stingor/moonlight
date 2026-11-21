--
-- La pile d'objets en int32 [Stingor] -- base du JEU (char_server_db)
--
-- Lot 1 de docs/plan-item-amount-int32.md. `struct item::amount` est un int32,
-- et le char-server le lit en SQLDT_INT32. Quand MAX_AMOUNT (src/common/mmo.hpp)
-- depassera 65 535, une colonne smallint unsigned tronquerait la pile -- ou,
-- en mode strict (STRICT_TRANS_TABLES, actif sur nos MariaDB), ferait echouer
-- la sauvegarde du personnage, du stockage ou de l'echoppe.
--
-- A executer AVANT de demarrer un serveur dont MAX_AMOUNT depasse 32 767.
-- L'executer plus tot est sans effet : un int tient toutes les piles
-- actuelles, et le code ne lit pas la largeur de la colonne.
--
-- Les tables sont reconstruites : serveurs arretes le temps des ALTER.
--
-- Deja en int(11) dans le schema amont, rien a faire : `inventory`,
-- `cart_inventory`, `guild_storage`, `guild_storage_log`, `mail_attachments`,
-- `market`, `sales`. La base des journaux (`picklog`.`amount`) est deja en int.
--
-- Hors champ : `barter`.`amount` (stock d'un PNJ de troc) et `db_roulette`
-- ne sont pas des piles d'objets d'un joueur.
--
-- Les stockages supplementaires suivent conf/import/inter_server.yml : si une
-- table y est ajoutee, l'ajouter ici aussi.
--
-- A verifier avant : SELECT TABLE_NAME, COLUMN_TYPE FROM information_schema.COLUMNS
--   WHERE TABLE_SCHEMA = DATABASE() AND COLUMN_NAME = 'amount';
--

-- Stockages : `storage` (ID 0) et ceux de conf/import/inter_server.yml.
-- Le schema amont declare `smallint(11) unsigned NOT NULL default '0'`.
ALTER TABLE `storage`     MODIFY `amount` int(11) unsigned NOT NULL default '0';
ALTER TABLE `storagealt1` MODIFY `amount` int(11) unsigned NOT NULL default '0';
ALTER TABLE `storagealt2` MODIFY `amount` int(11) unsigned NOT NULL default '0';
ALTER TABLE `storagealt3` MODIFY `amount` int(11) unsigned NOT NULL default '0';
ALTER TABLE `storagealt4` MODIFY `amount` int(11) unsigned NOT NULL default '0';
ALTER TABLE `storagecard` MODIFY `amount` int(11) unsigned NOT NULL default '0';

-- Echoppes et boutiques d'achat en autotrade (`smallint(5) unsigned NOT NULL`).
ALTER TABLE `vending_items`     MODIFY `amount` int(11) unsigned NOT NULL;
ALTER TABLE `buyingstore_items` MODIFY `amount` int(11) unsigned NOT NULL;

-- Reserve de l'album de cartes, bornee par MAX_AMOUNT (sql-files/card_album.sql).
ALTER TABLE `card_album` MODIFY `amount` int(11) unsigned NOT NULL DEFAULT 0;
