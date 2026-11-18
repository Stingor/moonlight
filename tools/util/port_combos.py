#!/usr/bin/env python3
"""Portage des combos amont listes par le controle 8 de audit_episode.py, par lots.

Un combo de db/re/item_combos.yml dont tous les objets sont charges mais absent de
db/import/item_combos.yml ne s'active jamais, sans message (la fiche client l'affiche).

Conversions appliquees (feedback_new_content_roadmap, §3b et §8) :
  - combo present aussi dans db/pre-re/item_combos.yml : le script pre-re est repris
    (il correspond a l'objet pre-re charge) ;
  - competences des classes fermees : table de docs/episodes/skills_classes_fermees.md ;
    « retire » et « garder » sont signales, pas appliques ;
  - bSkillCooldown, paliers de raffinage > 10, bonus 4e classe (bPow, bPAtk...) : signales
    pour decision manuelle, jamais convertis en silence.

Un bloc amont regroupe plusieurs `- Combo:` alternatifs sous un meme Script : seuls les
alternatifs manquants sont repris, et un bloc n'est jamais coupe entre deux lots.

Usage :
  python tools/util/port_combos.py --lot 1            # affiche le lot 1 (30 combos)
  python tools/util/port_combos.py --lot 1 --write    # l'ajoute a db/import/item_combos.yml
  python tools/util/port_combos.py --resume           # nombre de lots restants
"""
import argparse
import glob
import os
import re
import sys

import yaml

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
IMPORT = os.path.join(ROOT, "db", "import", "item_combos.yml")
LOT = 30
CLOSED = r"\b(?:LG|RK|SR|WM|AB|GC|NC|GN|KO|RL|WL|SO|RA|SC|SU|SJ|SP|MT|IQ|IG|CD|SHC|BO|TR|EM|ABC|WH|SKE|SOA|HN|DK|AG|NW|SH|SS|NPC)_[A-Z0-9_]+"
SUSPECTS = ["bPow", "bSta", "bWis", "bSpl", "bCon", "bCrt", "bPAtk", "bSMatk", "bRes", "bMRes",
            "bHPlus", "bCRate", "bAPRate", "getenchantgrade", "bSkillCooldown"]


