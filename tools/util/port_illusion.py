#!/usr/bin/env python3
"""
port_illusion.py
================
Inventaire du contenu Illusion (episode 16.2) a porter depuis db/re vers
db/import. Lecture seule sur le depot : ce script n'ecrit que dans --out.

Principes :
  - Toutes les bases sont parcourues via leurs `Footer: Imports:` recursifs,
    en ignorant `Mode: Renewal` (ce que le serveur pre-re charge reellement).
  - Un objet est compare par Id ET par AegisName : le fork a souvent le meme Id
    sous un autre nom. Un meme Id de nature differente est une collision, jamais
    ecrasee automatiquement.
  - Les stats des mobs passent par re_to_prere_mob.py, sans retouche.

Usage :
  python tools/util/port_illusion.py                 # rapport seul
  python tools/util/port_illusion.py --out <dossier> # + fichiers candidats
"""
import argparse
import collections
import glob
import os
import re
import subprocess
import sys

import yaml

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

SCRIPTS = [
    'npc/re/quests/quests_illusion_dungeons.txt',
    'npc/re/merchants/enchan_illusion_dungeons.txt',
    'npc/re/quests/illusion_investigation.txt',
]
RE_ITEMS = ['db/re/item_db_equip.yml', 'db/re/item_db_etc.yml', 'db/re/item_db_usable.yml']

# Equipement Doram : classe hors perimetre, retire des drops.
DROPS_RETIRES = {'Doram_High_Cape', 'Doram_High_Shoes', 'Doram_High_Suit'}

# Bonus a controler a la main dans les scripts d'objets.
BONUS_SUSPECTS = ['bAtkRate', 'bRes', 'bMRes', 'bPow', 'bSta', 'bWis', 'bSpl', 'bCon', 'bCrt',
                  'bPAtk', 'bSMatk', 'getenchantgrade']


def read_latin1(path):
    with open(path, encoding='latin-1', newline='') as f:
        return f.read()


def load(path):
    # Quelques descriptions contiennent des octets de controle C1, refuses par PyYAML.
    text = ''.join('?' if 0x7f <= ord(c) <= 0x9f else c for c in read_latin1(path))
    return yaml.safe_load(text) or {}


def walk(path, out, seen=None):
    """Charge une base en suivant ses imports, dans l'ordre ou le serveur le fait."""
    seen = set() if seen is None else seen
    if path in seen or not os.path.exists(path):
        return out
    seen.add(path)
    data = load(path)
    for entry in data.get('Body') or []:
        out[entry['Id']] = entry
    for imp in (data.get('Footer') or {}).get('Imports') or []:
        if imp.get('Mode') != 'Renewal':
            walk(imp['Path'], out, seen)
    return out


def blocks_by_id(path):
    """Blocs YAML bruts d'un fichier db/re, indexes par Id, sans reformatage."""
    text = read_latin1(path)
    blocks = {}
    for block in re.split(r'(?m)^(?=  - Id: \d+)', text)[1:]:
        match = re.match(r'  - Id: (\d+)', block)
        # Le dernier bloc peut embarquer un Footer : on le coupe.
        blocks[int(match.group(1))] = re.split(r'(?m)^Footer:', block)[0]
    return blocks


def strip_comments(text):
    return re.sub(r'//[^\n]*', '', text)


def same_nature(a, b):
    # Type absent = Etc, la valeur par defaut de item_db.
    return a.get('Type', 'Etc') == b.get('Type', 'Etc') and (a.get('Locations') or {}) == (b.get('Locations') or {})


def classify_items(wanted, loaded, by_name):
    """wanted : entrees db/re. Retourne (ok, a_renommer, collisions, nom_autre_id, absents)."""
    res = collections.defaultdict(list)
    for entry in sorted(wanted, key=lambda e: e['Id']):
        local = loaded.get(entry['Id'])
        name = entry['AegisName']
        if local is not None and local['AegisName'] == name:
            res['ok'].append(entry)
        elif local is not None and same_nature(entry, local):
            res['renommer'].append((entry, local))
        elif local is not None:
            res['collision'].append((entry, local))
        elif name in by_name:
            res['nom_autre_id'].append((entry, loaded[by_name[name]]))
        else:
            res['absent'].append(entry)
    return res


