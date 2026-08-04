-- ═══════════════════════════════════════════════════════════════════════════
--  Synchronisation de l'e-mail des comptes RO sur celui du compte web (phpBB)
-- ═══════════════════════════════════════════════════════════════════════════
--
--  POURQUOI
--  La suppression d'un personnage se fait par E-MAIL : conf/char_athena.conf a
--  char_del_option: 1 (= CHAR_DEL_EMAIL). Le char-server compare la saisie du
--  joueur à `login`.`email` du compte (chclif_delchar_check, src/char/char_clif.cpp)
--  avec stricmp -> insensible à la casse, mais l'adresse doit être la bonne.
--
--  Or rien ne propageait un changement d'e-mail du forum vers le compte de jeu :
--    - index.php ne copie `user_email` qu'à la CRÉATION du compte RO ;
--    - ucp_profile.php ne recopiait l'e-mail qu'en cas de changement de MOT DE
--      PASSE (et y écrivait l'ANCIENNE adresse).
--  Un joueur qui change d'adresse sur le site se retrouve donc à devoir saisir
--  une adresse qu'il ne connaît plus -> suppression impossible.
--
--  CE QUE FAIT CE SCRIPT
--  Aligne `rathena`.`login`.`email` sur `site`.`phpbb_users`.`user_email`, pour
--  les comptes RO liés à un membre du forum (`login`.`user_id` = id phpBB, la
--  colonne ajoutée par le fork ; c'est déjà la clé de liaison utilisée par
--  api/game_login.php et index.php).
--
--  À SAVOIR AVANT DE LANCER
--   1. `login`.`email` est un varchar(39) et char_session_data::email un char[40]
--      (39 utiles) : on écrit donc LEFT(user_email, 39). Ce n'est PAS un problème
--      pour le joueur — le char-server tronque de la même façon la saisie reçue
--      (safestrncpy dans un char[40]), la comparaison reste vraie s'il tape son
--      adresse complète.
--   2. Effet à la PROCHAINE connexion : le login-server relit `email` en base à
--      l'authentification et le transmet au char-server, qui le garde pour la
--      durée de la session. Un joueur déjà connecté garde l'ancienne adresse
--      jusqu'à sa reconnexion.
--   3. Aucun besoin d'arrêter les serveurs : le login-server ne réécrit `email`
--      qu'avec la valeur qu'il vient de lire (account.cpp, mmo_auth_tosql).
--   4. Les comptes RO NON liés (user_id = 0 : créés à la main, ou d'avant la
--      liaison au forum) ne sont pas touchés — cf. la requête de contrôle 5.
--
--  Bases : `site` = forum phpBB (préfixe phpbb_), `rathena` = jeu (prod).
--  Pour le serveur de test, remplacer `rathena` par `rathena-test`.
-- ───────────────────────────────────────────────────────────────────────────


-- ── 1. Constat : combien de comptes sont désalignés ? ──────────────────────
-- ⚠ CAST(... AS BINARY) des deux côtés : `login`.`email` (latin1) et
-- `user_email` (utf8) n'ont pas la même collation, une comparaison directe
-- lèverait « Illegal mix of collations ». LOWER() reproduit le stricmp du
-- char-server (la casse n'a jamais empêché une suppression).
SELECT
    COUNT(*)                                                    AS comptes_lies,
    SUM(CAST(LOWER(l.email) AS BINARY)
        <> CAST(LOWER(LEFT(u.user_email, 39)) AS BINARY))       AS desalignes,
    SUM(l.email = 'a@a.com')                                    AS email_par_defaut,
    SUM(CHAR_LENGTH(u.user_email) > 39)                         AS adresses_tronquees
  FROM `rathena`.`login` l
  JOIN `site`.`phpbb_users` u ON u.user_id = l.user_id
 WHERE l.user_id > 0;


-- ── 2. Détail des comptes qui vont changer (à garder sous la main) ─────────
SELECT l.account_id, l.userid            AS login_ro,
       l.email                           AS email_actuel,
       u.username                        AS membre_site,
       u.user_email                      AS email_site,
       LEFT(u.user_email, 39)            AS email_applique
  FROM `rathena`.`login` l
  JOIN `site`.`phpbb_users` u ON u.user_id = l.user_id
 WHERE l.user_id > 0
   AND u.user_email <> ''
   AND CAST(LOWER(l.email) AS BINARY)
       <> CAST(LOWER(LEFT(u.user_email, 39)) AS BINARY)
 ORDER BY l.account_id;


-- ── 3. Sauvegarde des anciennes valeurs (filet de sécurité) ────────────────
-- Table datée : on peut relire ou restaurer sans dépendre d'un dump complet.
-- Adapter la date au jour de l'exécution.
CREATE TABLE `rathena`.`login_email_backup_20260804` (
  `account_id` INT NOT NULL PRIMARY KEY,
  `userid`     VARCHAR(23) NOT NULL,
  `email`      VARCHAR(39) NOT NULL,
  `saved_at`   DATETIME NOT NULL
);

INSERT INTO `rathena`.`login_email_backup_20260804`
       (`account_id`, `userid`, `email`, `saved_at`)
SELECT l.account_id, l.userid, l.email, NOW()
  FROM `rathena`.`login` l
  JOIN `site`.`phpbb_users` u ON u.user_id = l.user_id
 WHERE l.user_id > 0
   AND u.user_email <> ''
   AND CAST(LOWER(l.email) AS BINARY)
       <> CAST(LOWER(LEFT(u.user_email, 39)) AS BINARY);


-- ── 4. LA SYNCHRONISATION ──────────────────────────────────────────────────
-- u.user_email <> ''   : on n'efface jamais une adresse de jeu au profit du vide.
-- u.user_type <> 2     : USER_IGNORE = comptes robots du forum (jamais de compte RO).
-- La condition de différence évite de toucher les lignes déjà bonnes (le nombre
-- de lignes affectées = exactement le compte annoncé en 1).
UPDATE `rathena`.`login` l
  JOIN `site`.`phpbb_users` u ON u.user_id = l.user_id
   SET l.email = LEFT(u.user_email, 39)
 WHERE l.user_id > 0
   AND u.user_email <> ''
   AND u.user_type <> 2
   AND CAST(LOWER(l.email) AS BINARY)
       <> CAST(LOWER(LEFT(u.user_email, 39)) AS BINARY);

-- Contrôle : doit renvoyer 0.
SELECT COUNT(*) AS restants_desalignes
  FROM `rathena`.`login` l
  JOIN `site`.`phpbb_users` u ON u.user_id = l.user_id
 WHERE l.user_id > 0
   AND u.user_email <> ''
   AND u.user_type <> 2
   AND CAST(LOWER(l.email) AS BINARY)
       <> CAST(LOWER(LEFT(u.user_email, 39)) AS BINARY);


-- ── 5. Restent à traiter à la main : les comptes NON liés au forum ──────────
-- Ceux-là n'ont aucun membre de référence ; leur joueur reste bloqué s'il ne
-- connaît pas l'adresse. Deux issues : les lier (poser user_id), ou leur poser
-- l'adresse par défaut 'a@a.com' — le char-server accepte alors une saisie VIDE
-- (chclif_delchar_check : e-mail par défaut + saisie vide = suppression permise).
SELECT l.account_id, l.userid, l.email, l.lastlogin
  FROM `rathena`.`login` l
 WHERE (l.user_id IS NULL OR l.user_id = 0)
   AND l.account_id NOT IN (1, 2)  -- comptes de service (login-server / NPC)
 ORDER BY l.lastlogin DESC;


-- ── 6. Restauration, si besoin ─────────────────────────────────────────────
-- UPDATE `rathena`.`login` l
--   JOIN `rathena`.`login_email_backup_20260804` b ON b.account_id = l.account_id
--    SET l.email = b.email;
