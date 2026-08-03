#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_skilldescript.py -- complete client-side skilldescript.lub

Finds every SKID constant declared in skillid.lub that has no entry in
skilldescript.lub, then generates an entry for it using the server-side
skill_db.yml (technical data: max level, type, target, element, range,
SP cost, cast time, cooldown...) plus a rule-based prose description.

Usage
-----
  # 1. report only (default, touches nothing)
  python tools/gen_skilldescript.py --client "E:/.../skillinfoz"

  # 2. export the list of skills to describe (to hand-write / feed an LLM)
  python tools/gen_skilldescript.py --client "..." --dump-todo todo.json

  # 3. write the entries into the client file (creates a .bak)
  python tools/gen_skilldescript.py --client "..." --apply

  # 3b. same, but with hand-written prose merged in
  python tools/gen_skilldescript.py --client "..." --apply --overrides todo.json

Notes
-----
* skilldescript.lub is pure ASCII: no accented characters can be written
  (the client codepage is cp949-compatible, which has no 'e-acute').
  The script refuses to write non-ASCII text.
* Sentinel constants (*_BEGIN, *_LAST, *_STARTMARK, SCRIPT_000, ...) are
  skipped by default: they are padding, not real skills. Use
  --include-markers to force them in.
* Serveur PRE-RENEWAL : seuls db/pre-re/skill_db.yml et db/import/skill_db.yml
  sont lus. db/re/ n'est jamais consulte.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import textwrap
from pathlib import Path

try:
    import yaml
except ImportError:  # pragma: no cover
    sys.exit("PyYAML requis : pip install pyyaml")

try:
    from yaml import CSafeLoader as YamlLoader
except ImportError:  # pragma: no cover
    from yaml import SafeLoader as YamlLoader


REPO = Path(__file__).resolve().parent.parent

# ---------------------------------------------------------------- constants

# Colour codes copied from the existing entries so generated blocks blend in.
C_GREY = "^777777"
C_OFF = "^000000"
FORM_ACTIVE = "^3F0099Active^000000"
FORM_PASSIVE = "^6666CCPassive^000000"

TYPE_LABEL = {
    "magic": "^FF0000Magical^000000",
    "melee": "^FF0000Physical Melee^000000",
    "ranged": "^FF0000Physical Ranged^000000",
    "support": "^339900Supportive^000000",
    "recovery": "^339900Recovery^000000",
    "debuff": "^FF0000Debuff^000000",
    "summon": "^339900Summon^000000",
    "trap": "^000099Trap^000000",
    "misc": "^339900Supportive^000000",
}

TARGET_LABEL = {
    "Attack": "^6666CC1 Character^000000",
    "Support": "^6666CC1 Character^000000",
    "Self": "^6666CCCaster^000000",
    "Ground": "^6666CCGround^000000",
    "Trap": "^6666CCGround^000000",
    "Passive": "^6666CCCaster^000000",
}

ELEMENT_LABEL = {
    "Neutral": "^777777Neutral", "Water": "^0000FFWater", "Earth": "^CC5500Earth",
    "Fire": "^FF0000Fire", "Wind": "^009900Wind", "Poison": "^990099Poison",
    "Holy": "^CCAA00Holy", "Dark": "^660066Shadow", "Ghost": "^6666CCGhost",
    "Undead": "^006666Undead", "Weapon": "^777777Weapon", "Endowed": "^777777Endowed",
}

# Constants that are markers / padding rather than real skills.
MARKER_RE = re.compile(
    r"(^|_)(BEGIN|LAST|START|END|STARTMARK|ENDMARK|START_MARK|END_MARK)$"
)
MARKER_EXTRA = {
    "LAST", "THIRDJOB_BEGIN", "THIRDJOB_END", "SCRIPT_000", "SCRIPT_999",
    "EFST_999", "SYS_FIRSTJOBLV", "SYS_SECONDJOBLV", "MUTATION_BASEJOB",
    "ETC_THIRDJOB_SKILL_START", "ETC_THIRDJOB_SKILL_END",
    "UPPER_EXTENDED_JOB_START", "UPPER_EXTENDED_JOB_END",
    "LEVEL_EXPANSION_START", "LEVEL_EXPANSION_END",
    "DORAM_TRIBE_START", "DORAM_TRIBE_END",
}

MAX_TEXT_WIDTH = 48  # visible characters per line, matching existing entries


# ------------------------------------------------------------------ helpers

