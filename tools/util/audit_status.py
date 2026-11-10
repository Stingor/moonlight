"""Audit des statuts (SC_*) utilises mais absents de la base de statuts pre-re.

En pre-re, status_change_start ne connait que db/pre-re/status.yml et
db/import/status.yml. Un SC_* qui n'est defini que dans db/re/status.yml
compile et charge sans bruit, puis chaque sc_start echoue en jeu sur
"status_change_start: Invalid status change (N)!".

Sources examinees :
- les scripts declares dans moon/scripts_moon.conf (seuls charges) ;
- les champs Script / EquipScript / UnEquipScript des YAML de db/import ;
- le champ Status des competences de db/import/skill_db.yml.

Usage : python tools/util/audit_status.py
Code de sortie 1 si un statut demarre (sc_start...) est absent.
"""
import os
import re
import sys

import yaml

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(ROOT)

# Commandes qui demarrent un statut : les seules qui provoquent l'erreur.
# transform / active_transform demarrent le SC_MTF_* passe en 3e argument.
START_CALL = re.compile(r'\b(?:sc_start\d?\s*\(?\s*|(?:active_)?transform\b[^;]*?)(SC_\w+)', re.I)
SC_TOKEN = re.compile(r'\bSC_[A-Z0-9_]+\b')


def load(path):
    with open(path, 'rb') as f:
        return yaml.safe_load(f.read().decode('utf-8', 'replace')) or {}


def statuses(path):
    if not os.path.exists(path):
        return set()
    return {('SC_' + e['Status']).upper() for e in (load(path).get('Body') or []) if 'Status' in e}


def enum_names():
    """Noms SC_* du source, pour ignorer les faux positifs (SC_ hors enum)."""
    names = set()
    for p in ('src/map/status.hpp', 'src/map/script_constants.hpp'):
        if os.path.exists(p):
            names |= set(SC_TOKEN.findall(open(p, encoding='utf-8', errors='replace').read()))
    return names


def loaded_scripts():
    seen, out = set(), []

    def conf(p):
        if p in seen or not os.path.exists(p):
            return
        seen.add(p)
        for line in open(p, encoding='latin-1'):
            line = line.split('//')[0].strip()
            if line.startswith('npc:'):
                out.append(line[4:].strip())
            elif line.startswith('import:'):
                conf(line[7:].strip())
    conf('moon/scripts_moon.conf')
    return out


def main():
    known = statuses('db/pre-re/status.yml') | statuses('db/import/status.yml')
    only_re = statuses('db/re/status.yml') - known
    enum = enum_names()
    started, mentioned = {}, {}

    def note(sc, where, start):
        # SC_ALL : pseudo-statut de sc_end, jamais dans la base
        if sc in known or sc not in enum or sc == 'SC_ALL':
            return
        (started if start else mentioned).setdefault(sc, set()).add(where)

    for p in loaded_scripts():
        if not os.path.exists(p):
            continue
        for n, line in enumerate(open(p, encoding='latin-1'), 1):
            code = line.split('//')[0]
            for sc in START_CALL.findall(code):
                note(sc.upper(), '%s:%d' % (p, n), True)
            for sc in SC_TOKEN.findall(code):
                note(sc, '%s:%d' % (p, n), False)

    for dp, _, fs in os.walk('db/import'):
        for f in fs:
            if not f.endswith('.yml'):
                continue
            p = os.path.join(dp, f).replace('\\', '/')
            try:
                body = load(p).get('Body') or []
            except yaml.YAMLError:
                continue
            for e in body:
                if not isinstance(e, dict):
                    continue
                tag = '%s (%s)' % (p, e.get('AegisName') or e.get('Name') or e.get('Id'))
                for k in ('Script', 'EquipScript', 'UnEquipScript'):
                    s = e.get(k)
                    if isinstance(s, str):
                        for sc in START_CALL.findall(s):
                            note(sc.upper(), tag, True)
                        for sc in SC_TOKEN.findall(s):
                            note(sc, tag, False)
                st = e.get('Status')
                if isinstance(st, str) and f.startswith('skill_db'):
                    note(('SC_' + st).upper(), tag, True)

    for title, d in (('Statuts DEMARRES mais absents (erreur en jeu)', started),
                     ('Statuts seulement cites (getstatus, sc_end...)', {k: v for k, v in mentioned.items() if k not in started})):
        print('==', title, ':', len(d))
        for sc in sorted(d):
            src = 'defini dans db/re' if sc in only_re else 'absent de toute base'
            print('  %s  [%s]' % (sc, src))
            for w in sorted(d[sc])[:5]:
                print('      ' + w)
            if len(d[sc]) > 5:
                print('      ... +%d' % (len(d[sc]) - 5))
    return 1 if started else 0


if __name__ == '__main__':
    sys.exit(main())
