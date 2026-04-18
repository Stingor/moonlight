#!/usr/bin/env python3
"""
re_to_prere_mob.py
==================
Convertit Defense, MagicDefense, BaseExp et JobExp d'un fichier mob_db.yml
du systÃƒÂ¨me renewal vers des valeurs compatibles pre-renewal.

Formule DEF/MDEF :
  pre_re_def  = round(Level * LV_DEF_COEFF  + Vit * VIT_COEFF)  + class_bonus_def
  pre_re_mdef = round(Level * LV_MDEF_COEFF + Int * INT_COEFF)  + class_bonus_mdef
  - Boss  : +DEF_BOSS_BONUS  / +MDEF_BOSS_BONUS
  - MVP   : +DEF_MVP_BONUS   / +MDEF_MVP_BONUS
  - RÃƒÂ©sultat plafonnÃƒÂ© entre 1 et DEF_CAP / MDEF_CAP

Formule EXP :
  new_base_exp = round(BaseExp * EXP_BASE_MULT)
  new_job_exp  = round(JobExp  * EXP_JOB_MULT)
  (multiplicateurs configurables, par dÃƒÂ©faut 0.70 / 0.50)

Usage :
  python re_to_prere_mob.py <fichier.yml>            # modifie le fichier en place
  python re_to_prere_mob.py <fichier.yml> -o <out>   # ÃƒÂ©crit dans un nouveau fichier
  python re_to_prere_mob.py <fichier.yml> --dry-run  # affiche les changements sans modifier
  python re_to_prere_mob.py <fichier.yml> --no-exp   # ne convertit pas les exp
  python re_to_prere_mob.py <fichier.yml> --no-def   # ne convertit pas DEF/MDEF
"""

import re
import sys
import os
import argparse

# ===========================================================================
# PARAMÃƒË†TRES CONFIGURABLES
# ===========================================================================
LV_DEF_COEFF    = 0.20   # poids du niveau  sur la DEF
VIT_COEFF       = 0.05   # poids du VIT     sur la DEF
LV_MDEF_COEFF   = 0.12   # poids du niveau  sur la MDEF
INT_COEFF       = 0.04   # poids de l'INT   sur la MDEF

DEF_BOSS_BONUS  = 5      # bonus DEF  pour Class: Boss
DEF_MVP_BONUS   = 10     # bonus DEF  pour Mvp: true
MDEF_BOSS_BONUS = 3      # bonus MDEF pour Class: Boss
MDEF_MVP_BONUS  = 7      # bonus MDEF pour Mvp: true

DEF_CAP         = 77     # valeur DEF  maximale autorisee
MDEF_CAP        = 44     # valeur MDEF maximale autorisee

# HP : formule avec poids du niveau
#   raw    = re_hp * HP_MULT_x * (HP_LV_REF / level) ^ HP_LV_EXP
#   new_hp = raw                          si raw <= HP_CAP_x
#          = cap + (raw - cap) * SOFT     sinon  (soft cap)
HP_LV_REF       = 100    # niveau de reference (pas de reduction a ce niveau)
HP_LV_EXP       = 2.0    # exposant du poids du niveau (plus eleve = reduction plus agressive)
HP_MULT_NORMAL  = 0.65   # multiplicateur normal  (lv >= HP_MULT_MIN_LV)
HP_CAP_NORMAL   = 400000 # seuil de soft cap normal
HP_MULT_BOSS    = 0.75   # multiplicateur boss
HP_CAP_BOSS     = 1500000# seuil de soft cap boss
HP_MULT_MVP     = 0.60   # multiplicateur MVP
HP_CAP_MVP      = 8000000# seuil de soft cap MVP
HP_SOFT_FACTOR  = 0.02   # croissance residuelle au-dela du seuil
HP_MULT_MIN_LV  = 100    # niveau minimum pour appliquer la reduction
HP_MIN          = 1      # valeur HP minimale

# EXP : derives des HP finaux (colle automatiquement a la difficulte du mob)
#   BaseExp = new_hp * EXP_BASE_PER_HP
#   JobExp  = BaseExp * EXP_JOB_RATIO
EXP_BASE_PER_HP = 1.7    # base exp = HP * ce ratio (ex: 200k HP -> 400k base exp)
EXP_JOB_RATIO   = 0.65   # job exp  = base exp * ce ratio
EXP_MIN         = 1      # valeur minimale pour BaseExp/JobExp
# ===========================================================================


