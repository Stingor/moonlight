#!/usr/bin/env python3
"""
restore_re_values.py
====================
Restaure Defense, MagicDefense, Hp, BaseExp et JobExp d'un fichier mob_db.yml
custom en reprenant les valeurs renewal depuis db/re/mob_db.yml.

Usage :
  python restore_re_values.py <fichier.yml>            # modifie en place
  python restore_re_values.py <fichier.yml> -o <out>   # ecrit dans un nouveau fichier
  python restore_re_values.py <fichier.yml> --dry-run  # apercu sans modifier
"""

import re
import sys
import os
import argparse

RE_DB_PATH = os.path.join(os.path.dirname(__file__), '..', 'db', 're', 'mob_db.yml')

FIELDS = ['Hp', 'Defense', 'MagicDefense', 'BaseExp', 'JobExp']


def fmt(n):
    """Formate un entier avec des points comme separateurs de milliers."""
    return f"{n:,}".replace(',', '.')


def parse_re_db(path):
    """
    Extrait depuis re/mob_db.yml un dict { mob_id: { field: value, ... } }
    pour les champs qui nous interessent.
    """
    content = open(path, encoding='utf-8').read()
    blocks  = re.split(r'(?m)(?=^  - Id:)', content)
    db = {}
    for b in blocks:
        id_m = re.search(r'- Id:\s*(\d+)', b)
        if not id_m:
            continue
        mob_id = int(id_m.group(1))
        db[mob_id] = {}
        for field in FIELDS:
            m = re.search(r'^\s+' + field + r':\s*(\d+)', b, re.MULTILINE)
            if m:
                db[mob_id][field] = int(m.group(1))
    return db


def split_mob_blocks(content):
    body_match = re.search(r'^Body:\s*\n', content, re.MULTILINE)
    if not body_match:
        return content, []
    header = content[:body_match.end()]
    body   = content[body_match.end():]
    blocks = re.split(r'(?=^  - Id:)', body, flags=re.MULTILINE)
    return header, blocks


def restore_block(block, re_db):
    id_m = re.search(r'- Id:\s*(\d+)', block)
    if not id_m:
        return block, None
    mob_id = int(id_m.group(1))
    if mob_id not in re_db:
        return block, f"  [{mob_id}] non trouve dans re/mob_db.yml — ignore"

    aegis_m = re.search(r'AegisName:\s*(\S+)', block)
    aegis   = aegis_m.group(1) if aegis_m else '???'
    lv_m    = re.search(r'^\s+Level:\s*(\d+)', block, re.MULTILINE)
    lv      = lv_m.group(1) if lv_m else '?'

    re_vals  = re_db[mob_id]
    changes  = []
    new_block = block

    for field in FIELDS:
        if field not in re_vals:
            continue
        re_val  = re_vals[field]
        cur_m   = re.search(r'^\s+' + field + r':\s*(\d+)', new_block, re.MULTILINE)
        cur_val = int(cur_m.group(1)) if cur_m else None

        if cur_val is None:
            continue  # champ absent du fichier custom, on ne touche pas

        if cur_val != re_val:
            new_block = re.sub(
                r'(^\s+' + field + r':\s*)\d+([^\n]*)',
                lambda m, v=re_val: f"{m.group(1)}{v}",
                new_block, count=1, flags=re.MULTILINE
            )
            changes.append(f"{field} {fmt(cur_val):>13} -> {fmt(re_val):>13}")

    if changes:
        report = f"  [{mob_id}] {aegis:<35} lv{lv} -- " + ', '.join(changes)
    else:
        report = None

    return new_block, report


def restore_file(input_path, output_path=None, dry_run=False):
    re_db_path = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(input_path)), '..', '..', '..', 'db', 're', 'mob_db.yml'))
    if not os.path.isfile(re_db_path):
        re_db_path = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'db', 're', 'mob_db.yml'))
    if not os.path.isfile(re_db_path):
        print(f"Erreur : re/mob_db.yml introuvable : {re_db_path}")
        sys.exit(1)

    print(f"Source renewal : {re_db_path}")
    re_db = parse_re_db(re_db_path)

    content = open(input_path, encoding='utf-8').read()
    header, blocks = split_mob_blocks(content)

    if not blocks:
        print("Aucun bloc 'Body:' trouve.")
        return

    new_blocks = []
    reports    = []

    for block in blocks:
        new_block, report = restore_block(block, re_db)
        new_blocks.append(new_block)
        if report:
            reports.append(report)

    new_content = header + ''.join(new_blocks)

    print(f"\n=== Restauration : {os.path.basename(input_path)} ===")
    if reports:
        print(f"{len(reports)} mob(s) modifie(s) :\n")
        for r in reports:
            print(r)
    else:
        print("Aucune difference trouvee (valeurs deja identiques a re/mob_db.yml).")

    if dry_run:
        print("\n[dry-run] Fichier non modifie.")
        return

    dest = output_path or input_path
    with open(dest, 'w', encoding='utf-8') as f:
        f.write(new_content)

    if output_path:
        print(f"\nFichier ecrit dans : {output_path}")
    else:
        print(f"\nFichier modifie en place : {input_path}")


def main():
    parser = argparse.ArgumentParser(
        description='Restaure les valeurs renewal (Hp/Def/MDef/Exp) depuis db/re/mob_db.yml'
    )
    parser.add_argument('input',  help='Fichier YML custom a restaurer')
    parser.add_argument('-o', '--output', default=None, help='Fichier de sortie (defaut : en place)')
    parser.add_argument('--dry-run', action='store_true', help='Apercu sans modifier le fichier')
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Erreur : fichier introuvable : {args.input}")
        sys.exit(1)

    restore_file(args.input, args.output, args.dry_run)


if __name__ == '__main__':
    main()
