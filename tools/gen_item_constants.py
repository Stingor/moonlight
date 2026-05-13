#!/usr/bin/env python3
"""
gen_item_constants.py
Génère db/import/const.yml avec toutes les constantes AegisName -> Id
en parsant db/import/item_db.yml et tous ses imports récursifs.

Usage: python tools/gen_item_constants.py [--root <racine_serveur>]
"""

import sys
import os
import re
import argparse
from pathlib import Path


def parse_args():
    parser = argparse.ArgumentParser(description="Génère les constantes AegisName pour les scripts NPC")
    parser.add_argument("--root", default=".", help="Racine du serveur (défaut: répertoire courant)")
    parser.add_argument("--output", default="db/import/const.yml", help="Fichier de sortie (défaut: db/import/const.yml)")
    parser.add_argument("--dry-run", action="store_true", help="Affiche le résultat sans écrire le fichier")
    return parser.parse_args()


def collect_import_paths(yml_path: Path, root: Path) -> list[Path]:
    """Extrait les chemins dans la section Footer.Imports d'un fichier YAML."""
    imports = []
    if not yml_path.exists():
        print(f"  [WARN] Fichier introuvable: {yml_path}", file=sys.stderr)
        return imports

    in_footer = False
    in_imports = False
    for line in yml_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if stripped == "Footer:":
            in_footer = True
            in_imports = False
        elif in_footer and stripped == "Imports:":
            in_imports = True
        elif in_imports:
            m = re.match(r"-\s*Path:\s*(.+)", stripped)
            if m:
                rel = m.group(1).strip().strip('"\'')
                imports.append(root / rel)
            elif stripped and not stripped.startswith("#") and not stripped.startswith("-"):
                in_imports = False
    return imports


def parse_items_from_file(yml_path: Path) -> list[tuple[int, str]]:
    """
    Parse les entrées Body d'un fichier item_db YAML sans dépendance externe.
    Retourne une liste de (Id, AegisName).
    """
    items = []
    if not yml_path.exists():
        print(f"  [WARN] Fichier introuvable: {yml_path}", file=sys.stderr)
        return items

    text = yml_path.read_text(encoding="utf-8")
    in_body = False
    current_id = None
    current_aegis = None

    for line in text.splitlines():
        stripped = line.strip()

        if stripped == "Body:":
            in_body = True
            current_id = None
            current_aegis = None
            continue

        if stripped in ("Footer:", "Header:"):
            in_body = False
            continue

        if not in_body:
            continue

        # Nouvelle entrée
        if re.match(r"^-\s+Id:", stripped):
            if current_id is not None and current_aegis is not None:
                items.append((current_id, current_aegis))
            current_id = None
            current_aegis = None
            m = re.match(r"^-\s+Id:\s*(\d+)", stripped)
            if m:
                current_id = int(m.group(1))
        elif re.match(r"^Id:\s*\d+", stripped):
            m = re.match(r"^Id:\s*(\d+)", stripped)
            if m:
                current_id = int(m.group(1))
        elif re.match(r"^AegisName:", stripped):
            m = re.match(r"^AegisName:\s*(.+)", stripped)
            if m:
                raw = m.group(1).strip().strip('"\'')
                current_aegis = raw.replace("'", "").replace(".", "").replace("-", "").replace("?", "")

    # Dernière entrée
    if current_id is not None and current_aegis is not None:
        items.append((current_id, current_aegis))

    return items


def collect_all_items(entry_file: Path, root: Path) -> dict[str, int]:
    """
    Parcourt récursivement entry_file et tous ses imports Footer.
    Retourne un dict {AegisName: Id}.
    """
    visited = set()
    queue = [entry_file]
    all_items: dict[str, int] = {}

    while queue:
        path = queue.pop(0)
        if path in visited:
            continue
        visited.add(path)

        print(f"  Parsing: {path.relative_to(root)}", file=sys.stderr)
        for item_id, aegis in parse_items_from_file(path):
            if aegis in all_items and all_items[aegis] != item_id:
                print(f"  [WARN] Doublon AegisName '{aegis}': {all_items[aegis]} vs {item_id}", file=sys.stderr)
            all_items[aegis] = item_id

        for imp in collect_import_paths(path, root):
            if imp not in visited:
                queue.append(imp)

    return all_items


CONST_YML_HEADER = """\
# This file is a part of rAthena.
#   Copyright(C) 2021 rAthena Development Team
#   https://rathena.org - https://github.com/rathena
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
###########################################################################
# Script Constant Database
###########################################################################
#
# Script Constant Settings
#
###########################################################################
# - Name             Unique name for the constant.
#   Value            Item ID associated with the AegisName.
###########################################################################
# AUTO-GENERATED by tools/gen_item_constants.py — NE PAS ÉDITER MANUELLEMENT
###########################################################################

Header:
  Type: CONSTANT_DB
  Version: 1

Body:
"""


def generate_const_yml(items: dict[str, int]) -> str:
    lines = [CONST_YML_HEADER]
    for aegis, item_id in sorted(items.items(), key=lambda x: x[1]):
        lines.append(f"  - Name: {aegis}\n")
        lines.append(f"    Value: {item_id}\n")
    return "".join(lines)


def main():
    args = parse_args()
    root = Path(args.root).resolve()
    entry = root / "db/import/item_db.yml"
    output = root / args.output

    print(f"Racine: {root}", file=sys.stderr)
    print(f"Entrée: {entry}", file=sys.stderr)
    print(f"Sortie: {output}", file=sys.stderr)
    print("", file=sys.stderr)

    if not entry.exists():
        print(f"ERREUR: Fichier introuvable: {entry}", file=sys.stderr)
        sys.exit(1)

    items = collect_all_items(entry, root)
    print(f"\n{len(items)} items collectés.", file=sys.stderr)

    content = generate_const_yml(items)

    if args.dry_run:
        print(content[:3000])
        if len(content) > 3000:
            print(f"... ({len(content)} caractères au total)")
    else:
        output.write_text(content, encoding="utf-8")
        print(f"Fichier écrit: {output} ({len(items)} constantes)", file=sys.stderr)


if __name__ == "__main__":
    main()
