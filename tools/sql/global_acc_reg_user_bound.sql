-- =====================================================================
--  Variables ## liees au COMPTE MOONLIGHT (login.user_id)
-- =====================================================================
--
--  Rangement APRES cette migration -- deux formes de ligne, exclusives :
--
--    par COMPTE MOONLIGHT  (account_id = 0, user_id = U)  <- defaut
--    par compte de JEU     (account_id = A, user_id = 0)  <- repli
--
--  Une colonne par numerotation, comme pour `achievement` : account_id et
--  user_id peuvent prendre les memes valeurs, et les reunir dans une seule
--  colonne ferait partager leurs variables a deux inconnus.
--
--  Aucune ligne n'est deplacee ici. Le login-server rapatrie celles d'un
--  compte de jeu rattache a sa prochaine connexion (mmo_global_accreg_adopt).
--
--  ---------------------------------------------------------------------
--  IMPORTANT
--    - SAUVEGARDER avant de lancer ceci :
--        mysqldump rathena global_acc_reg_num global_acc_reg_str > global_acc_reg_backup.sql
--    - A LANCER UNE SEULE FOIS, login-server ARRETE, AVANT de demarrer le
--      login-server recompile : le nouveau binaire exige la colonne, l'ancien
--      ne la remplit pas.
--    - A lancer aussi AVANT de deployer includes/functions_moon_merge.php du
--      site : son cliché de rapatriement lit la colonne.
--    - Une variable ## est lue a la connexion : deux comptes de jeu connectes
--      en meme temps ne voient pas tout de suite leurs ecritures respectives.
--      A reserver aux drapeaux, pas aux compteurs ni aux monnaies.
-- =====================================================================

ALTER TABLE `global_acc_reg_num`
  ADD COLUMN `user_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `account_id`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`account_id`,`user_id`,`key`,`index`);

ALTER TABLE `global_acc_reg_str`
  ADD COLUMN `user_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `account_id`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`account_id`,`user_id`,`key`,`index`);
