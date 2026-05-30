import re

def parse_blocks(path):
    content = open(path, encoding='utf-8').read()
    body = re.search(r'^Body:\s*\n', content, re.MULTILINE)
    if not body: return {}
    blocks = re.split(r'(?=^  - Id:)', content[body.end():], flags=re.MULTILINE)
    result = {}
    for b in blocks:
        id_m = re.search(r'- Id:\s*(\d+)', b)
        if not id_m: continue
        mid = int(id_m.group(1))
        def fi(f):
            m = re.search(r'^\s+(?:- )?' + f + r':\s*(\d+)', b, re.MULTILINE)
            return int(m.group(1)) if m else None
        aegis_m = re.search(r'AegisName:\s*(\S+)', b)
        mvp = bool(re.search(r'^\s+Mvp:\s*true', b, re.MULTILINE | re.IGNORECASE))
        boss = bool(re.search(r'^\s+Class:\s*Boss', b, re.MULTILINE))
        tier = 'MVP' if mvp else ('Boss' if boss else 'Normal')
        result[mid] = {
            'Hp': fi('Hp'), 'BaseExp': fi('BaseExp'), 'JobExp': fi('JobExp'),
            'AegisName': aegis_m.group(1) if aegis_m else '???',
            'tier': tier
        }
    return result

re_db = parse_blocks('db/re/mob_db.yml')
ami   = parse_blocks('db/import/mobs/amicitia.yml')
ba    = parse_blocks('db/import/mobs/ba_dun.yml')

combined = {**ami, **ba}

print('=== RATIOS HP / BaseExp / JobExp (converted / renewal) ===')
print(f"  {'ID':<8} {'AegisName':<30} {'Tier':<7} {'HP ratio':>10} {'Base ratio':>11} {'Job ratio':>10}")
print('  ' + '-'*80)

hp_ratios   = {'Normal': [], 'Boss': [], 'MVP': []}
base_ratios = []
job_ratios  = []

for mid, cur in combined.items():
    if mid not in re_db: continue
    rv = re_db[mid]
    if not rv['Hp']: continue
    rh = cur['Hp'] / rv['Hp']
    rb = cur['BaseExp'] / rv['BaseExp'] if rv.get('BaseExp') else 0
    rj = cur['JobExp'] / rv['JobExp'] if rv.get('JobExp') else 0
    tier = cur['tier']
    print(f"  {mid:<8} {cur['AegisName']:<30} {tier:<7} {rh:>10.4f} {rb:>11.4f} {rj:>10.4f}")
    hp_ratios[tier].append(rh)
    if rb: base_ratios.append(rb)
    if rj: job_ratios.append(rj)

print()
print('=== MOYENNES ===')
for tier, vals in hp_ratios.items():
    if vals:
        print(f"  HP {tier:<8}: avg={sum(vals)/len(vals):.4f}  min={min(vals):.4f}  max={max(vals):.4f}  (n={len(vals)})")
if base_ratios:
    print(f"  BaseExp    : avg={sum(base_ratios)/len(base_ratios):.4f}  min={min(base_ratios):.4f}  max={max(base_ratios):.4f}  (n={len(base_ratios)})")
if job_ratios:
    print(f"  JobExp     : avg={sum(job_ratios)/len(job_ratios):.4f}  min={min(job_ratios):.4f}  max={max(job_ratios):.4f}  (n={len(job_ratios)})")
