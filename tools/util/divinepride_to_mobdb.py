"""Convertit les fiches Divine Pride en cache vers une mob_db au format db/re.

Lit tools/util/cache/divinepride/monster_<id>.json (fetch_divinepride_mobs.py)
et écrit, dans le dossier de sortie :
- mob_db_re.yml : entrées au format rAthena renewal, à passer ensuite dans
  re_to_prere_mob.py ;
- spawns.txt : carte, mob, effectif et délai, prêts à devenir des monster ;
- rapport.md : ce qui est déduit ou à arbitrer, mob par mob.

Formules vérifiées sur EP18_MD_SCHULANG et PORING (db/re contre Divine Pride) :
- ATK affichée  = niveau + STR + Attack  x [0,8 ; 1,2]
- MATK affichée = niveau + INT + Attack2 x [0,7 ; 1,3]
- speed         = 1000 / WalkSpeed       (cases par seconde)
- attackSpeed   = 1000 / AttackMotion    (coups par seconde)
AttackDelay et DamageMotion ne sont pas publiés : AttackDelay = AttackMotion,
DamageMotion = 480, à arbitrer. Divine Pride ne publie pas non plus l'IA ni
les compétences des mobs.

Usage :
  python tools/util/divinepride_to_mobdb.py --aegis EP19_ <dossier_sortie>
"""
import glob
import json
import os
import re
import sys

import yaml

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CACHE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "cache", "divinepride")

RACES = {"Demi-Human": "Demihuman", "Formless": "Formless", "Brute": "Brute", "Fish": "Fish",
         "Insect": "Insect", "Plant": "Plant", "Demon": "Demon", "Undead": "Undead",
         "Dragon": "Dragon", "Angel": "Angel"}


class RathenaDumper(yaml.SafeDumper):
    """Indentation de db/re : « - Id: » à deux espaces sous Body (re_to_prere_mob.py en dépend)."""

    def increase_indent(self, flow=False, indentless=False):
        return super().increase_indent(flow, False)


def commented_aegis(prefix):
    """Id -> AegisName des entrées (souvent commentées) de db/re/mob_db.yml."""
    text = open(os.path.join(ROOT, "db", "re", "mob_db.yml"), encoding="utf-8").read()
    pat = rf"^#?\s*- Id: (\d+)\s*\n#?\s*AegisName: ({re.escape(prefix)}\S*)"
    return {int(i): a for i, a in re.findall(pat, text, re.M)}


def item_names():
    names = {}
    for f in glob.glob(os.path.join(ROOT, "db", "re", "item_db_*.yml")):
        for it in (yaml.safe_load(open(f, encoding="utf-8")) or {}).get("Body") or []:
            names[it["Id"]] = it["AegisName"]
    return names


def bounds(text):
    lo, hi = (float(x.replace(".", "")) for x in text.split(" - "))
    return lo, hi


# Noms absents en anglais (kROM seulement). Déduits des mobs frères et des cartes
# de db/re, sinon traduits du coréen. Grades Iwin : iRO les nomme tous
# « Fake Iwin Soldier » (EP20_FAKE_IWIN_*) ; Moonlight traduit les grades.
NAMES = {
    "EP19_NPC_IWIN": "Iwin",
    "EP19_NPC_IWIN_SN": "Iwin Trainee",           # 훈련병
    "EP19_NPC_IWIN_SY": "Iwin Private",           # 이병
    "EP19_NPC_IWIN_SB": "Iwin Private First Class",  # 일병
    "EP19_NPC_IWIN_SG": "Iwin Corporal",          # 상병
    "EP19_NPC_IWIN_SR": "Iwin Sergeant",          # 조병, sommet de l'échelle
    "EP19_NPC_LEHAR": "Lehar",                    # costume C_Lehar_Mini
    "EP19_RGAN_E": "Superior Rgan",               # 상급 르간, comme 21600
    "EP19_MD_JUNCEA": "Monster Juncea",
    "EP19_MD_RGAN_E": "Renovated Superior Rgan",  # nom coréen de 21600
    "EP19_MD_RGAN_D": "Entangled Intermediate Rgan",  # nom coréen de 21601
    "EP19_MD_HEARTHUNTER_AT": "Heart Hunter AT",  # 21599
    "EP19_MD_AQUILA_B": "Aquila",                 # 21531
    "EP19_MD_RGAN_ES": "Mana Overloaded Superior Rgan",  # 마력 폭주 상급 르간
    "EP19_MD_BOX1": "Suspicious Storage Stone",   # 수상한 보관석
    "EP19_MD_BOX2": "Very Suspicious Storage Stone",  # 매우 수상한 보관석
}


def display_name(d, aegis):
    if aegis in NAMES:
        return NAMES[aegis], False
    name = d.get("name") or ""
    if name and all(ord(c) < 128 for c in name):
        return name, False
    words = aegis.split("_")[1:] or [aegis]
    return " ".join(w.capitalize() for w in words), True


