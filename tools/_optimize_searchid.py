# -*- coding: latin-1 -*-
"""
Optimise les commandes @searchid / @deepsearchid dans atcommands.npc :
  1. Bug : accountid/charid stockes en vars permanentes -> vars temporaires
  2. .@iscard precalcule une seule fois au lieu de 5+ appels getiteminfo
  3. Blocs OR redondants (card0||card1||card2||card3) -> if (.@nb > 0) x6
  4. getarraysize(.@flaggeditemid) -> .@it dans if/for (pas dans ClearVars)
"""
import re

PATH = r'D:\Mes documents\GitHub\moonlight\moon\atcommands.npc'

with open(PATH, 'r', encoding='latin-1') as f:
    content = f.read()

results = []

# ── 1. Bug fix : vars permanentes accountid/charid -> vars temporaires ──────
OLD = '\taccountid = getcharid(3);\n\tcharid = getcharid(0);'
NEW = '\t.@accountid = getcharid(3);\n\t.@charid = getcharid(0);'
assert OLD in content, 'ERREUR: ligne definition accountid/charid introuvable'
content = content.replace(OLD, NEW)
results.append('  [1] definition accountid/charid corrigee')

# ── 2. Remplace toutes les references restantes via regex ─────────────────────
# (?<!@)\b : ne pas matcher .@accountid / .@charid deja corriges
# \b : evite de matcher getcharid (pas de word boundary entre t et c)
c1 = len(re.findall(r'(?<!@)\baccountid\b', content))
content = re.sub(r'(?<!@)\baccountid\b', '.@accountid', content)
c2 = len(re.findall(r'(?<!@)\bcharid\b', content))
content = re.sub(r'(?<!@)\bcharid\b', '.@charid', content)
results.append(f'  [2] references remplacees : {c1} x accountid, {c2} x charid')

# ── 3. Precalcul .@iscard juste apres l annonce de recherche ─────────────────
OLD = '\t\tdispbottom "[ Recherche de "+.@itemname$+" ("+.@itemid+") ]",0xF0FF0F;\n'
NEW = (OLD +
       '\t\t.@iscard = (getiteminfo(.@itemid, ITEMINFO_TYPE) == IT_CARD);\n')
assert OLD in content, 'ERREUR: ligne dispbottom Recherche introuvable'
content = content.replace(OLD, NEW)
results.append('  [3] .@iscard precalcule apres le dispbottom')

# ── 4. Remplace les checks IT_CARD dans le corps principal (.@itemid) ─────────
TOKEN_MAIN = 'getiteminfo( .@itemid, ITEMINFO_TYPE ) == IT_CARD'
c = content.count(TOKEN_MAIN)
assert c > 0, 'ERREUR: aucun check IT_CARD (.@itemid) trouve'
content = content.replace(TOKEN_MAIN, '.@iscard')
results.append(f'  [4] {c} checks IT_CARD (.@itemid) -> .@iscard')

# ── 5. Met a jour les appels SearchStorage : ajout .@iscard en 5e arg ─────────
replaced = 0
for i in range(6):
    old = f'SearchStorage(.@itemid, .@accountid, {i}, .@deep)'
    new = f'SearchStorage(.@itemid, .@accountid, {i}, .@deep, .@iscard)'
    if old in content:
        content = content.replace(old, new)
        replaced += 1
assert replaced == 6, f'ERREUR: seulement {replaced}/6 appels SearchStorage trouves'
results.append(f'  [5] {replaced} appels SearchStorage mis a jour (5e arg .@iscard)')

# ── 6. Met a jour la fonction SearchStorage (getarg + check IT_CARD interne) ──
OLD_SIG = '\t.@deep = getarg(3);\n\tClearVars();'
NEW_SIG = '\t.@deep = getarg(3);\n\t.@iscard = getarg(4, 0);\n\tClearVars();'
assert OLD_SIG in content, 'ERREUR: signature SearchStorage introuvable'
content = content.replace(OLD_SIG, NEW_SIG)

TOKEN_STOR = 'getiteminfo( .@id, ITEMINFO_TYPE ) == IT_CARD'
c = content.count(TOKEN_STOR)
assert c > 0, 'ERREUR: aucun check IT_CARD (.@id) dans SearchStorage'
content = content.replace(TOKEN_STOR, '.@iscard')
results.append(f'  [6] SearchStorage : getarg(4) + {c} check IT_CARD (.@id) -> .@iscard')

# ── 7. Remplace les blocs OR redondants par if (.@nb > 0) ────────────────────
# Regex : capture l indentation de la premiere ligne, remplace les 4 lignes OR
def replace_or_block(var_prefix, id_var):
    pattern = (
        r'(\t+)if\( @' + var_prefix + r'card0\[\.@i\] == ' + re.escape(id_var) + r' \|\|\n'
        r'\t+@' + var_prefix + r'card1\[\.@i\] == ' + re.escape(id_var) + r' \|\|\n'
        r'\t+@' + var_prefix + r'card2\[\.@i\] == ' + re.escape(id_var) + r' \|\|\n'
        r'\t+@' + var_prefix + r'card3\[\.@i\] == ' + re.escape(id_var) + r'\)\n'
    )
    return pattern, r'\1if (.@nb > 0)\n'

blocks = [
    ('inventorylist_', '.@itemid'),   # inventaire perso courant + autres persos x2
    ('cartlist_', '.@itemid'),        # cart perso courant + autres persos x2
    ('guildstoragelist_', '.@itemid'),# guild storage x1
    ('storagelist_', '.@id'),         # fonction SearchStorage x1
]
for prefix, idvar in blocks:
    pat, repl = replace_or_block(prefix, idvar)
    n = len(re.findall(pat, content))
    assert n > 0, f'ERREUR: bloc OR introuvable pour {prefix}'
    content = re.sub(pat, repl, content)
    results.append(f'  [7] {n} bloc(s) OR -> if (.@nb > 0) [{prefix.rstrip("_")}]')

# ── 8. getarraysize(.@flaggeditemid) -> .@it dans if/for (pas dans ClearVars) ──
# ClearVars a `) {` sur la meme ligne -> NE correspond PAS aux patterns ci-dessous
n_if1 = content.count('if( getarraysize(.@flaggeditemid) > 0 )\n')
n_if2 = content.count('if ( getarraysize(.@flaggeditemid) > 0 )\n')
content = content.replace('if( getarraysize(.@flaggeditemid) > 0 )\n', 'if (.@it > 0)\n')
content = content.replace('if ( getarraysize(.@flaggeditemid) > 0 )\n', 'if (.@it > 0)\n')
n_for = content.count('.@i < getarraysize(.@flaggeditemid);')
content = content.replace('.@i < getarraysize(.@flaggeditemid);', '.@i < .@it;')
assert n_if1 + n_if2 > 0, 'ERREUR: aucun if(getarraysize) trouve'
assert n_for > 0, 'ERREUR: aucun for(getarraysize) trouve'
results.append(f'  [8] getarraysize remplace : {n_if1+n_if2} if-conds, {n_for} for-loops')

# ── Ecriture + verification ────────────────────────────────────────────────────
with open(PATH, 'w', encoding='latin-1', newline='') as f:
    f.write(content)

with open(PATH, 'rb') as f:
    data = f.read()
assert b'\xef\xbf\xbd' not in data, 'ERREUR CRITIQUE: corruption UTF-8 detectee!'

print(f'OK - {len(data)} bytes, 0 corruption')
for r in results:
    print(r)
