-- [Stingor] Les clés de style des personnages passent de `bourgeon_style*$` à
-- `moonclient_style*$` (char_reg_str), avec le map-server qui les lit sous ce nom
-- et le site qui rend les avatars.
--
-- À jouer UNE fois, serveurs arrêtés, juste avant de démarrer le map-server qui
-- porte les nouveaux noms. Rejouée, elle ne change plus rien.
--
-- Contrôle, avant puis après :
--   SELECT `key`, COUNT(*) FROM `char_reg_str` WHERE `key` LIKE '%\_style%$' GROUP BY `key`;
UPDATE `char_reg_str` SET `key` = 'moonclient_style$'  WHERE `key` = 'bourgeon_style$';
UPDATE `char_reg_str` SET `key` = 'moonclient_style2$' WHERE `key` = 'bourgeon_style2$';
UPDATE `char_reg_str` SET `key` = 'moonclient_style3$' WHERE `key` = 'bourgeon_style3$';
UPDATE `char_reg_str` SET `key` = 'moonclient_style4$' WHERE `key` = 'bourgeon_style4$';
