-- =====================================================================
--  Account-bound achievements - schema migration
--  (account-bound by DEFAULT, per-character is the opt-out)
-- =====================================================================
--
--  Storage convention after this migration:
--    * account-bound achievement (DEFAULT): (char_id = 0, account_id = A)
--    * player-bound  achievement (opt-out) : (char_id = X, account_id = 0)
--
--  The C++ side decides the binding per achievement from the YAML field
--  "Unbound: true" (default: absent => account-bound). This script:
--    1) extends the `achievement` table with an `account_id` column,
--    2) merges EVERY character's existing progress into a single per-account
--       row, EXCEPT for the achievements you opt out (kept per-character).
--
--  ---------------------------------------------------------------------
--  IMPORTANT
--    - TAKE A BACKUP of the `achievement` table before running this.
--        mysqldump <db> achievement > achievement_backup.sql
--    - This script is meant to be run ONCE. Step 3 deletes the source
--      rows, so re-running it is NOT idempotent.
--    - The `NOT IN (...)` lists in steps 2 and 3 MUST match the achievements
--      flagged `Unbound: true` in achievement_db.yml (kept per-character).
--      They are pre-filled below from the current YAML; regenerate with
--      tools/ list_unbound.py if you change the flags.
--    - Table name assumed to be `achievement` (see achievement_table in
--      conf/inter_athena.conf) and the character table `char`.
-- =====================================================================

-- ---------------------------------------------------------------------
-- 1) Schema: add account_id and widen the primary key.
--    PK becomes (char_id, account_id, id) so that both a player-bound row
--    (X,0,id) and an account-bound row (0,A,id) can coexist without clash.
-- ---------------------------------------------------------------------
ALTER TABLE `achievement`
  ADD COLUMN `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `char_id`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`char_id`,`account_id`,`id`);

-- ---------------------------------------------------------------------
-- 2) Merge every character's progress into one per-account row, for all
--    achievements EXCEPT the opt-out (player-bound) ones.
--
--    Aggregation rules:
--      countN    -> MAX  (progress of the furthest-along character; safe,
--                         never inflates past what a single char reached)
--      completed -> MIN  (completed if ANY character completed it;
--                         earliest completion date, NULLs ignored)
--      rewarded  -> MIN  (rewarded if ANY character already claimed it;
--                         prevents claiming the item reward twice)
--
--    The NOT IN list below is the set of achievements flagged `Unbound: true`
--    in db/import/achievement_db.yml (kept per-character, NOT merged).
--    Regenerate it with tools/ list_unbound.py if you change the flags.
-- ---------------------------------------------------------------------
INSERT INTO `achievement`
  (`char_id`,`account_id`,`id`,
   `count1`,`count2`,`count3`,`count4`,`count5`,
   `count6`,`count7`,`count8`,`count9`,`count10`,
   `completed`,`rewarded`)
SELECT 0, c.`account_id`, a.`id`,
       MAX(a.`count1`), MAX(a.`count2`), MAX(a.`count3`), MAX(a.`count4`), MAX(a.`count5`),
       MAX(a.`count6`), MAX(a.`count7`), MAX(a.`count8`), MAX(a.`count9`), MAX(a.`count10`),
       MIN(a.`completed`), MIN(a.`rewarded`)
FROM `achievement` a
JOIN `char` c ON c.`char_id` = a.`char_id`
WHERE a.`char_id` <> 0             -- only real per-character rows
  AND a.`id` NOT IN (              -- ids KEPT per-character (Unbound: true)
        200000,200001,200002,200003,200004,          -- Goal_Level : auras 99/150/175 + job level 50/70
        200017,200018,200019,200020,200021,200022,   -- Goal_Status: stat >= 90
        200023,200024,200025,200026,200027,200028,   -- Goal_Status: stat >= 125
        200031,                                       -- Job_Change : Reborn in Valhalla (class window)
        200032,200033,200034,200035                   -- Goal_Level : base level 100/600/999/200
      )
GROUP BY c.`account_id`, a.`id`;

-- ---------------------------------------------------------------------
-- 3) Remove the now-migrated per-character rows.
--    !! Same opt-out list as step 2. !!
-- ---------------------------------------------------------------------
DELETE FROM `achievement`
WHERE `char_id` <> 0
  AND `id` NOT IN (                -- same Unbound list as step 2
        200000,200001,200002,200003,200004,
        200017,200018,200019,200020,200021,200022,
        200023,200024,200025,200026,200027,200028,
        200031,
        200032,200033,200034,200035
      );

-- ---------------------------------------------------------------------
-- 4) Verification (optional, read-only). Uncomment to inspect the result.
-- ---------------------------------------------------------------------
-- -- Account-bound rows produced by the merge:
-- SELECT `account_id`, `id`, `completed`, `rewarded`
-- FROM `achievement`
-- WHERE `char_id` = 0
-- ORDER BY `account_id`, `id`;
--
-- -- Sanity check: no achievement id should exist BOTH as a player row and
-- -- an account row (would create a duplicate in a character's log).
-- SELECT p.`id`
-- FROM (SELECT DISTINCT `id` FROM `achievement` WHERE `char_id` <> 0) p
-- JOIN (SELECT DISTINCT `id` FROM `achievement` WHERE `char_id`  = 0) acc
--   ON p.`id` = acc.`id`;
