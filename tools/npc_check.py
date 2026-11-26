#!/usr/bin/env python3
"""Contrôle des scripts NPC avant un commit : encodage et chargement au démarrage.

Les scripts rAthena (.npc, .txt sous moon/, npc/, conf/ et db/) s'écrivent en
Windows-1252 (ANSI) : le map-server envoie leurs octets tels quels au client,
qui les affiche en Windows-1252. Un accent écrit en UTF-8 s'y afficherait
« Ã© », et un U+FFFD (octets EF BF BD) est un accent déjà perdu, à retaper.

    python3 tools/npc_check.py [--fix] [--json] [chemin...]
    python3 tools/npc_check.py --all
    python3 tools/npc_check.py --status --json

Sans chemin : les fichiers que `git commit -a` emporterait, c'est-à-dire les
fichiers suivis modifiés ou ajoutés depuis HEAD. --all : tous les scripts
suivis. --status : tout ce que montre `git status`, fichiers non suivis
compris, avec leur état git et, pour ceux de moon/, leur déclaration dans un
.conf (chargés ou non au démarrage du map-server).

Verdicts : « erreur » refuse le commit (code de sortie 1), « avertissement »
le signale sans rien refuser. --fix corrige ce qui se corrige sans perte :
un BOM UTF-8 en tête, et un fichier écrit en UTF-8 dont chaque caractère
existe en Windows-1252.
"""
import argparse
import json
import os
import re
import subprocess
import sys
from collections import Counter
from dataclasses import asdict, dataclass, field

SCRIPT_SUFFIXES = (".npc", ".txt")
# Dossiers dont le texte parvient au jeu. doc/ et docs/ n'en sont pas : leur
# UTF-8 est voulu, et un commit de documentation ne doit rien refuser.
GAME_TEXT_DIRS = ("moon/", "npc/", "conf/", "db/")
MOON_PREFIX = "moon/"
CONF_SUFFIX = ".conf"
CONF_DIRS = ("moon", "npc")
TARGET_ENCODING = "cp1252"
REPLACEMENT_BYTES = b"\xef\xbf\xbd"
UTF8_BOM = b"\xef\xbb\xbf"
# Octets auxquels Windows-1252 n'attribue aucun caractère.
UNDEFINED_CP1252_BYTES = frozenset((0x81, 0x8D, 0x8F, 0x90, 0x9D))
# Une suite UTF-8 multi-octets valide au milieu d'un fichier ANSI : un accent
# collé depuis un éditeur UTF-8 (« é » devenu les deux octets C3 A9, « Ã© »).
# Seules comptent celles qui donnent un caractère de Windows-1252 : le texte
# coréen des scripts officiels, en EUC-KR, en forme d'autres par hasard.
UTF8_SEQUENCE_RE = re.compile(rb"[\xc2-\xdf][\x80-\xbf]|[\xe0-\xef][\x80-\xbf]{2}|[\xf0-\xf4][\x80-\xbf]{3}")
CONF_NPC_RE = re.compile(r"^\s*(//)?\s*npc:\s*(\S+)")
SHOWN_CHARACTERS = 5
SHOWN_SEQUENCES = 3

ERROR = "erreur"
WARNING = "avertissement"
OK = "ok"


@dataclass
class Problem:
    code: str
    level: str
    message: str
    line: int = 0
    fixable: bool = False


@dataclass
class FileReport:
    path: str
    verdict: str = OK
    problems: list = field(default_factory=list)
    state: str = ""          # état git, --status seulement
    origin: str = ""         # ancien chemin d'un fichier renommé
    tracked: bool = True
    checked: bool = False    # vrai pour un script dont l'encodage a été lu
    startup: str = ""        # déclaré | commenté | absent, pour moon/
    startup_conf: str = ""   # fichier.conf:ligne de la déclaration

    def add(self, problem):
        self.problems.append(problem)
        if problem.level == ERROR or (problem.level == WARNING and self.verdict == OK):
            self.verdict = problem.level


