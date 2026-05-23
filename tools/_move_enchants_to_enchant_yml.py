# -*- coding: utf-8 -*-
"""
One-shot move of items tagged `SubType: Enchant` out of item_db_card.yml
and into item_db_enchant.yml.

Block boundary heuristic: each item starts with a top-level "  - Id:" line
(two-space indent) and ends right before the next "  - Id:" line or EOF.
A block is treated as an enchant when it contains a line matching
`^    SubType:\\s*Enchant` (four-space indent = direct child of the item).
"""
import re
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), os.pardir))
CARD    = os.path.join(ROOT, 'db', 'import', 'items', 'item_db_card.yml')
ENCHANT = os.path.join(ROOT, 'db', 'import', 'items', 'item_db_enchant.yml')

with open(CARD, 'r', encoding='utf-8', newline='') as f:
    card_content = f.read()
with open(ENCHANT, 'r', encoding='utf-8', newline='') as f:
    enchant_content = f.read()

# Find all item-block starts in the card file
starts = [m.start() for m in re.finditer(r'(?m)^  - Id:\s', card_content)]
if not starts:
    raise SystemExit('No item blocks found in card file.')

# Add EOF as final boundary so each block has (start, end)
boundaries = list(zip(starts, starts[1:] + [len(card_content)]))

kept_blocks = []   # stays in card_content
moved_blocks = []  # goes to enchant_content
ENCHANT_LINE = re.compile(r'(?m)^    SubType:\s*Enchant\s*$')

for s, e in boundaries:
    block = card_content[s:e]
    if ENCHANT_LINE.search(block):
        moved_blocks.append(block)
    else:
        kept_blocks.append(block)

print(f'Total items in card file:   {len(boundaries)}')
print(f'  -> kept as cards:         {len(kept_blocks)}')
print(f'  -> moved to enchant file: {len(moved_blocks)}')

# Rebuild card file: everything before the first item + kept blocks
new_card = card_content[:starts[0]] + ''.join(kept_blocks)

# Append moved blocks to enchant file.
# Make sure the enchant file ends with a newline before we append.
if not enchant_content.endswith('\n'):
    enchant_content += '\n'
new_enchant = enchant_content + ''.join(moved_blocks)

# Write back
with open(CARD, 'w', encoding='utf-8', newline='') as f:
    f.write(new_card)
with open(ENCHANT, 'w', encoding='utf-8', newline='') as f:
    f.write(new_enchant)

# Sanity re-parse
def count_with_subtype_enchant(path):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    blocks = re.findall(r'(?m)^  - Id:\s', content)
    enchants = ENCHANT_LINE.findall(content)
    return len(blocks), len(enchants)

c_total, c_enc = count_with_subtype_enchant(CARD)
e_total, e_enc = count_with_subtype_enchant(ENCHANT)
print()
print(f'After move:')
print(f'  card    : {c_total} items, {c_enc} with SubType: Enchant')
print(f'  enchant : {e_total} items, {e_enc} with SubType: Enchant')
