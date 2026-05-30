"""Patch MissionMVP() dans func.npc : propagation aux membres de party en instance."""
import sys

PATH = "moon/func.npc"

with open(PATH, "rb") as f:
    raw = f.read()

old = (
    b'function\tscript\tMissionMVP\t{\n'
    b'\tif( playerattached() ) {\n'
    b'\t\tif( Mission_MVP != #Mission_Count_MVP && getarg(1, -1) == Mission_MVP_ID ) {\n'
    b'\t\t\tMission_MVP = 1;\n'
    b'\t\t\tdispbottom "[Chasse MVP] Vous avez tu\xe9 le MVP pour votre mission retournez voir Chuck !";\n'
    b'\t\t}\n'
    b'\t\tif( getarg(2) // Compte pour le classement ?\n'
    b'\t\t\tquery_sql("UPDATE `char` SET mvp_count = mvp_count + 1 WHERE `char_id` = \'' + b'" + getarg(0,getcharid(0,strcharinfo(0))) + "\'"'
    b');\n'
    b'\t\tif( getarg(3) ) // Message de log ?\n'
    b'\t\t\tlogmes getarg(3);\n'
    b'\t}\n'
    b'\treturn;\n'
    b'}'
)

new = (
    b'function\tscript\tMissionMVP\t{\n'
    b'\tif( !playerattached() ) return;\n'
    b'\n'
    b'\t.@killed_mob = getarg(1, -1);\n'
    b'\n'
    b'\t// --- Killer ---\n'
    b'\tif( Mission_MVP != #Mission_Count_MVP && .@killed_mob == Mission_MVP_ID ) {\n'
    b'\t\tMission_MVP = 1;\n'
    b'\t\tdispbottom "[Chasse MVP] Vous avez tu\xe9 le MVP de votre mission, retournez voir Chuck !";\n'
    b'\t}\n'
    b'\tif( getarg(2) ) // Compte pour le classement ?\n'
    b'\t\tquery_sql("UPDATE `char` SET mvp_count = mvp_count + 1 WHERE `char_id` = \'' + b'" + getarg(0, getcharid(0)) + "\'"'
    b');\n'
    b'\tif( getarg(3) ) // Message de log ?\n'
    b'\t\tlogmes getarg(3);\n'
    b'\n'
    b'\t// --- Membres de party dans la m\xeame instance ---\n'
    b'\tif( .@killed_mob < 1 ) return;\n'
    b'\t.@killer_cid  = getcharid(0);\n'
    b'\t.@killer_map$ = strcharinfo(3);\n'
    b'\t.@party_id    = getpartyid();\n'
    b'\tif( .@party_id <= 0 ) return;\n'
    b'\n'
    b'\tgetpartymember(.@party_id, 2); // remplit $@partymembercid[] et $@partymembercount\n'
    b'\tfor( .@i = 0; .@i < $@partymembercount; .@i++ ) {\n'
    b'\t\tif( $@partymembercid[.@i] == .@killer_cid ) continue; // d\xe9j\xe0 trait\xe9\n'
    b'\t\tattachrid($@partymembercid[.@i]);\n'
    b'\t\tif( !playerattached() ) continue;                     // hors-ligne\n'
    b'\t\tif( strcharinfo(3) != .@killer_map$ ) continue;       // pas dans l\'instance\n'
    b'\t\tif( Mission_MVP == #Mission_Count_MVP ) continue;     // pas de mission ou d\xe9j\xe0 valid\xe9e\n'
    b'\t\tif( Mission_MVP_ID != .@killed_mob ) continue;        // mission diff\xe9rente\n'
    b'\t\tMission_MVP = 1;\n'
    b'\t\tdispbottom "[Chasse MVP] Votre party a tu\xe9 le MVP de votre mission, retournez voir Chuck !";\n'
    b'\t}\n'
    b'\treturn;\n'
    b'}'
)

if old not in raw:
    print("ERREUR: pattern MissionMVP non trouvé")
    sys.exit(1)

result = raw.replace(old, new, 1)

if b'\xef\xbf\xbd' in result:
    print("ERREUR: bytes UTF-8 détectés !")
    sys.exit(1)

with open(PATH, "wb") as f:
    f.write(result)

print("OK - MissionMVP mise à jour avec propagation party")
