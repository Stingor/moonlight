#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
audit_instance.py
=================
Audit de portage d'une instance renewal vers ce serveur pre-renewal.

Deroule la checklist de portage : pour chaque ressource que le script
reclame (mobs, items, quetes, maps), dit si elle est chargee ici, et sinon si
db/re la fournit. Puis descend d'un cran : drops des mobs, mob_skill,
RandomOptionGroup, cartes et combos, bonus renewal-only, raccordement.

Usage :
  python tools/util/audit_instance.py moon/instances/SkyFortress.txt
  python tools/util/audit_instance.py moon/instances/Edda.txt --name "Half Moon In The Daylight"

Pieges couverts (chacun a produit un faux rapport au moins une fois) :
  - db/import/mob_db.yml et item_db.yml sont vides : tout est derriere
    Footer: Imports:. Resolution recursive, en ecartant Mode: Renewal et
    les imports commentes.
  - Audit par parseur YAML, jamais par regex ancree sur $ (les commentaires
    de fin de ligne '- Id: 4700 # Verified' cassent tout).
  - Un skill peut etre declare dans skill_db sans aucune implementation ;
    ce fork range une competence par fichier sous src/map/skills/.
  - mob_skill_db est un CSV, jamais lu depuis db/re.
  - Un bloc '- Combos:' contient plusieurs '- Combo:' qui partagent le
    Script: du bloc.
"""
import argparse
import glob
import io
import os
import re
import sys

import yaml

# uniquement en execution directe : re-emballer sys.stdout a l'import ferme le
# buffer de l'appelant quand son propre wrapper est collecte.
if __name__ == '__main__':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
os.chdir(ROOT)

CTRL = re.compile(r'[\x00-\x08\x0b\x0c\x0e-\x1f\x7f-\x9f]')
STUB_STATS = {'Str': 598, 'Dex': 857, 'Attack': 5000, 'Defense': 35}
TRAIT_BONUS = ('bPow', 'bSta', 'bWis', 'bSpl', 'bCon', 'bCrt', 'bPAtk',
               'bSMatk', 'bRes', 'bMRes', 'bHPlus', 'bCRate')


# ------------------------------------------------------------------ util --
def read(path):
    return CTRL.sub(' ', open(path, 'rb').read().decode('cp1252', 'replace'))


def load(rel, seen=None):
    """Body d'un YAML rAthena + ses imports non commentes, hors Mode: Renewal."""
    if seen is None:
        seen = set()
    if rel in seen or not os.path.exists(rel):
        return []
    seen.add(rel)
    d = yaml.safe_load(read(rel)) or {}
    body = list(d.get('Body') or [])
    for imp in ((d.get('Footer') or {}).get('Imports') or []):
        if str(imp.get('Mode', '')).lower() == 'renewal':
            continue
        body += load(imp['Path'], seen)
    return body


def load_re(*rels):
    body = []
    for r in rels:
        if os.path.exists(r):
            body += (yaml.safe_load(read(r)) or {}).get('Body') or []
    return body


# Sur ce fork, SEULE moon/scripts_moon.conf est chargee. Le dossier npc/ n'est
# qu'une reference (les confs npc/*.conf ne sont jamais lues).
SCRIPT_ROOTS = ('moon/scripts_moon.conf',)


def loaded_scripts(conf=None, seen=None):
    """Chemins des scripts charges : lignes npc: atteintes via les import: des confs."""
    if seen is None:
        seen = set()
    if conf is None:
        out = []
        for root in SCRIPT_ROOTS:
            out += loaded_scripts(root, seen)
        return out
    if conf in seen or not os.path.exists(conf):
        return []
    seen.add(conf)
    out = []
    for ln in open(conf, 'rb').read().decode('cp1252', 'replace').split('\n'):
        ln = ln.split('//', 1)[0].strip()
        m = re.match(r'^(import|npc):\s*(\S+)', ln)
        if not m:
            continue
        path = m.group(2).replace('\\', '/')
        if m.group(1) == 'import':
            out += loaded_scripts(path, seen)
        elif os.path.exists(path):
            out.append(path)
    return out


class Report:
    def __init__(self):
        self.issues = []

    def head(self, title):
        print('\n=== %s ===' % title)

    def ok(self, msg):
        print('  OK   %s' % msg)

    def ko(self, msg, detail=''):
        self.issues.append(msg)
        print('  KO   %s' % msg)
        if detail:
            print('       %s' % detail)

    def info(self, msg):
        print('       %s' % msg)


