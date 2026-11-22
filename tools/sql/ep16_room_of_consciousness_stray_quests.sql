-- =====================================================================
--  Room of Consciousness : quetes de rejeu posees AVANT l'etape 19
--  du Banquet des heros (episode 16.1)
-- =====================================================================
--
--  L'acces direct du Warp Agent ouvrait la Room of Consciousness sans
--  regarder la quete principale, et posait 7706 (Nillem Is Not Almighty,
--  delai) et 7705 (Hope You Cannot Let Go, mission de rejeu). Le scenario
--  ne les pose qu'a la fin de l'episode (ep16_royal 23, puis Friedrich).
--
--  Or la porte du scenario, Swaying Space#evt_gate02 (prt_lib_q 88,90),
--  REFUSE le passage des que 7706 est terminee ou expiree, ou que 7705 est
--  terminee. Un personnage passe par le Warp Agent avant l'etape 19 ne peut
--  donc plus entrer quand le scenario l'y envoie (ep16_royal 19, quete
--  7700 Once More!) : la porte lui dit que le passage a disparu.
--
--  Depuis le correctif du Warp Agent, l'acces direct attend ep16_royal 23.
--  Ce script retire les deux quetes de ceux qui les portent encore a
--  ep16_royal 19 ou moins : ils repassent la porte comme prevu, et le
--  scenario reposera lui-meme 7706 et 7705 le moment venu.
--
--  Une cle ep16_royal absente de char_reg_num vaut 0.
--
--  ---------------------------------------------------------------------
--  IMPORTANT
--    - SAUVEGARDER d'abord :
--        mysqldump rathena quest > ep16_quest_backup.sql
--    - PERSONNE EN JEU sur les personnages concernes pendant l'etape 2 :
--      le map-server tient le journal des quetes en memoire et le reecrit
--      a la deconnexion. L'etape 1 dit qui est en ligne.
--    - L'etape 2 n'est PAS rejouable a l'identique : elle supprime les
--      lignes (la sauvegarde les garde).
-- =====================================================================

-- ---------------------------------------------------------------------
--  ETAPE 1 — REGARDER (lecture seule)
--  Les personnages a ep16_royal <= 19 qui portent 7705 ou 7706.
-- ---------------------------------------------------------------------
SELECT c.`char_id`,
       c.`name`,
       c.`online`,
       COALESCE(r.`value`, 0)  AS `ep16_royal`,
       q.`quest_id`,
       q.`state`,
       FROM_UNIXTIME(q.`time`) AS `echeance`
  FROM `quest` q
  JOIN `char` c ON c.`char_id` = q.`char_id`
  LEFT JOIN `char_reg_num` r
         ON r.`char_id` = q.`char_id` AND r.`key` = 'ep16_royal' AND r.`index` = 0
 WHERE q.`quest_id` IN (7705, 7706)
   AND COALESCE(r.`value`, 0) <= 19
 ORDER BY c.`char_id`, q.`quest_id`;

-- ---------------------------------------------------------------------
--  ETAPE 2 — REPARER (a decommenter apres l'etape 1 et la sauvegarde)
-- ---------------------------------------------------------------------
-- DELETE q
--   FROM `quest` q
--   LEFT JOIN `char_reg_num` r
--          ON r.`char_id` = q.`char_id` AND r.`key` = 'ep16_royal' AND r.`index` = 0
--  WHERE q.`quest_id` IN (7705, 7706)
--    AND COALESCE(r.`value`, 0) <= 19;