def read_client(path: Path) -> str:
    data = path.read_bytes()
    try:
        return data.decode("ascii")
    except UnicodeDecodeError:
        return data.decode("cp949")


def write_atomic(path: Path, text: str, encoding: str = "ascii") -> None:
    """Encode first, then swap. Never truncate the target on an encode error."""
    try:
        blob = text.encode(encoding)
    except UnicodeEncodeError as exc:
        bad = text[exc.start:exc.end]
        raise SystemExit(
            f"Refus d'ecrire : caractere hors {encoding} {bad!r} a l'offset "
            f"{exc.start}. Le client ne sait pas afficher les accents ici."
        )
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_bytes(blob)
    os.replace(tmp, path)


def parse_skillid(text: str) -> "list[tuple[str, int]]":
    return [
        (m.group(1), int(m.group(2)))
        for m in re.finditer(r"^\s*([A-Za-z_]\w*)\s*=\s*(\d+)\s*,?\s*$", text, re.M)
    ]


def parse_described(text: str) -> "set[str]":
    return set(re.findall(r"\[SKID\.([A-Za-z_]\w*)\]", text))


def parse_blocks(text: str) -> "dict[str, tuple[int, int, list[str]]]":
    """Map each [SKID.NAME] = { ... } to (start, end, its quoted strings).

    Brace depth is tracked outside of string literals so that a '{' inside a
    description does not confuse the scan.
    """
    out: dict = {}
    for m in re.finditer(r"\[SKID\.([A-Za-z_]\w*)\]\s*=\s*\{", text):
        i, depth, instr = m.end() - 1, 0, False
        while i < len(text):
            c = text[i]
            if instr:
                if c == "\\":
                    i += 2
                    continue
                if c == '"':
                    instr = False
            else:
                if c == '"':
                    instr = True
                elif c == "{":
                    depth += 1
                elif c == "}":
                    depth -= 1
                    if depth == 0:
                        break
            i += 1
        body = text[m.end():i]
        strings = [s for s in re.findall(r'"((?:[^"\\]|\\.)*)"', body) if s.strip()]
        out[m.group(1)] = (m.start(), i + 1, strings)
    return out


def load_skill_db(paths: "list[Path]") -> dict:
    """Merge several rAthena skill_db.yml (later files win)."""
    merged: dict = {}
    for p in paths:
        if not p.exists():
            continue
        doc = yaml.load(p.read_text(encoding="utf-8"), Loader=YamlLoader) or {}
        for entry in doc.get("Body") or []:
            name = entry.get("Name")
            if name:
                merged.setdefault(name, {}).update(entry)
    return merged


def lv_value(field, level: int, key: str = "Level"):
    """skill_db fields are either a scalar or a list of {Level: n, <X>: v}."""
    if field is None or isinstance(field, (int, float, str, bool)):
        return field
    if isinstance(field, list):
        best = None
        for row in field:
            if not isinstance(row, dict):
                continue
            if row.get(key, 0) <= level:
                best = row
        row = best or (field[0] if field else None)
        if isinstance(row, dict):
            for k, v in row.items():
                if k != key:
                    return v
    return None


def pretty_name(const: str) -> str:
    """NPC_WIDEHEALTHFEAR -> 'Wide Health Fear' (best effort fallback)."""
    body = const.split("_", 1)[1] if "_" in const else const
    words = [w for w in body.split("_") if w]
    out = []
    for w in words:
        # split glued upper-case words on a small dictionary of prefixes
        for token in ("WIDE", "ALL", "SELF", "RANDOM", "CHANGE", "SUMMON"):
            if w.startswith(token) and len(w) > len(token):
                out.append(token.capitalize())
                w = w[len(token):]
                break
        out.append(w.capitalize())
    return " ".join(out)


def wrap(text: str, first_indent: int = 0) -> "list[str]":
    """Wrap on *visible* width; first_indent reserves room for a prefix."""
    return textwrap.wrap(
        text,
        width=MAX_TEXT_WIDTH,
        initial_indent=" " * first_indent,
        subsequent_indent="",
    ) or [""]


# ------------------------------------------------- rule-based prose fallback

