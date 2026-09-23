-- =====================================================================
--  Etats par COMPTE MOONLIGHT restes derriere un rapatriement
--  (album de cartes, succes partages, favoris MVP, ignores)
-- =====================================================================
--
--  Jusqu'au correctif de septembre 2026, `moon_merge_execute()` ne
--  deplacait que `login`.`user_id`, le mot de passe, l'e-mail et le
--  compteur de votes. Tout ce qui est cle sur le COMPTE MOONLIGHT restait
--  sur l'ancien compte web : plus aucun compte de jeu n'y menant, l'album
--  et les succes devenaient inaccessibles sans rien casser ni dupliquer.
--
--  Ce script REGARDE d'abord (etapes 1 et 2, lecture seule), puis REPARE
--  (etape 3, a decommenter) les rapatriements passes qui ont VIDE leur
--  compte source — les seuls ou la fusion est legitime, exactement comme
--  pour le compteur de votes. Un compte source encore joue garde ses etats.
--
--  ---------------------------------------------------------------------
--  IMPORTANT
--    - SAUVEGARDER d'abord :
--        mysqldump rathena card_album achievement mvp_favorite user_ignore \
--                  > moon_merge_states_backup.sql
--    - Base du forum : `site` (cf. qsdf25gfd1ds20ghgdfgconfig.php), base du
--      jeu : `rathena`. Les deux vivent sur la meme instance.
--    - 🔴 PERSONNE EN JEU sur les comptes concernes pendant l'etape 3 : le
--      map-server tient l'album, les succes et les ignores en memoire et les
--      reecrit au fil de l'eau. Le plus simple est map-server arrete.
--    - L'etape 3 n'est PAS rejouable : elle supprime les lignes sources.
-- =====================================================================

-- ---------------------------------------------------------------------
-- 1) Les rapatriements qui ont vide leur compte source.
--    `src_closed` vaut 1 sur les lignes de journal de ces demandes.
-- ---------------------------------------------------------------------
DROP TEMPORARY TABLE IF EXISTS `moon_closed`;
CREATE TEMPORARY TABLE `moon_closed` AS
SELECT DISTINCT `old_user_id` AS `src`, `new_user_id` AS `dst`
  FROM `site`.`phpbb_moon_merge_log`
 WHERE `src_closed` = 1
   AND `old_user_id` <> 0
   AND `new_user_id` <> 0;

-- Un compte source qui a RECU des comptes de jeu depuis (ou qui n'a jamais
-- ete vide pour de bon) n'est pas concerne : ses etats lui appartiennent
-- toujours.
DELETE FROM `moon_closed`
 WHERE `src` IN (SELECT `user_id` FROM `rathena`.`login`);

-- ---------------------------------------------------------------------
-- 2) RELEVE : ce qui dort sur ces comptes morts.
-- ---------------------------------------------------------------------
SELECT `c`.`src`, `c`.`dst`,
       (SELECT COUNT(*) FROM `rathena`.`card_album` a
         WHERE a.`user_id` = `c`.`src`)                     AS `pochettes`,
       (SELECT COALESCE(SUM(a.`amount`), 0) FROM `rathena`.`card_album` a
         WHERE a.`user_id` = `c`.`src`)                     AS `cartes_en_reserve`,
       (SELECT COUNT(*) FROM `rathena`.`achievement` h
         WHERE h.`char_id` = 0 AND h.`account_id` = 0 AND h.`user_id` = `c`.`src`) AS `succes`,
       (SELECT COUNT(*) FROM `rathena`.`mvp_favorite` f
         WHERE f.`user_id` = `c`.`src`)                     AS `favoris_mvp`,
       (SELECT COUNT(*) FROM `rathena`.`user_ignore` i
         WHERE i.`user_id` = `c`.`src`)                     AS `ignores`
  FROM `moon_closed` `c`
 ORDER BY `pochettes` DESC, `succes` DESC;

-- Les collisions a prevoir : une carte que les DEUX comptes possedent.
-- Leurs copies s'additionneront (plafond MAX_AMOUNT = 30000).
SELECT `c`.`src`, `c`.`dst`, COUNT(*) AS `cartes_communes`
  FROM `moon_closed` `c`
  JOIN `rathena`.`card_album` `s` ON `s`.`user_id` = `c`.`src`
  JOIN `rathena`.`card_album` `d` ON `d`.`user_id` = `c`.`dst` AND `d`.`nameid` = `s`.`nameid`
 GROUP BY `c`.`src`, `c`.`dst`;

