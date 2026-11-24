"""La file de vérification des parties classées du mode survivors de moonclient.

Le serveur de carte range chaque partie classée avec son journal
(`survivors_run.journal`, reçu par CZ 0x0F52). Ce programme prend ces parties
une à une, fait rejouer leur journal par `survivors-verifier` — l'outil de
moonclient, bâti à la révision de la version de règles qu'il juge — et range le
verdict (`sql-files/moonclient_survivors.sql` dit les colonnes) :

    proven   -> `survivors_run.verified` = 1 ; la ligne du tableau
                (`survivors_best`) est marquée prouvée si cette partie la tient ;
    refuted  -> `survivors_run.verified` = 2, avec ce qui la réfute ; si elle
                tenait la ligne du tableau, la ligne revient à la meilleure
                partie non réfutée du compte, ou disparaît ;
    autre    -> `survivors_run.verified` = 3 : la ligne ou l'outil a fait
                défaut, à reprendre à la main (remettre 0 la refait passer).

IL TOURNE HORS DU SERVEUR DE CARTE, en priorité basse : un rejeu prend une
vingtaine de secondes d'un cœur, et le serveur de carte n'a qu'un fil.

    python3 survivors_verify_queue.py \\
        --defaults-file /chemin/survivors-verifier.cnf \\
        --database <base du serveur de carte> \\
        --verifiers /chemin/du/dossier/des/binaires

`--defaults-file` : un fichier d'options MariaDB (`[client]`, `user`,
`password`, `host`), lisible du seul compte qui lance la file : le mot de passe
ne passe jamais par la ligne de commande.
`--verifiers` : le dossier des binaires `survivors-verifier*`. Chacun dit la
version de règles qu'il juge (`--ruleset`) ; une partie d'une version sans
binaire attend, sans bloquer les autres.
`--once` : vide la file puis s'arrête ; sinon elle se relit toutes les
`--interval` secondes.
"""

import argparse
import logging
import os
import subprocess
import sys
import time

LOG = logging.getLogger("survivors_verify_queue")

# Les colonnes d'une partie, dans l'ordre des champs que lit `survivors-verifier`
# (moonclient, tools/survivors-verifier/verifier_lines.h) : le journal en
# hexadécimal, en dernier.
RUN_COLUMNS = (
    "`run_id`, `ruleset`, `arena`, `class_id`, `board`, `seed`, `issued_at`, "
    "`survived_steps`, `kill_points`, `kills`, `boss_kills`, `level`, `outcome`, "
    "`journal_bytes`, HEX(`journal`)"
)
RUN_FIELDS = 15
VERDICT_FIELDS = 5

# Les valeurs de `survivors_run.verified`.
PENDING = 0
PROVEN = 1
REFUTED = 2
UNREADABLE = 3

# La colonne `verify_note` est un varchar(255).
NOTE_MOST_CHARS = 255
# Au-delà, l'outil est tenu pour bloqué : le rejeu d'une partie entière prend
# une vingtaine de secondes sur la machine du serveur.
VERIFIER_TIMEOUT_S = 600
RULESET_TIMEOUT_S = 10
VERIFIER_PREFIX = "survivors-verifier"


class QueueError(Exception):
    """La base ou l'outil n'a pas répondu comme attendu."""


class Database:
    """La base du serveur de carte, par le client en ligne de commande."""

    def __init__(self, client, defaults_file, database):
        self.client = client
        self.defaults_file = defaults_file
        self.database = database

    def run(self, sql):
        """Joue `sql` d'un seul tenant et rend ses lignes, champs séparés."""
        done = subprocess.run(
            [self.client, f"--defaults-extra-file={self.defaults_file}", "--batch", "--raw",
             "--skip-column-names", self.database, "-e", sql],
            capture_output=True, check=False)
        if done.returncode != 0:
            raise QueueError("base : " + done.stderr.decode("utf-8", errors="replace").strip())
        return [line.split("\t") for line in done.stdout.decode("utf-8").splitlines()]


