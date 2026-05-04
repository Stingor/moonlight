"""
update_hateffect_desc.py

Adds a "Use @testcostume to preview the hat effect." line to the description
of costume items that have a hateffect, in itemInfo Lua files.

The line is added only once (idempotent). It is inserted before the closing
brace of the Desc table, just after the Position line.

Processes files in binary mode to preserve EUC-KR encoding.

Usage:
    python tools/update_hateffect_desc.py
"""

import re

YAML_PATH = "db/import/items/item_costumes.yml"

FILES = [
    "client/SystemEN/itemInfomoon.lua",
    "client/SystemEN/itemInfokro.lua",
]

HATEFFECT_RE = re.compile(r'hateffect[\s(]+?(HAT_EF_\w+)')

DESC_LINE = b'"Utilisez @testcostume pour decouvrir l\'effet graphique du costume"'


def get_hateffect_ids(yaml_text):
    blocks = re.split(r'\n  - Id:', yaml_text)
    ids = set()
    for block in blocks[1:]:
        id_match = re.match(r'\s*(\d+)', block)
        if not id_match:
            continue
        item_id = int(id_match.group(1))

        has_head = re.search(rb'Costume_Head:\s*true', block.encode('utf-8', 'replace'))
        if not has_head:
            if not re.search(r'Costume_Head:\s*true', block):
                continue

        script_match = re.search(r'\n\s+Script:\s+"([^"]*)"', block)
        if script_match:
            script = script_match.group(1)
        else:
            block_match = re.search(r'\n\s+Script:\s*\|\n((?:\s{6,}.*\n?)*)', block)
            script = block_match.group(1) if block_match else ""

        if HATEFFECT_RE.search(script):
            ids.add(item_id)

    return ids


def process_file(path, item_ids):
    with open(path, "rb") as f:
        content = f.read()

    # Match item blocks: [ID] = { ... costume = true ... }
    BLOCK_RE = re.compile(rb'\[(\d+)\]\s*=\s*\{.*?\n\t\}', re.DOTALL)
    already_done = 0
    added = 0
    skipped = 0

    def patch_block(match):
        nonlocal already_done, added, skipped
        block = match.group(0)
        item_id = int(match.group(1))

        if item_id not in item_ids:
            return block

        if b'costume = true' not in block:
            return block

        if DESC_LINE in block:
            already_done += 1
            return block

        # Find the closing of the Desc table and insert before it
        # Desc ends with a line like: \t\t\t"..."\n\t\t},
        # We insert our line after the last desc string entry
        desc_close = block.rfind(b'\n\t\t}')
        if desc_close == -1:
            skipped += 1
            return block

        # Find the last desc string line (ends with '" or '",')
        # We insert our line just before the desc closing brace
        insert_at = desc_close
        added += 1
        return block[:insert_at] + b',\n\t\t\t' + DESC_LINE + block[insert_at:]

    new_content = BLOCK_RE.sub(patch_block, content)

    with open(path, "wb") as f:
        f.write(new_content)

    print(f"{path}: +{added} added, {already_done} already present, {skipped} skipped")


def main():
    with open(YAML_PATH, "r", encoding="utf-8") as f:
        yaml_text = f.read()

    item_ids = get_hateffect_ids(yaml_text)
    print(f"Items with hateffect: {len(item_ids)}")

    for path in FILES:
        process_file(path, item_ids)


if __name__ == "__main__":
    main()