STATUS_WORDS = {
    "STONE": "Stone Curse", "FREEZE": "Freeze", "STUN": "Stun", "SLEEP": "Sleep",
    "CURSE": "Curse", "SILENCE": "Silence", "CONFUSE": "Confusion",
    "BLIND": "Blind", "POISON": "Poison", "BLEEDING": "Bleeding",
    "DEEP_SLEEP": "Deep Sleep", "COLD": "Frozen (Crystalize)",
    "FROSTMISTY": "Frost Misty", "SIREN": "Deep Sleep Lullaby",
    "HEALTHFEAR": "Fear", "BODYBURNNING": "Burning",
}

ELEM_WORDS = {
    "WATER": "Water", "FIRE": "Fire", "WIND": "Wind", "GROUND": "Earth",
    "EARTH": "Earth", "HOLY": "Holy", "DARKNESS": "Shadow", "POISON": "Poison",
    "TELEKINESIS": "Ghost", "UNDEAD": "Undead",
}


def guess_prose(const: str, sk: dict) -> str:
    """Heuristic English description. Deliberately conservative."""
    body = const.split("_", 1)[1] if "_" in const else const
    label = (sk.get("Description") or pretty_name(const)).strip()

    if const.endswith("_ATK") or const.endswith("_S"):
        base = re.sub(r"_(ATK|S)$", "", const)
        return (f"Internal damage component of {pretty_name(base)}. "
                "Not castable on its own.")

    for key, word in STATUS_WORDS.items():
        if body.startswith("WIDE") and key in body:
            return (f"Inflicts the {word} status on every target "
                    "around the caster.")
    for key, word in STATUS_WORDS.items():
        if body.endswith("ATTACK") and body.startswith(key):
            return (f"Attack that has a chance to inflict "
                    f"the {word} status on the target.")

    m = re.match(r"^CHANGE([A-Z]+)$", body)
    if m and m.group(1) in ELEM_WORDS:
        return (f"Changes the caster's armor property to "
                f"{ELEM_WORDS[m.group(1)]}.")

    m = re.match(r"^([A-Z]+)ATTACK$", body)
    if m and m.group(1) in ELEM_WORDS:
        return (f"Attack inflicting {ELEM_WORDS[m.group(1)]} property "
                "damage on the target.")

    if "SUMMON" in body:
        return "Summons monsters to fight alongside the caster."
    if "HEAL" in body:
        return "Restores HP."
    if body.startswith("WIDE"):
        return ("Affects every target within the caster's area "
                "of influence.")

    # Generic: rebuild something from the server-side data.
    tt = sk.get("TargetType")
    if tt == "Passive":
        return f"{label}. Passive effect."
    dmg = sk.get("DamageFlags") or {}
    if tt == "Attack" and not dmg.get("NoDamage"):
        elem = lv_value(sk.get("Element"), 1) or "Neutral"
        kind = "magic" if sk.get("Type") == "Magic" else "physical"
        return f"Inflicts {elem} property {kind} damage on the target."
    if tt in ("Self", "Support"):
        return f"{label}. Support effect on the caster or an ally."
    if tt in ("Ground", "Trap"):
        return f"{label}. Placed on the ground at the selected cell."
    return f"{label}."


# --------------------------------------------------------- block generation

def level_stats(sk: dict, lv: int, dmg: dict) -> str:
    """One line of hard numbers for a given skill level, or '' if nothing known."""
    bits = []
    sp = lv_value((sk.get("Requires") or {}).get("SpCost"), lv)
    if sp:
        bits.append(f"SP {sp}")
    rng = lv_value(sk.get("Range"), lv)
    if isinstance(rng, int) and rng > 0:
        bits.append(f"Range {rng} cells")
    hc = lv_value(sk.get("HitCount"), lv)
    if isinstance(hc, int) and abs(hc) > 1:
        bits.append(f"{abs(hc)} hits")
    splash = lv_value(sk.get("SplashArea"), lv)
    if isinstance(splash, int) and splash > 0:
        side = splash * 2 + 1
        bits.append(f"AoE {side}x{side} cells")
    cast = lv_value(sk.get("CastTime"), lv)
    if isinstance(cast, int) and cast > 0:
        bits.append(f"{cast / 1000:.1f} sec Cast Time")
    cd = lv_value(sk.get("Cooldown"), lv)
    if isinstance(cd, int) and cd > 0:
        bits.append(f"{cd / 1000:.1f} sec Cooldown")
    # Duration1 is the buff length on support skills, but the ground unit
    # lifetime on offensive ones -- only the former is worth showing.
    if dmg.get("NoDamage"):
        dur = lv_value(sk.get("Duration1"), lv, key="Level")
        if isinstance(dur, int) and dur > 0:
            bits.append(f"Duration {dur / 60000:.0f} min" if dur >= 120000
                        else f"Duration {dur / 1000:.0f} sec")
    return " / ".join(bits)