def find_verifiers(folder):
    """Les binaires du dossier, par version de règles jugée."""
    verifiers = {}
    try:
        names = sorted(os.listdir(folder))
    except OSError as error:
        raise QueueError(f"dossier des binaires : {error}") from error
    for name in names:
        path = os.path.join(folder, name)
        if not name.startswith(VERIFIER_PREFIX) or not os.path.isfile(path):
            continue
        if not os.access(path, os.X_OK):
            continue
        try:
            done = subprocess.run([path, "--ruleset"], capture_output=True, check=False,
                                  timeout=RULESET_TIMEOUT_S)
            ruleset = int(done.stdout.decode("ascii", errors="replace").strip())
        except (OSError, ValueError, subprocess.TimeoutExpired) as error:
            LOG.warning("%s ne dit pas sa version de règles : %s", path, error)
            continue
        verifiers[ruleset] = path
    return verifiers


def next_run(database, rulesets):
    """La plus ancienne partie à vérifier d'une version qui a son binaire."""
    if not rulesets:
        return None
    listed = ", ".join(str(int(ruleset)) for ruleset in sorted(rulesets))
    rows = database.run(
        f"SELECT {RUN_COLUMNS} FROM `survivors_run` WHERE `verified` = {PENDING} "
        f"AND `journal` IS NOT NULL AND `ruleset` IN ({listed}) ORDER BY `run_id` LIMIT 1")
    if not rows:
        return None
    if len(rows[0]) != RUN_FIELDS:
        raise QueueError(f"partie lue en {len(rows[0])} champs, attendu {RUN_FIELDS}")
    return rows[0]


def judge(verifier, row, timeout_s=VERIFIER_TIMEOUT_S):
    """Le verdict de l'outil sur une partie : (verdict, faille, détail)."""
    line = "\t".join(row) + "\n"
    try:
        done = subprocess.run([verifier], input=line.encode("ascii"), capture_output=True,
                              check=False, timeout=timeout_s)
    except (OSError, subprocess.TimeoutExpired, UnicodeEncodeError) as error:
        return "failed", "tool", f"outil en échec : {error}"
    fields = done.stdout.decode("utf-8", errors="replace").rstrip("\r\n").split("\t")
    if done.returncode != 0 or len(fields) != VERDICT_FIELDS or fields[0] != row[0]:
        return "failed", "tool", f"outil en échec : code {done.returncode}, sortie illisible"
    return fields[1], fields[2], fields[4]


def text_literal(text):
    """Un texte en littéral SQL, sans qu'aucun de ses caractères ne s'y lise."""
    if not text:
        return "''"
    return "CONVERT(UNHEX('" + text[:NOTE_MOST_CHARS].encode("utf-8").hex() + "') USING utf8mb4)"