def convert(d, aegis, items, notes):
    lvl = d["level"]
    lo, hi = bounds(d["attackRange"])
    atk = round((lo - lvl - d["str"]) / 0.8)
    atk_hi = round((hi - lvl - d["str"]) / 1.2)
    if abs(atk - atk_hi) > 2:
        notes.append(f"ATK incohérente ({atk} contre {atk_hi}), borne basse retenue")
    matk = 0
    if d.get("magicAttackRange"):
        mlo, _ = bounds(d["magicAttackRange"])
        matk = max(0, round((mlo - lvl - d["int"]) / 0.7))
    motion = round(1000 / d["attackSpeed"] / 4) * 4 if d.get("attackSpeed") else 1000
    walk = round(1000 / d["speed"]) if d.get("speed") else 200
    element, el_lv = d["element"].rsplit(" ", 1)
    name, guessed = display_name(d, aegis)
    if guessed:
        notes.append("nom anglais absent, tiré de l'AegisName")
    entry = {
        "Id": d["id"], "AegisName": aegis, "Name": name, "Level": lvl, "Hp": d["health"],
        "BaseExp": d["baseExperience"] or 0, "JobExp": d["jobExperience"] or 0,
        "Attack": atk, "Attack2": matk, "Defense": d["def"], "MagicDefense": d["mDef"],
        "Resistance": d.get("res") or 0, "MagicResistance": d.get("mRes") or 0,
        "Str": d["str"], "Agi": d["agi"], "Vit": d["vit"], "Int": d["int"], "Dex": d["dex"],
        "Luk": d["luk"], "AttackRange": d["range"], "SkillRange": 10, "ChaseRange": 12,
        "Size": d["size"], "Race": RACES.get(d["race"], d["race"]), "Element": element,
        "ElementLevel": int(el_lv), "WalkSpeed": walk, "AttackDelay": motion,
        "AttackMotion": motion, "DamageMotion": 480,
        "Ai": "21" if d["type"] in ("Boss", "MVP") else "04",
    }
    if d["type"] in ("Boss", "MVP"):
        entry["Class"] = "Boss"
    if d["type"] == "MVP":
        entry["MvpExp"] = (d["baseExperience"] or 0) // 2
        notes.append("MVP : MvpExp posé à la moitié de BaseExp, à arbitrer")
    for attr in d.get("attributes") or []:
        notes.append(f"attribut : {attr}")
    drops, mvp = [], []
    for src, dst in ((d.get("drops") or [], drops), (d.get("mvpDrops") or [], mvp)):
        for it in src:
            aegis_item = items.get(it["itemId"])
            if not aegis_item:
                notes.append(f"objet {it['itemId']} absent de db/re, drop écarté")
                continue
            if it.get("probability") is None:
                notes.append(f"drop {aegis_item} sans taux publié, écarté")
                continue
            row = {"Item": aegis_item, "Rate": max(1, round(it["probability"] * 10000))}
            if it.get("isStealProtected"):
                row["StealProtected"] = True
            dst.append(row)
    if mvp:
        entry["MvpDrops"] = mvp
    if drops:
        entry["Drops"] = drops
    return entry


def main():
    if len(sys.argv) != 4 or sys.argv[1] != "--aegis":
        sys.exit(__doc__)
    prefix, out = sys.argv[2], sys.argv[3]
    os.makedirs(out, exist_ok=True)
    aegis = commented_aegis(prefix)
    items = item_names()
    body, spawns, report = [], [], []
    for mob_id, name in sorted(aegis.items()):
        path = os.path.join(CACHE, f"monster_{mob_id}.json")
        if not os.path.exists(path):
            report.append(f"- {mob_id} {name} : pas en cache")
            continue
        d = json.load(open(path, encoding="utf-8"))
        notes = []
        body.append(convert(d, name, items, notes))
        for s in d.get("spawns") or []:
            spawns.append((s["mapName"], name, s["quantity"], s.get("respawnTime") or 0))
        if not d.get("spawns"):
            notes.append("aucun spawn publié (instance ou invocation)")
        report.append(f"- {mob_id} {name} ({d['type']}, niv. {d['level']}) : "
                      + ("; ".join(notes) if notes else "rien à signaler"))

    header = {"Header": {"Type": "MOB_DB", "Version": 4}, "Body": body}
    with open(os.path.join(out, "mob_db_re.yml"), "w", encoding="utf-8", newline="\n") as f:
        f.write(f"# Épisode 19 : {len(body)} mobs depuis Divine Pride (kROM), format db/re.\n")
        yaml.dump(header, f, Dumper=RathenaDumper, allow_unicode=True, sort_keys=False,
                  width=200, indent=2)
    with open(os.path.join(out, "spawns.txt"), "w", encoding="utf-8", newline="\n") as f:
        for m, mob, qty, respawn in sorted(spawns):
            f.write(f"{m}\t{mob}\t{qty}\t{respawn}\n")
    with open(os.path.join(out, "rapport.md"), "w", encoding="utf-8", newline="\n") as f:
        f.write("# Épisode 19 : conversion Divine Pride\n\n"
                "Communs à tous : IA non publiée (04 pour les normaux, 21 pour boss et MVP), "
                "AttackDelay = AttackMotion, DamageMotion = 480, aucune compétence publiée.\n\n")
        f.write("\n".join(report) + "\n")
    print(f"{len(body)} mobs, {len(spawns)} lignes de spawn -> {out}")


if __name__ == "__main__":
    main()
