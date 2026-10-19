--
-- La banque en int64 [Stingor] -- base des JOURNAUX (log_db), pas la base du jeu
--
-- Le plafond de la banque (MAX_BANK_ZENY, src/common/mmo.hpp) passe a
-- 9 999 999 999 999 999 zeny. log_zeny ecrit desormais son montant en int64 :
-- un `set BankVault` qui fait varier le solde de plus de 2^31-1 zeny ecrirait,
-- dans une colonne int(11), un montant tronque par MySQL (ou refuse en mode
-- strict).
--
-- Le solde lui-meme n'a besoin d'aucune migration : il vit dans la variable de
-- compte #BANKVAULT, dont la colonne `acc_reg_num`.`value` est deja un bigint.
--
-- A verifier avant de l'executer : `SHOW CREATE TABLE zenylog;` sur la base des
-- journaux. Le schema amont de rAthena declare `amount` en int(11) NOT NULL
-- default '0'.
--

ALTER TABLE `zenylog` MODIFY `amount` bigint(20) NOT NULL default '0';