def build_block(const: str, skid: int, sk: dict, prose: str | None,
                per_level: bool, label: str | None = None) -> str:
    sk = sk or {}
    # An existing stub already carries the official client name: keep it
    # rather than the server's, so both client tables stay consistent.
    label = label or (sk.get("Description") or pretty_name(const)).strip()
    maxlv = int(sk.get("MaxLevel") or 1)

    lines = [f'"{label} - ID:{skid}"', f'"Max Level: {maxlv}"']

    tt = sk.get("TargetType")
    lines.append(f'"Skill Form: {FORM_PASSIVE if tt == "Passive" else FORM_ACTIVE}"')

    dmg = sk.get("DamageFlags") or {}
    stype = sk.get("Type")
    if tt == "Passive":
        kind = None
    elif stype == "Magic":
        kind = "magic"
    elif stype == "Weapon" and not dmg.get("NoDamage"):
        rng = lv_value(sk.get("Range"), maxlv)
        # Range < 0 means "same as the caster's weapon range".
        kind = "ranged" if isinstance(rng, int) and rng > 3 else "melee"
    elif tt in ("Ground", "Trap"):
        kind = "trap"
    elif "SUMMON" in const:
        kind = "summon"
    elif tt == "Attack":
        kind = "debuff"
    else:
        kind = "support"
    if kind:
        lines.append(f'"Type: {TYPE_LABEL[kind]}"')

    if tt in TARGET_LABEL:
        lines.append(f'"Target: {TARGET_LABEL[tt]}"')

    # A per-level Element list means the property changes with the level:
    # showing a single one would be wrong, so it is left out.
    elem = sk.get("Element")
    if isinstance(elem, str) and elem not in ("Neutral", "Weapon"):
        lines.append(f'"Property: {ELEMENT_LABEL.get(elem, C_GREY + elem)}{C_OFF}"')

    if prose is None:
        prose = guess_prose(const, sk)
    prefix = "Description: "
    body = wrap(prose, first_indent=len(prefix))
    body[0] = prefix + C_GREY + body[0][len(prefix):]
    body[-1] = body[-1] + C_OFF
    lines.extend(f'"{ln}"' for ln in body)

    if per_level and sk:
        rows = [level_stats(sk, lv, dmg) for lv in range(1, maxlv + 1)]
        rows = [r for r in rows if r]
        if len(set(rows)) == 1:
            # Same values on every level: one summary line beats ten copies.
            lines.append(f'"{C_GREY}{rows[0]}{C_OFF}"')
        else:
            for lv, row in enumerate(rows, start=1):
                lines.append(f'"[Lv {lv}]: {C_GREY}{row}{C_OFF}"')

    inner = ",\r\n".join("\t\t" + ln for ln in lines)
    return f"\t[SKID.{const}] = {{\r\n{inner}\r\n\t}}"