# ------------------------------------------------------------- script ----
def parse_script(path):
    src = open(path, 'rb').read().decode('cp1252', 'replace')
    code = '\n'.join(re.sub(r'//.*$', '', ln) for ln in src.split('\n'))

    # Les ID sont souvent portes par des variables (.@guard = 3510; ... Easy/Hard)
    # -> on collecte toutes les valeurs affectees a chaque variable.
    var_ids = {}
    for m in re.finditer(r"(\.@?[A-Za-z_]\w*)\s*=\s*(\d{3,5})\s*;", code):
        var_ids.setdefault(m.group(1), set()).add(int(m.group(2)))

    mobs = {}
    pat = re.compile(
        r'\b(?:area)?monster\s*(?:\(\s*)?'
        r'(?:"[^"]*"|\'?[\w.@$\[\]]+\$?)\s*,'
        r'(?:\s*[^,]+,){2,4}?'
        r'\s*"([^"]*)"\s*,\s*(\d+|\.@?[A-Za-z_]\w*)\s*,')
    for m in pat.finditer(code):
        ref = m.group(2)
        ids = [int(ref)] if ref.isdigit() else sorted(var_ids.get(ref, ()))
        for i in ids:
            mobs.setdefault(i, set()).add(m.group(1))
    for m in re.finditer(r'getmonsterinfo\s*\(\s*(\d+)', code):
        mobs.setdefault(int(m.group(1)), set())

    items = set()
    for kw in ('getitem2?', 'delitem2?', 'countitem2?', 'getitembound', 'makeitem',
               'rentitem', 'checkweight', 'getnameditem'):
        for m in re.finditer(r'\b%s\s*(?:\(\s*)?(\d+)\s*[,)]' % kw, code):
            items.add(int(m.group(1)))
    # getitem par AegisName
    item_names = set()
    for kw in ('getitem2?', 'delitem2?', 'countitem2?', 'getitembound'):
        for m in re.finditer(r'\b%s\s*(?:\(\s*)?([A-Z][A-Za-z0-9_]+)\s*[,)]' % kw, code):
            item_names.add(m.group(1))

    quests = set()
    for kw in ('setquest', 'erasequest', 'completequest', 'checkquest', 'isbegin_quest',
               'questprogress', 'changequest'):
        for m in re.finditer(r'\b%s\s*(?:\(\s*)?(\d+)' % kw, code):
            quests.add(int(m.group(1)))
        for m in re.finditer(r'\bchangequest\s*(?:\(\s*)?\d+\s*,\s*(\d+)', code):
            quests.add(int(m.group(1)))

    maps = set(re.findall(r'"(\d@[a-z0-9_]+)"', code))
    names = set(re.findall(r'instance_create\s*\(\s*"([^"]+)"', code))
    for m in re.finditer(r'\.@md_name\$\s*=\s*"([^"]+)"', code):
        names.add(m.group(1))
    npc_maps = set(re.findall(r'^([a-z0-9_@]+),\d+,\d+,\d+\tscript\t', src, re.M))

    # sprites de NPC : dernier champ d'une ligne script/duplicate
    views = {}
    for m in re.finditer(r'^[a-z0-9_@]+,\d+,\d+,\d+\t(?:script|duplicate\([^)]*\))'
                         r'\t[^\t]+\t([A-Za-z_]\w*)', src, re.M):
        views[m.group(1)] = views.get(m.group(1), 0) + 1
    mercs = {int(m.group(1)) for m in re.finditer(r'mercenary_create\s+(\d+)', code)}
    return mobs, items, item_names, quests, maps, names, npc_maps, views, mercs