def fmt(n):
    """Formate un entier avec des points comme separateurs de milliers."""
    return f"{n:,}".replace(',', '.')


def compute_prere(level, vit, int_, is_boss, is_mvp):
    """Calcule les nouvelles valeurs pre-re DEF et MDEF."""
    def_val  = level * LV_DEF_COEFF  + vit  * VIT_COEFF
    mdef_val = level * LV_MDEF_COEFF + int_ * INT_COEFF

    if is_mvp:
        def_val  += DEF_MVP_BONUS
        mdef_val += MDEF_MVP_BONUS
    elif is_boss:
        def_val  += DEF_BOSS_BONUS
        mdef_val += MDEF_BOSS_BONUS

    def_val  = max(1, min(DEF_CAP,  round(def_val)))
    mdef_val = max(1, min(MDEF_CAP, round(mdef_val)))
    return def_val, mdef_val


def split_mob_blocks(content):
    """
    DÃƒÂ©coupe le fichier en deux parties : l'en-tÃƒÂªte (avant Body:)
    et une liste de blocs mob bruts.
    Retourne (header_str, [bloc1, bloc2, ...])
    """
    body_match = re.search(r'^Body:\s*\n', content, re.MULTILINE)
    if not body_match:
        return content, []

    header = content[:body_match.end()]
    body   = content[body_match.end():]

    # Chaque bloc commence par "  - Id:" (2 espaces)
    blocks = re.split(r'(?=^  - Id:)', body, flags=re.MULTILINE)
    return header, blocks


def parse_int_field(block, field):
    """Extrait la valeur entiÃƒÂ¨re d'un champ YAML simple (ex: Level: 145 ou - Id: 145)."""
    m = re.search(r'^\s+(?:- )?' + field + r':\s*(\d+)', block, re.MULTILINE)
    return int(m.group(1)) if m else None


def has_flag(block, flag):
    """VÃƒÂ©rifie si un flag boolÃƒÂ©en est prÃƒÂ©sent et ÃƒÂ  true."""
    m = re.search(r'^\s+' + flag + r':\s*(true|false)', block, re.MULTILINE | re.IGNORECASE)
    return bool(m and m.group(1).lower() == 'true')


