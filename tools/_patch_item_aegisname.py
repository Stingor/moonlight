# -*- coding: latin-1 -*-
"""Replace numeric item IDs with AegisName constants in rAthena NPC scripts.

Reads the item database from:
  db/import/items/item_db_ammo.yml
  db/import/items/item_db_armor.yml
  db/import/items/item_db_card.yml
  db/import/items/item_db_cash.yml
  db/import/items/item_db_costumes.yml
  db/import/items/item_db_enchant.yml
  db/import/items/item_db_etc.yml
  db/import/items/item_db_healing.yml
  db/import/items/item_db_pet.yml
  db/import/items/item_db_usable.yml
  db/import/items/item_db_weapon.yml

These are the only item DB files loaded by this server (db/re/ is commented
out in db/item_db.yml).

then rewrites .npc files passed as arguments (or ALL moon/**/*.npc if none),
replacing item IDs **only in known safe contexts** (whitelist approach) to
avoid clobbering unrelated numbers.

Safe contexts replaced
----------------------
1. First argument of item-related script functions:
     getitem / getitem2 / getitem3 / getitem4
     delitem / delitem2
     countitem / countitem2
     checkweight / checkweight2
     getiteminfo / getitemname
     makeitem
     rentitem
   Pattern matches both forms:  funcname(id  and  funcname id

2. rAthena shop definition lines (single-line format):
     map,x,y,dir<TAB>shop<TAB>NPC Name<TAB>sprite,id:price,id:price,...
   Only the id in each  id:price  pair is replaced;  sprite_names (which
   never contain  id:price  sequences) are left untouched.

Files are written back in latin-1 (project rule) with a post-write check
for stray UTF-8 replacement bytes (0xEF 0xBF 0xBD).

Usage
-----
  python tools/_patch_item_aegisname.py              # patch all moon/ .npc
  python tools/_patch_item_aegisname.py path1 path2  # patch specific files
  python tools/_patch_item_aegisname.py --dry-run    # show counts, no write
"""

import os
import re
import glob
import sys

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), os.pardir))

_IMPORT_ITEMS = os.path.join(ROOT, 'db', 'import', 'items')
ITEM_DB_FILES = [
    os.path.join(_IMPORT_ITEMS, 'item_db_ammo.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_armor.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_card.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_cash.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_costumes.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_enchant.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_etc.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_healing.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_pet.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_usable.yml'),
    os.path.join(_IMPORT_ITEMS, 'item_db_weapon.yml'),
]

# ---------------------------------------------------------------------------
# Regex patterns
# ---------------------------------------------------------------------------

# 1. Item-related function calls whose FIRST argument is an item ID.
_FUNCS = (
    'getitem4', 'getitem3', 'getitem2', 'getitem',   # longest first avoids partial match
    'delitem2', 'delitem',
    'countitem2', 'countitem',
    'checkweight2', 'checkweight',
    'getiteminfo', 'getitemname',
    'makeitem',
    'rentitem',
)
# Group 1 = function name + separating whitespace/paren
# Group 2 = numeric item ID
FUNC_RE = re.compile(
    r'(\b(?:' + '|'.join(re.escape(f) for f in _FUNCS) + r')\s*\(?\s*)(\d{3,6})\b',
    re.IGNORECASE,
)

# 2. Shop item entries: id:price  (price may be -1 for default)
# Applied only inside shop definition lines (detected separately).
# Group 1 = preceding separator (comma or start of items field)
# Group 2 = numeric item ID
# Group 3 = colon + price
SHOP_ITEM_RE = re.compile(r'(,)(\d{3,6})(:-?\d+)')

# Detect a shop definition line: starts with map header + TAB + 'shop' + TAB
SHOP_LINE_RE = re.compile(r'^[^\t]+\tshop\t', re.IGNORECASE)

# ---------------------------------------------------------------------------
# Load item DB
# ---------------------------------------------------------------------------