def verdict_sql(run_id, verdict, flaw, detail):
    """Ce qui range le verdict d'une partie, d'un seul tenant."""
    run_id = int(run_id)
    if verdict == "proven":
        return (
            "START TRANSACTION; "
            f"UPDATE `survivors_run` SET `verified` = {PROVEN}, `verify_note` = '', "
            f"`verified_at` = UNIX_TIMESTAMP() WHERE `run_id` = {run_id} AND `verified` = {PENDING}; "
            f"UPDATE `survivors_best` SET `verified` = 1 WHERE `run_id` = {run_id}; "
            "COMMIT;")
    note = text_literal(f"{flaw} : {detail}" if detail else flaw)
    if verdict != "refuted":
        return (
            f"UPDATE `survivors_run` SET `verified` = {UNREADABLE}, `verify_note` = {note}, "
            f"`verified_at` = UNIX_TIMESTAMP() WHERE `run_id` = {run_id} AND `verified` = {PENDING};")
    # La partie réfutée quitte le tableau : si elle tenait la ligne de son
    # compte, la ligne revient à sa meilleure partie non réfutée, dans l'ordre
    # du tableau, ou disparaît.
    return (
        "START TRANSACTION; "
        f"UPDATE `survivors_run` SET `verified` = {REFUTED}, `verify_note` = {note}, "
        f"`verified_at` = UNIX_TIMESTAMP() WHERE `run_id` = {run_id} AND `verified` = {PENDING}; "
        f"SELECT COUNT(*) INTO @held FROM `survivors_best` WHERE `run_id` = {run_id}; "
        f"DELETE FROM `survivors_best` WHERE `run_id` = {run_id}; "
        "INSERT INTO `survivors_best` (`user_id`, `ruleset`, `arena`, `class_id`, `board`, `week`, "
        "`run_id`, `char_id`, `name`, `score`, `survived_steps`, `level`, `verified`) "
        "SELECT kept.`user_id`, kept.`ruleset`, kept.`arena`, kept.`class_id`, kept.`board`, "
        "kept.`week`, kept.`run_id`, kept.`char_id`, kept.`name`, kept.`score`, "
        f"kept.`survived_steps`, kept.`level`, IF(kept.`verified` = {PROVEN}, 1, 0) "
        "FROM `survivors_run` AS refuted JOIN `survivors_run` AS kept "
        "ON kept.`user_id` = refuted.`user_id` AND kept.`ruleset` = refuted.`ruleset` "
        "AND kept.`arena` = refuted.`arena` AND kept.`class_id` = refuted.`class_id` "
        "AND kept.`board` = refuted.`board` AND kept.`week` = refuted.`week` "
        f"WHERE refuted.`run_id` = {run_id} AND kept.`verified` <> {REFUTED} AND @held > 0 "
        "ORDER BY kept.`score` DESC, kept.`survived_steps` DESC, kept.`run_id` ASC LIMIT 1; "
        "COMMIT;")


def drain(database, verifiers):
    """Vide la file des versions qui ont leur binaire ; rend le nombre de parties jugées."""
    judged = 0
    while True:
        row = next_run(database, verifiers.keys())
        if row is None:
            return judged
        run_id, ruleset = row[0], int(row[1])
        began = time.monotonic()
        verdict, flaw, detail = judge(verifiers[ruleset], row)
        database.run(verdict_sql(run_id, verdict, flaw, detail))
        elapsed_s = time.monotonic() - began
        if verdict == "proven":
            LOG.info("partie %s : prouvée (%.1f s)", run_id, elapsed_s)
        else:
            LOG.warning("partie %s : %s, %s — %s (%.1f s)", run_id, verdict, flaw, detail, elapsed_s)
        judged += 1


def main(argv=None):
    parser = argparse.ArgumentParser(description="File de vérification des parties classées "
                                                 "du mode survivors.")
    parser.add_argument("--defaults-file", required=True,
                        help="fichier d'options MariaDB ([client] user, password, host)")
    parser.add_argument("--database", required=True, help="la base du serveur de carte")
    parser.add_argument("--verifiers", required=True,
                        help="le dossier des binaires survivors-verifier*")
    parser.add_argument("--client", default="mariadb", help="le client MariaDB (mariadb)")
    parser.add_argument("--interval", type=int, default=30,
                        help="secondes entre deux lectures de la file (30)")
    parser.add_argument("--once", action="store_true", help="vider la file puis s'arrêter")
    args = parser.parse_args(argv)
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")

    database = Database(args.client, args.defaults_file, args.database)
    known = None
    while True:
        try:
            # Relu à chaque passe : un binaire posé pour une version neuve est
            # pris sans relancer la file.
            verifiers = find_verifiers(args.verifiers)
            if sorted(verifiers) != known:
                known = sorted(verifiers)
                LOG.info("versions de règles jugées : %s", known or "aucune")
            drain(database, verifiers)
        except QueueError as error:
            LOG.error("%s", error)
            if args.once:
                return 1
        if args.once:
            return 0
        time.sleep(args.interval)


if __name__ == "__main__":
    sys.exit(main())