def line_of(data, offset):
    return data.count(b"\n", 0, offset) + 1


def describe(counter):
    shown = ", ".join(f"{c} ×{n}" for c, n in counter.most_common(SHOWN_CHARACTERS))
    return shown + (", …" if len(counter) > SHOWN_CHARACTERS else "")


def check_bytes(report, data):
    """Remplit `report` des problèmes d'encodage du contenu `data`."""
    report.checked = True
    if REPLACEMENT_BYTES in data:
        offset = data.index(REPLACEMENT_BYTES)
        count = data.count(REPLACEMENT_BYTES)
        report.add(Problem("fffd", ERROR, f"U+FFFD (EF BF BD) ×{count}, dès la ligne "
                           f"{line_of(data, offset)} : un accent perdu, à retaper", line_of(data, offset)))
    body = data
    if data.startswith(UTF8_BOM):
        report.add(Problem("bom", ERROR, "BOM UTF-8 en tête du fichier", 1, fixable=True))
        body = data[len(UTF8_BOM):]
    if not any(b >= 0x80 for b in body):
        return
    try:
        text = body.decode("utf-8")
    except UnicodeDecodeError:
        check_ansi(report, body)
        return
    check_utf8(report, text)


def check_utf8(report, text):
    """Un fichier qui se lit en UTF-8 : ses accents ne passeront pas en jeu."""
    representable, foreign = Counter(), Counter()
    first_line = 0
    for number, line in enumerate(text.split("\n"), 1):
        for char in line:
            if ord(char) < 0x80 or char == "�":
                continue
            first_line = first_line or number
            try:
                char.encode(TARGET_ENCODING)
                representable[char] += 1
            except UnicodeEncodeError:
                foreign[char] += 1
    if representable and not foreign:
        # Un U+FFFD ne se convertit pas : il reste à retaper avant toute conversion.
        report.add(Problem("utf8", ERROR, f"écrit en UTF-8 ({describe(representable)}, dès la ligne "
                           f"{first_line}) : à convertir en Windows-1252", first_line,
                           fixable="�" not in text))
    elif representable:
        report.add(Problem("utf8", ERROR, f"écrit en UTF-8 ({describe(representable)}), avec "
                           f"{describe(foreign)} qui n'existent pas en Windows-1252 : à reprendre "
                           f"à la main", first_line))
    elif foreign:
        report.add(Problem("symbols", WARNING, f"caractères hors Windows-1252, laissés en UTF-8 "
                           f"({describe(foreign)}, dès la ligne {first_line})", first_line))


def is_cp1252_accent(sequence):
    try:
        char = sequence.decode("utf-8")
        char.encode(TARGET_ENCODING)
    except (UnicodeDecodeError, UnicodeEncodeError):
        return False
    return ord(char) >= 0x80


def is_game_text(path):
    return path.endswith(SCRIPT_SUFFIXES) and path.startswith(GAME_TEXT_DIRS)


def check_ansi(report, body):
    """Un fichier ANSI : octets sans caractère, et accents UTF-8 collés dedans."""
    undefined = [i for i, b in enumerate(body) if b in UNDEFINED_CP1252_BYTES]
    if undefined:
        first = undefined[0]
        report.add(Problem("undefined", WARNING, f"octet 0x{body[first]:02X} sans caractère en "
                           f"Windows-1252 ×{len(undefined)}, dès la ligne {line_of(body, first)}",
                           line_of(body, first)))
    sequences = [m for m in UTF8_SEQUENCE_RE.finditer(body) if is_cp1252_accent(m.group())]
    if sequences:
        first = sequences[0].start()
        shown = ", ".join(f"« {m.group().decode(TARGET_ENCODING, 'replace')} » pour "
                          f"« {m.group().decode('utf-8', 'replace')} »" for m in sequences[:SHOWN_SEQUENCES])
        report.add(Problem("mixed", WARNING, f"accent UTF-8 dans un fichier ANSI ×{len(sequences)}, "
                           f"dès la ligne {line_of(body, first)} : {shown}", line_of(body, first)))


