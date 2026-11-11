"""Audit des competences lancees par les mobs (mob_skill_db) contre la base pre-re.

Deux pannes que ni le boot ni audit_status.py ne signalent :

1. Stub pre-re sans Type : db/pre-re/skill_db.yml declare beaucoup de skills
   NPC renewal sous forme de coquille (MaxLevel 1, pas de Type). Le mob les
   lance quand meme -> "battle_calc_attack: unknown attack type! 0 (skill_id=N)".
   Correctif : recopier l'entree de db/re/skill_db.yml dans db/import/skill_db.yml.

2. Statut demarre en dur dans le C++ : src/map/skills/**/*.cpp fait parfois
   sc_start(..., SC_X, ...) ou type = SC_X sans passer par le champ Status du
   skill_db. Si SC_X n'est que dans db/re/status.yml -> "status_change_start:
   Invalid status change (N)!". Les blocs #ifdef RENEWAL sont ignores.
   Correctif : recopier le statut dans db/import/status.yml.

Les depassements de MaxLevel ne sont pas signales : le niveau d'un mob est
plafonne au MaxLevel, c'est courant et sans erreur.

Usage : python tools/util/audit_mob_skills.py
Code de sortie 1 si un probleme est trouve.
"""
import glob
import os
import re
import sys

import yaml

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(ROOT)


def body(path):
    if not os.path.exists(path):
        return []
    with open(path, 'rb') as f:
        return (yaml.safe_load(f.read().decode('utf-8', 'replace')) or {}).get('Body') or []


def mob_skill_uses():
    uses = {}
    for p in ('db/pre-re/mob_skill_db.txt', 'db/import/mob_skill_db.txt'):
        if not os.path.exists(p):
            continue
        for line in open(p, encoding='latin-1'):
            f = line.split('//')[0].strip().split(',')
            if len(f) > 4 and f[3].strip().isdigit():
                uses.setdefault(int(f[3]), set()).add(f[1].split('@')[0])
    return uses


def strip_renewal(src):
    """Retire les branches compilees seulement en renewal."""
    out, stack = [], []  # stack : True si la branche courante est renewal-only
    for line in src.split('\n'):
        s = line.strip()
        if re.match(r'#\s*ifdef\s+RENEWAL\b', s):
            stack.append(True); continue
        if re.match(r'#\s*ifndef\s+RENEWAL\b', s):
            stack.append(False); continue
        if re.match(r'#\s*if(n?def)?\b', s):
            stack.append(None); continue
        if re.match(r'#\s*else\b', s) and stack:
            if stack[-1] is not None:
                stack[-1] = not stack[-1]
            continue
        if re.match(r'#\s*endif\b', s) and stack:
            stack.pop(); continue
        if not any(x is True for x in stack):
            out.append(line)
    return '\n'.join(out)


def short(mobs):
    mobs = sorted(mobs)
    return ', '.join(mobs[:5]) + (' ...' if len(mobs) > 5 else '')


def main():
    uses = mob_skill_uses()
    skills = {}
    for p in ('db/pre-re/skill_db.yml', 'db/import/skill_db.yml'):
        for e in body(p):
            skills.setdefault(e['Id'], {}).update(e)
    re_skills = {e['Id']: e for e in body('db/re/skill_db.yml')}
    name_to_id = {}
    for d in (skills, re_skills):
        for sid, e in d.items():
            name_to_id.setdefault(e.get('Name'), sid)
    known_sc = set()
    for p in ('db/pre-re/status.yml', 'db/import/status.yml'):
        known_sc |= {('SC_' + e['Status']).upper() for e in body(p) if 'Status' in e}

    ko = 0
    print('== Skills de mobs sans Type en pre-re (Type present en renewal)')
    for sid in sorted(uses):
        e, r = skills.get(sid), re_skills.get(sid, {})
        if e is None:
            print('  %d %s : ABSENT de pre-re/import | %s' % (sid, r.get('Name'), short(uses[sid])))
            ko += 1
        elif 'Type' not in e and r.get('Type', 'None') != 'None':
            print('  %d %s : re=%s | %s' % (sid, e.get('Name'), r['Type'], short(uses[sid])))
            ko += 1

    print('== Statuts demarres en dur par le C++ des skills de mobs, absents du pre-re')
    start = re.compile(r'sc_start\d?\s*\([^;]*?\b(SC_[A-Z0-9_]+)|\btype\s*=\s*(SC_[A-Z0-9_]+)')
    for f in sorted(glob.glob('src/map/skills/**/*.cpp', recursive=True)):
        src = strip_renewal(open(f, encoding='utf-8', errors='replace').read())
        ctors = re.findall(r'::\w+\(\)\s*:\s*\w+\(\s*([A-Z][A-Z0-9_]+)', src)
        ids = [name_to_id[c] for c in ctors if name_to_id.get(c) in uses]
        if not ids:
            continue
        scs = {a or b for a, b in start.findall(src)} - {'SC_NONE'}
        miss = sorted(s for s in scs if s not in known_sc)
        if miss:
            mobs = set().union(*(uses[i] for i in ids))
            print('  %s (%s) : %s | %s' % (os.path.relpath(f).replace('\\', '/'),
                                         ', '.join(ctors), ', '.join(miss), short(mobs)))
            ko += 1

    print('== %d probleme(s)' % ko)
    return 1 if ko else 0


if __name__ == '__main__':
    sys.exit(main())
