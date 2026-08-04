#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_skill_db_site.py -- generate ressources/skill_db_site.sql for the website

The website turns item scripts into readable French descriptions. Every time a
script mentions a skill (bAutoSpell, bSkillAtk, autobonus3, equip conditions...)
processskill() looks the skill up in `skill_db_site` to print its display name.
That table used to live in the frozen `server` archive database, so anything
added after april 2020 rendered as a blank hole in the description.

This script rebuilds the table from the live sources:

  * skillid.lub        (client) -- SKID constant -> numeric id
  * skillinfolist.lub  (client) -- SKID constant -> SkillName, the display name
                                   a player actually reads in game
  * skill_db.yml       (server) -- authoritative id + Description fallback

Output is a dump the website replays through its ACP « Gestion ressources »
panel, exactly like item_db2.sql / mob_db2.sql.

Usage
-----
  python tools/gen_skill_db_site.py                       # write the dump
  python tools/gen_skill_db_site.py --dry-run             # report only
  python tools/gen_skill_db_site.py --client "E:/.../skillinfoz" --out x.sql

Notes
-----
* ext_name comes from the client (SkillName) so the site prints the same
  wording as the game window. skill_db.yml's Description only fills the gaps
  (currently a single skill: ALL_EQSWITCH, absent from skillinfolist.lub).
* The server wins on ids: item scripts are server-side, so a numeric skill
  reference means a server id. Client-only constants that collide with a
  server id are dropped (BA_FROSTJOKE, MH_SONIC_CLAW: historical typos;
  RL_GLITTERING_GREED_ATK: a genuine client/server id clash).
* Sentinel constants (*_BEGIN, *_LAST, SCRIPT_000, ...) are skipped: padding,
  not skills.
* Serveur PRE-RENEWAL : seuls db/pre-re/skill_db.yml et db/import/skill_db.yml
  sont lus. db/re/ n'est jamais consulte.
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from gen_skilldescript import (  # noqa: E402
    MARKER_EXTRA, MARKER_RE, REPO,
    load_skill_db, parse_skillid, pretty_name, write_atomic,
)

TABLE = "skill_db_site"

# Dépôts frères : moonlight/ , moonlightsite/ , Moonlight-Client/
DEFAULT_CLIENT = REPO.parent / "Moonlight-Client/data/luafiles514/luafiles/skillinfoz"
DEFAULT_OUT = REPO.parent / "moonlightsite/ressources/skill_db_site.sql"

SKILL_DB_PATHS = [REPO / "db/pre-re/skill_db.yml", REPO / "db/import/skill_db.yml"]

BLOCK_RE = re.compile(r"\[SKID\.([A-Za-z_]\w*)\]\s*=\s*\{")
SKILLNAME_RE = re.compile(r'SkillName\s*=\s*"((?:[^"\\]|\\.)*)"')

# Longueurs de colonne : marge sur le plus long observé (39 caractères).
NAME_LEN = 64


def read_latin1(path: Path) -> str:
    """latin-1 maps every byte 1:1, so this round-trips the file exactly."""
    return path.read_bytes().decode("latin-1")


def parse_skillnames(text: str) -> "dict[str, str]":
    """Map each [SKID.NAME] block to its SkillName value.

    The search is bounded by the next block so a SkillName can never be picked
    up from the following entry when one is missing.
    """
    out: dict = {}
    starts = [(m.group(1), m.end()) for m in BLOCK_RE.finditer(text)]
    for i, (const, pos) in enumerate(starts):
        end = starts[i + 1][1] if i + 1 < len(starts) else len(text)
        m = SKILLNAME_RE.search(text, pos, end)
        if m:
            out[const] = m.group(1)
    return out


def is_marker(const: str) -> bool:
    return bool(MARKER_RE.search(const)) or const in MARKER_EXTRA


def usable(name: str) -> bool:
    """A display name the site can print: non-empty, pure ASCII.

    skillinfolist.lub still holds a few mangled Korean names; those must fall
    back to the server Description rather than reach the website as mojibake.
    """
    return bool(name) and bool(name.strip()) and all(ord(c) < 127 for c in name)


def sql_escape(value: str) -> str:
    return value.replace("\\", "\\\\").replace("'", "\\'")


