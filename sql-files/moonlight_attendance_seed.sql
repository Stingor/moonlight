--
-- Presence Moonlight [Stingor] -- premier pool et premiere edition.
--
-- Le pool de depart est le calendrier de novembre 2026 (high rate) : les
-- jours ordinaires donnent une monnaie d'event a la quantite d'une trouvaille
-- du cafard, les paliers deux lots. L'edition 1 est ce meme calendrier, pour
-- le mois de novembre 2026 ; les suivantes se tirent sur le site.
--
-- Une seule fois, sur une base dont les tables sont vides :
--   mysql rathena < moonlight_attendance_seed.sql
--

INSERT INTO `moonlight_attendance_pool` (`tier`, `item_id`, `amount`, `weight`, `enabled`) VALUES
  ('day', 12208, 2, 10, 1),
  ('day', 6919, 30, 10, 1),
  ('day', 7642, 30, 10, 1),
  ('day', 6607, 25, 10, 1),
  ('day', 6820, 100, 10, 1),
  ('day', 12210, 1, 10, 1),
  ('milestone', 6608, 100, 10, 1),
  ('milestone', 6419, 4, 10, 1),
  ('day', 6719, 30, 10, 1),
  ('day', 25155, 30, 10, 1),
  ('day', 6671, 30, 10, 1),
  ('day', 1000104, 30, 10, 1),
  ('day', 1000103, 20, 10, 1),
  ('day', 14592, 2, 10, 1),
  ('milestone', 25669, 25, 10, 1),
  ('milestone', 6418, 2, 10, 1),
  ('day', 1000405, 50, 10, 1),
  ('day', 1000608, 25, 10, 1),
  ('day', 1001217, 20, 10, 1),
  ('day', 25899, 20, 10, 1),
  ('day', 1000367, 50, 10, 1),
  ('day', 12412, 1, 10, 1),
  ('milestone', 25271, 10, 10, 1),
  ('milestone', 25723, 5, 10, 1),
  ('day', 1000257, 20, 10, 1),
  ('day', 1000263, 20, 10, 1),
  ('day', 1000373, 50, 10, 1),
  ('day', 1000363, 10, 10, 1),
  ('day', 1000316, 10, 10, 1),
  ('day', 25787, 10, 10, 1),
  ('day', 25739, 10, 10, 1),
  ('milestone', 12246, 1, 10, 1),
  ('milestone', 25814, 30, 10, 1);

INSERT INTO `moonlight_attendance_edition` (`id`, `month`, `drawn_by`) VALUES (1, '2026-11-01', NULL);

INSERT INTO `moonlight_attendance_calendar` (`edition_id`, `day`, `slot`, `item_id`, `amount`, `milestone`) VALUES
  (1, 1, 0, 12208, 2, 0),
  (1, 2, 0, 6919, 30, 0),
  (1, 3, 0, 7642, 30, 0),
  (1, 4, 0, 6607, 25, 0),
  (1, 5, 0, 6820, 100, 0),
  (1, 6, 0, 12210, 1, 0),
  (1, 7, 0, 0, 2, 1),
  (1, 7, 1, 6608, 100, 1),
  (1, 7, 2, 6419, 4, 1),
  (1, 8, 0, 6719, 30, 0),
  (1, 9, 0, 25155, 30, 0),
  (1, 10, 0, 6671, 30, 0),
  (1, 11, 0, 1000104, 30, 0),
  (1, 12, 0, 1000103, 20, 0),
  (1, 13, 0, 14592, 2, 0),
  (1, 14, 0, 0, 3, 1),
  (1, 14, 1, 25669, 25, 1),
  (1, 14, 2, 6418, 2, 1),
  (1, 15, 0, 1000405, 50, 0),
  (1, 16, 0, 1000608, 25, 0),
  (1, 17, 0, 1001217, 20, 0),
  (1, 18, 0, 25899, 20, 0),
  (1, 19, 0, 1000367, 50, 0),
  (1, 20, 0, 12412, 1, 0),
  (1, 21, 0, 0, 4, 1),
  (1, 21, 1, 25271, 10, 1),
  (1, 21, 2, 25723, 5, 1),
  (1, 22, 0, 1000257, 20, 0),
  (1, 23, 0, 1000263, 20, 0),
  (1, 24, 0, 1000373, 50, 0),
  (1, 25, 0, 1000363, 10, 0),
  (1, 26, 0, 1000316, 10, 0),
  (1, 27, 0, 25787, 10, 0),
  (1, 27, 1, 25739, 10, 0),
  (1, 28, 0, 0, 6, 1),
  (1, 28, 1, 12246, 1, 1),
  (1, 28, 2, 25814, 30, 1);
