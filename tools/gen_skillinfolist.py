#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_skillinfolist.py -- complete client-side skillinfolist.lub

Companion to gen_skilldescript.py. SKILL_INFO_LIST is the master table the
client reads to know a skill's display name, maximum level, SP cost and
attack range; SKILL_DESCRIPT only holds the tooltip text. A skill missing
from SKILL_INFO_LIST is not shown properly even if it has a description.

Usage
-----
  python tools/gen_skillinfolist.py --client "E:/.../skillinfoz"           # report
  python tools/gen_skillinfolist.py --client "..." --apply                 # write
  python tools/gen_skillinfolist.py --client "..." --apply --fix-mojibake  # + repair

Notes
-----
* Unlike skilldescript.lub, this file is NOT pure ASCII: a few Korean
  SkillName values survive in it. It is therefore read and written back as
  latin-1, which round-trips every byte untouched; only the generated block
  is constrained to ASCII.
* --fix-mojibake replaces SkillName values that contain U+FFFD replacement
  characters (irrecoverably mangled Korean text) with the English name from
  skill_db.yml. Off by default since it rewrites existing entries.
* Serveur PRE-RENEWAL : seuls db/pre-re/skill_db.yml et db/import/skill_db.yml
  sont lus. db/re/ n'est jamais consulte.
* No _NeedSkillList is generated: the missing skills are almost all monster
  or internal skills, which have no place in a player skill tree.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from gen_skilldescript import (  # noqa: E402
    MARKER_EXTRA, MARKER_RE, REPO,
    load_skill_db, lv_value, parse_skillid, pretty_name, write_atomic,
)

# The file is read as latin-1, so a UTF-8 encoded U+FFFD shows up as its
# three raw bytes (EF BF BD) rather than as a single replacement character.
MOJIBAKE_RE = re.compile("\ufffd|\u00ef\u00bf\u00bd")


def read_latin1(path: Path) -> str:
    """latin-1 maps every byte 1:1, so this round-trips the file exactly."""
    return path.read_bytes().decode("latin-1")


def parse_listed(text: str) -> "set[str]":
    return set(re.findall(r"\[SKID\.([A-Za-z_]\w*)\]\s*=\s*\{", text))


def lv_list(field, maxlv: int, default=None) -> "list | None":
    """Expand a scalar / per-level skill_db field into a maxlv-long list."""
    out = []
    for lv in range(1, maxlv + 1):
        v = lv_value(field, lv)
        out.append(v if isinstance(v, int) else default)
    if all(v is None for v in out):
        return None
    return [0 if v is None else v for v in out]


def build_entry(const: str, sk: dict) -> str:
    sk = sk or {}
    name = (sk.get("Description") or pretty_name(const)).strip()
    maxlv = int(sk.get("MaxLevel") or 1)

    rows = [f'"{const}"', f'SkillName = "{name}"', f"MaxLv = {maxlv}"]

    sp = lv_list((sk.get("Requires") or {}).get("SpCost"), maxlv)
    if sp and any(sp):
        rows.append("SpAmount = { " + ", ".join(str(v) for v in sp) + " }")

    # Range < 0 in skill_db means "the caster's weapon range"; monsters use
    # its absolute value, which is what the client wants to display here.
    rng = lv_list(sk.get("Range"), maxlv)
    if rng and any(rng):
        rows.append("AttackRange = { " + ", ".join(str(abs(v)) for v in rng) + " }")

    body = ",\r\n".join("\t\t" + r for r in rows)
    return f"\t[SKID.{const}] = {{\r\n{body}\r\n\t}}"


def fix_mojibake(text: str, skdb: dict) -> "tuple[str, list[str]]":
    """Replace SkillName values mangled into U+FFFD with the English name."""
    fixed = []

    def repl(m: re.Match) -> str:
        const, name = m.group(1), m.group(3)
        if not MOJIBAKE_RE.search(name):
            return m.group(0)
        clean = (skdb.get(const, {}).get("Description") or pretty_name(const)).strip()
        fixed.append(const)
        return f'{m.group(2)}SkillName = "{clean}"'

    pattern = re.compile(
        r'\[SKID\.([A-Za-z_]\w*)\]\s*=\s*\{(.*?)SkillName = "([^"]*)"', re.S)
    return pattern.sub(repl, text), fixed


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--client", required=True,
                    help="dossier .../luafiles514/lua files/skillinfoz")
    ap.add_argument("--apply", action="store_true",
                    help="ecrire dans skillinfolist.lub (sinon rapport seul)")
    ap.add_argument("--fix-mojibake", action="store_true",
                    help="reparer les SkillName contenant des caracteres U+FFFD")
    ap.add_argument("--include-markers", action="store_true",
                    help="inclure aussi les constantes sentinelles (*_BEGIN, ...)")
    args = ap.parse_args()

    cdir = Path(args.client)
    f_id, f_list = cdir / "skillid.lub", cdir / "skillinfolist.lub"
    for f in (f_id, f_list):
        if not f.exists():
            return print(f"Introuvable : {f}") or 1

    ids = parse_skillid(read_latin1(f_id))
    text = read_latin1(f_list)
    listed = parse_listed(text)

    skdb = load_skill_db([
        REPO / "db/pre-re/skill_db.yml",
        REPO / "db/import/skill_db.yml",
    ])

    missing, markers = [], []
    for const, _ in ids:
        if const in listed:
            continue
        if not args.include_markers and (
                MARKER_RE.search(const) or const in MARKER_EXTRA):
            markers.append(const)
            continue
        missing.append(const)

    mangled = sorted({
        m.group(1) for m in re.finditer(
            r'\[SKID\.([A-Za-z_]\w*)\]\s*=\s*\{(?:(?!\[SKID\.).)*?'
            r'SkillName = "([^"]*)"', text, re.S)
        if MOJIBAKE_RE.search(m.group(2))})

    print(f"SKID declares          : {len(ids)}")
    print(f"deja dans la liste     : {len(listed)}")
    print(f"sentinelles ignorees   : {len(markers)}")
    print(f"a generer              : {len(missing)}")
    print(f"  - avec donnees serveur : {sum(1 for c in missing if c in skdb)}")
    print(f"  - sans donnees serveur : {sum(1 for c in missing if c not in skdb)}")
    print(f"SkillName corrompus    : {len(mangled)} {mangled}")

    if not args.apply:
        print("\n(rapport seul -- relancer avec --apply pour ecrire)")
        return 0

    if args.fix_mojibake and mangled:
        text, fixed = fix_mojibake(text, skdb)
        print(f"\nSkillName repares : {fixed}")

    blocks = [build_entry(c, skdb.get(c, {})) for c in missing]
    if not blocks and not (args.fix_mojibake and mangled):
        print("\nRien a faire.")
        return 0

    if blocks:
        close = text.rstrip()
        if not close.endswith("}"):
            return print("Format inattendu : le fichier ne finit pas par '}'") or 1
        head = close[:-1].rstrip()
        if not head.endswith(","):
            head += ","
        text = (head
                + "\r\n\t-- ==== entrees generees par tools/gen_skillinfolist.py ====\r\n"
                + ",\r\n".join(blocks) + "\r\n}\r\n")

    bak = f_list.with_suffix(".lub.bak")
    if not bak.exists():
        bak.write_bytes(f_list.read_bytes())
        print(f"Sauvegarde : {bak}")
    write_atomic(f_list, text, encoding="latin-1")
    print(f"{len(blocks)} entrees ajoutees dans {f_list}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
