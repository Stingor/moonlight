#!/usr/bin/env python3
"""
convert_mob_to_prere.py
=======================
Pipeline complet de conversion d'un fichier mob_db.yml vers le pre-renewal :
  1. restore_re_values  : restaure les valeurs RE (HP, stats, DEF/MDEF, EXP, vitesses)
  2. re_to_prere_mob    : convertit les valeurs RE en pre-renewal
  3. rapport final      : compare les valeurs originales aux valeurs finales

Usage :
  python convert_mob_to_prere.py <fichier.yml>            # modifie en place
  python convert_mob_to_prere.py <fichier.yml> -o <out>   # ecrit dans un nouveau fichier
  python convert_mob_to_prere.py <fichier.yml> --dry-run  # apercu sans modifier
  python convert_mob_to_prere.py <fichier.yml> --no-exp   # ne convertit pas les EXP
  python convert_mob_to_prere.py <fichier.yml> --no-def   # ne convertit pas DEF/MDEF
  python convert_mob_to_prere.py <fichier.yml> --no-hp    # ne convertit pas les HP
"""

import re
import sys
import os
import argparse

# Importer les deux scripts depuis le meme dossier
sys.path.insert(0, os.path.dirname(__file__))
import restore_re_values
import re_to_prere_mob

SNAPSHOT_FIELDS = [
    'Hp', 'Defense', 'MagicDefense', 'BaseExp', 'JobExp',
    'Str', 'Agi', 'Vit', 'Int', 'Dex', 'Luk',
    'WalkSpeed', 'AttackDelay', 'AttackMotion', 'DamageMotion',
]

FIELD_ABBR = {
    'Hp': 'HP', 'Defense': 'DEF', 'MagicDefense': 'MDEF',
    'BaseExp': 'Base', 'JobExp': 'Job',
    'WalkSpeed': 'Spd', 'AttackDelay': 'AtkDly',
    'AttackMotion': 'AtkMot', 'DamageMotion': 'DmgMot',
    'Str': 'STR', 'Agi': 'AGI', 'Vit': 'VIT',
    'Int': 'INT', 'Dex': 'DEX', 'Luk': 'LUK',
}


def fmt(n):
    return f"{n:,}".replace(',', '.')


def parse_snapshot(content):
    """
    Retourne un dict { mob_id: { 'aegis': str, 'level': int, fields... } }
    depuis le contenu brut d'un fichier mob_db.yml.
    """
    _, blocks = re_to_prere_mob.split_mob_blocks(content)
    snapshot = {}
    for block in blocks:
        mob_id = re_to_prere_mob.parse_int_field(block, 'Id')
        if mob_id is None:
            continue
        aegis_m = re.search(r'AegisName:\s*(\S+)', block)
        level   = re_to_prere_mob.parse_int_field(block, 'Level')
        entry   = {
            'aegis': aegis_m.group(1) if aegis_m else '???',
            'level': level or 0,
        }
        for field in SNAPSHOT_FIELDS:
            entry[field] = re_to_prere_mob.parse_int_field(block, field)
        snapshot[mob_id] = entry
    return snapshot


def print_final_report(before, after):
    print()
    print("=" * 60)
    print("RAPPORT FINAL  (original -> apres conversion)")
    print("=" * 60)

    changed = 0
    for mob_id in sorted(before):
        if mob_id not in after:
            continue
        b = before[mob_id]
        a = after[mob_id]
        diffs = []
        for field in SNAPSHOT_FIELDS:
            bv, av = b.get(field), a.get(field)
            if bv is None and av is None:
                continue
            if bv != av:
                label = FIELD_ABBR.get(field, field)
                bstr = fmt(bv) if bv is not None else '∅'
                astr = fmt(av) if av is not None else '∅'
                diffs.append(f"{label}:{bstr}→{astr}")
        if diffs:
            changed += 1
            tier = ''
            print(f"  [{mob_id}] {b['aegis']:<30} lv{b['level']:>3}{tier}  " + '  '.join(diffs))

    if changed:
        print(f"\n{changed} mob(s) modifie(s) au total.")
    else:
        print("Aucune difference entre l'original et le resultat final.")


def main():
    parser = argparse.ArgumentParser(
        description='Restaure les valeurs RE puis convertit en pre-renewal'
    )
    parser.add_argument('input',  help='Fichier YML a convertir')
    parser.add_argument('-o', '--output', default=None,
                        help='Fichier de sortie (defaut : modifie en place)')
    parser.add_argument('--dry-run', action='store_true',
                        help='Affiche les changements sans modifier le fichier')
    parser.add_argument('--no-exp', action='store_true',
                        help='Ne convertit pas BaseExp/JobExp')
    parser.add_argument('--no-def', action='store_true',
                        help='Ne convertit pas Defense/MagicDefense')
    parser.add_argument('--no-hp', action='store_true',
                        help='Ne convertit pas les HP')
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Erreur : fichier introuvable : {args.input}")
        sys.exit(1)

    dest = args.output or args.input

    # --- Snapshot avant toute modification ---
    with open(args.input, encoding='utf-8') as f:
        original_content = f.read()
    snapshot_before = parse_snapshot(original_content)

    # --- Etape 1 : restore_re_values ---
    print("=" * 60)
    print("ETAPE 1 : Restauration des valeurs RE")
    print("=" * 60)
    restore_re_values.restore_file(args.input, dest, dry_run=args.dry_run)

    # --- Etape 2 : re_to_prere_mob ---
    print()
    print("=" * 60)
    print("ETAPE 2 : Conversion RE -> pre-renewal")
    print("=" * 60)
    re_to_prere_mob.convert_file(
        dest,
        dry_run=args.dry_run,
        convert_def=not args.no_def,
        convert_exp=not args.no_exp,
        convert_hp=not args.no_hp,
    )

    # --- Rapport final original -> resultat ---
    if not args.dry_run:
        with open(dest, encoding='utf-8') as f:
            final_content = f.read()
    else:
        final_content = original_content  # dry-run : rien n'a change sur disque
    snapshot_after = parse_snapshot(final_content)
    print_final_report(snapshot_before, snapshot_after)


if __name__ == '__main__':
    main()