-- 🔴 DOIT RENDRE ZERO LIGNE : un compte qui a ete rapatrie APRES avoir lui-meme
-- recu un rapatriement. La cible de la premiere fusion est alors morte a son
-- tour, et l'album ferait une etape de trop. Ces cas se traitent A LA MAIN,
-- dans l'ordre chronologique des demandes, avant de lancer l'etape 3.
SELECT `a`.`src` AS `source`, `a`.`dst` AS `cible_elle_meme_rapatriee`, `b`.`dst` AS `cible_finale`
  FROM `moon_closed` `a`
  JOIN `site`.`phpbb_moon_merge_log` `b` ON `b`.`old_user_id` = `a`.`dst` AND `b`.`src_closed` = 1;

-- ---------------------------------------------------------------------
-- 3) REPARATION — a decommenter APRES avoir lu le releve.
--
--    Meme fusion que le site applique desormais a chaud :
--      album   : les copies s'additionnent, la date de deblocage la plus
--                ancienne l'emporte ;
--      succes  : MAXIMUM de chaque compteur, date la PLUS ANCIENNE pour
--                `completed` et `rewarded` — un lot deja encaisse le reste ;
--      favoris / ignores : simple deplacement, les doublons disparaissent.
--
--    `UPDATE IGNORE` deplace d'abord tout ce qui n'entre pas en collision ;
--    ce qui reste sur le source est exactement ce que le maitre possede
--    deja, et se cumule ensuite.
-- ---------------------------------------------------------------------

