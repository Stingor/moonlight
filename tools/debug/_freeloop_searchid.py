# -*- coding: latin-1 -*-
"""
Ajoute freeloop(1) en debut de zone de recherche et freeloop(0) avant le end final
dans @searchid / @deepsearchid. Necessaire car @deepsearchid peut iterer plusieurs
centaines d'items (storage premium plein) dans ses boucles de remplissage, ce qui
peut hit la limite anti-runaway du script engine.
"""
PATH = r'D:\Mes documents\GitHub\moonlight\moon\atcommands.npc'

with open(PATH, 'r', encoding='latin-1') as f:
    content = f.read()

# 1. freeloop(1) juste apres le calcul de .@iscard, avant tout le travail de recherche
OLD = '\t\t.@iscard = (getiteminfo(.@itemid, ITEMINFO_TYPE) == IT_CARD);\n'
NEW = OLD + '\t\tfreeloop(1);\n'
assert OLD in content, 'ERREUR: ligne .@iscard introuvable'
content = content.replace(OLD, NEW)

# 2. freeloop(0) juste avant le end; final (apres ClearVars())
OLD = '\t\tClearVars();\n\t\tend;\n\t}\n\nfunction SearchStorage {'
NEW = '\t\tClearVars();\n\t\tfreeloop(0);\n\t\tend;\n\t}\n\nfunction SearchStorage {'
assert OLD in content, 'ERREUR: bloc ClearVars/end final introuvable'
content = content.replace(OLD, NEW)

with open(PATH, 'w', encoding='latin-1', newline='') as f:
    f.write(content)

with open(PATH, 'rb') as f:
    data = f.read()
assert b'\xef\xbf\xbd' not in data, 'ERREUR: corruption UTF-8!'

print(f'OK - {len(data)} bytes, freeloop(1)/freeloop(0) inseres')
