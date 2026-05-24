# -*- coding: latin-1 -*-
"""Migrate huntmission.npc + mvps.npc from SQL mob_db / $mvps[] indexed
selection to summon-group picks (MOBG_BRANCH_OF_DEAD_TREE for normal hunts,
MOBG_BLOODY_DEAD_BRANCH + MD_MVP filter for MVPs).

Files must be written back in latin-1 (per project rule, NPC scripts are
Windows-1252, not UTF-8).
"""
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), os.pardir))
HM   = os.path.join(ROOT, 'moon', 'quests', 'huntmission.npc')
MVP  = os.path.join(ROOT, 'moon', 'mvps.npc')


def patch(path, replacements):
    with open(path, 'r', encoding='latin-1') as f:
        content = f.read()
    for old, new in replacements:
        assert old in content, f'pattern not found in {path}:\n{old[:160]!r}'
        content = content.replace(old, new, 1)
    with open(path, 'w', encoding='latin-1', newline='') as f:
        f.write(content)
    # sanity check
    with open(path, 'rb') as f:
        data = f.read()
    assert b'\xef\xbf\xbd' not in data, f'UTF-8 replacement bytes in {path}!'


# ---------------------------------------------------------------------------
# huntmission.npc
# ---------------------------------------------------------------------------

OLD_QUERY = (
    'query_sql("SELECT ID FROM `mob_db` WHERE left(Sprite, 4) != \'meta\' AND '
    'left(Sprite, 2) != \'E_\' AND ~Mode & 32 AND EXP > 0 AND MVP1id = 0 AND '
    'DropCardid > 4000 AND DropCardid < 5000 AND DropCardper = 1 AND ID < 2000 AND '
)

# Patch 1: first SQL query (case 1, initial pick)
hm_patches = [
    (
        '\t\t\tmes "Vous devez chasser:";\n'
        '\t\t\t' + OLD_QUERY + 'instr(\'"+.Blacklist$+"\',ID) = 0 ORDER BY rand() LIMIT " + .Quests, .@mob);\n'
        '\t\t\tfor( .@i = 0; .@i < .Quests; .@i++ ) {\n'
        '\t\t\t\tsetd "Mission" + .@i, .@mob[.@i];\n'
        '\t\t\t\tsetd "Mission" + .@i +"_", 0;\n'
        '\t\t\t}\n'
        '\t\t\t#Mission_Count = 50;\n',
        '\t\t\tmes "Vous devez chasser:";\n'
        '\t\t\tcallsub L_PickMissionMobs;\n'
        '\t\t\tfor( .@i = 0; .@i < .Quests; .@i++ ) {\n'
        '\t\t\t\tsetd "Mission" + .@i, @hm_mob[.@i];\n'
        '\t\t\t\tsetd "Mission" + .@i +"_", 0;\n'
        '\t\t\t}\n'
        '\t\t\t#Mission_Count = 50;\n',
    ),
    # Patch 2: second SQL query (case 3 / reset path)
    (
        '\t\t\t\t\t' + OLD_QUERY + 'instr(\'"+ .Blacklist$ +"\',ID) = 0 ORDER BY rand() LIMIT " + .Quests, .@mob);\n'
        '\t\t\t\t\tfor( .@i = 0; .@i < .Quests; .@i++ ) {\n'
        '\t\t\t\t\t\tsetd "Mission" + .@i, .@mob[.@i];\n'
        '\t\t\t\t\t\tsetd "Mission" + .@i +"_", 0;\n'
        '\t\t\t\t\t}\n'
        '\t\t\t\t\t#Mission_Count = 50;\n',
        '\t\t\t\t\tcallsub L_PickMissionMobs;\n'
        '\t\t\t\t\tfor( .@i = 0; .@i < .Quests; .@i++ ) {\n'
        '\t\t\t\t\t\tsetd "Mission" + .@i, @hm_mob[.@i];\n'
        '\t\t\t\t\t\tsetd "Mission" + .@i +"_", 0;\n'
        '\t\t\t\t\t}\n'
        '\t\t\t\t\t#Mission_Count = 50;\n',
    ),
    # Patch 3: MVP initial pick (case 4)
    (
        '\t\t\tMission_MVP = 0;\n'
        '\t\t\t.@i = rand(2, getarraysize($mvps) + 1);\n'
        '\t\t\tif( .@i%2 == 0 )\n'
        '\t\t\t\tMission_MVP_ID = .@i;\n'
        '\t\t\telse\n'
        '\t\t\t\tMission_MVP_ID = .@i - 1;\n'
        '\t\t\t#Mission_Count_MVP = 1;\n',
        '\t\t\tMission_MVP = 0;\n'
        '\t\t\tcallsub L_PickMvpMob;\n'
        '\t\t\tMission_MVP_ID = @hm_mvp;\n'
        '\t\t\t#Mission_Count_MVP = 1;\n',
    ),
    # Patch 4: MVP reset pick (case 6 -> reset)
    (
        '\t\t\t\t\t#Mission_MVP_Reset++;\n'
        '\t\t\t\t\tMission_MVP = 0;\n'
        '\t\t\t\t\tMission_MVP_ID = rand(2, getarraysize($mvps) + 1);\n'
        '\t\t\t\t\tif( Mission_MVP_ID%2 != 0 )\n'
        '\t\t\t\t\t\tMission_MVP_ID--;\n'
        '\t\t\t\t\t#Mission_Count_MVP = 1;\n',
        '\t\t\t\t\t#Mission_MVP_Reset++;\n'
        '\t\t\t\t\tMission_MVP = 0;\n'
        '\t\t\t\t\tcallsub L_PickMvpMob;\n'
        '\t\t\t\t\tMission_MVP_ID = @hm_mvp;\n'
        '\t\t\t\t\t#Mission_Count_MVP = 1;\n',
    ),
    # Patch 5: $mvps[Mission_MVP_ID-2] in menu label (L_main)
    (
        '\t\t.@mvp_option$ = ": ~ Status mission MVP: ~ Abandonner "+'
        '(Mission_MVP_ID>0?"- "+getmonsterinfo($mvps[(Mission_MVP_ID-2 < 0?0:Mission_MVP_ID-2)], MOB_NAME):"");',
        '\t\t.@mvp_option$ = ": ~ Status mission MVP: ~ Abandonner "+'
        '(Mission_MVP_ID>0?"- "+getmonsterinfo(Mission_MVP_ID, MOB_NAME):"");',
    ),
]

