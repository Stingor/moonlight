#!/usr/bin/env python3
"""Génère skilltreeguild.lub (client) depuis db/[pre-re|re]/guild_skill_tree.yml.

Le client ne connaît PAS l'arbre des compétences de guilde : SKILL_TREEVIEW_FOR_JOB
n'en contient aucune, et le serveur n'envoie même pas les compétences dont les
prérequis manquent (clif_guild_skillinfo filtre par guild_check_skill_require).
Ce fichier est donc le miroir client de la table serveur, sur le même principe que
skillinfolist.lub pour l'arbre des personnages.

Clés = ids NUMÉRIQUES : le fichier ne dépend d'aucune autre table Lua (un SKID.<nom>
non résolu ferait une clé nil et ferait échouer tout le script). Nom en commentaire.

Le mode pre-re / re est DÉTECTÉ depuis src/config/renewal.hpp — ne pas le coder en dur,
et n'utiliser --mode que pour forcer volontairement.

Usage:
    python gen_guild_skill_tree_lub.py [--mode pre-re|re] [-o chemin.lub]

Destination côté client (à placer dans le patch) :
    data\\luafiles514\\lua files\\skillinfoz\\skilltreeguild.lub
"""

import argparse
import datetime
import re
import pathlib
import sys

import yaml

REPO = pathlib.Path(__file__).resolve().parents[2]


def load_body(mode):
    """Corps de la DB : la base puis l'import du mode (le second complète/écrase)."""
    entries = {}
    order = []
    for rel in (f"db/{mode}/guild_skill_tree.yml", "db/guild_skill_tree.yml"):
        path = REPO / rel
        if not path.exists():
            continue
        doc = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        for entry in doc.get("Body") or []:
            skill_id = entry.get("Id")
            if not skill_id:
                continue
            if skill_id not in entries:
                order.append(skill_id)
            entries[skill_id] = entry
    return [entries[k] for k in order]


def detect_mode():
    """Mode du SERVEUR, lu dans src/config/renewal.hpp — pas une valeur à retenir.

    On suit la sémantique du préprocesseur, pas un seul marqueur, parce que le mode se
    bascule de DEUX façons ici : soit `#define PRERE` (qui neutralise tout le bloc
    renouveau), soit en commentant `#define RENEWAL` à l'intérieur de ce bloc. Ne
    regarder que l'un des deux se trompe dès que l'autre est utilisé.

        renewal  <=>  PRERE absent ET RENEWAL présent

    ⚠ Une ligne `#define RENEWAL` existe TOUJOURS dans le fichier tant qu'elle n'est pas
    commentée, même quand PRERE la neutralise : la trouver ne prouve RIEN à elle seule.

    Se tromper de mode donne un arbre qui ne correspond pas au serveur : en re, la DB de
    guilde ajoute quatre compétences que le serveur ignorerait, affichées verrouillées à
    vie côté client.
    """
    header = REPO / "src/config/renewal.hpp"
    if not header.exists():
        return "pre-re"  # header absent : le mode le plus restrictif
    text = header.read_text(encoding="utf-8", errors="replace")
    defined = lambda macro: any(
        re.match(rf"\s*#define\s+{macro}\b", line) for line in text.splitlines()
    )
    if defined("PRERE"):
        return "pre-re"
    return "re" if defined("RENEWAL") else "pre-re"


def load_skill_ids(mode):
    """Nom de compétence -> id numérique, depuis skill_db.yml.

    Le fichier généré porte les ids EN CLAIR plutôt que des SKID.<nom> : la table SKID
    n'est pas garantie présente dans l'état Lua où on charge le fichier, et une clé nil
    ferait échouer tout le script. Le nom reste en commentaire pour la lisibilité.
    """
    ids = {}
    for rel in ("db/skill_db.yml", f"db/{mode}/skill_db.yml"):
        path = REPO / rel
        if not path.exists():
            continue
        doc = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        for entry in doc.get("Body") or []:
            if entry.get("Name") and entry.get("Id") is not None:
                ids[entry["Name"]] = entry["Id"]
    return ids


