# -*- coding: latin-1 -*-
"""Migrate the $mvps[] global array in moon/mvps.npc from numeric mob_ids
to AegisName constants now that mob AegisNames are registered as script
constants (see src/map/mob.cpp MobDatabase::loadingFinished).

Before:
    setarray $mvps[0],  1511,2,    //Amon Ra
                        1272,2,    //Dark Lord
                        ...
After:
    setarray $mvps[0],  AMON_RA,2,
                        DARK_LORD,2,
                        ...

The mapping mob_id -> AegisName is read from db/re/mob_db.yml plus
db/import/mob_db.yml (import overrides base). NPC file is written back
in latin-1 (project rule, see feedback_latin1_npc_files.md).
"""
import os
import re

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), os.pardir))
MVPS_NPC   = os.path.join(ROOT, 'moon', 'mvps.npc')
MOB_DB_RE  = os.path.join(ROOT, 'db', 're', 'mob_db.yml')
MOB_DB_IMP = os.path.join(ROOT, 'db', 'import', 'mob_db.yml')


def load_mob_id_to_aegis():
    """Returns {mob_id: AegisName}. Import overrides base."""
    mapping = {}
    for path in (MOB_DB_RE, MOB_DB_IMP):
        if not os.path.exists(path):
            continue
        with open(path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
        for m in re.finditer(
            r'(?m)^\s*-\s*Id:\s*(\d+)\s*\n(?:.*\n)*?\s*AegisName:\s*(\S+)',
            content,
        ):
            # Only capture up to the next " - Id:" boundary to avoid leaking
            # AegisName from the next item; the lazy regex above can drift.
            pass
        # safer parser: split into blocks
        blocks = re.split(r'(?m)^  - Id:\s*', content)
        for b in blocks[1:]:
            m_id = re.match(r'(\d+)', b)
            if not m_id:
                continue
            mob_id = int(m_id.group(1))
            m_ae = re.search(r'\n\s*AegisName:\s*(\S+)', b)
            if m_ae:
                mapping[mob_id] = m_ae.group(1)
    return mapping


def patch_mvps_npc(id_to_aegis):
    with open(MVPS_NPC, 'r', encoding='latin-1') as f:
        content = f.read()

    # Find the setarray $mvps[0], ... ; block (multi-line, ends at ;)
    m = re.search(r'setarray\s+\$mvps\[0\]\s*,\s*((?:[^;])+);', content)
    if not m:
        raise SystemExit('setarray $mvps[0] block not found')

    payload = m.group(1)
    # Each entry is "<id>,<pts>" possibly followed by whitespace and a
    # "//comment" then a newline. We rewrite each numeric id to its AegisName.
    # We match a numeric id at the start of a comma-pair: digit run preceded by
    # whitespace/comma, followed by ",<digits>" (the points).

    replaced_count = [0]
    missing = []

    def repl(match):
        prefix = match.group(1)
        mob_id = int(match.group(2))
        rest = match.group(3)
        aegis = id_to_aegis.get(mob_id)
        if aegis is None:
            missing.append(mob_id)
            return match.group(0)
        replaced_count[0] += 1
        return f'{prefix}{aegis}{rest}'

    # Pattern: leading separator (start of array body or comma + optional
    # whitespace/newline/tab) + numeric mob_id + comma+points.
    # We deliberately require the points portion to be present so we don't
    # accidentally rewrite a stray number (e.g. inside a comment).
    pattern = re.compile(
        r'(^|[,\s])(\d{3,6})(\s*,\s*\d+)',
        re.MULTILINE,
    )
    new_payload = pattern.sub(repl, payload)

    if missing:
        print(f'WARNING: {len(missing)} mob_ids absent from mob_db ({missing[:10]}...) -- left as-is')

    new_content = content[:m.start(1)] + new_payload + content[m.end(1):]
    with open(MVPS_NPC, 'w', encoding='latin-1', newline='') as f:
        f.write(new_content)

    with open(MVPS_NPC, 'rb') as f:
        data = f.read()
    assert b'\xef\xbf\xbd' not in data, 'UTF-8 replacement bytes detected!'
    print(f'mvps.npc: {len(data)} bytes, {replaced_count[0]} mob_ids -> AegisName')


if __name__ == '__main__':
    id_to_aegis = load_mob_id_to_aegis()
    print(f'Loaded {len(id_to_aegis)} mob_id -> AegisName mappings')
    patch_mvps_npc(id_to_aegis)
