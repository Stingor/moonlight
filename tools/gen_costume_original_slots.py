# -*- coding: utf-8 -*-
"""
Generate tools/costume_original_slots.sql from the pre-conversion item_costumes.yml.

Context: commit ecd1dbcc4 ("Costumes : les headgears peuvent maintenant se
mettre dans n'importe quel slot costume") collapsed every Costume_Head_Top /
Costume_Head_Mid / Costume_Head_Low into the generic Costume_Head, losing the
original head-slot information needed to sort costumes on the website.

This script reads the file state of ecd1dbcc4~1 (just before the conversion)
and emits an idempotent SQL file that recreates a `costume_original_slots`
table keyed by item id, with one boolean column per original location.

Usage: python tools/gen_costume_original_slots.py
       The output is written to tools/costume_original_slots.sql.
"""
import os
import re
import subprocess
import sys

REPO = os.path.normpath(os.path.join(os.path.dirname(__file__), os.pardir))
PRE_COMMIT = 'ecd1dbcc4~1'
PRE_PATH = 'db/import/items/item_costumes.yml'
OUT_PATH = os.path.join(REPO, 'tools', 'costume_original_slots.sql')


def load_pre_conversion_yaml():
    """Read the YAML file as it existed just before the head-slot collapse."""
    result = subprocess.run(
        ['git', 'show', f'{PRE_COMMIT}:{PRE_PATH}'],
        capture_output=True, cwd=REPO, check=True,
    )
    return result.stdout.decode('utf-8', errors='replace')


def parse_items(content):
    """Yield (item_id, aegis_name, locations_set) for every block."""
    blocks = re.split(r'(?m)^  - Id:\s*', content)
    for b in blocks[1:]:
        m_id = re.match(r'(\d+)', b)
        if not m_id:
            continue
        item_id = int(m_id.group(1))

        m_aegis = re.search(r'\n\s*AegisName:\s*(\S+)', b)
        aegis = m_aegis.group(1) if m_aegis else ''

        m_loc = re.search(r'\n\s*Locations:\s*\n((?:\s+\S.*\n)+)', b)
        locs = set()
        if m_loc:
            for k in re.findall(r'(\w+):\s*true', m_loc.group(1)):
                locs.add(k)

        yield item_id, aegis, locs


def sql_escape(s):
    return s.replace("\\", "\\\\").replace("'", "''")


def generate_sql(items):
    """items: iterable of (item_id, aegis, locations_set)."""
    rows = []
    for item_id, aegis, locs in items:
        head_top = 1 if 'Costume_Head_Top' in locs else 0
        head_mid = 1 if 'Costume_Head_Mid' in locs else 0
        head_low = 1 if 'Costume_Head_Low' in locs else 0
        garment = 1 if 'Costume_Garment' in locs else 0
        if not (head_top or head_mid or head_low or garment):
            continue  # not a costume slot we track
        rows.append((item_id, aegis, head_top, head_mid, head_low, garment))

    rows.sort(key=lambda r: r[0])

    lines = [
        "-- Mapping of original head-slot for costumes, extracted from the",
        f"-- file state of git commit {PRE_COMMIT} ({PRE_PATH}),",
        "-- just before all Costume_Head_Top/Mid/Low were collapsed into",
        "-- the generic Costume_Head. Regenerate with",
        "-- `python tools/gen_costume_original_slots.py`.",
        "",
        "DROP TABLE IF EXISTS `costume_original_slots`;",
        "CREATE TABLE `costume_original_slots` (",
        "  `item_id`    INT UNSIGNED  NOT NULL PRIMARY KEY,",
        "  `aegis_name` VARCHAR(50)   NOT NULL DEFAULT '',",
        "  `head_top`   TINYINT(1)    NOT NULL DEFAULT 0,",
        "  `head_mid`   TINYINT(1)    NOT NULL DEFAULT 0,",
        "  `head_low`   TINYINT(1)    NOT NULL DEFAULT 0,",
        "  `garment`    TINYINT(1)    NOT NULL DEFAULT 0,",
        "  KEY `idx_head_top` (`head_top`),",
        "  KEY `idx_head_mid` (`head_mid`),",
        "  KEY `idx_head_low` (`head_low`),",
        "  KEY `idx_garment`  (`garment`)",
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;",
        "",
        "INSERT INTO `costume_original_slots`",
        "  (`item_id`, `aegis_name`, `head_top`, `head_mid`, `head_low`, `garment`) VALUES",
    ]
    value_lines = []
    for r in rows:
        item_id, aegis, ht, hm, hl, g = r
        value_lines.append(
            f"  ({item_id}, '{sql_escape(aegis)}', {ht}, {hm}, {hl}, {g})"
        )
    lines.append(',\n'.join(value_lines) + ';')
    lines.append('')
    return '\n'.join(lines), len(rows)


def main():
    yaml_content = load_pre_conversion_yaml()
    items = list(parse_items(yaml_content))
    sql, n = generate_sql(items)
    with open(OUT_PATH, 'w', encoding='utf-8', newline='\n') as f:
        f.write(sql)
    print(f'Wrote {OUT_PATH}')
    print(f'  {n} costume rows inserted')


if __name__ == '__main__':
    main()
