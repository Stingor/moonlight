import re

def parse_blocks(content):
    blocks = re.split(r'(?m)(?=^  - Id:)', content)
    d = {}
    for b in blocks:
        id_m  = re.search(r'- Id:\s*(\d+)', b)
        if not id_m: continue
        lv    = re.search(r'^\s+Level:\s*(\d+)', b, re.M)
        hp    = re.search(r'^\s+Hp:\s*(\d+)', b, re.M)
        aegis = re.search(r'AegisName:\s*(\S+)', b)
        mvp   = bool(re.search(r'Mvp:\s*true', b))
        boss  = bool(re.search(r'Class:\s*Boss', b))
        if lv and hp:
            d[int(id_m.group(1))] = {
                'lv':    int(lv.group(1)),
                'hp':    int(hp.group(1)),
                'mvp':   mvp,
                'boss':  boss,
                'aegis': aegis.group(1) if aegis else '?'
            }
    return d

re_db  = parse_blocks(open(r'D:\Mes documents\Bureau\moonlight\db\re\mob_db.yml',     encoding='utf-8').read())
pre_db = parse_blocks(open(r'D:\Mes documents\Bureau\moonlight\db\pre-re\mob_db.yml', encoding='utf-8').read())
common = sorted(set(re_db) & set(pre_db))

categories = [
    ('NORMAL lv80+', lambda r: not r['mvp'] and not r['boss'] and r['lv'] >= 80),
    ('BOSS lv80+',   lambda r: r['boss'] and not r['mvp'] and r['lv'] >= 80),
    ('MVP lv80+',    lambda r: r['mvp'] and r['lv'] >= 80),
]

for label, filt in categories:
    lst    = [(i, re_db[i], pre_db[i]) for i in common if filt(re_db[i])]
    ratios = [pre_db[i]['hp'] / re_db[i]['hp'] for i, _, __ in lst]
    if not ratios:
        continue
    ratios_s = sorted(ratios)
    avg    = sum(ratios) / len(ratios)
    median = ratios_s[len(ratios_s) // 2]
    print(f'\n=== {label} (n={len(ratios)}) | avg={avg:.3f}  median={median:.3f}  min={min(ratios):.3f}  max={max(ratios):.3f} ===')
    for i, r, p in lst:
        aegis = r['aegis']
        ratio = p['hp'] / r['hp']
        print(f'  [{i}] {aegis:<28} lv{r["lv"]:>3}  RE={r["hp"]:>9}  Pre={p["hp"]:>9}  ratio={ratio:.3f}')

