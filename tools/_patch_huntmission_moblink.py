# -*- coding: latin-1 -*-
"""Make mob/MVP names in huntmission.npc dialogs clickable bestiary links.

Adds .HMURL$ constant in OnInit, then wraps the mob name in the two mes
lines (normal mission status + MVP status) with meshyperlink() pointing to
https://moonlight-destiny.fr/index.php?page=bestiary&mobid=<mob_id>.

URL tags are not rendered in select() options nor in dispbottom/logmes, so
those occurrences are intentionally left untouched.
"""
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), os.pardir))
HM = os.path.join(ROOT, 'moon', 'quests', 'huntmission.npc')


def patch(path, replacements):
    with open(path, 'r', encoding='latin-1') as f:
        content = f.read()
    for old, new in replacements:
        assert old in content, f'pattern not found:\n{old[:160]!r}'
        content = content.replace(old, new, 1)
    with open(path, 'w', encoding='latin-1', newline='') as f:
        f.write(content)
    with open(path, 'rb') as f:
        data = f.read()
    assert b'\xef\xbf\xbd' not in data, 'UTF-8 replacement bytes detected!'
    print(f'{os.path.basename(path)}: {len(data)} bytes, '
          f'U+FFFD={data.count(b"\xef\xbf\xbd")}, 0xE9={data.count(b"\xe9")}')


patches = [
    # 1) Inject .HMURL$ constant in OnInit (right after .Quests = 3)
    (
        '\t.Quests = 3;           // Number of subquests per mission (increases rewards).\n',
        '\t.Quests = 3;           // Number of subquests per mission (increases rewards).\n'
        '\t.HMURL$ = "https://moonlight-destiny.fr/index.php?page=bestiary&mobid=";  // base URL bestiaire (cliquable via meshyperlink)\n',
    ),
    # 2) Mission_Status: wrap mob name in clickable link
    (
        '\t\tmes " > "+Chk(getd("Mission"+.@i+"_"),#Mission_Count) + getmonsterinfo(.@j[.@i], MOB_NAME) + " [id:" + .@j[.@i] + "] - (" + getd("Mission"+.@i+"_") + "/" + #Mission_Count + ")^000000";\n',
        '\t\tmes " > "+Chk(getd("Mission"+.@i+"_"),#Mission_Count) + meshyperlink(getmonsterinfo(.@j[.@i], MOB_NAME), .HMURL$ + .@j[.@i]) + " [id:" + .@j[.@i] + "] - (" + getd("Mission"+.@i+"_") + "/" + #Mission_Count + ")^000000";\n',
    ),
    # 3) Mission_Status_MVP: wrap MVP name in clickable link
    (
        '\tmes "Wanted: "+Chk(Mission_MVP,1) + getmonsterinfo(Mission_MVP_ID, MOB_NAME) + " [id:" + Mission_MVP_ID + "] - (" + Mission_MVP + "/1)^000000";\n',
        '\tmes "Wanted: "+Chk(Mission_MVP,1) + meshyperlink(getmonsterinfo(Mission_MVP_ID, MOB_NAME), .HMURL$ + Mission_MVP_ID) + " [id:" + Mission_MVP_ID + "] - (" + Mission_MVP + "/1)^000000";\n',
    ),
]
patch(HM, patches)