# Conversions tranchees a la main, par combo (cle = noms locaux tries) : (script, raison).
OVERRIDES = {
    # amont : bSkillCooldown WL_COMET +20 s / +40 s, qui annule le -20 s / -40 s de la paire
    # KarduiEar + Lapine_Shield (fiche client : « la reduction ne s'active pas »). La paire est
    # deja portee en bDelayrate,-25 : le trio l'annule a l'identique.
    ("Fallen_Bishop_Card", "KarduiEar", "Lapine_Shield"):
        (".@r = getequiprefinerycnt(EQI_HAND_L);\nif (.@r >= 7)\n   bonus bDelayrate,25;\n",
         "bSkillCooldown +20 s -> bDelayrate,25 (annule la paire)"),
    ("KarduiEar", "Lapine_Shield", "Runaway_Magic"):
        (".@r = getequiprefinerycnt(EQI_HAND_L);\nif (.@r >= 7)\n   bonus bDelayrate,25;\n",
         "bSkillCooldown +20 s -> bDelayrate,25 (annule la paire)"),
    # §3b : bonus bDef renewal / 10
    ("Protect_Cloth", "Protect_Feathers"):
        ("bonus bMdef,5;\nbonus bDef,5;\nbonus bAspdRate,5;\nbonus bVit,5;\n"
         "bonus2 bSubRace,RC_Player_Human,3;\n"
         "if (readparam(bVit) >= 108) {\n   bonus bMdef,5;\n   bonus bDef,5;\n   bonus bAspdRate,5;\n   bonus bVit,5;\n}\n"
         "if (readparam(bVit) >= 120) {\n   bonus bMdef,10;\n   bonus bDef,10;\n   bonus bVit,10;\n}\n",
         "bDef 50/50/100 -> 5/5/10"),
    # RK_DRAGONBREATH_WATER : « retire » dans la table, ses deux lignes tombent
    ("Beta_Cleaner_Card", "Papila_Ruba_H_Card", "Verporta_Card"):
        ('bonus2 bSkillAtk,"KN_BOWLINGBASH",20;\n'
         ".@r_shoes = getequiprefinerycnt(EQI_SHOES);\n"
         'bonus2 bSkillAtk,"KN_BOWLINGBASH",5*(.@r_shoes/2);\n',
         "RK_DRAGONBREATH -> KN_BOWLINGBASH ; RK_DRAGONBREATH_WATER retire"),
    # WL_CRIMSONROCK et WL_COMET -> WZ_METEOR : regle des doublons, la plus forte valeur reste
    ("Assistant_Card", "Pitaya_R_Card", "Roaming_Splbook_Card"):
        ('bonus2 bSkillAtk,"WZ_METEOR",20;\n'
         ".@r_weapon = getequiprefinerycnt(EQI_HAND_R);\n"
         ".@r_shoes = getequiprefinerycnt(EQI_SHOES);\n"
         'bonus2 bSkillAtk,"WZ_METEOR",5*(max(.@r_shoes,.@r_weapon)/2);\n',
         "WL_CRIMSONROCK + WL_COMET -> WZ_METEOR, doublon : la plus forte valeur"),
    # SR_RAMPAGEBLASTER et SR_TIGERCANNON -> MO_EXTREMITYFIST : regle des doublons
    ("Mutant_Plaga_Card", "Papila_Ruba_Card", "Plasma_R_Card"):
        ('bonus2 bSkillAtk,"MO_EXTREMITYFIST",20;\n'
         ".@r_weapon = getequiprefinerycnt(EQI_HAND_R);\n"
         ".@r_shoes = getequiprefinerycnt(EQI_SHOES);\n"
         'bonus2 bSkillAtk,"MO_EXTREMITYFIST",5*(max(.@r_shoes,.@r_weapon)/2);\n',
         "SR_RAMPAGEBLASTER + SR_TIGERCANNON -> MO_EXTREMITYFIST, doublon : la plus forte valeur"),
    # table : SJ_FALLINGSTAR_ATK / _ATK2 -> TK_COUNTER, une seule ligne ; SJ_FULLMOONKICK -> TK_TURNKICK
    ("Alnoldi_Ex_Card", "B_Scissore_Ng_H_Card", "EP17_2_Phen_Card"):
        ('bonus2 bSkillAtk,"TK_COUNTER",20;\n'
         'bonus2 bSkillAtk,"TK_TURNKICK",20;\n'
         ".@r_weapon = getequiprefinerycnt(EQI_HAND_R);\n"
         ".@r_shoes = getequiprefinerycnt(EQI_SHOES);\n"
         'bonus2 bSkillAtk,"TK_COUNTER",5*(.@r_shoes/2);\n'
         'bonus2 bSkillAtk,"TK_TURNKICK",5*(.@r_weapon/2);\n',
         "SJ_FALLINGSTAR_ATK/_ATK2 -> TK_COUNTER (une ligne) ; SJ_FULLMOONKICK -> TK_TURNKICK"),
    # table : AB_DUPLELIGHT_MELEE / _MAGIC -> bonus de race Mort-vivant, physique / magique
    ("Chrismas_Tree", "Hurt_Mind", "Noble_Cross"):
        ("bonus bMatk,30;\n"
         "bonus2 bAddRace,RC_Undead,40;\n"
         "bonus2 bMagicAddRace,RC_Undead,40;\n"
         "if (getequiprefinerycnt(EQI_HAND_R) >= 10) {\n"
         "   bonus2 bAddRace,RC_Undead,20;\n"
         "   bonus2 bMagicAddRace,RC_Undead,20;\n}\n",
         "AB_DUPLELIGHT_MELEE/_MAGIC -> bAddRace / bMagicAddRace RC_Undead"),
}


