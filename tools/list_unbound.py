# -*- coding: utf-8 -*-
"""
List every achievement flagged `Unbound: true` in achievement_db.yml and emit
the SQL `NOT IN (...)` snippet used by tools/sql/achievement_account_bound.sql.

Those achievements stay per-character (are NOT merged into per-account rows).
Run this after changing any `Unbound` flag, then paste the snippet into the
migration's step 2 and step 3.

Usage:  python tools/list_unbound.py
"""
import re
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATH = os.path.join(ROOT, 'db', 'import', 'achievement_db.yml')

raw = open(PATH, 'rb').read()

# Encoding integrity check (these files must stay latin-1, no UTF-8 debris).
fffd = raw.count(b'\xef\xbf\xbd')
if fffd:
    print('!! WARNING: %d x U+FFFD (0xEF 0xBF 0xBD) found - file encoding is corrupted !!' % fffd)

text = raw.decode('latin-1')

id_re = re.compile(r'^\s*-\s*Id:\s*(\d+)')
grp_re = re.compile(r'^\s*Group:\s*(\S+)')
name_re = re.compile(r'^\s*Name:\s*(.+?)\s*$')
unb_re = re.compile(r'^\s*Unbound:\s*(\S+)')

cur_id = cur_group = cur_name = None
unbound = []
for ln in text.splitlines():
    m = id_re.match(ln)
    if m:
        cur_id, cur_group, cur_name = int(m.group(1)), None, None
        continue
    if cur_id is None:
        continue
    g = grp_re.match(ln)
    if g:
        cur_group = g.group(1)
    n = name_re.match(ln)
    if n and cur_name is None:
        cur_name = n.group(1)
    u = unb_re.match(ln)
    if u and u.group(1).lower() in ('true', 'yes', 'on', '1'):
        unbound.append((cur_id, cur_group, cur_name))

unbound.sort()
print('Unbound achievements (%d):' % len(unbound))
for aid, grp, name in unbound:
    print('  %-8d %-12s %s' % (aid, grp or '?', name or ''))

ids = [a for a, _, _ in unbound]
joined = ','.join(str(i) for i in ids) if ids else '0'
print('\n-- SQL snippet for tools/sql/achievement_account_bound.sql (steps 2 & 3):')
print('  AND `id` NOT IN (%s)' % joined)
