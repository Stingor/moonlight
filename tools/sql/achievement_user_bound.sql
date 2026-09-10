-- =====================================================================
--  Succes lies au COMPTE MOONLIGHT (login.user_id)
--  Migration depuis le liage par compte de JEU (account_id), juillet 2026
-- =====================================================================
--
--  Rangement APRES cette migration -- trois formes de ligne, exclusives :
--
--    par personnage        (char_id = X, account_id = 0, user_id = 0)
--    par COMPTE MOONLIGHT  (char_id = 0, account_id = 0, user_id = U)  <- defaut
--    par compte de JEU     (char_id = 0, account_id = A, user_id = 0)  <- repli
--
--  Le repli sert aux comptes de jeu qu'aucun compte Moonlight ne rattache
--  (login.user_id = 0 ou absent). Ils gardent le comportement d'avant plutot
--  que de se retrouver ENSEMBLE sur une ligne « user_id = 0 », qui serait une
--  progression commune a des inconnus.
--
--  🔴 POURQUOI UNE COLONNE DE PLUS ET NON UNE REUTILISATION. `account_id` et
--  `user_id` sont deux NUMEROTATIONS DIFFERENTES. Mettre le user_id dans la
--  colonne account_id quand il existe, et l'account_id sinon, marche jusqu'au
--  jour ou les deux espaces se croisent : deux personnes sans aucun rapport
--  partagent alors leurs succes, et rien dans le schema ne l'interdit. Une
--  colonne par espace de nommage rend la collision impossible a ECRIRE.
--
--  ---------------------------------------------------------------------
--  IMPORTANT
--    - SAUVEGARDER la table `achievement` avant de lancer ceci :
--        mysqldump <base> achievement > achievement_backup.sql
--    - A LANCER UNE SEULE FOIS. L'etape 4 supprime les lignes sources, donc
--      relancer le script n'est PAS sans effet.
--    - A lancer AVANT de demarrer le char-server recompile : l'ancien binaire
--      ne connait pas la colonne, le nouveau l'exige (verification d'integrite
--      au demarrage, char.cpp).
--    - Recompiler char ET map ensemble.
--    - Nom de table suppose `achievement` (voir achievement_table dans
--      conf/inter_athena.conf), table de comptes `login`.
-- =====================================================================

-- ---------------------------------------------------------------------
-- 1) Schema : la colonne user_id, et la cle primaire elargie.
--    La PK devient (char_id, account_id, user_id, id) pour que les trois
--    formes de ligne coexistent sans jamais se marcher dessus.
-- ---------------------------------------------------------------------
ALTER TABLE `achievement`
  ADD COLUMN `user_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `account_id`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`char_id`,`account_id`,`user_id`,`id`);

-- ---------------------------------------------------------------------
-- 2) Table de travail : pour chaque ligne PARTAGEE d'un compte de jeu
--    RATTACHE, la progression fusionnee de son compte Moonlight.
--
--    La fusion prend le MAXIMUM de chaque compteur, et la date la PLUS
--    ANCIENNE pour `completed` / `rewarded` : deux comptes de jeu de la meme
--    personne ont pu avancer chacun de leur cote, et on ne retire jamais rien
--    a personne. Un `rewarded` non nul quelque part rend la ligne fusionnee
--    « deja encaissee » — c'est la verite : le lot est bien parti une fois.
-- ---------------------------------------------------------------------
DROP TEMPORARY TABLE IF EXISTS `ach_merge`;
CREATE TEMPORARY TABLE `ach_merge` AS
SELECT
  `l`.`user_id`                AS `user_id`,
  `a`.`id`                     AS `id`,
  MIN(`a`.`completed`)         AS `completed`,
  MIN(`a`.`rewarded`)          AS `rewarded`,
  MAX(`a`.`count1`)  AS `count1`,  MAX(`a`.`count2`)  AS `count2`,
  MAX(`a`.`count3`)  AS `count3`,  MAX(`a`.`count4`)  AS `count4`,
  MAX(`a`.`count5`)  AS `count5`,  MAX(`a`.`count6`)  AS `count6`,
  MAX(`a`.`count7`)  AS `count7`,  MAX(`a`.`count8`)  AS `count8`,
  MAX(`a`.`count9`)  AS `count9`,  MAX(`a`.`count10`) AS `count10`
FROM `achievement` AS `a`
JOIN `login` AS `l` ON `l`.`account_id` = `a`.`account_id`
WHERE `a`.`char_id` = 0
  AND `a`.`account_id` <> 0
  AND `l`.`user_id` <> 0
GROUP BY `l`.`user_id`, `a`.`id`;

-- ---------------------------------------------------------------------
-- 3) Poser les lignes fusionnees, cote compte Moonlight.
-- ---------------------------------------------------------------------
INSERT INTO `achievement`
  (`char_id`, `account_id`, `user_id`, `id`, `completed`, `rewarded`,
   `count1`,`count2`,`count3`,`count4`,`count5`,`count6`,`count7`,`count8`,`count9`,`count10`)
SELECT
  0, 0, `m`.`user_id`, `m`.`id`, `m`.`completed`, `m`.`rewarded`,
  `m`.`count1`,`m`.`count2`,`m`.`count3`,`m`.`count4`,`m`.`count5`,
  `m`.`count6`,`m`.`count7`,`m`.`count8`,`m`.`count9`,`m`.`count10`
FROM `ach_merge` AS `m`;

-- ---------------------------------------------------------------------
-- 4) Retirer les lignes sources : celles des comptes de JEU rattaches.
--    Les lignes des comptes NON rattaches restent telles quelles — c'est le
--    repli, et il continue de fonctionner sans rien changer.
--    🔴 C'est cette etape qui rend le script non rejouable.
-- ---------------------------------------------------------------------
DELETE `a` FROM `achievement` AS `a`
JOIN `login` AS `l` ON `l`.`account_id` = `a`.`account_id`
WHERE `a`.`char_id` = 0
  AND `a`.`account_id` <> 0
  AND `l`.`user_id` <> 0;

DROP TEMPORARY TABLE IF EXISTS `ach_merge`;

-- ---------------------------------------------------------------------
-- 5) Verification. Les trois comptes doivent avoir un sens :
--      - « par personnage » inchange depuis la sauvegarde
--      - « par compte Moonlight » non nul
--      - « par compte de jeu » = uniquement des comptes non rattaches
-- ---------------------------------------------------------------------
SELECT 'par personnage'        AS `portee`, COUNT(*) AS `lignes` FROM `achievement` WHERE `char_id` <> 0
UNION ALL
SELECT 'par compte Moonlight', COUNT(*) FROM `achievement` WHERE `char_id` = 0 AND `user_id` <> 0
UNION ALL
SELECT 'par compte de jeu (repli)', COUNT(*) FROM `achievement` WHERE `char_id` = 0 AND `account_id` <> 0;

-- Doit rendre ZERO ligne : un repli qui serait en fait rattache.
SELECT `a`.`account_id`, `l`.`user_id`, COUNT(*) AS `lignes`
FROM `achievement` AS `a`
JOIN `login` AS `l` ON `l`.`account_id` = `a`.`account_id`
WHERE `a`.`char_id` = 0 AND `a`.`account_id` <> 0 AND `l`.`user_id` <> 0
GROUP BY `a`.`account_id`, `l`.`user_id`;
