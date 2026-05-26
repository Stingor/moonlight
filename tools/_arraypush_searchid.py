# -*- coding: latin-1 -*-
"""
Remplace l'indexation manuelle .@flaggeditem*[.@it] / .@it++ par arraypush
dans les 6 boucles de remplissage de @searchid / @deepsearchid.

  Avant :
      .@it = 0;

      for (...) {
          if (.@nb > 0) {
              .@flaggeditemid[.@it]     = EXPR; // commentaire
              .@flaggeditemnb[.@it]     = .@nb; // commentaire
              .@flaggeditemrefine[.@it] = EXPR; // commentaire
              .@it++;
              COUNTER += .@nb;
          }
      }
      if (.@it > 0)

  Apres :
      for (...) {
          if (.@nb > 0) {
              arraypush .@flaggeditemid,     EXPR;
              arraypush .@flaggeditemnb,     .@nb;
              arraypush .@flaggeditemrefine, EXPR;
              COUNTER += .@nb;
          }
      }
      .@it = getarraysize(.@flaggeditemid);
      if (.@it > 0)
"""
import re

PATH = r'D:\Mes documents\GitHub\moonlight\moon\atcommands.npc'

with open(PATH, 'r', encoding='latin-1') as f:
    content = f.read()

results = []

# A. Supprime ".@it = 0;" + la ligne vide qui suit
n = len(re.findall(r'\t+\.@it = 0;\n\t*\n', content))
content = re.sub(r'\t+\.@it = 0;\n\t*\n', '', content)
assert n == 6, f'ERREUR: {n}/6 blocs ".@it = 0" trouves'
results.append(f'  [A] {n} x ".@it = 0" + ligne vide supprimes')

# B. Remplace les assignations indexees par arraypush (supprime les commentaires)
fields = [
    (r'\.@flaggeditemid\[\.@it\]',     '.@flaggeditemid'),
    (r'\.@flaggeditemeq\[\.@it\]',     '.@flaggeditemeq'),
    (r'\.@flaggeditemnb\[\.@it\]',     '.@flaggeditemnb'),
    (r'\.@flaggeditemrefine\[\.@it\]', '.@flaggeditemrefine'),
]
for pat, arr in fields:
    n = len(re.findall(r'\t+' + pat + r' = [^;]+;[^\n]*\n', content))
    content = re.sub(
        r'(\t+)' + pat + r' = ([^;]+);[^\n]*\n',
        lambda m, a=arr: m.group(1) + f'arraypush {a}, ' + m.group(2).rstrip() + ';\n',
        content
    )
    results.append(f'  [B] {n} x {arr}[.@it] -> arraypush')

# C. Supprime les lignes ".@it++;"
n = len(re.findall(r'\t+\.@it\+\+;\n', content))
content = re.sub(r'\t+\.@it\+\+;\n', '', content)
assert n == 6, f'ERREUR: {n}/6 ".@it++" trouves'
results.append(f'  [C] {n} x ".@it++" supprimes')

# D. Insere ".@it = getarraysize(.@flaggeditemid);" juste avant "if (.@it > 0)"
# La backreference \1 garantit que le "}" et le "if" ont la meme indentation
# (= fermeture de la boucle de remplissage, pas un "}" plus profond)
n = len(re.findall(r'(\t+)\}\n\1if \(\.@it > 0\)', content))
content = re.sub(
    r'(\t+)\}\n(\1if \(\.@it > 0\))',
    lambda m: m.group(1) + '}\n' + m.group(1) + '.@it = getarraysize(.@flaggeditemid);\n' + m.group(2),
    content
)
assert n == 6, f'ERREUR: {n}/6 insertions ".@it = getarraysize(...)" faites'
results.append(f'  [D] {n} x ".@it = getarraysize(...)" inseres')

# Verification + ecriture
with open(PATH, 'w', encoding='latin-1', newline='') as f:
    f.write(content)

with open(PATH, 'rb') as f:
    data = f.read()
assert b'\xef\xbf\xbd' not in data, 'ERREUR CRITIQUE: corruption UTF-8!'

print(f'OK - {len(data)} bytes, 0 corruption')
for r in results:
    print(r)