def convert_block(block, dry_run=False, convert_def=True, convert_exp=True, convert_hp=True):
    """
    Convertit Defense, MagicDefense, Hp, BaseExp et JobExp d'un bloc mob.
    Retourne (nouveau_bloc, rapport_str ou None).
    """
    mob_id   = parse_int_field(block, 'Id')
    level    = parse_int_field(block, 'Level')
    vit      = parse_int_field(block, 'Vit')
    int_     = parse_int_field(block, 'Int')
    old_def  = parse_int_field(block, 'Defense')
    old_mdef = parse_int_field(block, 'MagicDefense')
    old_hp   = parse_int_field(block, 'Hp')
    old_base = parse_int_field(block, 'BaseExp')
    old_job  = parse_int_field(block, 'JobExp')

    is_mvp        = has_flag(block, 'Mvp')
    is_class_boss = bool(re.search(r'^\s+Class:\s*Boss', block, re.MULTILINE))

    report_parts = []
    new_block = block

    # --- DEF / MDEF ---
    if convert_def and level is not None and vit is not None and int_ is not None:
        if old_def is not None or old_mdef is not None:
            new_def, new_mdef = compute_prere(level, vit, int_, is_class_boss, is_mvp)

            if old_def is not None:
                new_block = re.sub(
                    r'(^\s+Defense:\s*)\d+([^\n]*)',
                    lambda m: f"{m.group(1)}{new_def}",
                    new_block, count=1, flags=re.MULTILINE
                )
                if old_def != new_def:
                    report_parts.append(f"DEF {fmt(old_def):>6} -> {fmt(new_def):>5}")

            if old_mdef is not None:
                new_block = re.sub(
                    r'(^\s+MagicDefense:\s*)\d+([^\n]*)',
                    lambda m: f"{m.group(1)}{new_mdef}",
                    new_block, count=1, flags=re.MULTILINE
                )
                if old_mdef != new_mdef:
                    report_parts.append(f"MDEF {fmt(old_mdef):>6} -> {fmt(new_mdef):>5}")

    # --- HP ---
    new_hp = None
    if convert_hp and old_hp is not None and level is not None and level >= HP_MULT_MIN_LV:
        if is_mvp:
            mult, cap = HP_MULT_MVP, HP_CAP_MVP
        elif is_class_boss:
            mult, cap = HP_MULT_BOSS, HP_CAP_BOSS
        else:
            mult, cap = HP_MULT_NORMAL, HP_CAP_NORMAL
        raw = old_hp * mult * (HP_LV_REF / level) ** HP_LV_EXP
        if raw <= cap:
            new_hp = max(HP_MIN, round(raw))
        else:
            new_hp = max(HP_MIN, round(cap + (raw - cap) * HP_SOFT_FACTOR))
        new_block = re.sub(
            r'(^\s+Hp:\s*)\d+([^\n]*)',
            lambda m: f"{m.group(1)}{new_hp}",
            new_block, count=1, flags=re.MULTILINE
        )
        if old_hp != new_hp:
            report_parts.append(f"HP {fmt(old_hp):>13} -> {fmt(new_hp):>13}")

    # --- BaseExp / JobExp (bases sur les HP finaux) ---
    if convert_exp:
        hp_ref = new_hp if new_hp is not None else old_hp
        if hp_ref and hp_ref > 0:
            new_base = max(EXP_MIN, round(hp_ref * EXP_BASE_PER_HP))
            new_job  = max(EXP_MIN, round(new_base * EXP_JOB_RATIO))
            if old_base is not None and old_base > 0:
                new_block = re.sub(
                    r'(^\s+BaseExp:\s*)\d+([^\n]*)',
                    lambda m: f"{m.group(1)}{new_base}",
                    new_block, count=1, flags=re.MULTILINE
                )
                if old_base != new_base:
                    report_parts.append(f"Base {fmt(old_base):>13} -> {fmt(new_base):>13}")
            if old_job is not None and old_job > 0:
                new_block = re.sub(
                    r'(^\s+JobExp:\s*)\d+([^\n]*)',
                    lambda m: f"{m.group(1)}{new_job}",
                    new_block, count=1, flags=re.MULTILINE
                )
                if old_job != new_job:
                    report_parts.append(f"Job {fmt(old_job):>13} -> {fmt(new_job):>13}")

    if report_parts:
        aegis_m = re.search(r'^\s+AegisName:\s*(\S+)', block, re.MULTILINE)
        aegis   = aegis_m.group(1) if aegis_m else '???'
        tier    = 'MVP' if is_mvp else ('Boss' if is_class_boss else 'Normal')
        report  = f"  [{mob_id}] {aegis:<35} lv{level:>3} ({tier}) -- {', '.join(report_parts)}"
    else:
        report = None

    return new_block, report


def convert_file(input_path, output_path=None, dry_run=False, convert_def=True, convert_exp=True, convert_hp=True):
    with open(input_path, 'r', encoding='utf-8') as f:
        content = f.read()

    header, blocks = split_mob_blocks(content)
    if not blocks:
        print("Aucun bloc 'Body:' trouvÃƒÂ© dans le fichier.")
        return

    converted_blocks = []
    reports = []

    for block in blocks:
        new_block, report = convert_block(block, dry_run, convert_def, convert_exp, convert_hp)
        converted_blocks.append(new_block)
        if report:
            reports.append(report)

    new_content = header + ''.join(converted_blocks)

    print(f"\n=== Conversion : {os.path.basename(input_path)} ===")
    if reports:
        print(f"{len(reports)} mob(s) modifie(s) :\n")
        for r in reports:
            print(r)
    else:
        print("Aucune valeur a modifier (deja en pre-re ?).")

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
        description='Convertit Defense/MagicDefense renewal Ã¢â€ â€™ pre-renewal dans un mob_db.yml'
    )
    parser.add_argument('input', help='Fichier YML ÃƒÂ  convertir')
    parser.add_argument('-o', '--output', default=None,
                        help='Fichier de sortie (dÃƒÂ©faut : modifie en place)')
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

    convert_file(args.input, args.output, args.dry_run,
                 convert_def=not args.no_def,
                 convert_exp=not args.no_exp,
                 convert_hp=not args.no_hp)


if __name__ == '__main__':
    main()