def fix_bytes(data):
    """Le contenu corrigé sans perte, ou None si rien ne se corrige ainsi."""
    body = data[len(UTF8_BOM):] if data.startswith(UTF8_BOM) else data
    try:
        text = body.decode("utf-8")
    except UnicodeDecodeError:
        return body if body is not data else None
    try:
        converted = text.encode(TARGET_ENCODING)
    except UnicodeEncodeError:
        return body if body is not data else None
    return converted if converted != data else None


# ---- git -------------------------------------------------------------------
def git(*args):
    result = subprocess.run(["git", *args], capture_output=True, check=False)
    if result.returncode != 0:
        sys.stderr.write(result.stderr.decode("utf-8", "replace"))
        sys.exit(2)
    return result.stdout


def split_z(raw):
    return [p.decode("utf-8", "surrogateescape") for p in raw.split(b"\0") if p]


def commit_a_paths():
    return split_z(git("diff", "--name-only", "-z", "--diff-filter=ACMR", "HEAD"))


def tracked_scripts():
    return [p for p in split_z(git("ls-files", "-z")) if is_game_text(p)]


def status_entries():
    """(état, chemin, ancien chemin) de chaque entrée de `git status`."""
    tokens = split_z(git("status", "--porcelain=v1", "-z", "--untracked-files=all"))
    entries = []
    i = 0
    while i < len(tokens):
        token = tokens[i]
        code, path = token[:2], token[3:]
        origin = ""
        if "R" in code or "C" in code:
            i += 1
            origin = tokens[i]
        entries.append((code, path, origin))
        i += 1
    return entries


def state_label(code):
    if code == "??":
        return "nouveau"
    if "U" in code or code in ("AA", "DD"):
        return "conflit"
    if "D" in code:
        return "supprimé"
    if "R" in code:
        return "renommé"
    if code[0] == "A":
        return "ajouté"
    return "modifié"


def branch_state():
    upstream = subprocess.run(["git", "rev-parse", "--abbrev-ref", "@{u}"],
                              capture_output=True, text=True, check=False)
    name = git("branch", "--show-current").decode().strip()
    if upstream.returncode != 0:
        return {"name": name, "upstream": "", "ahead": 0, "behind": 0}
    counts = git("rev-list", "--left-right", "--count", "@{u}...HEAD").decode().split()
    return {"name": name, "upstream": upstream.stdout.strip(),
            "behind": int(counts[0]), "ahead": int(counts[1])}


def startup_declarations():
    """chemin -> (« déclaré » ou « commenté », fichier.conf:ligne)."""
    declared = {}
    for top in CONF_DIRS:
        for folder, _dirs, files in os.walk(top):
            for name in sorted(files):
                if not name.endswith(CONF_SUFFIX):
                    continue
                conf = os.path.join(folder, name).replace(os.sep, "/")
                with open(conf, "rb") as f:
                    for number, raw in enumerate(f, 1):
                        m = CONF_NPC_RE.match(raw.decode(TARGET_ENCODING, "replace"))
                        if not m:
                            continue
                        state = "commenté" if m.group(1) else "déclaré"
                        # Une déclaration active l'emporte sur une ligne commentée.
                        if declared.get(m.group(2), ("",))[0] != "déclaré":
                            declared[m.group(2)] = (state, f"{conf}:{number}")
    return declared


# ---- rapports ----------------------------------------------------------------
def check_path(path):
    report = FileReport(path)
    if is_game_text(path) and os.path.isfile(path):
        with open(path, "rb") as f:
            check_bytes(report, f.read())
    return report