# Patch 6: full rewrite of Mission_Status_MVP block to use Mission_MVP_ID as
# mob_id directly and look up points via L_MvpPoints.
old_status_mvp = (
    'Mission_Status_MVP:\n'
    '\tif( !Mission_MVP_ID ) {\n'
    '\t\tmes "Pas de mission en cours.";\n'
    '\t\tclose;\n'
    '\t}\n'
    '\t@f = false;\n'
    '\tdeletearray .@j[0], getarraysize(.@j);\n'
    '\tmes "Wanted: "+Chk(Mission_MVP,1) + getmonsterinfo($mvps[Mission_MVP_ID-2], MOB_NAME) + " [id:" + $mvps[Mission_MVP_ID-2] + "] - (" + Mission_MVP + "/1)^000000";\n'
    '\n'
    '\t// Zeny formula:\n'
    '\t.@zeny = $mvps[Mission_MVP_ID-1] * 1000000;\n'
    '\tmes "R\xe9compenses:";\n'
    '\tmes " > Points d\'event: ^0055FF"+ $mvps[Mission_MVP_ID-1] +"pts^000000";\n'
    '\tmes " > Zenys: ^0055FF" + F_InsertComma(.@zeny) + "^000000";\n'
    '\tmes " > "+mesitemicon(12214) +" -> 1 "+ mesitemlink(12214); // Convex_Mirror\n'
)

