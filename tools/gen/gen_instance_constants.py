#!/usr/bin/env python3
"""
gen_instance_constants.py
Génère db/import/const_instances.yml avec une constante INST_<NOM> -> Id
par instance, en parsant db/instance_db.yml et tous ses imports récursifs.

L'Id d'instance_db est déjà un identifiant stable et unique : il sert
directement d'enum pour les scripts NPC (menus d'instances, switch, etc.),
sans liste à maintenir à la main.

Le préfixe est INST_ et non MD_ : MD_ est déjà pris par les modes de monstres
(MD_AGGRESSIVE, MD_MVP, ...).

Usage: python tools/gen/gen_instance_constants.py [--root <racine_serveur>]
"""

import sys
import re
import argparse
from pathlib import Path

PREFIX = "INST_"

CONST_YML_HEADER = """\
###########################################################################
# Script Constant Database - Instances
###########################################################################
#
# Script Constant Settings
#
###########################################################################
# - Name             Nom unique de la constante (INST_<NOM_DE_L_INSTANCE>).
#   Value            Id de l'instance dans instance_db.yml.
###########################################################################
# AUTO-GENERATED par tools/gen/gen_instance_constants.py
# NE PAS EDITER MANUELLEMENT - relancer le generateur a la place.
#
# Pour que ce fichier soit charge, db/const.yml doit l'importer :
#   Footer:
#     Imports:
#     - Path: db/import/const.yml
#     - Path: db/import/const_instances.yml
###########################################################################

Header:
  Type: CONSTANT_DB
  Version: 1

Body:
"""


def parse_args():
    parser = argparse.ArgumentParser(
        description="Génère les constantes INST_* d'instances pour les scripts NPC")
    parser.add_argument("--root", default=".",
                        help="Racine du serveur (défaut: répertoire courant)")
    parser.add_argument("--output", default="db/import/const_instances.yml",
                        help="Fichier de sortie (défaut: db/import/const_instances.yml)")
    parser.add_argument("--dry-run", action="store_true",
                        help="Affiche le résultat sans écrire le fichier")
    return parser.parse_args()


def collect_import_paths(yml_path: Path, root: Path) -> list[Path]:
    """Extrait les chemins de la section Footer.Imports d'un fichier YAML.

    Les lignes commentées sont ignorées : db/instance_db.yml garde les imports
    pre-re et re commentés, seul db/import/ est réellement chargé.
    """
    imports = []
    if not yml_path.exists():
        print(f"  [WARN] Fichier introuvable: {yml_path}", file=sys.stderr)
        return imports

    in_footer = False
    in_imports = False
    for line in yml_path.read_text(encoding="latin-1").splitlines():
        stripped = line.strip()
        if stripped == "Footer:":
            in_footer = True
            in_imports = False
        elif in_footer and stripped == "Imports:":
            in_imports = True
        elif in_imports:
            if stripped.startswith("#"):
                continue
            m = re.match(r"-\s*Path:\s*(.+)", stripped)
            if m:
                imports.append(root / m.group(1).strip().strip('"\''))
            elif stripped and not stripped.startswith("-"):
                in_imports = False
    return imports


def parse_instances_from_file(yml_path: Path) -> list[tuple[int, str]]:
    """Parse les entrées Body d'un instance_db YAML. Retourne (Id, Name)."""
    out = []
    if not yml_path.exists():
        print(f"  [WARN] Fichier introuvable: {yml_path}", file=sys.stderr)
        return out

    text = yml_path.read_text(encoding="latin-1")
    for m in re.finditer(r"^  - Id:\s*(\d+)\s*$\n\s*Name:\s*(.+?)\s*$",
                         text, re.M):
        name = m.group(2).strip().strip('"\'')
        out.append((int(m.group(1)), name))
    return out


def collect_all_instances(entry: Path, root: Path) -> dict[int, str]:
    """Parcourt l'entrée et ses imports récursifs. Les imports tardifs gagnent."""
    found: dict[int, str] = {}
    seen: set[Path] = set()

    def walk(path: Path):
        resolved = path.resolve()
        if resolved in seen:
            return
        seen.add(resolved)
        entries = parse_instances_from_file(path)
        if entries:
            print(f"  {path}: {len(entries)} instance(s)", file=sys.stderr)
        for iid, name in entries:
            found[iid] = name
        for sub in collect_import_paths(path, root):
            walk(sub)

    walk(entry)
    return found


def to_constant(name: str) -> str:
    """"Temple of the Demon God" -> INST_TEMPLE_OF_THE_DEMON_GOD.

    Les apostrophes sont supprimées et non converties en séparateur, pour
    obtenir INST_ORCS_MEMORY plutôt que INST_ORC_S_MEMORY.
    """
    ident = re.sub(r"['’´`]", "", name)
    ident = re.sub(r"[^A-Za-z0-9]+", "_", ident).strip("_").upper()
    if not ident:
        return ""
    if ident[0].isdigit():
        ident = "N" + ident
    return PREFIX + ident


def generate_const_yml(instances: dict[int, str]) -> str:
    lines = [CONST_YML_HEADER]
    used: dict[str, int] = {}

    for iid in sorted(instances):
        const = to_constant(instances[iid])
        if not const:
            print(f"  [WARN] Id {iid}: nom vide, ignoré", file=sys.stderr)
            continue
        if const in used:
            # Deux instances de meme nom : on suffixe par l'Id pour rester unique.
            print(f"  [WARN] {const} déjà pris par l'Id {used[const]}, "
                  f"l'Id {iid} devient {const}_{iid}", file=sys.stderr)
            const = f"{const}_{iid}"
        used[const] = iid
        lines.append(f"  - Name: {const}\n")
        lines.append(f"    Value: {iid}\n")
    return "".join(lines)


def main():
    args = parse_args()
    root = Path(args.root).resolve()
    entry = root / "db/instance_db.yml"
    output = root / args.output

    print(f"Racine: {root}", file=sys.stderr)
    print(f"Entrée: {entry}", file=sys.stderr)
    print(f"Sortie: {output}", file=sys.stderr)
    print("", file=sys.stderr)

    if not entry.exists():
        print(f"ERREUR: Fichier introuvable: {entry}", file=sys.stderr)
        sys.exit(1)

    instances = collect_all_instances(entry, root)
    print(f"\n{len(instances)} instance(s) collectée(s).", file=sys.stderr)

    content = generate_const_yml(instances)

    if args.dry_run:
        print(content)
        print("[dry-run] Fichier non écrit.", file=sys.stderr)
        return

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(content, encoding="latin-1", newline="\n")
    print(f"Écrit: {output}", file=sys.stderr)


if __name__ == "__main__":
    main()
