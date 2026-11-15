--
-- Les coiffes propres a Moonlight quittent la plage de Gravity [Stingor]
--
-- Gravity numerote ses apparences de coiffe en milliers (2934 a la mise a jour
-- du client 2025-07-16) : les vues 2998, 2999 et 3000 seraient rattrapees par
-- une prochaine mise a jour. Elles passent dans la plage 20000-32767, reservee
-- aux apparences du serveur :
--
--   2998 -> 20000   _angelwing
--   2999 -> 20001   _evilwing
--   3000 -> 20002   _Tatatatata (Helm_of_Tachanka, objet 30000)
--
-- Le `View` de l'objet change dans db/import/items/item_db_armor.yml, et
-- moonclient lit les memes numeros (headgear_sprites.yml, item_info.yml). Ce
-- script met a jour l'apparence que la table `char` garde pour l'ecran de
-- selection ; le serveur de carte la recalcule de toute facon a la connexion,
-- d'apres l'equipement.
--
-- A executer UNE fois, serveur arrete, sur la base du jeu.
--

UPDATE `char` SET `head_top` = 20000 WHERE `head_top` = 2998;
UPDATE `char` SET `head_top` = 20001 WHERE `head_top` = 2999;
UPDATE `char` SET `head_top` = 20002 WHERE `head_top` = 3000;

UPDATE `char` SET `head_mid` = 20000 WHERE `head_mid` = 2998;
UPDATE `char` SET `head_mid` = 20001 WHERE `head_mid` = 2999;
UPDATE `char` SET `head_mid` = 20002 WHERE `head_mid` = 3000;

UPDATE `char` SET `head_bottom` = 20000 WHERE `head_bottom` = 2998;
UPDATE `char` SET `head_bottom` = 20001 WHERE `head_bottom` = 2999;
UPDATE `char` SET `head_bottom` = 20002 WHERE `head_bottom` = 3000;
