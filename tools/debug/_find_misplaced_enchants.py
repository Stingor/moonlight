# -*- coding: utf-8 -*-
"""
Genere des requetes SQL qui identifient les items dont un enchant est place
dans un slot de carte (consequence du bug reroll dans enchanter.npc).

Logique :
  - Weapon/armor avec slots >= 1 : card0 ne devrait JAMAIS contenir un enchant
  - slots >= 2 : card0 ET card1 ne devraient pas contenir d'enchant
  - slots >= 3 : card0, card1, card2 ne devraient pas contenir d'enchant
  - slots = 0 (arme sans slot, ex: bow non-slotted) : tous les card peuvent
    contenir un enchant, pas d'anomalie possible

Genere 3 fichiers SQL :
  tools/_sql_misplaced_inventory.sql
  tools/_sql_misplaced_cart.sql
  tools/_sql_misplaced_storage.sql
"""
import re
import glob
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# 1. Charger les IDs des enchants
with open(os.path.join(ROOT, 'db/import/items/item_db_enchant.yml'), 'r', encoding='utf-8', errors='replace') as f:
    enchant_yaml = f.read()
enchant_ids = sorted(set(int(x) for x in re.findall(r'^\s*-\s*Id:\s*(\d+)', enchant_yaml, re.MULTILINE)))
print(f'Enchants charges : {len(enchant_ids)} IDs ({enchant_ids[0]}-{enchant_ids[-1]})')

# 2. Parser les item DBs pour obtenir : nameid -> slots (pour weapons/armors uniquement)
# rAthena Type: Weapon ou Armor. On veut le champ Slots: N
item_slots = {}  # nameid -> slots count

def parse_item_db(path):
    """Yield (nameid, slots) tuples for weapons/armors."""
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()
    # Split by entries
    entries = re.split(r'\n  - Id:', content)
    for entry in entries[1:]:
        m = re.match(r'\s*(\d+)', entry)
        if not m:
            continue
        nameid = int(m.group(1))
        # Look for Type: Weapon or Armor in the first ~2000 chars
        head = entry[:3000]
        if not re.search(r'^\s*Type:\s*(Weapon|Armor)\b', head, re.MULTILINE):
            continue
        # Find Slots: N (default 0 if absent)
        sm = re.search(r'^\s*Slots:\s*(\d+)', head, re.MULTILINE)
        slots = int(sm.group(1)) if sm else 0
        yield nameid, slots

# Parse priority order: import overrides re/pre-re
db_files = [
    'db/pre-re/item_db_equip.yml',
    'db/pre-re/item_db_armor.yml',
    'db/pre-re/item_db_weapon.yml',
    'db/import/items/item_db_armor.yml',
    'db/import/items/item_db_weapon.yml',
    'db/import/items/item_db_equip.yml',
]
for rel in db_files:
    full = os.path.join(ROOT, rel)
    if not os.path.exists(full):
        continue
    count = 0
    for nameid, slots in parse_item_db(full):
        item_slots[nameid] = slots
        count += 1
    print(f'  {rel}: {count} weapons/armors')

print(f'Total weapons/armors mappes : {len(item_slots)}')

# 3. Regrouper les nameids par slot count (1, 2, 3 — slot=0 et slot=4 non concernes)
by_slots = {1: [], 2: [], 3: []}
for nameid, slots in item_slots.items():
    if slots in by_slots:
        by_slots[slots].append(nameid)

for s, lst in by_slots.items():
    print(f'  slots={s}: {len(lst)} items')

# 4. Construire les conditions SQL
enchant_in = ','.join(str(i) for i in enchant_ids)

def build_where(table_alias):
    """Conditions pour detecter un enchant dans un slot de carte."""
    conds = []
    if by_slots[1]:
        ids = ','.join(str(i) for i in by_slots[1])
        conds.append(f"({table_alias}.nameid IN ({ids}) AND {table_alias}.card0 IN ({enchant_in}))")
    if by_slots[2]:
        ids = ','.join(str(i) for i in by_slots[2])
        conds.append(
            f"({table_alias}.nameid IN ({ids}) AND "
            f"({table_alias}.card0 IN ({enchant_in}) OR {table_alias}.card1 IN ({enchant_in})))"
        )
    if by_slots[3]:
        ids = ','.join(str(i) for i in by_slots[3])
        conds.append(
            f"({table_alias}.nameid IN ({ids}) AND "
            f"({table_alias}.card0 IN ({enchant_in}) OR {table_alias}.card1 IN ({enchant_in}) "
            f"OR {table_alias}.card2 IN ({enchant_in})))"
        )
    return '\n    OR '.join(conds)

# 5. Generer SQL par table
def make_sql(table, char_join):
    return f"""-- Items avec enchant mal place (bug enchanter.npc reroll)
-- Table : {table}
SELECT
    i.id AS row_id, i.char_id, c.name AS char_name, c.account_id,
    i.nameid, i.refine, i.equip,
    i.card0, i.card1, i.card2, i.card3,
    i.amount, i.unique_id
FROM {table} i
{char_join}
WHERE
    {build_where('i')}
ORDER BY c.account_id, c.name, i.nameid;
"""

sqls = {
    'inventory': make_sql('inventory', 'JOIN `char` c ON c.char_id = i.char_id'),
    'cart_inventory': make_sql('cart_inventory', 'JOIN `char` c ON c.char_id = i.char_id'),
    'storage': make_sql('storage', 'JOIN `char` c ON c.account_id = i.account_id AND c.char_num = 0  -- premier perso du compte (storage est par account)'),
}

# 6. Ecrire les fichiers SQL
out_dir = os.path.join(ROOT, 'tools')
for name, sql in sqls.items():
    out = os.path.join(out_dir, f'_sql_misplaced_{name}.sql')
    with open(out, 'w', encoding='utf-8') as f:
        f.write(sql)
    print(f'Genere : {out} ({len(sql)} bytes)')

print('\nOK - lance les .sql dans phpMyAdmin/HeidiSQL pour identifier les items affectes.')
