"""
generate_testcostume.py

Parses db/import/items/item_costumes.yml and regenerates the @testcostume
NPC arrays in moon/atcommands.npc between AUTO-GENERATED-START/END markers.

Items with Costume_Head or Costume_Garment location are included.
Hateffect constants are extracted from the Script field automatically.

Usage:
    python tools/generate_testcostume.py
"""

import re

YAML_PATH = "db/import/items/item_costumes.yml"
CASH_PATH = "db/import/item_cash.yml"
NPC_PATH = "moon/atcommands.npc"

MARKER_START = "// AUTO-GENERATED-START"
MARKER_END = "// AUTO-GENERATED-END"

HATEFFECT_RE = re.compile(r'hateffect[\s(]+?(HAT_EF_\w+)')


def parse_cash_names(cash_text):
    # Use ^ anchor to avoid matching commented-out lines (# - Item: ...)
    return set(re.findall(r'(?m)^\s+- Item:\s+(\S+)', cash_text))


def parse_items(yaml_text, cash_names):
    # Split into individual item blocks by the "  - Id:" pattern
    blocks = re.split(r'\n  - Id:', yaml_text)
    items = []

    for block in blocks[1:]:  # skip file header
        id_match = re.match(r'\s*(\d+)', block)
        if not id_match:
            continue
        item_id = int(id_match.group(1))

        aegis_match = re.search(r'\n\s+AegisName:\s+(\S+)', block)
        if not aegis_match or aegis_match.group(1) not in cash_names:
            continue

        is_costume_head = bool(re.search(r'Costume_Head:\s*true', block))
        is_costume_garment = bool(re.search(r'Costume_Garment:\s*true', block))

        if not (is_costume_head or is_costume_garment):
            continue

        # Extract Script field (quoted single-line or block scalar)
        script_match = re.search(r'\n\s+Script:\s+"([^"]*)"', block)
        if script_match:
            script = script_match.group(1)
        else:
            # Block scalar: collect lines after "Script: |" until next key
            block_match = re.search(r'\n\s+Script:\s*\|\n((?:\s{6,}.*\n?)*)', block)
            script = block_match.group(1) if block_match else ""

        effect_match = HATEFFECT_RE.search(script)
        effect = effect_match.group(1) if effect_match else None

        if not effect:
            continue

        items.append({
            "id": item_id,
            "is_garment": is_costume_garment,
            "effect": effect,
        })

    return items


def chunk(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def fmt_setarray(name, values, per_line=10):
    lines = [f"\tsetarray {name}[0],"]
    total = len(values)
    offset = 0
    for group in chunk(values, per_line):
        offset += len(group)
        sep = ";" if offset >= total else ","
        lines.append("\t\t" + ",".join(group) + sep)
    return "\n".join(lines)


def generate_block(items):
    ids = [str(item["id"]) for item in items]
    effects = [item["effect"] if item["effect"] else "0" for item in items]

    n_effects = sum(1 for item in items if item["effect"])
    lines = [
        f"\t// {len(items)} costume items ({n_effects} with hateffect) - run tools/generate_testcostume.py to regenerate",
        fmt_setarray(".@items", ids),
        fmt_setarray(".@effects", effects),
    ]
    return "\n".join(lines)


def main():
    with open(YAML_PATH, "r", encoding="utf-8") as f:
        yaml_text = f.read()

    with open(CASH_PATH, "r", encoding="utf-8") as f:
        cash_names = parse_cash_names(f.read())

    items = parse_items(yaml_text, cash_names)
    n_effects = sum(1 for i in items if i["effect"])
    print(f"Parsed: {len(items)} items ({n_effects} with hateffect)")

    generated = generate_block(items)

    with open(NPC_PATH, "r", encoding="latin-1") as f:
        npc_text = f.read()

    pattern = re.compile(
        re.escape(MARKER_START) + r'.*?' + re.escape(MARKER_END),
        re.DOTALL,
    )

    replacement = f"{MARKER_START}\n{generated}\n\t{MARKER_END}"
    new_npc, count = pattern.subn(replacement, npc_text)

    if count == 0:
        print("ERROR: markers not found in NPC file — add // AUTO-GENERATED-START and // AUTO-GENERATED-END")
        return

    with open(NPC_PATH, "w", encoding="latin-1") as f:
        f.write(new_npc)

    print(f"Updated {NPC_PATH}")


if __name__ == "__main__":
    main()