new_status_mvp = (
    'Mission_Status_MVP:\n'
    '\tif( !Mission_MVP_ID ) {\n'
    '\t\tmes "Pas de mission en cours.";\n'
    '\t\tclose;\n'
    '\t}\n'
    '\t@f = false;\n'
    '\tdeletearray .@j[0], getarraysize(.@j);\n'
    '\t// Mission_MVP_ID stocke maintenant le mob_id directement (pas un index dans $mvps[]).\n'
    '\t// Les points proviennent toujours de $mvps[] via L_MvpPoints (fallback 4).\n'
    '\tcallsub L_MvpPoints, Mission_MVP_ID;\n'
    '\t.@pts = @hm_mvp_pts;\n'
    '\tmes "Wanted: "+Chk(Mission_MVP,1) + getmonsterinfo(Mission_MVP_ID, MOB_NAME) + " [id:" + Mission_MVP_ID + "] - (" + Mission_MVP + "/1)^000000";\n'
    '\n'
    '\t// Zeny formula:\n'
    '\t.@zeny = .@pts * 1000000;\n'
    '\tmes "R\xe9compenses:";\n'
    '\tmes " > Points d\'event: ^0055FF"+ .@pts +"pts^000000";\n'
    '\tmes " > Zenys: ^0055FF" + F_InsertComma(.@zeny) + "^000000";\n'
    '\tmes " > "+mesitemicon(12214) +" -> 1 "+ mesitemlink(12214); // Convex_Mirror\n'
)
hm_patches.append((old_status_mvp, new_status_mvp))

# Patch 7: .@pointss = $mvps[Mission_MVP_ID-1];
hm_patches.append((
    '\t.@pointss = $mvps[Mission_MVP_ID-1];\n',
    '\t.@pointss = .@pts;\n',
))

# Patch 8: logmes in the "bug" branch
hm_patches.append((
    '\t\tlogmes "un bug est arriv\xe9, stingor est au courant xD  "+$mvps[Mission_MVP_ID-1]+" -> "+.@pointss;\n',
    '\t\tlogmes "un bug est arriv\xe9, stingor est au courant xD  "+.@pts+" -> "+.@pointss;\n',
))

# Patch 9: dispbottom at end of MVP status
hm_patches.append((
    '\tdispbottom "Vous gagnez "+$mvps[Mission_MVP_ID-1]+" points d\'event et 1 Convex Mirror.";\n',
    '\tdispbottom "Vous gagnez "+.@pts+" points d\'event et 1 Convex Mirror.";\n',
))

# Patch 10: final logmes (mob name via getmonsterinfo)
hm_patches.append((
    '\tlogmes "Termine sa mission MVP \xe0 ["+.@prepoints+"->"+#KAFRAPOINTS+"] pts: "+getmonsterinfo($mvps[Mission_MVP_ID-2], MOB_NAME);\n',
    '\tlogmes "Termine sa mission MVP \xe0 ["+.@prepoints+"->"+#KAFRAPOINTS+"] pts: "+getmonsterinfo(Mission_MVP_ID, MOB_NAME);\n',
))

