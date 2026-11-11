"""Porte vers db/import les donnees renewal qui manquent a MD_C_HEMEL (20572).

- NPC_RAYOFGENESIS (737) et 722, 723, 729, 733 : les stubs pre-re (sans Type) donnent
  "battle_calc_attack: unknown attack type! 0 (skill_id=737, skill_lv=3)".
- Immune_Property_* : NPC_IMMUNE_PROPERTY (754) demarre en dur un
  SC_IMMUNE_PROPERTY_* (src/map/skills/npc/propertyimmune.cpp) absent de la
  base pre-re -> "status_change_start: Invalid status change (922)!".

Les blocs sont recopies tels quels depuis db/re. Idempotent.
Usage : python tools/util/port_hemel_npc_skills.py
"""
import os
import re

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))


def read(p):
    return open(os.path.join(ROOT, p), 'rb').read().decode('utf-8')


def write(p, text):
    path = os.path.join(ROOT, p)
    data = text.encode('utf-8')
    tmp = path + '.tmp'
    with open(tmp, 'wb') as f:
        f.write(data)
    os.replace(tmp, path)


def block(text, start_pat, end_pat):
    m = re.search(start_pat, text, re.M)
    e = re.compile(end_pat, re.M).search(text, m.end())
    return text[m.start():e.start()]


# --- statuts
re_status = read('db/re/status.yml')
statuses = block(re_status, r'^  - Status: Immune_Property_Nothing$', r'^  - Status: Relieve_On$')
imp = read('db/import/status.yml')
if 'Status: Immune_Property_Nothing' not in imp:
    if not imp.endswith('\n'):
        imp += '\n'
    imp += '# NPC_IMMUNE_PROPERTY (754) : statuts demarres en dur par propertyimmune.cpp, absents du pre-re\n'
    imp += statuses
    write('db/import/status.yml', imp)
    print('status.yml : 10 statuts Immune_Property_* ajoutes')
else:
    print('status.yml : deja present')

# --- skills NPC : stubs pre-re sans Type -> "unknown attack type"
SKILLS = {
    722: 'NPC_WIDESUCK',        # Bomi
    723: 'NPC_STORMGUST2',      # ILL_FREEZER, ILL_SEDORA, ILL_TEDDY_BEAR_B
    729: 'NPC_CHEAL',           # ACIDUS_B, ACIDUS_S, DELETER2_H, ...
    733: 'NPC_MAGMA_ERUPTION',  # ILL_HEATER
    737: 'NPC_RAYOFGENESIS',    # MD_C_HEMEL
}
re_skill = read('db/re/skill_db.yml')
imp = read('db/import/skill_db.yml')
changed = False
for sid, name in SKILLS.items():
    if re.search(r'^  - Id: %d$' % sid, imp, re.M):
        print('skill_db.yml : %s (%d) deja present' % (name, sid))
        continue
    entry = block(re_skill, r'^  - Id: %d$' % sid, r'^  - Id: \d+$')
    assert 'Name: %s\n' % name in entry, name
    anchor = re.search(r'^  - Id: 751$', imp, re.M)
    imp = imp[:anchor.start()] + entry + imp[anchor.start():]
    changed = True
    print('skill_db.yml : %s (%d) ajoute' % (name, sid))
if changed:
    write('db/import/skill_db.yml', imp)