def build_rows(skill_db: dict, skill_ids: dict, skill_names: dict, report: list):
    """Merge the three sources into (id, int_name, ext_name) rows.

    Server entries first so they own their id; client-only constants are then
    added unless their id is already taken.
    """
    rows, taken, fallbacks = [], {}, []

    def add(const: str, skid: int, source: str) -> None:
        name = skill_names.get(const, "")
        if usable(name):
            ext = name
        else:
            desc = str(skill_db.get(const, {}).get("Description") or "")
            if usable(desc):
                ext = desc
                fallbacks.append(f'{const} -> Description serveur "{desc}"')
            else:
                ext = pretty_name(const)
                fallbacks.append(f'{const} -> nom deduit "{ext}"')
        taken[skid] = const
        rows.append((skid, const, ext))

    for const in sorted(skill_db):
        skid = skill_db[const].get("Id")
        if skid is None:
            report.append(f"! {const} : aucun Id dans skill_db.yml, ignore.")
            continue
        if is_marker(const):
            continue
        if skid in taken:
            report.append(f"! {const} : id {skid} deja pris par {taken[skid]}, ignore.")
            continue
        add(const, int(skid), "serveur")

    for const in sorted(set(skill_ids) - set(skill_db)):
        if is_marker(const):
            continue
        skid = int(skill_ids[const])
        if skid in taken:
            report.append(
                f"- {const} (client seul) : id {skid} deja pris par "
                f"{taken[skid]} cote serveur, ignore."
            )
            continue
        add(const, skid, "client")

    rows.sort(key=lambda r: r[0])
    return rows, fallbacks


def render_sql(rows: "list[tuple[int, str, str]]") -> str:
    """Emit a dump the site's moon_res_import_sql() can replay statement by
    statement: one instruction per line, table name unqualified."""
    out = [
        "#",
        f"# Table structure for table `{TABLE}`",
        "#",
        "# Genere par tools/gen_skill_db_site.py -- ne pas editer a la main.",
        "# Sources : client skillid.lub + skillinfolist.lub, serveur skill_db.yml",
        "#",
        "",
        f"DROP TABLE IF EXISTS `{TABLE}`;",
        f"CREATE TABLE `{TABLE}` (",
        "  `id` smallint(5) UNSIGNED NOT NULL DEFAULT 0,",
        f"  `int_name` varchar({NAME_LEN}) NOT NULL DEFAULT '',",
        f"  `ext_name` varchar({NAME_LEN}) NOT NULL DEFAULT '',",
        "  PRIMARY KEY (`int_name`),",
        "  UNIQUE KEY `id` (`id`)",
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;",
        "",
    ]
    for skid, int_name, ext_name in rows:
        out.append(
            f"REPLACE INTO `{TABLE}` (`id`,`int_name`,`ext_name`) VALUES "
            f"({skid},'{sql_escape(int_name)}','{sql_escape(ext_name)}');"
        )
    out.append("")
    return "\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--client", type=Path, default=DEFAULT_CLIENT,
                    help="dossier skillinfoz du client (defaut : depot frere)")
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT,
                    help="dump SQL a produire (defaut : moonlightsite/ressources)")
    ap.add_argument("--dry-run", action="store_true",
                    help="afficher le rapport sans rien ecrire")
    args = ap.parse_args()

    skillid = args.client / "skillid.lub"
    skillinfo = args.client / "skillinfolist.lub"
    for p in (skillid, skillinfo):
        if not p.is_file():
            sys.exit(f"Introuvable : {p}\nPrecise le dossier avec --client.")

    skill_ids = dict(parse_skillid(read_latin1(skillid)))
    skill_names = parse_skillnames(read_latin1(skillinfo))
    skill_db = load_skill_db(SKILL_DB_PATHS)
    if not skill_db:
        sys.exit(f"Aucun skill lu depuis {SKILL_DB_PATHS[0]} : chemin serveur invalide ?")

    report: list = []
    rows, fallbacks = build_rows(skill_db, skill_ids, skill_names, report)

    print(f"skillid.lub       : {len(skill_ids)} constantes")
    print(f"skillinfolist.lub : {len(skill_names)} noms affiches")
    print(f"skill_db.yml      : {len(skill_db)} skills serveur")
    print(f"-> {len(rows)} lignes pour `{TABLE}`")

    if fallbacks:
        print(f"\n{len(fallbacks)} nom(s) sans SkillName client :")
        for line in fallbacks[:20]:
            print(f"  {line}")
        if len(fallbacks) > 20:
            print(f"  ... et {len(fallbacks) - 20} autre(s).")
    if report:
        print(f"\n{len(report)} entree(s) ecartee(s) :")
        for line in report[:20]:
            print(f"  {line}")
        if len(report) > 20:
            print(f"  ... et {len(report) - 20} autre(s).")

    if not rows:
        sys.exit("Aucune ligne produite : rien n'est ecrit.")

    sql = render_sql(rows)
    if args.dry_run:
        print(f"\n--dry-run : {args.out} laisse intact.")
        return 0

    args.out.parent.mkdir(parents=True, exist_ok=True)
    write_atomic(args.out, sql, encoding="utf-8")
    print(f"\nEcrit : {args.out} ({len(sql.encode('utf-8')) / 1024:.1f} Ko)")
    print("Regenere la table depuis l'ACP : Moonlight -> Gestion ressources.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