# Patch 11: inject the three helper subroutines right before OnInit:
subs = (
    '// ===========================================================================\n'
    '// Picker des mobs de mission normale via le groupe summon Branch_Of_Dead_Tree.\n'
    '// Remplace l\'ancienne requete SQL sur mob_db. Remplit @hm_mob[0..Quests-1].\n'
    '// Filtre : exclut les boss et les plantes (RMF flags), exclut les ID\n'
    '// presents dans .Blacklist$, evite les doublons.\n'
    '// ===========================================================================\n'
    'L_PickMissionMobs:\n'
    '\tdeletearray @hm_mob[0], getarraysize(@hm_mob);\n'
    '\t.@count = 0;\n'
    '\t.@tries = 0;\n'
    '\twhile( .@count < .Quests && .@tries < 500 ) {\n'
    '\t\t.@tries++;\n'
    '\t\t.@id = getrandmobid(MOBG_BRANCH_OF_DEAD_TREE, RMF_MOB_NOT_BOSS | RMF_MOB_NOT_PLANT);\n'
    '\t\tif( .@id <= 0 ) continue;\n'
    '\t\t// Blacklist (encadre de virgules pour eviter les sous-chaines, ex: 88 vs 1088)\n'
    '\t\tif( compare(","+.Blacklist$+",", ","+.@id+",") ) continue;\n'
    '\t\t// Anti-doublon\n'
    '\t\t.@dup = 0;\n'
    '\t\tfor( .@j = 0; .@j < .@count; .@j++ ) {\n'
    '\t\t\tif( @hm_mob[.@j] == .@id ) { .@dup = 1; break; }\n'
    '\t\t}\n'
    '\t\tif( .@dup ) continue;\n'
    '\t\t@hm_mob[.@count] = .@id;\n'
    '\t\t.@count++;\n'
    '\t}\n'
    '\treturn;\n'
    '\n'
    '// ===========================================================================\n'
    '// Picker MVP via le groupe summon Bloody_Dead_Branch.\n'
    '// Filtre par le flag MD_MVP : le groupe contient aussi des mini-boss sans\n'
    '// le flag MVP qu\'on doit ecarter. Retourne le mob_id dans @hm_mvp\n'
    '// (0 si echec apres 500 essais).\n'
    '// ===========================================================================\n'
    'L_PickMvpMob:\n'
    '\t@hm_mvp = 0;\n'
    '\t.@tries = 0;\n'
    '\twhile( !@hm_mvp && .@tries < 500 ) {\n'
    '\t\t.@tries++;\n'
    '\t\t.@cand = getrandmobid(MOBG_BLOODY_DEAD_BRANCH, RMF_NONE);\n'
    '\t\tif( .@cand <= 0 ) continue;\n'
    '\t\tif( getmonsterinfo(.@cand, MOB_MODE) & MD_MVP )\n'
    '\t\t\t@hm_mvp = .@cand;\n'
    '\t}\n'
    '\treturn;\n'
    '\n'
    '// ===========================================================================\n'
    '// Lookup des points d\'event associes a un mob MVP.\n'
    '// Cherche le mob_id dans $mvps[] (table conservee comme reference de points)\n'
    '// et retourne le score associe via @hm_mvp_pts. Defaut : 4 (valeur la plus\n'
    '// courante dans $mvps[]).\n'
    '//   getarg(0) = mob_id\n'
    '// ===========================================================================\n'
    'L_MvpPoints:\n'
    '\t@hm_mvp_pts = 4;\n'
    '\t.@n = getarraysize($mvps);\n'
    '\tfor( .@k = 0; .@k < .@n; .@k += 2 ) {\n'
    '\t\tif( $mvps[.@k] == getarg(0) ) {\n'
    '\t\t\t@hm_mvp_pts = $mvps[.@k+1];\n'
    '\t\t\tbreak;\n'
    '\t\t}\n'
    '\t}\n'
    '\treturn;\n'
    '\n'
)
hm_patches.append((
    'OnInit:\n'
    '\t.Delay = 4;            // Quest delay, in hours (0 to disable).\n',
    subs +
    'OnInit:\n'
    '\t.Delay = 4;            // Quest delay, in hours (0 to disable).\n',
))

patch(HM, hm_patches)

# ---------------------------------------------------------------------------
# mvps.npc : remplace $mvps[Mission_MVP_ID-2] par Mission_MVP_ID dans les
# deux endroits restants.
# ---------------------------------------------------------------------------
mvp_patches = [
    (
        '\t\t\tif( .@mobid == $mvps[Mission_MVP_ID-2] ) {\n'
        '\t\t\t\tMission_MVP = 1;\n',
        '\t\t\t// Mission_MVP_ID stocke maintenant le mob_id directement (cf. huntmission.npc L_PickMvpMob)\n'
        '\t\t\tif( .@mobid == Mission_MVP_ID ) {\n'
        '\t\t\t\tMission_MVP = 1;\n',
    ),
    (
        '\tif( Mission_MVP != #Mission_Count_MVP && Mission_MVP_ID > 1) {\n'
        '\t\tif( killedrid == $mvps[Mission_MVP_ID-2] ) {\n',
        '\tif( Mission_MVP != #Mission_Count_MVP && Mission_MVP_ID > 1) {\n'
        '\t\t// Mission_MVP_ID stocke maintenant le mob_id directement\n'
        '\t\tif( killedrid == Mission_MVP_ID ) {\n',
    ),
]
patch(MVP, mvp_patches)

# ---------------------------------------------------------------------------
# Final verification
# ---------------------------------------------------------------------------
for p in [HM, MVP]:
    with open(p, 'rb') as f:
        data = f.read()
    print(f'{os.path.basename(p)}: {len(data)} bytes, '
          f'U+FFFD={data.count(b"\xef\xbf\xbd")}, '
          f'0xE9={data.count(b"\xe9")}')
