--
-- Les clones de consommables rejoignent leur item de base [Stingor] -- base du JEU (char_server_db)
--
-- Les items de db/import/items listes dans `moon_clone_map` sont des copies de
-- leur item de base sous un autre identifiant : meme nom, meme type, meme
-- script, meme poids, et aucune restriction d'echange. En stockage, ils
-- forment une seconde pile que rien ne distingue ("Orange Potion" 502 et
-- 11569). Plus aucune table chargee ne les donne ; ce script convertit ceux que
-- les joueurs possedent deja.
--
-- Hors liste, parce qu'ils ne sont PAS des copies : Berserk_Potion_B (22543)
-- et Awakening_Potion_B (22544) n'ont ni les classes ni le niveau minimal de
-- leur base.
--
-- Pour chaque table :
--   1. les piles d'un clone s'ajoutent a une pile existante de l'item de base,
--      du meme proprietaire et du meme `bound`, sans date d'expiration, tant
--      que la somme tient dans MAX_AMOUNT (99 999) ;
--   2. les piles qui restent changent simplement de `nameid`.
-- Une pile de chariot mise en vente (`vending_items`) n'est jamais fusionnee :
-- sa ligne serait supprimee sous l'echoppe. Elle est seulement renommee.
--
-- Les tables sont en MyISAM : pas de transaction, donc SAUVEGARDER AVANT.
-- Serveurs (char et map) ARRETES : un personnage connecte reecrirait son
-- inventaire par-dessus la conversion a sa prochaine sauvegarde.
--
-- Les stockages supplementaires suivent conf/import/inter_server.yml : si une
-- table y est ajoutee, l'ajouter aux appels ci-dessous.
--
-- Rejouable : un second passage ne trouve plus aucun clone.
--

DROP TEMPORARY TABLE IF EXISTS `moon_clone_map`;
CREATE TEMPORARY TABLE `moon_clone_map` (
  `clone_id` int(10) unsigned NOT NULL PRIMARY KEY,
  `base_id` int(10) unsigned NOT NULL
);
INSERT INTO `moon_clone_map` (`clone_id`, `base_id`) VALUES
  (11565,   504),  -- White_Potion_B        -> White_Potion
  (11566,   503),  -- Yellow_Potion_        -> Yellow_Potion
  (11569,   502),  -- Orange_Potion_B       -> Orange_Potion
  (11570,   501),  -- Red_Potion_           -> Red_Potion
  (11572,   505),  -- Blue_Potion_B         -> Blue_Potion
  (11577,   564),  -- Rice_Ball_B           -> Rice_Ball
  (11580,   529),  -- Candy_B               -> Candy
  (11581,   539),  -- Piece_Of_Cake_B       -> Piece_Of_Cake
  (11582,   538),  -- Well_Baked_Cookie_B   -> Well_Baked_Cookie
  (11583,   573),  -- Chocolate_Drink_B     -> Chocolate_Drink
  (11584,   561),  -- White_Chocolate_B     -> White_Chocolate
  (11585,   559),  -- HandMade_Chocolate_B  -> HandMade_Chocolate
  (11587,   558),  -- Chocolate_B           -> Chocolate
  (22542,   645),  -- Center_Potion_B       -> Center_Potion
  (22545, 12016),  -- Speed_Potion          -> Speed_Up_Potion
  (22546, 12017),  -- Slow_Potion           -> Slow_Down_Potion
  (22549,   678);  -- Poison_Bottle_        -> Poison_Bottle

