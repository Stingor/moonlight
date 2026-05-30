# -*- coding: latin-1 -*-
"""
Fix bug enchanter.npc : reroll utilise select() comme index de slot alors que
select() retourne la position dans le menu, pas le slot reel.

Resultat actuel : si une arme a un enchant en card3 et un en card4, le joueur
qui choisit "reroll card3" (position 1 du menu) ecrit en realite dans card1
qui peut etre un slot de carte -> ecrase une vraie carte !

Fix : mapper position du menu vers slot reel via un array .@slot_map.
"""
PATH = r'D:\Mes documents\GitHub\moonlight\moon\enchanter.npc'

with open(PATH, 'r', encoding='latin-1') as f:
    content = f.read()

# Bloc original (note : "\xe8" = è, "\xe9" = é)
OLD = (
    '\t\tcase 1: // reroll a specific enchant slot\n'
    '\t\t\tfor (.@i = 0; .@i < MAX_SLOTS; ++.@i) {\n'
    '\t\t\t\tif (getiteminfo(getd(".@itembyuid_card" + (.@i + 1)), ITEMINFO_SUBTYPE) == CARD_ENCHANT)\n'
    '\t\t\t\t\t.@menu$ += "Reroll " + getitemname(getd(".@itembyuid_card" + (.@i + 1))) + " (" + (.@i + 1) + "\xe8me slot):";\n'
    '\t\t\t}\n'
    '\t\t\t.@selected = select(.@menu$);\n'
)

NEW = (
    '\t\tcase 1: // reroll a specific enchant slot\n'
    '\t\t\t.@map_size = 0;\n'
    '\t\t\tfor (.@i = 0; .@i < MAX_SLOTS; ++.@i) {\n'
    '\t\t\t\tif (getiteminfo(getd(".@itembyuid_card" + (.@i + 1)), ITEMINFO_SUBTYPE) == CARD_ENCHANT) {\n'
    '\t\t\t\t\t.@slot_map[.@map_size] = .@i + 1;\n'
    '\t\t\t\t\t.@map_size++;\n'
    '\t\t\t\t\t.@menu$ += "Reroll " + getitemname(getd(".@itembyuid_card" + (.@i + 1))) + " (" + (.@i + 1) + "\xe8me slot):";\n'
    '\t\t\t\t}\n'
    '\t\t\t}\n'
    '\t\t\t.@selected = .@slot_map[select(.@menu$) - 1];\n'
)

assert OLD in content, 'ERREUR: bloc original introuvable'
content = content.replace(OLD, NEW)

with open(PATH, 'w', encoding='latin-1', newline='') as f:
    f.write(content)

with open(PATH, 'rb') as f:
    data = f.read()
assert b'\xef\xbf\xbd' not in data, 'ERREUR: corruption UTF-8!'

print(f'OK - {len(data)} bytes, fix applique')