def count_references(names):
    """Occurrences de chaque nom local hors fichiers generes, pour chiffrer un renommage."""
    files = [f for pat in ('moon/**/*.npc', 'moon/**/*.txt', 'db/import/**/*.yml', 'db/import/*.txt', 'db/pre-re/quest_db.yml')
             for f in glob.glob(pat, recursive=True)]
    rx = re.compile(r'(?<![\w\'])(' + '|'.join(re.escape(n) for n in names) + r')(?![\w\'])') if names else None
    hits = collections.Counter()
    if rx:
        for path in files:
            hits.update(rx.findall(read_latin1(path)))
    return hits


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--out', help='Dossier ou ecrire les fichiers candidats (jamais db/import)')
    args = parser.parse_args()
    os.chdir(ROOT)

    mobs = walk('db/mob_db.yml', {})
    items = walk('db/item_db.yml', {})
    quests = walk('db/quest_db.yml', {})
    skills = walk('db/skill_db.yml', {})
    items_by_name = {e['AegisName']: i for i, e in items.items()}

    re_mobs = {e['Id']: e for e in load('db/re/mob_db.yml')['Body']}
    re_items = {}
    for path in RE_ITEMS:
        re_items.update({e['Id']: e for e in load(path)['Body']})
    re_items_by_name = {e['AegisName']: e for e in re_items.values()}
    re_quests = {e['Id']: e for e in load('db/re/quest_db.yml')['Body']}
    re_skills = {e['Id']: e.get('Name') for e in load('db/re/skill_db.yml')['Body']}

    # --- Mobs
    ill = sorted(i for i, e in re_mobs.items() if e['AegisName'].startswith('ILL_') and i not in mobs)
    print(f'=== MOBS : {len(ill)} ILL_* absents ({sum(1 for i, e in re_mobs.items() if e["AegisName"].startswith("ILL_"))} en db/re)')

    # --- Compétences de mobs
    skill_lines = []
    skill_ids = collections.Counter()
    for line in read_latin1('db/re/mob_skill_db.txt').splitlines(keepends=True):
        fields = line.split(',')
        if len(fields) > 5 and fields[0].strip().isdigit() and int(fields[0]) in ill:
            skill_lines.append(line)
            skill_ids[int(fields[3])] += 1
    source = ''.join(read_latin1(f) for f in glob.glob('src/map/skills/**/*.cpp', recursive=True))
    source += read_latin1('src/map/skill.cpp') + read_latin1('src/map/battle.cpp') + read_latin1('src/map/status.cpp')
    missing_skills = sorted(i for i in skill_ids if i not in skills)
    print(f'=== MOB SKILLS : {len(skill_lines)} lignes, {len(skill_ids)} skills')
    for i in missing_skills:
        name = re_skills.get(i)
        print(f'  ABSENT skill_db  {i} {name} ({skill_ids[i]} lignes), code C++ : {"oui" if name and name in source else "non"}')
    print('  mobs sans ligne :', [re_mobs[i]['AegisName'] for i in ill if not any(l.startswith(f'{i},') for l in skill_lines)])

    # --- Objets : drops + scripts
    drop_names = set()
    for i in ill:
        for drop in (re_mobs[i].get('Drops') or []) + (re_mobs[i].get('MvpDrops') or []):
            drop_names.add(drop['Item'])
    drop_names -= DROPS_RETIRES

    script_names, script_ids, script_quests = set(), collections.defaultdict(set), set()
    # Les tables de l'enchanteur citent leurs objets en `case <Id>:` et `.@enchant = <Id>;`.
    id_ctx = re.compile(r'(?:\b(?:getitem2?|getitem3|delitem2?|countitem2?|rentitem|getnameditem|setarray|case)\b|\.@\w+\s*=)[^;\n]*')
    for path in SCRIPTS:
        text = strip_comments(read_latin1(path))
        script_names |= {t for t in re.findall(r"[A-Za-z_][\w']*", text) if t in re_items_by_name}
        for ctx in id_ctx.findall(text):
            # Un Id de quete ou un prix peut coincider avec un Id d'objet.
            if 'Zeny' in ctx or 'quest' in ctx or '.@cost' in ctx:
                continue
            for tok in re.findall(r'\b\d{3,7}\b', ctx):
                if int(tok) in re_items:
                    script_ids[int(tok)].add(os.path.basename(path))
        script_quests |= {int(t) for t in re.findall(r'\b(?:setquest|completequest|erasequest|checkquest|isbegin_quest|changequest)\s*\(?\s*(\d{3,6})', text)}

    wanted = {re_items_by_name[n]['Id']: re_items_by_name[n] for n in drop_names | script_names if n in re_items_by_name}
    wanted.update({i: re_items[i] for i in script_ids})
    unresolved = sorted(n for n in drop_names if n not in re_items_by_name and n not in items_by_name)
    cls = classify_items(wanted.values(), items, items_by_name)

    print(f'=== OBJETS : {len(wanted)} references (drops {len(drop_names)}, scripts par nom {len(script_names)}, par Id {len(script_ids)})')
    print(f'  deja charges     {len(cls["ok"])}')
    print(f'  a renommer       {len(cls["renommer"])}')
    print(f'  collisions       {len(cls["collision"])}')
    print(f'  nom sur autre Id {len(cls["nom_autre_id"])}')
    print(f'  absents          {len(cls["absent"])}  {dict(collections.Counter(e.get("Type") for e in cls["absent"]))}')
    if unresolved:
        print('  introuvables     ', unresolved)

    print('--- Collisions (meme Id, autre nature) : a arbitrer')
    for entry, local in cls['collision']:
        print(f'  {entry["Id"]:>7} re={entry["AegisName"]} ({entry.get("Type")}) / local={local["AegisName"]} ({local.get("Type")})')
    print('--- Meme nom, Id different')
    for entry, local in cls['nom_autre_id']:
        print(f'  {entry["AegisName"]} re={entry["Id"]} local={local["Id"]}')

    print('--- A renommer (local -> re), avec occurrences locales')
    refs = count_references([local['AegisName'] for _, local in cls['renommer']])
    for entry, local in cls['renommer']:
        print(f'  {entry["Id"]:>7} {local["AegisName"]} -> {entry["AegisName"]}  [{refs[local["AegisName"]]}]')

    print('--- Absents de type equipement : controle un par un')
    for entry in cls['absent']:
        if entry.get('Type') in ('Armor', 'Weapon'):
            print(f'  {entry["Id"]:>7} {entry["AegisName"]} ({entry.get("Type")})')

    print('--- Bonus a controler')
    to_check = cls['absent'] + [e for e, _ in cls['renommer']]
    for entry in to_check:
        script = entry.get('Script') or ''
        found = [b for b in BONUS_SUSPECTS if re.search(r'\b' + b + r'\b', script)]
        if found:
            print(f'  {entry["Id"]:>7} {entry["AegisName"]} : {found}')

    # --- Quêtes
    q_missing = sorted(q for q in script_quests if q not in quests)
    q_nowhere = [q for q in q_missing if q not in re_quests]
    print(f'=== QUETES : {len(script_quests)} referencees, {len(q_missing)} absentes, dont {len(q_nowhere)} hors db/re : {q_nowhere}')

    # --- Candidats
    if args.out:
        os.makedirs(args.out, exist_ok=True)
        re_mob_blocks = blocks_by_id('db/re/mob_db.yml')
        head = 'Header:\n  Type: MOB_DB\n  Version: 5\n\nBody:\n'
        raw = os.path.join(args.out, 'illusion_mobs_re.yml')
        body = ''.join(re_mob_blocks[i] for i in ill)
        body = re.sub(r'(?m)^      - Item: (?:' + '|'.join(DROPS_RETIRES) + r')\n        Rate: \d+\n', '', body)
        with open(raw, 'w', encoding='latin-1', newline='') as f:
            f.write(head + body)
        conv = os.path.join(args.out, 'illusion_mobs.yml')
        env = dict(os.environ, PYTHONIOENCODING='utf-8')
        subprocess.run([sys.executable, 'tools/util/re_to_prere_mob.py', raw, '-o', conv], check=True,
                       capture_output=True, env=env)
        with open(os.path.join(args.out, 'illusion_mob_skill.txt'), 'w', encoding='latin-1', newline='') as f:
            f.writelines(skill_lines)
        skill_blocks = blocks_by_id('db/re/skill_db.yml')
        with open(os.path.join(args.out, 'illusion_skills.yml'), 'w', encoding='latin-1', newline='') as f:
            f.write('Header:\n  Type: SKILL_DB\n  Version: 4\n\nBody:\n')
            f.writelines(skill_blocks[i] for i in missing_skills)
        item_blocks = {}
        for path in RE_ITEMS:
            item_blocks.update(blocks_by_id(path))
        with open(os.path.join(args.out, 'illusion_items.yml'), 'w', encoding='latin-1', newline='') as f:
            f.write('Header:\n  Type: ITEM_DB\n  Version: 3\n\nBody:\n')
            f.writelines(item_blocks[e['Id']] for e in cls['absent'])
        quest_blocks = blocks_by_id('db/re/quest_db.yml')
        with open(os.path.join(args.out, 'illusion_quests.yml'), 'w', encoding='latin-1', newline='') as f:
            f.write('Header:\n  Type: QUEST_DB\n  Version: 3\n\nBody:\n')
            f.writelines(quest_blocks[q] for q in q_missing if q in quest_blocks)
        print(f'=== Candidats ecrits dans {args.out}')


if __name__ == '__main__':
    main()
