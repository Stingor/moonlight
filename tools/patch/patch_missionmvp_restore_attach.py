"""Patch MissionMVP() : restaure le joueur attaché original avant le return."""
import sys

PATH = "moon/func.npc"

with open(PATH, "rb") as f:
    raw = f.read()

# Ajouter .@original_cid juste après le premier playerattached()
old_start = (
    b'function\tscript\tMissionMVP\t{\n'
    b'\tif( !playerattached() ) return;\n'
    b'\n'
    b'\t.@killed_mob = getarg(1, -1);\n'
)
new_start = (
    b'function\tscript\tMissionMVP\t{\n'
    b'\tif( !playerattached() ) return;\n'
    b'\t.@original_cid = getcharid(0); // sauvegarde pour restauration en fin de fonction\n'
    b'\n'
    b'\t.@killed_mob = getarg(1, -1);\n'
)

# Remplacer les deux return; orphelins par attachrid+return, et le return final
old_early_return = (
    b'\tif( .@party_id <= 0 ) return;\n'
)
new_early_return = (
    b'\tif( .@party_id <= 0 ) { attachrid(.@original_cid); return; }\n'
)

old_end = (
    b'\t\tMission_MVP = 1;\n'
    b'\t\tdispbottom "[Chasse MVP] Votre party a tu\xe9 le MVP de votre mission, retournez voir Chuck !";\n'
    b'\t}\n'
    b'\treturn;\n'
    b'}'
)
new_end = (
    b'\t\tMission_MVP = 1;\n'
    b'\t\tdispbottom "[Chasse MVP] Votre party a tu\xe9 le MVP de votre mission, retournez voir Chuck !";\n'
    b'\t}\n'
    b'\tattachrid(.@original_cid); // restaure le killer comme joueur attach\xe9\n'
    b'\treturn;\n'
    b'}'
)

errors = []
if old_start        not in raw: errors.append("start")
if old_early_return not in raw: errors.append("early_return")
if old_end          not in raw: errors.append("end")

if errors:
    print("ERREUR patterns non trouvés:", errors)
    sys.exit(1)

raw = raw.replace(old_start,        new_start,        1)
raw = raw.replace(old_early_return, new_early_return, 1)
raw = raw.replace(old_end,          new_end,          1)

if b'\xef\xbf\xbd' in raw:
    print("ERREUR: bytes UTF-8 détectés !")
    sys.exit(1)

with open(PATH, "wb") as f:
    f.write(raw)

print("OK - attachrid(.@original_cid) restauré avant chaque return")