def emit(body, mode, ids):
    stamp = datetime.date.today().isoformat()
    # Commentaires en ASCII pur : ce fichier part dans le GRF et sera ouvert par des
    # outils qui lisent en CP949 (les .lub du client le sont), ou l'UTF-8 s'affiche en
    # charabia. Lua ignore le contenu des commentaires, c'est donc purement lisibilite.
    out = [
        f"-- Client mirror of db/{mode}/guild_skill_tree.yml - generated {stamp}.",
        "-- DO NOT EDIT: re-run tools/gen/gen_guild_skill_tree_lub.py.",
        "-- Guild skill tree (max levels + requirements) for the Bourgeon guild tab;",
        "-- the server remains the only authority on what it actually allows.",
        "-- Numeric ids on purpose: this file depends on NO other Lua table.",
        "GUILD_SKILL_TREE = {",
    ]
    missing = []
    for i, entry in enumerate(body):
        name = entry["Id"]
        if name not in ids:
            missing.append(name)
            continue
        lines = [f'\t\t"{name}"', f"\t\tMaxLv = {entry.get('MaxLevel', 1)}"]
        required = [r for r in (entry.get("Required") or []) if r["Id"] in ids]
        missing += [r["Id"] for r in (entry.get("Required") or []) if r["Id"] not in ids]
        if required:
            reqs = ",\n".join(
                f"\t\t\t{{ {ids[r['Id']]}, {r.get('Level', 1)} }}  -- {r['Id']}"
                for r in required
            )
            lines.append("\t\t_NeedSkillList = {\n" + reqs + "\n\t\t}")
        out.append(f"\t[{ids[name]}] = {{  -- {name}")
        out.append(",\n".join(lines))
        out.append("\t}" + ("," if i + 1 < len(body) else ""))
    out.append("}")
    out.append(DUMPER)
    if missing:
        print("Ignorées (absentes de skill_db.yml) :", ", ".join(sorted(set(missing))),
              file=sys.stderr)
    return "\n".join(out) + "\n"


# Accesseur lu par Bourgeon : un seul appel Lua rend toute la table sérialisée, au
# lieu d'un aller-retour par compétence. Nom court (<= 15 car.) : le wrapper natif
# Lua_CallGlobal_va passe le nom en std::string PAR VALEUR et un nom plus long
# sortirait du SSO, ce qui corrompt la libération côté appelant.
# Format : "id,maxLv,prereq:lvl|prereq:lvl;" répété. Sans prérequis -> champ vide.
DUMPER = """
function GdDump()
\tlocal out = ""
\tfor id, e in pairs(GUILD_SKILL_TREE) do
\t\tlocal req = ""
\t\tif e._NeedSkillList ~= nil then
\t\t\tfor _, r in ipairs(e._NeedSkillList) do
\t\t\t\tif req ~= "" then req = req .. "|" end
\t\t\t\treq = req .. r[1] .. ":" .. r[2]
\t\t\tend
\t\tend
\t\tout = out .. id .. "," .. e.MaxLv .. "," .. req .. ";"
\tend
\treturn out
end
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    # Par défaut le mode est DÉTECTÉ depuis renewal.hpp ; --mode ne sert qu'à forcer.
    parser.add_argument("--mode", choices=("pre-re", "re"), default=None)
    parser.add_argument("-o", "--output", default=None)
    args = parser.parse_args()

    mode = args.mode or detect_mode()
    if args.mode:
        print(f"Mode FORCÉ : {mode} (détecté : {detect_mode()}).", file=sys.stderr)
    else:
        print(f"Mode détecté depuis src/config/renewal.hpp : {mode}.")

    body = load_body(mode)
    if not body:
        print("Aucune entrée lue — vérifier le chemin de la DB.", file=sys.stderr)
        return 1

    ids = load_skill_ids(mode)
    if not ids:
        print("skill_db.yml illisible : impossible de résoudre les ids.", file=sys.stderr)
        return 1
    text = emit(body, mode, ids)
    target = pathlib.Path(args.output) if args.output else \
        pathlib.Path(__file__).with_name("skilltreeguild.lub")
    target.write_text(text, encoding="utf-8", newline="\r\n")
    with_req = sum(1 for e in body if e.get("Required"))
    print(f"{target} : {len(body)} compétences, {with_req} avec prérequis.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
