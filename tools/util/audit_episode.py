"""Audit de raccordement du contenu porte : ce qu'aucun chargement ne signale.

Ecrit le 2026-11-18, apres Gray Wolf Village (episode 18) : ville avec le drapeau town, mais
absente du menu Villes du Warp Agent, sans agent sur place et hors de la liste navi. Le client
ne trouvait aucun chemin, ni a l'aller ni au retour. Aucune erreur au boot.

Controles (sortie 1 s'il reste un KO) :
  1. navigation : cartes jouables injoignables depuis prontera, ou d'ou l'on ne revient pas
     (graphe de dist/data/yaml_files/navigation.yml du client, issu de map-server-generator) ;
  2. villes : drapeau town (moon/mapflag/town.npc) sans entree au menu Villes du Warp Agent,
     ou sans duplicate warpa_<ville> sur la carte ;
  3. Warp Agent : cartes du menu Villes et de la liste navi (.@navm$) sans Name dans map_index ;
  4. drops de carte : carte de db/re d'un mob porte (meme AegisName) absente de ses Drops ;
  5. albums : carte qui tombe d'un mob charge mais n'est dans aucun album
     (CARDALBUM, MAGICCARDALBUM) ;
  6. map_drops : carte de db/re/map_drops.yml jouee chez nous (spawn ou instance) mais absente
     de db/import/map_drops.yml ;
  7. journal : quete du serveur, citee par un script charge, sans entree dans quests.yml du
     client (le journal l'affiche sous « Quete N ») ;
  8. combos : combo de db/re/item_combos.yml dont tous les objets sont charges, absent de l'import.

Reste manuel : les images cutin des scripts portes (le client n'en publie pas la liste).

Usage : python tools/util/audit_episode.py [--client G:/GitHub/moonclient] [--carte <map> ...]
        --carte restreint les controles 1 et 6 aux cartes donnees (un episode).
Les listes d'exceptions voulues sont dans EXCEPTIONS, chacune avec sa raison.
"""
import argparse
import glob
import os
import re
import sys
from collections import defaultdict, deque

import yaml

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Ecarts voulus, avec leur raison. Ne rien y mettre sans la raison.
EXCEPTIONS = {
    "navigation": {
        # cartes que le generateur charge mais qu'aucun joueur n'atteint par le monde
    },
    "ville_sans_menu": {
        "izlude_a": "canal d'Izlude, rejoint par izlude",
        "izlude_b": "canal d'Izlude, rejoint par izlude",
        "izlude_c": "canal d'Izlude, rejoint par izlude",
        "izlude_d": "canal d'Izlude, rejoint par izlude",
        "moc_ruins": "quartier de Morroc, agent sur place",
        "pay_arche": "quartier de Payon, agent sur place",
    },
    "ville_sans_agent": {
        "izlude_a": "canal d'Izlude", "izlude_b": "canal d'Izlude",
        "izlude_c": "canal d'Izlude", "izlude_d": "canal d'Izlude",
    },
    # fichiers de mobs dont les drops de carte amont absents sont voulus (controle 4)
    "drop_carte": {
        "mob_db.yml": "mobs equilibres a la main (champions C1_..C5_, copies d'evenement ou"
                      " de quete E_/Q_, boss retouches comme IMMORTAL_CO_CMDER) : ils restent sans carte",
    },
}


def p(*a):
    return os.path.join(ROOT, *a)