# ------------------------------------------------------------------ main --
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('script')
    ap.add_argument('--name', help='nom instance_db (deduit du script sinon)')
    a = ap.parse_args()
    R = Report()
    script = a.script.replace('\\', '/')
    base = os.path.basename(script)

    (mobs, items, item_names, quests, maps, names, npc_maps,
     views, mercs) = parse_script(script)
    inst_names = {a.name} if a.name else names

    # ------------------------------------------------ bases locales -------
    mob_db = {m['Id']: m for m in load('db/import/mob_db.yml') + load('db/mob_db.yml')}
    item_db = {i['Id']: i for i in load('db/import/item_db.yml') + load('db/item_db.yml')}
    aegis = {i['AegisName']: i for i in item_db.values()}
    quest_db = {q['Id']: q for q in load('db/import/quest_db.yml') + load('db/quest_db.yml')}
    inst_db = {i['Name']: i for i in load('db/import/instance_db.yml') + load('db/instance_db.yml')}
    rog = {g['Group'] for g in load('db/import/item_randomopt_group.yml')}

    re_mob = {m['Id']: m for m in load_re('db/re/mob_db.yml')}
    re_item = {i['Id']: i for i in load_re('db/re/item_db_equip.yml', 'db/re/item_db_etc.yml',
                                           'db/re/item_db_usable.yml')}
    re_aegis = {i['AegisName']: i for i in re_item.values()}
    re_quest = {q['Id']: q for q in load_re('db/re/quest_db.yml')}
    re_inst = {i['Name']: i for i in load_re('db/re/instance_db.yml')}

    mapidx = set(re.findall(r'^(\S+)', open('db/map_index.txt', encoding='latin-1').read(), re.M))

    def mob_skills(path):
        d = {}
        if os.path.exists(path):
            for ln in open(path, encoding='latin-1'):
                f = ln.strip().split(',')
                if f and f[0].isdigit():
                    d.setdefault(int(f[0]), []).append(ln.rstrip('\n'))
        return d
    msk_loc = mob_skills('db/pre-re/mob_skill_db.txt')
    msk_loc.update({k: v for k, v in mob_skills('db/import/mob_skill_db.txt').items()})
    msk_re = mob_skills('db/re/mob_skill_db.txt')

    print('Audit : %s' % script)
    print('references : %d mobs, %d items, %d quetes, %d maps d\'instance, NPC sur %s'
          % (len(mobs), len(items) + len(item_names), len(quests), len(maps),
             ', '.join(sorted(npc_maps)) or '?'))

    # ------------------------------------------------ 1. chargement -------
    R.head('CHARGEMENT')
    # "charge" se juge sur la conf, pas sur l'extension : 327 .txt sont declares.
    # Le .npc n'est que la convention pour les fichiers portes dans moon/.
    loaded = {p.lower() for p in loaded_scripts()}
    if script.lower() in loaded:
        R.ok('declare dans une conf chargee')
    else:
        R.ko('declare dans aucune conf chargee (scripts_moon.conf attendu)')
    if not base.endswith('.npc'):
        R.ko('extension %s : la convention moon/ est .npc' % os.path.splitext(base)[1])

    # ------------------------------------------------ 2. instance_db ------
    R.head('INSTANCE_DB')
    for n in sorted(inst_names):
        if n in inst_db:
            R.ok('"%s" (Id %s)' % (n, inst_db[n]['Id']))
        elif n in re_inst:
            e = re_inst[n]
            R.ko('"%s" absente du local' % n,
                 'db/re : Id %s, Enter %s, AdditionalMaps %s' % (
                     e['Id'], e['Enter']['Map'], list((e.get('AdditionalMaps') or {}).keys())))
        else:
            R.ko('"%s" introuvable, meme en db/re' % n)
    if not inst_names:
        R.ko('nom d\'instance non detecte (--name)')

    # ------------------------------------------------ 3. maps -------------
    R.head('MAPS')
    flags = {}
    for f in glob.glob('moon/mapflag/*'):
        for ln in open(f, 'rb').read().decode('cp1252', 'replace').split('\n'):
            m = re.match(r'^(\S+)\tmapflag\t(\S+)', ln)
            if m:
                flags.setdefault(m.group(1), set()).add(m.group(2))
    for mp in sorted(maps):
        fl = flags.get(mp, set())
        if mp not in mapidx:
            R.ko('%s absente de map_index.txt' % mp)
        elif not fl:
            R.ko('%s : aucun mapflag dans moon/mapflag/' % mp)
        else:
            R.ok('%-10s %d mapflags' % (mp, len(fl)))

    # ------------------------------------------------ 4. mobs -------------
    R.head('MOBS')
    missing_mobs, stub_mobs = [], []
    for i in sorted(mobs):
        m = mob_db.get(i)
        if m is None:
            missing_mobs.append(i)
            R.ko('%d absent%s' % (i, '' if i in re_mob else ' -- ET absent de db/re'),
                 ', '.join(sorted(mobs[i])) if mobs[i] else '')
        elif all(m.get(k) == v for k, v in STUB_STATS.items()):
            stub_mobs.append(i)
            R.ko('%d %s : stats GABARIT (Str 598 / Dex 857 / Atk 5000 / Def 35)'
                 % (i, m['AegisName']))
        else:
            R.ok('%d %s' % (i, m['AegisName']))
    todo_mobs = sorted(set(missing_mobs + stub_mobs))

    # ------------------------------------------------ 5. mob skills -------
    R.head('MOB SKILLS')
    for i in sorted(mobs):
        if i in msk_loc:
            R.ok('%d : %d ligne(s)' % (i, len(msk_loc[i])))
        elif i in msk_re:
            R.ko('%d : aucun skill local, %d ligne(s) en db/re' % (i, len(msk_re[i])))
        else:
            R.info('%d : aucun skill, ni ici ni en db/re' % i)

    # ------------------------------------------------ 6. drops ------------
    R.head('DROPS DES MOBS (AegisName)')
    drop_names, rog_used = {}, set()
    for i in sorted(mobs):
        src = mob_db.get(i) if i not in todo_mobs else re_mob.get(i)
        if not src:
            continue
        for d in (src.get('Drops') or []) + (src.get('MvpDrops') or []):
            drop_names.setdefault(d['Item'], set()).add(i)
            if d.get('RandomOptionGroup'):
                rog_used.add(d['RandomOptionGroup'])
    for n in sorted(drop_names):
        if n in aegis:
            continue
        cand = re_aegis.get(n)
        same = None
        if cand and cand['Id'] in item_db:
            same = item_db[cand['Id']]['AegisName']
        if same:
            R.ko('%s absent -- MEME ID %d present sous le nom %s' % (n, cand['Id'], same))
        elif cand:
            R.ko('%s absent (db/re Id %d, libre ici)' % (n, cand['Id']))
        else:
            R.ko('%s absent, introuvable meme en db/re' % n)
    n_ok = sum(1 for n in drop_names if n in aegis)
    R.info('%d/%d noms de drop resolus' % (n_ok, len(drop_names)))

    R.head('RANDOM OPTION GROUP')
    if not rog_used:
        R.info('aucun groupe reclame par ces drops')
    for g in sorted(rog_used):
        (R.ok if g in rog else R.ko)('%s' % g)

    # ------------------------------------------------ 7. items du script --
    R.head('ITEMS DU SCRIPT')
    for i in sorted(items):
        if i in item_db:
            R.ok('%d %s' % (i, item_db[i]['AegisName']))
        elif i in re_item:
            R.ko('%d absent (db/re : %s)' % (i, re_item[i]['AegisName']))
        else:
            R.ko('%d absent, introuvable meme en db/re' % i)
    for n in sorted(item_names):
        if n in aegis:
            R.ok('%s' % n)
        elif n in re_aegis:
            R.ko('%s absent (db/re Id %d)' % (n, re_aegis[n]['Id']))
        else:
            R.ko('%s absent, introuvable meme en db/re' % n)

    # ------------------------------------------------ 8. quetes -----------
    R.head('QUETES')
    for q in sorted(quests):
        if q in quest_db:
            R.ok('%d %s' % (q, quest_db[q].get('Title', '')))
        elif q in re_quest:
            e = re_quest[q]
            R.ko('%d absente (db/re : "%s"%s)' % (
                q, e.get('Title', ''), ', TimeLimit %s' % e['TimeLimit'] if e.get('TimeLimit') else ''))
        else:
            R.ko('%d absente, introuvable meme en db/re' % q)

    # ------------------------------------------------ 9. cartes + combos --
    R.head('CARTES ET COMBOS')
    cards = sorted(n for n in drop_names if n.lower().endswith('_card'))
    sc = open('src/map/script_constants.hpp', encoding='utf-8', errors='replace').read()
    skill_impl = ''
    for f in glob.glob('src/**/*.cpp', recursive=True):
        skill_impl += open(f, encoding='utf-8', errors='replace').read()
    for c in cards:
        src = aegis.get(c) or re_aegis.get(c)
        if not src:
            continue
        s = src.get('Script') or ''
        bad = []
        if re.search(r'\bbonus\s+bAtkRate\b', s):
            bad.append('bAtkRate (refuse en pre-re, pc.cpp)')
        for t in TRAIT_BONUS:
            if re.search(r'\bbonus\d?\s+%s\b' % t, s):
                bad.append('%s (trait 4e job, jamais lu en pre-re)' % t)
        for sk in re.findall(r'"([A-Z][A-Z0-9_]+)"', s):
            if re.match(r'^[A-Z]{2,3}_', sk) and not re.search(r'\b%s\b' % sk, skill_impl):
                bad.append('%s : aucune implementation dans src/' % sk)
        where = 'local' if c in aegis else 'db/re seulement'
        if bad:
            R.ko('%s (%s)' % (c, where), ' ; '.join(bad))
        else:
            R.ok('%s (%s) : script sain' % (c, where))
    if cards:
        loc_combos = []
        for e in load('db/import/item_combos.yml') + load('db/item_combos.yml'):
            for cb in (e.get('Combos') or []):
                loc_combos.append(frozenset(cb['Combo']))
        for e in load_re('db/re/item_combos.yml'):
            script = (e.get('Script') or '').strip().replace('\n', ' ')
            for cb in (e.get('Combos') or []):
                s = frozenset(cb['Combo'])
                if not (s & set(cards)):
                    continue
                lacking = sorted(x for x in s if x not in aegis and x not in drop_names)
                if s in loc_combos:
                    R.ok('combo %s deja local' % ' + '.join(sorted(s)))
                elif lacking:
                    R.info('combo %s : non portable (%s)' % (' + '.join(sorted(s)), lacking))
                else:
                    R.ko('combo PORTABLE non porte : %s' % ' + '.join(sorted(s)), script[:80])
    else:
        R.info('aucune carte dans les drops')

    # ------------------------------------------------ 9b. sprites de NPC ---
    # npc_parseview (npc.cpp:3917) : entier, sinon script_get_constant(), sinon
    # AegisName d'un mob CHARGE. Sinon -> warning au boot et NPC INVISIBLE.
    R.head('SPRITES DE NPC')
    sc_txt = open('src/map/script_constants.hpp', encoding='utf-8', errors='replace').read()
    known = set(re.findall(r'export_constant2?\(\s*"([^"]+)"', sc_txt))
    known |= set(re.findall(r'export_constant\(\s*(\w+)\s*\)', sc_txt))
    # export_constant_npc(JT_X) exporte "X" : la macro retire le prefixe JT_
    known |= {n[3:] for n in re.findall(r'export_constant_npc\(\s*(JT_\w+)\s*\)', sc_txt)}
    known |= {e['Name'] for e in load('db/const.yml') if isinstance(e, dict) and 'Name' in e}
    mob_names = {m['AegisName'] for m in mob_db.values()}
    bad_views = {v: n for v, n in views.items() if v not in known and v not in mob_names}
    for v, n in sorted(bad_views.items(), key=lambda x: -x[1]):
        src_v = ('mob db/re Id %d' % next(i for i, m in re_mob.items() if m['AegisName'] == v)
                 if any(m['AegisName'] == v for m in re_mob.values()) else 'INTROUVABLE')
        R.ko('%s x%d -> NPC invisible (%s)' % (v, n, src_v))
    R.info('%d sprite(s) distinct(s), %d non resolu(s)' % (len(views), len(bad_views)))

    # ------------------------------------------------ 9c. mercenaires ------
    # mercenary_create sort en SILENCE si la classe n'existe pas (script.cpp).
    R.head('MERCENAIRES')
    merc_db = {m['Id']: m['AegisName'] for m in load('db/mercenary_db.yml')}
    re_merc = {m['Id']: m['AegisName'] for m in load_re('db/re/mercenary_db.yml')}
    for i in sorted(mercs):
        if i in merc_db:
            R.ok('%d %s' % (i, merc_db[i]))
        else:
            R.ko('%d absent -> mercenary_create ne fait RIEN, sans message (%s)'
                 % (i, 'db/re : %s' % re_merc[i] if i in re_merc else 'nulle part'))
    if not mercs:
        R.info('aucun mercenary_create')

    # ------------------------------------------------ 10. scripts associes -
    # Un enchanteur, un marchand ou une quete de l'episode peut vivre ailleurs
    # dans npc/re (jamais charge) et consommer les items de l'instance.
    R.head('AUTRES SCRIPTS npc/re CITANT CES ITEMS')
    keys = set()
    for i in items:
        keys.add(str(i))
        if i in item_db:
            keys.add(item_db[i]['AegisName'])
        elif i in re_item:
            keys.add(re_item[i]['AegisName'])
    for n in list(item_names) + list(drop_names):
        keys.add(n)
        src_i = aegis.get(n) or re_aegis.get(n)
        if src_i:
            keys.add(str(src_i['Id']))
    # Un item n'est propre a l'instance que si AUCUN autre mob du serveur ne le
    # droppe : Silver_Bracelet ou Bloody_Page tombent partout, les citer ailleurs
    # ne prouve rien. Les ID numeriques ne sont gardes qu'au-dela de 20000.
    generic = set()
    for mid, m in mob_db.items():
        if mid in mobs:
            continue
        for d in (m.get('Drops') or []) + (m.get('MvpDrops') or []):
            generic.add(d['Item'])
    # ... ni s'il est deja cite par un script CHARGE (matiere de craft, quete...).
    # "Charge" = declare par une ligne npc: atteinte depuis conf/map_athena.conf en
    # suivant les import: -- l'extension ne dit rien (des .txt sont charges).
    loaded_txt = ''
    for f in loaded_scripts():
        if os.path.basename(f) != base:
            loaded_txt += open(f, 'rb').read().decode('cp1252', 'replace')
    # un script cite l'item par son AegisName OU par son Id numerique
    def cited(name):
        forms = [re.escape(name)]
        src_i = aegis.get(name) or re_aegis.get(name)
        if src_i:
            forms.append(str(src_i['Id']))
        return re.search(r'\b(?:%s)\b' % '|'.join(forms), loaded_txt) is not None
    generic |= {k for k in keys if not k.isdigit() and cited(k)}
    keys = {k for k in keys
            if (k.isdigit() and int(k) > 20000) or (not k.isdigit() and k not in generic)}
    pat = re.compile(r'\b(%s)\b' % '|'.join(re.escape(k) for k in sorted(keys))) if keys else None
    # ... et les quetes de l'instance, dans un contexte de commande de quete
    qpat = re.compile(r'\b(?:setquest|checkquest|completequest|erasequest|isbegin_quest|'
                      r'questprogress|changequest)\s*\(?\s*(%s)\b'
                      % '|'.join(str(q) for q in sorted(quests))) if quests else None
    ported = {os.path.splitext(os.path.basename(f))[0].lower()
              for f in glob.glob('moon/**/*.npc', recursive=True)}
    hits = 0
    if pat:
        for f in glob.glob('npc/re/**/*.txt', recursive=True):
            if os.path.basename(f) == base.replace('.npc', '.txt'):
                continue
            txt = open(f, 'rb').read().decode('cp1252', 'replace')
            found = sorted(set(pat.findall(txt)))
            if qpat:
                found += ['quete %s' % q for q in sorted(set(qpat.findall(txt)))]
            if not found:
                continue
            hits += 1
            stem = os.path.splitext(os.path.basename(f))[0].lower()
            if stem in ported:
                R.ok('%s (deja porte dans moon/) : %s' % (f, ', '.join(found[:5])))
            else:
                R.ko('%s non porte : cite %s' % (f, ', '.join(found[:5])))
    if not hits:
        R.info('aucun')

    # ------------------------------------------------ 11. warp agent ------
    R.head('WARP AGENT')
    wa = open('moon/warp_agent.npc', 'rb').read().decode('cp1252', 'replace')
    for n in sorted(inst_names):
        const = 'INST_' + re.sub(r'[^A-Za-z0-9]+', '_', re.sub(r"['’]", '', n)).strip('_').upper()
        (R.ok if const in wa else R.ko)('%s%s' % (const, '' if const in wa else ' absent du warp agent'))

    # ------------------------------------------------ bilan ---------------
    print('\n' + '=' * 70)
    print('%d point(s) a traiter' % len(R.issues))
    if todo_mobs:
        print('mobs a (re)generer via re_to_prere_mob.py : %s' % todo_mobs)


if __name__ == '__main__':
    main()
