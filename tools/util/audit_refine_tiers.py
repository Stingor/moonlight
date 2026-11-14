#!/usr/bin/env python3
"""
audit_refine_tiers.py
=====================
Paliers de raffinage hors d'atteinte en pre-renewal, dans les objets et les combos charges.

Le serveur plafonne le raffinage a +10 par piece (MAX_REFINE). Un palier renewal au-dela
ne s'active jamais, sans le moindre message. Trois formes sont controlees :
  - une piece seule : getrefine(), getequiprefinerycnt(X) ou leur variable, au-dessus de 10 ;
  - une somme de k pieces (variable ou en ligne), au-dessus de 10 x k ;
  - getallrefine() au-dessus de 60 (main droite, main gauche, coiffe haute, armure, cape,
    chaussures).

Regles de conversion validees par l'utilisateur :
  - une piece : `.@r >= N` (N > 10) devient `getallrefine() >= 2*N` ;
  - une somme de combo : chaque +11 renewal vaut +8 ici, soit -3 par piece (2026-11-14 :
    set dragon d'Abyss Lake F4, 33 -> 24 ; King Schmidt, 22 -> 16 ; YSF01, 39 -> 30).

Usage :
  python tools/util/audit_refine_tiers.py      # sort 1 s'il reste un cas
"""
import os
import re
import sys

import yaml

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.dirname(__file__))
import port_illusion as p  # noqa: E402

ALL_MAX = 60
EQUIP = r'getequiprefinerycnt\(\s*\w+\s*\)'


def scan(label, script, rows):
    s = script or ''
    singles = set(re.findall(r'(\.@\w+)\s*=\s*getrefine\(\)\s*;', s))
    singles |= set(re.findall(rf'(\.@\w+)\s*=\s*{EQUIP}\s*;', s))
    sums = {}
    for m in re.finditer(rf'(\.@\w+)\s*=\s*((?:{EQUIP}\s*\+\s*)+{EQUIP})\s*;', s):
        sums[m.group(1)] = len(re.findall('getequiprefinerycnt', m.group(2)))
    # une piece : la variable ou l'appel ne doit pas etre le dernier terme d'une somme
    for m in re.finditer(rf'(?<![+\w])\s*(getrefine\(\)|{EQUIP})\s*>=?\s*(\d+)', s):
        if int(m.group(2)) > 10 and not re.search(r'\+\s*$', s[:m.start(1)]):
            rows.append((label, f'{m.group(1)} >= {m.group(2)}', 'une piece, max 10'))
    for v in singles:
        for m in re.finditer(rf'{re.escape(v)}\s*>=?\s*(\d+)', s):
            if int(m.group(1)) > 10 and not re.search(r'\+\s*$', s[:m.start()]):
                rows.append((label, f'{v} >= {m.group(1)}', 'une piece, max 10'))
    for v, k in sums.items():
        for t in re.findall(rf'{re.escape(v)}\s*>=?\s*(\d+)', s):
            if int(t) > 10 * k:
                rows.append((label, f'{v} >= {t}', f'{k} pieces, max {10 * k}'))
    for m in re.finditer(r'((?:\.@\w+\s*\+\s*)+\.@\w+)\s*\)?\s*>=?\s*(\d+)', s):
        vs = re.findall(r'\.@\w+', m.group(1))
        if all(v in singles for v in vs) and int(m.group(2)) > 10 * len(vs):
            rows.append((label, m.group(0), f'{len(vs)} pieces, max {10 * len(vs)}'))
    for m in re.finditer(rf'((?:{EQUIP}\s*\+\s*)+{EQUIP})\s*\)?\s*>=?\s*(\d+)', s):
        k = len(re.findall('getequiprefinerycnt', m.group(1)))
        if int(m.group(2)) > 10 * k:
            rows.append((label, f'somme en ligne >= {m.group(2)}', f'{k} pieces, max {10 * k}'))
    for t in re.findall(r'getallrefine\(\)\s*>=?\s*(\d+)', s):
        if int(t) > ALL_MAX:
            rows.append((label, f'getallrefine() >= {t}', f'max {ALL_MAX}'))


def main():
    os.chdir(ROOT)
    rows = []
    for i, e in p.walk('db/item_db.yml', {}).items():
        for key in ('Script', 'EquipScript'):
            if e.get(key):
                scan(f'{i} {e["AegisName"]}', e[key], rows)
    combos = yaml.safe_load(open('db/import/item_combos.yml', encoding='latin-1'))['Body']
    for blk in combos:
        scan('combo ' + '+'.join(blk['Combos'][0]['Combo']), blk.get('Script'), rows)
    seen = sorted(set(rows))
    for r in seen:
        print(' | '.join(r))
    print(f'== {len(seen)} palier(s) hors d\'atteinte')
    return 1 if seen else 0


if __name__ == '__main__':
    sys.exit(main())
