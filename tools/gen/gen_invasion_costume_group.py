"""
Génère le groupe INVASION_COSTUME dans db/import/item_group_db.yml
en croisant item_db_costumes.yml x item_cash.yml,
puis patche invasion.npc pour utiliser groupranditem(IG_INVASION_COSTUME).
"""
import re, sys

# ── 1. Collecter les AegisNames des costumes en cashshop ────────────────────
with open("db/import/items/item_db_costumes.yml", encoding="utf-8") as f:
    costume_aegis = set(re.findall(r"^\s+AegisName:\s+(\S+)", f.read(), re.MULTILINE))

with open("db/import/item_cash.yml", encoding="utf-8") as f:
    cash_items = set(re.findall(r"^\s+- Item:\s+(\S+)", f.read(), re.MULTILINE))

pool = sorted(costume_aegis & cash_items)
print(f"Pool INVASION_COSTUME : {len(pool)} costumes")

# ── 2. Vérifier que le groupe n'existe pas déjà ──────────────────────────────
GROUP_DB = "db/import/item_group_db.yml"
with open(GROUP_DB, encoding="utf-8") as f:
    gdb = f.read()

if "Group: INVASION_COSTUME" in gdb:
    print("Groupe INVASION_COSTUME déjà présent dans item_group_db.yml — skip.")
else:
    # Construire le bloc YAML
    lines = ["  - Group: INVASION_COSTUME\n"]
    lines.append("    SubGroups:\n")
    lines.append("      - SubGroup: 1\n")
    lines.append("        Algorithm: All\n")
    lines.append("        List:\n")
    for i, aegis in enumerate(pool):
        lines.append(f"          - Index: {i}\n")
        lines.append(f"            Item: {aegis}\n")

    block = "".join(lines)
    with open(GROUP_DB, "a", encoding="utf-8") as f:
        f.write("\n" + block)
    print(f"Groupe ajouté à {GROUP_DB} ({len(pool)} entrées)")

# ── 3. Patcher invasion.npc : remplacer le pool codé en dur ─────────────────
NPC = "moon/event/invasion.npc"
with open(NPC, encoding="latin-1") as f:
    npc = f.read()

# Pattern à remplacer dans end_invasion — le pool + la ligne rand
old_pool = (
    '\t// Pool de costumes (commun aux 3 places)\n'
    '\tsetarray .@reward[0], 20219, 20297, 31251, 20026, 19857, 19959, 20103,\n'
    '\t                       31106, 31107, 19817, 20075, 20433, 19521, 19514,\n'
    '\t                       20259, 31518, 20183;\n'
    '\n'
    '\t// === 1\xe8re place : mail direct (fonctionne en ligne et hors-ligne) ===\n'
    '\tif( $@CharId1 > 0 ) {\n'
    '\t\t.@item1 = .@reward[rand(getarraysize(.@reward))];\n'
)
new_pool = (
    '\t// === 1\xe8re place : mail direct (fonctionne en ligne et hors-ligne) ===\n'
    '\tif( $@CharId1 > 0 ) {\n'
    '\t\t.@item1 = groupranditem(IG_INVASION_COSTUME);\n'
)

# Idem pour place 2 et 3
old_p2 = (
    '\t// === 2\xe8me place ===\n'
    '\tif( $@CharId2 > 0 ) {\n'
    '\t\t.@item2 = .@reward[rand(getarraysize(.@reward))];\n'
)
new_p2 = (
    '\t// === 2\xe8me place ===\n'
    '\tif( $@CharId2 > 0 ) {\n'
    '\t\t.@item2 = groupranditem(IG_INVASION_COSTUME);\n'
)

errors = []
if old_pool not in npc: errors.append("pool_p1")
if old_p2   not in npc: errors.append("pool_p2")

if errors:
    print("ERREUR patterns non trouvés:", errors)
    sys.exit(1)

npc = npc.replace(old_pool, new_pool, 1)
npc = npc.replace(old_p2,   new_p2,   1)

raw = npc.encode("latin-1")
if b'\xef\xbf\xbd' in raw:
    print("ERREUR: bytes UTF-8 détectés !")
    sys.exit(1)

with open(NPC, "w", encoding="latin-1") as f:
    f.write(npc)

print("invasion.npc patché — groupranditem(IG_INVASION_COSTUME) utilisé pour place 1 et 2")
