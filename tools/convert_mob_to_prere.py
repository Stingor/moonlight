#!/usr/bin/env python3
"""
convert_mob_to_prere.py
=======================
Pipeline complet de conversion d'un fichier mob_db.yml vers le pre-renewal :
  1. restore_re_values  : restaure les valeurs RE (HP, stats, DEF/MDEF, EXP, vitesses)
  2. re_to_prere_mob    : convertit les valeurs RE en pre-renewal

Usage :
  python convert_mob_to_prere.py <fichier.yml>            # modifie en place
  python convert_mob_to_prere.py <fichier.yml> -o <out>   # ecrit dans un nouveau fichier
  python convert_mob_to_prere.py <fichier.yml> --dry-run  # apercu sans modifier
  python convert_mob_to_prere.py <fichier.yml> --no-exp   # ne convertit pas les EXP
  python convert_mob_to_prere.py <fichier.yml> --no-def   # ne convertit pas DEF/MDEF
  python convert_mob_to_prere.py <fichier.yml> --no-hp    # ne convertit pas les HP
"""

import sys
import os
import argparse

# Importer les deux scripts depuis le meme dossier
sys.path.insert(0, os.path.dirname(__file__))
import restore_re_values
import re_to_prere_mob


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


if __name__ == '__main__':
    main()