DROP PROCEDURE IF EXISTS `moon_merge_clones`;
DELIMITER //
-- tbl : la table de piles ; owner : sa colonne de proprietaire ;
-- skip : une condition SQL sur `s` qui exclut des piles de la fusion.
CREATE PROCEDURE `moon_merge_clones`(IN tbl VARCHAR(64), IN owner VARCHAR(64), IN skip TEXT)
BEGIN
  -- Le plan : pour chaque proprietaire, item de base et `bound`, le total des
  -- piles de clones et la pile de base qui le recoit.
  DROP TEMPORARY TABLE IF EXISTS `moon_merge`;
  SET @q = CONCAT(
    'CREATE TEMPORARY TABLE `moon_merge` AS ',
    'SELECT c.owner_id, c.base_id, c.bound, c.total, MIN(b.id) AS target_id ',
    'FROM (SELECT s.`', owner, '` AS owner_id, m.base_id, s.bound, SUM(s.amount) AS total ',
    '      FROM `', tbl, '` s JOIN `moon_clone_map` m ON s.nameid = m.clone_id ',
    '      WHERE s.expire_time = 0 AND NOT (', skip, ') ',
    '      GROUP BY s.`', owner, '`, m.base_id, s.bound) c ',
    'JOIN `', tbl, '` b ON b.`', owner, '` = c.owner_id AND b.nameid = c.base_id ',
    '     AND b.bound = c.bound AND b.expire_time = 0 ',
    'GROUP BY c.owner_id, c.base_id, c.bound, c.total');
  PREPARE st FROM @q; EXECUTE st; DEALLOCATE PREPARE st;

  -- Une somme qui deborderait MAX_AMOUNT reste une pile a part.
  SET @q = CONCAT(
    'DELETE mm FROM `moon_merge` mm JOIN `', tbl, '` b ON b.id = mm.target_id ',
    'WHERE b.amount + mm.total > 99999');
  PREPARE st FROM @q; EXECUTE st; DEALLOCATE PREPARE st;

  SET @q = CONCAT(
    'UPDATE `', tbl, '` b JOIN `moon_merge` mm ON b.id = mm.target_id ',
    'SET b.amount = b.amount + mm.total');
  PREPARE st FROM @q; EXECUTE st; DEALLOCATE PREPARE st;

  SET @q = CONCAT(
    'DELETE s FROM `', tbl, '` s ',
    'JOIN `moon_clone_map` m ON s.nameid = m.clone_id ',
    'JOIN `moon_merge` mm ON mm.owner_id = s.`', owner, '` AND mm.base_id = m.base_id ',
    '     AND mm.bound = s.bound ',
    'WHERE s.expire_time = 0 AND NOT (', skip, ')');
  PREPARE st FROM @q; EXECUTE st; DEALLOCATE PREPARE st;

  -- Ce qui reste devient l'item de base, sur sa propre pile.
  SET @q = CONCAT(
    'UPDATE `', tbl, '` s JOIN `moon_clone_map` m ON s.nameid = m.clone_id ',
    'SET s.nameid = m.base_id');
  PREPARE st FROM @q; EXECUTE st; DEALLOCATE PREPARE st;

  DROP TEMPORARY TABLE IF EXISTS `moon_merge`;
END //
DELIMITER ;

CALL `moon_merge_clones`('inventory',      'char_id',    'FALSE');
CALL `moon_merge_clones`('cart_inventory', 'char_id',
  's.id IN (SELECT `cartinventory_id` FROM `vending_items`)');
CALL `moon_merge_clones`('storage',        'account_id', 'FALSE');
CALL `moon_merge_clones`('storagealt1',    'account_id', 'FALSE');
CALL `moon_merge_clones`('storagealt2',    'account_id', 'FALSE');
CALL `moon_merge_clones`('storagealt3',    'account_id', 'FALSE');
CALL `moon_merge_clones`('storagealt4',    'account_id', 'FALSE');
CALL `moon_merge_clones`('storagecard',    'account_id', 'FALSE');
CALL `moon_merge_clones`('guild_storage',  'guild_id',   'FALSE');

-- Pieces jointes d'un courrier et demandes d'achat : une ligne par objet, sans
-- pile a fusionner.
UPDATE `mail_attachments` s JOIN `moon_clone_map` m ON s.nameid = m.clone_id
  SET s.nameid = m.base_id;
UPDATE `buyingstore_items` s JOIN `moon_clone_map` m ON s.item_id = m.clone_id
  SET s.item_id = m.base_id;

DROP PROCEDURE IF EXISTS `moon_merge_clones`;
DROP TEMPORARY TABLE IF EXISTS `moon_clone_map`;