def add_startup(report, declarations, code, origin):
    """Un script de moon/ ne se charge au démarrage que déclaré par une ligne
    `npc:` active d'un .conf : un nom neuf ou changé doit l'être, un nom
    disparu ne doit plus l'être."""
    if not report.path.startswith(MOON_PREFIX) or not report.path.endswith(SCRIPT_SUFFIXES):
        return
    state, conf = declarations.get(report.path, ("absent", ""))
    report.startup, report.startup_conf = state, conf
    new_name = code == "??" or code[0] == "A" or "R" in code
    if "D" in code and state == "déclaré":
        report.add(Problem("startup", WARNING, f"supprimé mais toujours déclaré ({conf}) : le "
                           f"map-server le cherchera au démarrage"))
    elif new_name and state == "absent":
        report.add(Problem("startup", WARNING, "déclaré dans aucun .conf : non chargé au démarrage"))
    elif new_name and state == "commenté":
        report.add(Problem("startup", WARNING, f"déclaration commentée ({conf}) : non chargé au démarrage"))
    if origin:
        old_state, old_conf = declarations.get(origin, ("absent", ""))
        if old_state == "déclaré":
            report.add(Problem("startup", WARNING, f"l'ancien nom {origin} reste déclaré ({old_conf}) : "
                               f"le map-server le cherchera au démarrage"))


def status_reports():
    declarations = startup_declarations()
    reports = []
    for code, path, origin in status_entries():
        report = check_path(path) if "D" not in code else FileReport(path)
        report.state, report.origin, report.tracked = state_label(code), origin, code != "??"
        add_startup(report, declarations, code, origin)
        reports.append(report)
    return reports


def print_human(reports):
    for report in reports:
        if not report.checked and not report.problems:
            continue
        head = {OK: "OK          ", WARNING: "AVERTISSEMENT", ERROR: "ERREUR      "}[report.verdict]
        print(f"{head} {report.path}")
        for problem in report.problems:
            print(f"    - {problem.message}" + ("  [--fix]" if problem.fixable else ""))
    errors = sum(r.verdict == ERROR for r in reports)
    warnings = sum(r.verdict == WARNING for r in reports)
    checked = sum(r.checked for r in reports)
    print(f"{checked} script(s) contrôlé(s) : {errors} en erreur, {warnings} avec avertissement.")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("paths", nargs="*", help="chemins relatifs au dépôt")
    parser.add_argument("--all", action="store_true", help="tous les scripts suivis")
    parser.add_argument("--status", action="store_true", help="tout ce que montre git status")
    parser.add_argument("--fix", action="store_true", help="corriger ce qui se corrige sans perte")
    parser.add_argument("--json", action="store_true", help="sortie JSON")
    args = parser.parse_args()
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    os.chdir(git("rev-parse", "--show-toplevel").decode().strip())

    if args.status:
        reports = status_reports()
        if args.json:
            print(json.dumps({"branch": branch_state(), "files": [asdict(r) for r in reports]}))
        else:
            print_human(reports)
        return 0

    paths = [p[2:] if p.startswith("./") else p for p in args.paths]
    if args.all:
        paths = tracked_scripts()
    elif not paths:
        paths = commit_a_paths()
    reports = [check_path(p) for p in paths]

    if args.fix:
        for report in reports:
            if not any(p.fixable for p in report.problems):
                continue
            with open(report.path, "rb") as f:
                fixed = fix_bytes(f.read())
            if fixed is not None:
                with open(report.path, "wb") as f:
                    f.write(fixed)
                print(f"corrigé : {report.path}  ({', '.join(p.code for p in report.problems if p.fixable)})")
        reports = [check_path(p) for p in paths]

    if args.json:
        print(json.dumps({"files": [asdict(r) for r in reports]}))
    else:
        print_human(reports)
    return 1 if any(r.verdict == ERROR for r in reports) else 0


if __name__ == "__main__":
    sys.exit(main())