def yload(path):
    with open(path, encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def loaded_items():
    """Noms charges, et traduction nom amont -> nom local par l'Id.

    Le fork garde parfois un autre AegisName pour le meme Id, voire l'echange avec un autre
    objet (Pocket_Watch__ amont = Pocket_Watch local, 2971 ; Pocket_Watch__ local = 7513, un
    objet divers) : comparer les noms bruts fabrique des combos sur le mauvais objet.
    """
    names, by_id = set(), {}
    for f in glob.glob(os.path.join(ROOT, "db", "import", "items", "*.yml")):
        for it in yload(f).get("Body") or []:
            names.add(it["AegisName"])
            by_id[it["Id"]] = it["AegisName"]
    re_id = {}
    for f in glob.glob(os.path.join(ROOT, "db", "re", "item_db_*.yml")):
        for it in yload(f).get("Body") or []:
            re_id[it["AegisName"]] = it["Id"]
    alias = lambda n: by_id.get(re_id[n], n) if n in re_id else n
    return names, alias


def skill_table():
    """Table validee : {competence: remplacement | 'garder' | 'retire'}."""
    out = {}
    path = os.path.join(ROOT, "docs", "episodes", "skills_classes_fermees.md")
    for line in open(path, encoding="utf-8"):
        m = re.match(r"\|\s*`([A-Z0-9_]+)`\s*\|\s*([^|]+?)\s*\|", line)
        if not m:
            continue
        r = m.group(2)
        if "garder" in r:
            out[m.group(1)] = "garder"
        elif "retir" in r:
            out[m.group(1)] = "retire"
        else:
            mm = re.match(r"`([A-Z0-9_]+)`$", r)
            out[m.group(1)] = mm.group(1) if mm else "manuel: " + r
    return out


def key(combo):
    return tuple(sorted(combo))


def missing_blocks():
    items, alias = loaded_items()
    have = {key(cb["Combo"]) for c in yload(IMPORT).get("Body") or [] for cb in c.get("Combos") or []}
    prere = {}
    for c in yload(os.path.join(ROOT, "db", "pre-re", "item_combos.yml")).get("Body") or []:
        for cb in c.get("Combos") or []:
            prere[key(cb["Combo"])] = c.get("Script") or ""
    out = []
    for c in yload(os.path.join(ROOT, "db", "re", "item_combos.yml")).get("Body") or []:
        # le pre-re a les noms locaux : sa cle se calcule apres traduction
        miss = []
        for cb in c.get("Combos") or []:
            combo = [alias(x) for x in cb["Combo"]]
            if all(x in items for x in combo) and key(combo) not in have and combo not in miss:
                miss.append(combo)
        if not miss:
            continue
        # les alternatifs d'un bloc partagent un script ; un script pre-re different les separe
        groups = {}
        for cb in miss:
            src = prere.get(key(cb))
            groups.setdefault(src, []).append(cb)
        for src, cbs in groups.items():
            out.append({"combos": cbs, "re": c.get("Script") or "", "prere": src})
    return out


def traits(script, notes):
    """Bonus sans effet en pre-re (docs/episodes/README.md, regle du 2026-11-16).

    bAtkRate : pc.cpp le refuse hors RENEWAL (ShowError, aucun effet) ; il devient
    bAddClass,Class_All comme dans les ports precedents (EP18, EP19, D2c).
    """
    simple = [("bPow", "bStr"), ("bSta", "bVit"), ("bSpl", "bInt"), ("bWis", "bInt"),
              ("bCon", "bDex"), ("bCrt", "bLuk"), ("bAllTraitStats", "bAllStats"),
              ("bCRate", "bCritAtkRate"), ("bHPlus", "bHealPower")]
    for a, b in simple:
        script, n = re.subn(rf"\bbonus {a},", f"bonus {b},", script)
        if n:
            notes.append(f"{a} -> {b}")
    for a, b in [("bAtkRate", "bAddClass"), ("bPAtk", "bAddClass"), ("bSMatk", "bMagicAddClass")]:
        script, n = re.subn(rf"\bbonus {a},([^;]+);", rf"bonus2 {b},Class_All,\1;", script)
        if n:
            notes.append(f"{a} -> {b},Class_All")

    def res(m, kinds):
        v = m.group(1)
        if not v.isdigit():
            notes.append(f"{m.group(0)} : valeur calculee, a convertir a la main")
            return m.group(0)
        n = int(v)
        d = max(1, round(80 * n / (n + 400)))
        notes.append(f"{m.group(0)[:-1]} -> {'/'.join(kinds)} {d}")
        return " ".join(f"bonus {k},{d};" for k in kinds)
    script = re.sub(r"\bbonus bRes,([^;]+);", lambda m: res(m, ["bNearAtkDef", "bLongAtkDef"]), script)
    script = re.sub(r"\bbonus bMRes,([^;]+);", lambda m: res(m, ["bMagicAtkDef"]), script)
    return script


def convert(blk, table):
    ov = {OVERRIDES.get(key(cb)) for cb in blk["combos"]}
    if len(ov) == 1 and None not in ov:
        script, why = ov.pop()
        return script, ["manuel : " + why]
    script = blk["prere"] if blk["prere"] is not None else blk["re"]
    notes = []
    if blk["prere"] is not None:
        notes.append("script pre-re")

    def sub(m):
        s = m.group(0)
        r = table.get(s)
        if r is None:
            notes.append(f"{s} : classe fermee absente de la table")
            return s
        if r in ("garder", "retire") or r.startswith("manuel"):
            notes.append(f"{s} : {r}")
            return s
        notes.append(f"{s} -> {r}")
        return r
    script = re.sub(CLOSED, sub, script)
    script = traits(script, notes)
    for s in SUSPECTS:
        if re.search(rf"\b{s}\b", script):
            notes.append(f"{s} a trancher")
    # regle validee : une piece `>= N` (N > 10) devient `getallrefine() >= 2*N` (11 -> 22)
    def tier(m):
        n = int(m.group(2))
        if n <= 10:
            return m.group(0)
        notes.append(f"{m.group(1)} >= {n} -> getallrefine() >= {2 * n}")
        return f"getallrefine() >= {2 * n}"
    script = re.sub(r"(getrefine\(\)|getequiprefinerycnt\(\s*\w+\s*\))\s*>=\s*(\d+)", tier, script)
    for m in re.finditer(r"(getrefine\(\)|getequiprefinerycnt\(\s*\w+\s*\)|\.@\w+)\s*>=?\s*(\d+)", script):
        if int(m.group(2)) > 10:
            notes.append(f"palier {m.group(0)} a verifier")
    return script, notes


def lots(blocks):
    out, cur, n = [], [], 0
    for b in blocks:
        if cur and n + len(b["combos"]) > LOT:
            out.append(cur)
            cur, n = [], 0
        cur.append(b)
        n += len(b["combos"])
    if cur:
        out.append(cur)
    return out


def render(lot, table, num, total):
    lines = [f"  # Combos amont manquants, lot {num}/{total} (port_combos.py, controle 8 d'audit_episode)"]
    report = []
    for b in lot:
        script, notes = convert(b, table)
        lines.append("  - Combos:")
        for cb in b["combos"]:
            lines.append("      - Combo:")
            lines += [f"          - {x}" for x in cb]
        body = script.strip("\n")
        if body:
            lines.append("    Script: |")
            lines += [("      " + l.rstrip()) if l.strip() else "" for l in body.split("\n")]
        report.append((" / ".join(" + ".join(cb) for cb in b["combos"]), notes))
    return "\n".join(lines) + "\n", report


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--lot", type=int)
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--resume", action="store_true")
    a = ap.parse_args()
    table = skill_table()
    blocks = missing_blocks()
    ls = lots(blocks)
    print(f"{sum(len(b['combos']) for b in blocks)} combos manquants, {len(blocks)} blocs, {len(ls)} lots")
    if a.resume or not a.lot:
        return 0
    lot = ls[0]  # les lots deja ecrits sortent de la liste : le lot courant est toujours le premier
    text, report = render(lot, table, a.lot, a.lot + len(ls) - 1)
    for combos, notes in report:
        print(f"- {combos}" + (f"   [{'; '.join(notes)}]" if notes else ""))
    if not a.write:
        print()
        print(text)
        return 0
    raw = open(IMPORT, "rb").read()
    new = raw.rstrip(b"\n") + b"\n" + text.encode("utf-8")
    yaml.safe_load(new.decode("utf-8"))  # echoue avant ecriture si le YAML est casse
    tmp = IMPORT + ".tmp"
    open(tmp, "wb").write(new)
    os.replace(tmp, IMPORT)
    print(f"ecrit : {sum(len(b['combos']) for b in lot)} combos dans db/import/item_combos.yml")
    return 0


if __name__ == "__main__":
    sys.exit(main())
