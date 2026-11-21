#!/usr/bin/env python3
"""Bonus de competence que l'arme qui les porte ne permet pas d'utiliser.

Pour chaque arme de db/import/items et chaque combo de db/import/item_combos.yml qui
contient une arme, les bonus bSkillAtk / bSkillUseSP(rate) / bSkillVariableCast /
bSkillFixedCast / bSkillCooldown / bSkillDelay sont confrontes a `Requires.Weapon`
de db/pre-re/skill_db.yml (+ import). Un combo regroupe parfois plusieurs `- Combo:`
alternatifs sous un meme Script : leurs armes sont reunies.

Ne couvre pas les auto-sorts (bAutoSpell...), qui ne verifient pas l'arme.
Regle de remplacement : docs/episodes/skills_classes_fermees.md, « Arme requise ».

Usage : python tools/util/audit_skill_weapon.py
"""
import glob
import os
import re
from collections import defaultdict

import yaml

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def body(f):
    return (yaml.safe_load(open(os.path.join(ROOT, f), encoding="utf-8")) or {}).get("Body") or []


def main():
    sk = {}
    for f in ("db/pre-re/skill_db.yml", "db/import/skill_db.yml"):
        if os.path.exists(os.path.join(ROOT, f)):
            for e in body(f):
                sk[e["Name"]] = e

    def need(name):
        w = ((sk.get(name) or {}).get("Requires") or {}).get("Weapon")
        return {k for k, v in w.items() if v} if isinstance(w, dict) else None

    items, imported = {}, set()
    for f in sorted(glob.glob(os.path.join(ROOT, "db", "pre-re", "item_db*.yml"))) + \
            sorted(glob.glob(os.path.join(ROOT, "db", "import", "items", "*.yml"))):
        for it in body(os.path.relpath(f, ROOT)):
            items[it["AegisName"]] = it
            if os.sep + "import" + os.sep in f or "/import/" in f:
                imported.add(it["AegisName"])
    rx = re.compile(r'bonus[2-5]?\s+b(?:SkillAtk|SkillUseSP|SkillUseSPrate|SkillVariableCast|SkillFixedCast|SkillCooldown|SkillDelay)'
                    r'\s*,\s*\\?"?([A-Z0-9_]+)')
    hits = []

    def check(owner, weapons, script, where):
        for m in rx.finditer(script or ""):
            req = need(m.group(1))
            if req and weapons and not (set(weapons) & req):
                hits.append((m.group(1), where, owner, sorted(req), sorted(set(weapons))))

    for n, it in items.items():
        if it.get("Type") == "Weapon" and n in imported:
            check(n, [it.get("SubType")], it.get("Script"), "objet")
    for c in body("db/import/item_combos.yml"):
        names = [i for cb in c.get("Combos") or [] for i in cb["Combo"]]
        ws = [items[i].get("SubType") for i in names if i in items and items[i].get("Type") == "Weapon"]
        check(" | ".join("+".join(cb["Combo"]) for cb in c.get("Combos") or []), ws, c.get("Script"), "combo")
    by = defaultdict(list)
    for h in hits:
        by[h[0]].append(h)
    print(f"{len(hits)} bonus inutilisables, {len(by)} competence(s)")
    for s, hs in sorted(by.items(), key=lambda kv: -len(kv[1])):
        print(f"\n{s} (exige {', '.join(hs[0][3])}) : {len(hs)}")
        for _, where, owner, _, ws in hs:
            print(f"   {where:6} {owner[:100]:100} arme(s) {', '.join(ws)}")


if __name__ == "__main__":
    main()
