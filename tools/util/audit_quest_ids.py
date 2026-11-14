"""Audit des ID de quete utilises par les scripts mais absents de la quest_db pre-re.

En pre-re, quest_search ne connait que db/pre-re/quest_db.yml et
db/import/quest_db.yml. Un script renewal porte sans synchroniser la base
appelle setquest / changequest sur un ID inconnu : le map-server le refuse
(depuis le garde-fou de script.cpp, le script s'arrete et un bug_report est
ouvert), mais seulement au moment ou un joueur y passe. Cet audit le voit
avant.

Sources examinees : les scripts declares dans moon/scripts_moon.conf (seuls
charges). Seuls les ID ecrits en litteral ENTIER sont vus : dans
« setquest 7280+.@i », 7280 n'est qu'une base de calcul et n'est pas
signale. Un ID calcule echappe donc a l'audit et reste couvert par le
garde-fou en jeu.

Trois niveaux :
- setquest / changequest (nouvel ID) : le script s'arrete a coup sur ;
- completequest / erasequest : le garde-fou laisse passer si le joueur a
  la quete, et ces appels sont souvent proteges par un isbegin_quest. A
  verifier a la main ;
- checkquest & co : renvoie -1, rien ne s'arrete.

Usage : python tools/util/audit_quest_ids.py
Code de sortie 1 seulement pour le premier niveau.
"""
import os
import re
import sys

import yaml

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(ROOT)

ADD_CMDS = ('setquest',)
OWN_CMDS = ('completequest', 'erasequest')
READ_CMDS = ('checkquest', 'isbegin_quest', 'questprogress', 'questactive')
# Un argument litteral entier : suivi d'une fin d'argument, pas d'un operateur.
END = r'(?=\s*(?:[;,)]|//|$))'
ONE_ARG = re.compile(r'\b(%s)\b\s*\(?\s*(\d+)%s' % ('|'.join(ADD_CMDS + OWN_CMDS + READ_CMDS), END), re.I)
# changequest <ancien>,<nouveau> : seul le nouveau doit exister en base
CHANGE = re.compile(r'\bchangequest\b\s*\(?\s*[^,;]+,\s*(\d+)%s' % END, re.I)


def ids(path):
    if not os.path.exists(path):
        return set()
    with open(path, 'rb') as f:
        data = yaml.safe_load(f.read().decode('utf-8', 'replace')) or {}
    return {int(e['Id']) for e in (data.get('Body') or []) if isinstance(e, dict) and 'Id' in e}


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
    known = ids('db/pre-re/quest_db.yml') | ids('db/import/quest_db.yml')
    only_re = ids('db/re/quest_db.yml')
    add, own, read = {}, {}, {}

    for p in loaded_scripts():
        if not os.path.exists(p):
            continue
        for n, line in enumerate(open(p, encoding='latin-1'), 1):
            code = line.split('//')[0]
            where = '%s:%d' % (p, n)
            for cmd, q in ONE_ARG.findall(code):
                q, cmd = int(q), cmd.lower()
                if q in known:
                    continue
                bucket = add if cmd in ADD_CMDS else own if cmd in OWN_CMDS else read
                bucket.setdefault(q, set()).add('%s  (%s)' % (where, cmd))
            for q in CHANGE.findall(code):
                q = int(q)
                if q not in known:
                    add.setdefault(q, set()).add('%s  (changequest)' % where)

    def dump(title, d):
        if not d:
            return
        print('== %s : %d ID' % (title, len(d)))
        for q in sorted(d):
            tag = ' (present dans db/re)' if q in only_re else ''
            print('  %d%s' % (q, tag))
            for w in sorted(d[q]):
                print('      ' + w)

    dump('Ajout d\'un ID absent (le script s\'arrete en jeu)', add)
    dump('completequest / erasequest sur un ID absent (verifier la garde isbegin_quest)', own)
    dump('Lecture d\'un ID absent (checkquest renvoie -1)', read)
    if not add and not own and not read:
        print('Aucun ID de quete absent.')
    return 1 if add else 0


if __name__ == '__main__':
    sys.exit(main())