def load_item_id_to_aegis():
    """Return {item_id: AegisName}.  db/import overrides db/re."""
    mapping: dict[int, str] = {}
    for path in ITEM_DB_FILES:
        if not os.path.exists(path):
            continue
        with open(path, encoding='utf-8', errors='replace') as f:
            content = f.read()
        # Split on YAML list entries "  - Id: <n>"
        blocks = re.split(r'(?m)^  - Id:\s*', content)
        for block in blocks[1:]:
            m_id = re.match(r'(\d+)', block)
            m_ae = re.search(r'\n\s*AegisName:\s*(\S+)', block)
            if m_id and m_ae:
                mapping[int(m_id.group(1))] = m_ae.group(1)
    return mapping

# ---------------------------------------------------------------------------
# Patch a single file
# ---------------------------------------------------------------------------

def patch_file(path: str, id_to_aegis: dict, *, dry_run: bool = False):
    """
    Replace item IDs in *path* with AegisName constants.

    Returns (replaced_count, missing_ids_set).
    When dry_run=True, nothing is written to disk.
    """
    with open(path, encoding='latin-1') as f:
        content = f.read()

    replaced_count = [0]
    missing: set[int] = set()

    # --- helper: build a replacement callable for FUNC_RE matches ---
    def func_repl(m: re.Match) -> str:
        item_id = int(m.group(2))
        aegis = id_to_aegis.get(item_id)
        if aegis is None:
            missing.add(item_id)
            return m.group(0)
        replaced_count[0] += 1
        return m.group(1) + aegis

    # --- helper: build a replacement callable for SHOP_ITEM_RE matches ---
    def shop_repl(m: re.Match) -> str:
        item_id = int(m.group(2))
        aegis = id_to_aegis.get(item_id)
        if aegis is None:
            missing.add(item_id)
            return m.group(0)
        replaced_count[0] += 1
        return m.group(1) + aegis + m.group(3)

    # Apply function-call replacements globally
    new_content = FUNC_RE.sub(func_repl, content)

    # Apply shop-line replacements line by line
    lines = new_content.splitlines(keepends=True)
    new_lines = []
    for line in lines:
        if SHOP_LINE_RE.match(line):
            line = SHOP_ITEM_RE.sub(shop_repl, line)
        new_lines.append(line)
    new_content = ''.join(new_lines)

    if new_content == content:
        return 0, missing   # nothing changed

    if not dry_run:
        with open(path, 'w', encoding='latin-1', newline='') as f:
            f.write(new_content)
        # Sanity check: no UTF-8 replacement bytes must have snuck in
        with open(path, 'rb') as f:
            raw = f.read()
        if b'\xef\xbf\xbd' in raw:
            raise RuntimeError(f'UTF-8 replacement bytes detected in {path}!')

    return replaced_count[0], missing

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    args = sys.argv[1:]
    dry_run = '--dry-run' in args
    if dry_run:
        args = [a for a in args if a != '--dry-run']

    if args:
        paths = args
    else:
        moon = os.path.join(ROOT, 'moon')
        paths = glob.glob(os.path.join(moon, '**', '*.npc'), recursive=True)
        paths += glob.glob(os.path.join(moon, '*.npc'))
        paths = sorted(set(paths))

    id_to_aegis = load_item_id_to_aegis()
    print(f'Loaded {len(id_to_aegis)} item_id -> AegisName mappings')
    if dry_run:
        print('(dry-run mode -- no files will be written)')

    all_missing: set[int] = set()
    total_replaced = 0
    patched_files = 0

    for path in paths:
        count, missing = patch_file(path, id_to_aegis, dry_run=dry_run)
        all_missing |= missing
        if count:
            rel = os.path.relpath(path, ROOT)
            print(f'  {rel}: {count} replacement(s)')
            patched_files += 1
            total_replaced += count

    if all_missing:
        print(
            f'\nWARNING: {len(all_missing)} item ID(s) absent from item_db '
            f'(left as-is): {sorted(all_missing)[:20]}'
            + ('...' if len(all_missing) > 20 else '')
        )

    print(
        f'\nDone: {total_replaced} replacement(s) across '
        f'{patched_files}/{len(paths)} file(s)'
        + (' [dry-run, nothing written]' if dry_run else '')
    )


if __name__ == '__main__':
    main()