def yload(path):
    with open(path, encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def read_latin1(path):
    with open(path, "rb") as f:
        return f.read().decode("latin-1")


def loaded_scripts():
    out = []
    for line in read_latin1(p("moon", "scripts_moon.conf")).splitlines():
        m = re.match(r"\s*npc:\s*(\S+)", line)
        if m and os.path.exists(p(m[1])):
            out.append(m[1])
    return out


def import_bodies(pattern):
    out = []
    for f in sorted(glob.glob(p(*pattern.split("/")))):
        out += yload(f).get("Body") or []
    return out


def nav_graph(client):
    path = os.path.join(client, "dist", "data", "yaml_files", "navigation.yml")
    d = yload(path)
    edges = defaultdict(set)
    for link in (d.get("links") or {}).values():
        if link.get("to") and link.get("map"):
            edges[link["map"]].add(link["to"])
    maps = d.get("maps") or {}
    spawn_maps = {s.get("map") for s in (d.get("spawns") or {}).values()}
    npc_maps = {n.get("map") for n in (d.get("npcs") or {}).values()}
    return maps, edges, spawn_maps, npc_maps


def reach(start, edges):
    seen, q = {start}, deque([start])
    while q:
        for n in edges.get(q.popleft(), ()):
            if n not in seen:
                seen.add(n)
                q.append(n)
    return seen


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--client", default="G:/GitHub/moonclient")
    ap.add_argument("--carte", nargs="*", default=None)
    a = ap.parse_args()
    only = set(a.carte) if a.carte else None
    ko = defaultdict(list)

    # ---- 1. navigation
    maps, edges, spawn_maps, npc_maps = nav_graph(a.client)
    rev = defaultdict(set)
    for s, ts in edges.items():
        for t in ts:
            rev[t].add(s)
    fwd, back = reach("prontera", edges), reach("prontera", rev)
    wa = read_latin1(p("moon", "warp_agent.npc"))
    navm = set(re.findall(r'"([a-z0-9_@]+)"', "".join(re.findall(r"setarray \.@navm\$\[\d+\],(.*?);", wa, re.S))))
    i = wa.index("L_VillesCmd:")
    vm_blk = wa[i:wa.index("setarray .@vx", i)]
    vm = re.findall(r'"([a-z0-9_]+)"', vm_blk[vm_blk.index("setarray .@vm$"):])
    # 1a. toute destination du Warp Agent doit etre declaree au generateur (.@navm$ ou menu Villes)
    wa_live = re.sub(r"//[^\n]*", "", wa)
    targets = set(re.findall(r'warp_fct\(\s*"([a-z0-9_]+)"', wa_live))
    for m in sorted(targets - navm - set(vm)):
        if only and m not in only:
            continue
        ko["1a. Warp Agent -> navi"].append(
            f"{m} : destination warp_fct du Warp Agent absente de .@navm$"
            + ("" if m in fwd else " (injoignable dans navigation.yml)"))
    # 1b. cartes d'un episode (--carte) injoignables
    unreachable = []
    for m in sorted(maps):
        if "@" in m or m in EXCEPTIONS["navigation"]:
            continue
        if m not in spawn_maps and m not in npc_maps:
            continue  # carte vide : rien a atteindre
        if m not in fwd:
            unreachable.append(m)
            if only and m in only:
                ko["1b. navigation"].append(f"{m} : injoignable depuis prontera"
                                            + ("" if m in back else ", et sans retour"))
    print(f"(info) {len(unreachable)} cartes jouables injoignables dans tout le graphe, la plupart"
          " atteintes par un PNJ de quete dont la teleportation n'est pas declaree au generateur")

    # ---- 2. villes
    agents = set(re.findall(r"^([a-z0-9_]+),\d+,\d+,\d+\tduplicate\(warp_agent\)", wa, re.M))
    agents |= set(re.findall(r"^([a-z0-9_]+),\d+,\d+,\d+\tscript\t[^\t]*::warp_agent\t", wa, re.M))
    towns = set()
    for line in read_latin1(p("moon", "mapflag", "town.npc")).splitlines():
        m = re.match(r"([a-z0-9_]+)\tmapflag\ttown\b", line)
        if m:
            towns.add(m[1])
    for t in sorted(towns):
        if t not in vm and t not in EXCEPTIONS["ville_sans_menu"]:
            ko["2. villes"].append(f"{t} : drapeau town, absente du menu Villes du Warp Agent")
        if t not in agents and t not in EXCEPTIONS["ville_sans_agent"]:
            ko["2. villes"].append(f"{t} : pas de duplicate warpa_{t} sur la carte")

    # ---- 3. noms de carte
    named = set()
    for f in (p("db", "map_index.yml"), p("db", "import", "map_index.yml")):
        if os.path.exists(f):
            for e in yload(f).get("Body") or []:
                if e.get("Name"):
                    named.add(e["Map"])
    for m in sorted(set(vm) | navm):
        if m not in named:
            ko["3. noms de carte"].append(f"{m} : au Warp Agent sans Name dans map_index")

    # ---- 4. drops de carte des mobs portes
    items = {}
    for it in import_bodies("db/import/items/*.yml"):
        items[it["AegisName"]] = it
    re_items = {}
    for f in glob.glob(p("db", "re", "item_db_*.yml")):
        for it in yload(f).get("Body") or []:
            re_items[it["AegisName"]] = it
    # item_db_card.yml garde souvent l'ancien AegisName (V_Magaleta_Card pour Archbishop_Card) :
    # un nom amont se traduit par son Id, jamais par comparaison de chaines
    imp_by_id = {it["Id"]: n for n, it in items.items()}
    alias = lambda n: imp_by_id.get(re_items[n]["Id"], n) if n in re_items else n
    is_card = lambda n: (items.get(n) or re_items.get(n) or {}).get("Type") == "Card" and \
        (items.get(n) or re_items.get(n) or {}).get("SubType") != "Enchant"
    mobs, mob_file = {}, {}
    for f in [p("db", "import", "mob_db.yml")] + sorted(glob.glob(p("db", "import", "mobs", "*.yml"))):
        for m in yload(f).get("Body") or []:
            if m.get("AegisName"):
                mobs[m["AegisName"]] = m
                mob_file[m["AegisName"]] = os.path.basename(f)
    re_mobs = {m["AegisName"]: m for m in yload(p("db", "re", "mob_db.yml")).get("Body") or []}
    unported = defaultdict(set)
    manual = []
    for name, m in sorted(mobs.items()):
        rm = re_mobs.get(name)
        if not rm:
            continue
        have = {d["Item"] for d in (m.get("Drops") or []) + (m.get("MvpDrops") or [])}
        for d in rm.get("Drops") or []:
            it = alias(d["Item"])
            if not is_card(it) or it in have:
                continue
            if it not in items:
                unported[mob_file[name]].add(it)
            elif mob_file[name] in EXCEPTIONS["drop_carte"]:
                manual.append(f"{name}:{it}")
            else:
                ko["4. drops de carte"].append(f"{name} ({mob_file[name]}) : {it}, carte portee, drop absent")
    if manual:
        why = "; ".join(f"{k} : {v}" for k, v in EXCEPTIONS["drop_carte"].items())
        print(f"(info) {len(manual)} drops de carte amont absents, exception voulue ({why}) : "
              + ", ".join(manual))
    for f, cs in sorted(unported.items()):
        print(f"(info) {f} : {len(cs)} carte(s) amont jamais portee(s) : {', '.join(sorted(cs))}")

    # ---- 5. albums
    album = set()
    for f in ("oca.yml", "mca.yml"):
        for g in yload(p("db", "import", "item_group_db", f)).get("Body") or []:
            for sg in g.get("SubGroups") or []:
                album |= {x["Item"] for x in sg.get("List") or []}
    dropped = defaultdict(list)
    for name, m in mobs.items():
        for d in (m.get("Drops") or []) + (m.get("MvpDrops") or []):
            if is_card(d["Item"]) and d["Item"] in items:
                dropped[d["Item"]].append(name)
    for c in sorted(dropped):
        if c not in album:
            ko["5. albums"].append(f"{c} (de {', '.join(sorted(dropped[c])[:3])}) : dans aucun album")

    # ---- 6. map_drops
    played = set(spawn_maps)
    for f in loaded_scripts():
        for m in re.findall(r"^([0-9]@[a-z0-9_]+|[a-z][a-z0-9_]+),\d+,\d+", read_latin1(p(f)), re.M):
            played.add(m)
    for e in yload(p("db", "import", "instance_db.yml")).get("Body") or []:
        played.add(e.get("Enter", {}).get("Map"))
        played |= set(e.get("AdditionalMaps") or {})
    imp_md = {e["Map"] for e in yload(p("db", "import", "map_drops.yml")).get("Body") or []}
    for e in yload(p("db", "re", "map_drops.yml")).get("Body") or []:
        mp = e["Map"]
        if (only and mp not in only) or mp not in played or mp in imp_md:
            continue
        ko["6. map_drops"].append(f"{mp} : bloc de db/re/map_drops.yml absent de l'import")

    # ---- 7. journal de quetes du client
    qids = set()
    for f in (p("db", "pre-re", "quest_db.yml"), p("db", "import", "quest_db.yml")):
        for e in yload(f).get("Body") or []:
            qids.add(e["Id"])
    used = defaultdict(set)
    for f in loaded_scripts():
        for m in re.finditer(r"\b(?:setquest|completequest|erasequest|checkquest|isbegin_quest|questprogress|changequest)\s*\(?\s*(\d{4,6})",
                             read_latin1(p(f))):
            used[int(m[1])].add(f)
    qpath = os.path.join(a.client, "dist", "data", "yaml_files", "quests.yml")
    client_q = set((yload(qpath).get("quests") or {}).keys())
    by_file = defaultdict(list)
    for q in sorted(qids - client_q):
        if q in used:
            by_file[sorted(used[q])[0]].append(q)
    for f, qs in sorted(by_file.items()):
        ko["7. journal de quetes"].append(f"{f} : {len(qs)} quete(s) sans texte au client ({qs[0]}..{qs[-1]})")

    # ---- 8. combos amont dont tous les objets sont charges
    have_cb = set()
    for c in yload(p("db", "import", "item_combos.yml")).get("Body") or []:
        for cb in c.get("Combos") or []:
            have_cb.add(tuple(sorted(cb["Combo"])))
    miss_cb = defaultdict(int)
    for c in yload(p("db", "re", "item_combos.yml")).get("Body") or []:
        for cb in c.get("Combos") or []:
            combo = [alias(x) for x in cb["Combo"]]
            if all(x in items for x in combo) and tuple(sorted(combo)) not in have_cb:
                miss_cb[" + ".join(combo)] += 1
    for k in sorted(miss_cb):
        ko["8. combos"].append(f"{k} : objets charges, combo amont absent de db/import/item_combos.yml")

    total = 0
    for k in sorted(ko):
        print(f"== {k} : {len(ko[k])} KO")
        for line in ko[k]:
            print("   ", line)
        total += len(ko[k])
    print(f"Total : {total} KO")
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