# -------------------------------------------------------------------- main

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--client", required=True,
                    help="dossier .../luafiles514/lua files/skillinfoz")
    ap.add_argument("--apply", action="store_true",
                    help="ecrire dans skilldescript.lub (sinon rapport seul)")
    ap.add_argument("--dump-todo", metavar="FILE",
                    help="exporter en JSON les skills a decrire")
    ap.add_argument("--overrides", metavar="FILE", action="append", default=[],
                    help="JSON {SKID_NAME: 'texte'} de descriptions manuelles "
                         "(repetable, les fichiers suivants ecrasent)")
    ap.add_argument("--include-markers", action="store_true",
                    help="inclure aussi les constantes sentinelles (*_BEGIN, ...)")
    ap.add_argument("--no-levels", action="store_true",
                    help="ne pas generer les lignes [Lv n]")
    ap.add_argument("--fill-empty", action="store_true",
                    help="remplacer aussi les blocs vides [SKID.X] = {}")
    ap.add_argument("--fill-stubs", action="store_true",
                    help="remplacer aussi les blocs reduits au seul nom "
                         "(implique --fill-empty)")
    args = ap.parse_args()
    if args.fill_stubs:
        args.fill_empty = True

    cdir = Path(args.client)
    f_id, f_desc = cdir / "skillid.lub", cdir / "skilldescript.lub"
    for f in (f_id, f_desc):
        if not f.exists():
            return print(f"Introuvable : {f}") or 1

    id_text, desc_text = read_client(f_id), read_client(f_desc)
    ids = parse_skillid(id_text)
    blocks = parse_blocks(desc_text)

    # Serveur PRE-RENEWAL : db/pre-re + db/import uniquement. Jamais db/re.
    skdb = load_skill_db([
        REPO / "db/pre-re/skill_db.yml",
        REPO / "db/import/skill_db.yml",
    ])

    # A block counts as poor when it holds no text at all, or nothing beyond
    # its own name -- the client then shows the skill without any tooltip.
    def poor(const: str) -> bool:
        n = len(blocks[const][2])
        return n == 0 if not args.fill_stubs else n <= 1

    missing, refill, markers = [], [], []
    for const, skid in ids:
        if not args.include_markers and (
                MARKER_RE.search(const) or const in MARKER_EXTRA):
            markers.append((const, skid))
            continue
        if const not in blocks:
            missing.append((const, skid))
        elif args.fill_empty and poor(const):
            refill.append((const, skid))

    todo_all = missing + refill
    no_db = [c for c, _ in todo_all if c not in skdb]

    print(f"SKID declares          : {len(ids)}")
    print(f"blocs presents         : {len(blocks)}")
    print(f"sentinelles ignorees   : {len(markers)}")
    print(f"entrees absentes       : {len(missing)}")
    print(f"blocs a re-remplir     : {len(refill)}")
    print(f"total a generer        : {len(todo_all)}")
    print(f"  - avec donnees serveur : {len(todo_all) - len(no_db)}")
    print(f"  - sans donnees serveur : {len(no_db)}")
    if no_db:
        print("    " + ", ".join(no_db[:15]) + (" ..." if len(no_db) > 15 else ""))

    overrides = {}
    for path in args.overrides:
        raw = json.loads(Path(path).read_text(encoding="utf-8"))
        for k, v in raw.items():
            overrides[k] = v["description"] if isinstance(v, dict) else v

    if args.dump_todo:
        todo = {
            const: {
                "id": skid,
                "label": (skdb.get(const, {}).get("Description") or pretty_name(const)),
                "maxlevel": skdb.get(const, {}).get("MaxLevel"),
                "type": skdb.get(const, {}).get("Type"),
                "targettype": skdb.get(const, {}).get("TargetType"),
                "element": skdb.get(const, {}).get("Element"),
                "description": overrides.get(const, ""),
            }
            for const, skid in todo_all
        }
        Path(args.dump_todo).write_text(
            json.dumps(todo, indent=2, ensure_ascii=False), encoding="utf-8")
        print(f"\nTODO ecrit : {args.dump_todo} ({len(todo)} entrees)")

    if not args.apply:
        print("\n(rapport seul -- relancer avec --apply pour ecrire)")
        return 0

    if not todo_all:
        print("\nRien a faire.")
        return 0

    def block_for(const: str, skid: int, label: str | None = None) -> str:
        return build_block(const, skid, skdb.get(const, {}),
                           overrides.get(const) or None, not args.no_levels,
                           label=label)

    new_text = desc_text

    # Rewrite poor blocks in place, back to front so earlier offsets hold.
    for const, skid in sorted(refill, key=lambda t: blocks[t[0]][0], reverse=True):
        start, end, strings = blocks[const]
        # Reuse the official client name when the stub carried one.
        label = strings[0].split(" - ID:")[0].strip() if strings else None
        new_text = new_text[:start] + block_for(const, skid, label).lstrip("\t") \
            + new_text[end:]

    if missing:
        close = new_text.rstrip()
        if not close.endswith("}"):
            return print("Format inattendu : le fichier ne finit pas par '}'") or 1
        head = close[:-1].rstrip()
        if not head.endswith(","):
            head += ","
        new_text = (head
                    + "\r\n\t-- ==== entrees generees par "
                      "tools/gen_skilldescript.py ====\r\n"
                    + ",\r\n".join(block_for(c, i) for c, i in missing)
                    + "\r\n}\r\n")

    bak = f_desc.with_suffix(".lub.bak")
    if not bak.exists():
        bak.write_bytes(f_desc.read_bytes())
        print(f"\nSauvegarde : {bak}")
    write_atomic(f_desc, new_text)
    print(f"{len(missing)} entrees ajoutees, {len(refill)} blocs reecrits "
          f"dans {f_desc}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