-- -- Album de cartes
-- UPDATE IGNORE `rathena`.`card_album` `a`
--   JOIN `moon_closed` `c` ON `c`.`src` = `a`.`user_id`
--    SET `a`.`user_id` = `c`.`dst`;
--
-- UPDATE `rathena`.`card_album` `d`
--   JOIN `moon_closed` `c` ON `c`.`dst` = `d`.`user_id`
--   JOIN `rathena`.`card_album` `s` ON `s`.`user_id` = `c`.`src` AND `s`.`nameid` = `d`.`nameid`
--    SET `d`.`amount` = LEAST(30000, `d`.`amount` + `s`.`amount`),
--        `d`.`unlocked_at` = LEAST(`d`.`unlocked_at`, `s`.`unlocked_at`);
--
-- DELETE `a` FROM `rathena`.`card_album` `a`
--   JOIN `moon_closed` `c` ON `c`.`src` = `a`.`user_id`;
--
-- -- Succes partages (les lignes par personnage ne bougent pas : elles
-- -- suivent le personnage, donc le compte de jeu deja deplace)
-- UPDATE IGNORE `rathena`.`achievement` `h`
--   JOIN `moon_closed` `c` ON `c`.`src` = `h`.`user_id`
--    SET `h`.`user_id` = `c`.`dst`
--  WHERE `h`.`char_id` = 0 AND `h`.`account_id` = 0;
--
-- UPDATE `rathena`.`achievement` `d`
--   JOIN `moon_closed` `c` ON `c`.`dst` = `d`.`user_id`
--   JOIN `rathena`.`achievement` `s`
--     ON `s`.`char_id` = 0 AND `s`.`account_id` = 0
--    AND `s`.`user_id` = `c`.`src` AND `s`.`id` = `d`.`id`
--    SET `d`.`count1`  = GREATEST(`d`.`count1`,  `s`.`count1`),
--        `d`.`count2`  = GREATEST(`d`.`count2`,  `s`.`count2`),
--        `d`.`count3`  = GREATEST(`d`.`count3`,  `s`.`count3`),
--        `d`.`count4`  = GREATEST(`d`.`count4`,  `s`.`count4`),
--        `d`.`count5`  = GREATEST(`d`.`count5`,  `s`.`count5`),
--        `d`.`count6`  = GREATEST(`d`.`count6`,  `s`.`count6`),
--        `d`.`count7`  = GREATEST(`d`.`count7`,  `s`.`count7`),
--        `d`.`count8`  = GREATEST(`d`.`count8`,  `s`.`count8`),
--        `d`.`count9`  = GREATEST(`d`.`count9`,  `s`.`count9`),
--        `d`.`count10` = GREATEST(`d`.`count10`, `s`.`count10`),
--        `d`.`completed` = LEAST(IFNULL(`d`.`completed`, `s`.`completed`),
--                                IFNULL(`s`.`completed`, `d`.`completed`)),
--        `d`.`rewarded`  = LEAST(IFNULL(`d`.`rewarded`,  `s`.`rewarded`),
--                                IFNULL(`s`.`rewarded`,  `d`.`rewarded`))
--  WHERE `d`.`char_id` = 0 AND `d`.`account_id` = 0;
--
-- DELETE `h` FROM `rathena`.`achievement` `h`
--   JOIN `moon_closed` `c` ON `c`.`src` = `h`.`user_id`
--  WHERE `h`.`char_id` = 0 AND `h`.`account_id` = 0;
--
-- -- Favoris du traqueur MVP
-- UPDATE IGNORE `rathena`.`mvp_favorite` `f`
--   JOIN `moon_closed` `c` ON `c`.`src` = `f`.`user_id`
--    SET `f`.`user_id` = `c`.`dst`;
-- DELETE `f` FROM `rathena`.`mvp_favorite` `f`
--   JOIN `moon_closed` `c` ON `c`.`src` = `f`.`user_id`;
--
-- -- Liste d'ignores, des deux cotes de la relation
-- UPDATE IGNORE `rathena`.`user_ignore` `i`
--   JOIN `moon_closed` `c` ON `c`.`src` = `i`.`user_id`
--    SET `i`.`user_id` = `c`.`dst`;
-- UPDATE IGNORE `rathena`.`user_ignore` `i`
--   JOIN `moon_closed` `c` ON `c`.`src` = `i`.`ignored_user_id`
--    SET `i`.`ignored_user_id` = `c`.`dst`;
-- DELETE `i` FROM `rathena`.`user_ignore` `i`
--   JOIN `moon_closed` `c` ON `c`.`src` = `i`.`user_id` OR `c`.`src` = `i`.`ignored_user_id`;
-- DELETE FROM `rathena`.`user_ignore` WHERE `user_id` = `ignored_user_id`;
--
-- -- Groupes de chasse MVP : le compte mort en SORT, rien n'est deplace.
-- -- Le map-server charge les groupes AU DEMARRAGE et ne les relit jamais :
-- -- deplacer une adhesion ferait echouer le prochain INSERT sur la cle
-- -- primaire (user_id). Le nettoyage ne prend effet qu'au redemarrage.
-- DELETE `v` FROM `rathena`.`mvp_group_invite` `v`
--   JOIN `moon_closed` `c` ON `c`.`src` = `v`.`user_id` OR `c`.`src` = `v`.`from_user_id`;
-- DELETE `m` FROM `rathena`.`mvp_group_member` `m`
--   JOIN `moon_closed` `c` ON `c`.`src` = `m`.`user_id`;
-- -- Succession : le membre le plus ancien reprend le groupe.
-- UPDATE `rathena`.`mvp_group` `g`
--   JOIN `moon_closed` `c` ON `c`.`src` = `g`.`owner_user_id`
--    SET `g`.`owner_user_id` = COALESCE(
--          (SELECT `m`.`user_id` FROM `rathena`.`mvp_group_member` `m`
--            WHERE `m`.`group_id` = `g`.`group_id`
--            ORDER BY `m`.`joined_at` ASC LIMIT 1), 0);
-- -- Plus personne dedans : le groupe disparait.
-- DELETE `v` FROM `rathena`.`mvp_group_invite` `v`
--   JOIN `rathena`.`mvp_group` `g` ON `g`.`group_id` = `v`.`group_id`
--  WHERE `g`.`owner_user_id` = 0;
-- DELETE FROM `rathena`.`mvp_group` WHERE `owner_user_id` = 0;

-- ---------------------------------------------------------------------
-- 4) Verification : le releve de l'etape 2 doit desormais rendre des zeros.
-- ---------------------------------------------------------------------
-- DROP TEMPORARY TABLE IF EXISTS `moon_closed`;
