# Régénère data/MetalProcessItemList.txt depuis db/pre-re/produce_db.txt.
#
# Pourquoi : le fichier livré ne couvre que 90 produits sur les 254 que le serveur
# sait fabriquer, et rien ne garantit que ses noms correspondent à l'itemInfo du
# client. Le régénérer depuis la source serveur supprime les deux problèmes d'un
# coup.
#
# Format de sortie : STRICTEMENT celui d'origine (le natif rend ces lignes
# verbatim dans sa fenêtre 80), avec l'id du matériau ajouté entre crochets —
# extension rétro-compatible, lisible à l'écran, et qui dispense le client de
# toute résolution par le nom.
#
#     998#
#     1 Iron Ore [1002]
#     #
#
# produce_db.txt : ID,ProduceItemID,ItemLV,RequireSkill,RequireSkillLv,MatID1,MatAmt1,...
# Une quantité de 0 signifie « doit être possédé mais n'est pas consommé »
# (les guides de fabrication) : on l'écrit « 1 » suivi d'une mention, sinon la
# ligne dirait « 0 Marine Sphere Bottle », ce qui n'a pas de sens à l'écran.

import re
import sys
from pathlib import Path

SERVER = Path(r"d:/Mes documents/GitHub/moonlight")
CLIENT = Path(r"d:/Mes documents/GitHub/Moonlight-Client/SystemEN")
OUT    = Path(sys.argv[1] if len(sys.argv) > 1 else "MetalProcessItemList.txt")

# ── Noms d'items : itemInfokro d'abord, itemInfomoon par-dessus (il gagne) ──────
name_by_id = {}
entry_re = re.compile(r"\[(\d+)\]\s*=\s*\{")
name_re  = re.compile(r'Name\s*=\s*"((?:[^"\\]|\\.)*)"')

for fname in ("itemInfokro.lua", "itemInfomoon.lua"):
    path = CLIENT / fname
    if not path.exists():
        print(f"  (absent : {fname})")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    count = 0
    for m in entry_re.finditer(text):
        item_id = int(m.group(1))
        # Le Name est la première occurrence après l'ouverture de l'entrée ;
        # on borne la recherche au bloc pour ne pas capter celui du suivant.
        chunk = text[m.end(): m.end() + 4000]
        nm = name_re.search(chunk)
        if nm:
            name_by_id[item_id] = nm.group(1)
            count += 1
    print(f"  {fname} : {count} noms")

print(f"Total noms connus : {len(name_by_id)}")

# ── Recettes ───────────────────────────────────────────────────────────────────
recipes = []       # [(product_id, itemlv, req_skill, req_skill_lv, [(mat_id, qty), ...])]
seen_products = set()
missing_names = set()

for raw in (SERVER / "db/pre-re/produce_db.txt").read_text(
        encoding="utf-8", errors="replace").splitlines():
    line = raw.strip()
    if not line or line.startswith("//"):
        continue
    parts = [p.strip() for p in line.split(",")]
    if len(parts) < 6:
        continue
    try:
        product = int(parts[1])
    except ValueError:
        continue
    if product == 0:            # entrée neutralisée par un import
        continue
    if product in seen_products:  # doublon serveur (cf. Steel 999) : on garde le 1er
        continue

    mats = []
    rest = parts[5:]
    for i in range(0, len(rest) - 1, 2):
        try:
            mat_id, amount = int(rest[i]), int(rest[i + 1])
        except ValueError:
            continue
        if mat_id:
            mats.append((mat_id, amount))
    if not mats:
        continue

    # itemlv et compétence requise : inutiles au fichier NATIF (son format ne sait
    # pas les exprimer) mais indispensables au nôtre — c'est `itemlv` qui classe
    # arme/nourriture/autre côté serveur, et `req_skill` qui permet l'index par
    # compétence dont l'Atlas a besoin.
    try:
        itemlv, req_skill, req_skill_lv = int(parts[2]), int(parts[3]), int(parts[4])
    except ValueError:
        itemlv, req_skill, req_skill_lv = 0, 0, 0

    seen_products.add(product)
    recipes.append((product, itemlv, req_skill, req_skill_lv, mats))
    for mat_id, _ in mats:
        if mat_id not in name_by_id:
            missing_names.add(mat_id)

# ── Flèches : create_arrow_db.yml ──────────────────────────────────────────────
# La fabrication de flèches ne passe PAS par produce_db : elle vit dans
# create_arrow_db.yml, et le client n'en a jamais rien su — d'où « Faisable : — »
# sur toutes les lignes du Hunter. Rien n'empêche de les verser ici : le natif ne
# lit ce fichier que depuis sa fenêtre 80, que les flèches n'ouvrent jamais.
#
# ⚠ Deux différences de nature avec produce_db :
#  1. la clé est le MATÉRIAU (c'est lui que la liste propose, et lui que
#     `skill_arrow_create` supprime), pas le produit ;
#  2. il y a un RENDEMENT à décrire. On l'écrit avec une quantité NÉGATIVE —
#     « -40 Arrow [1750] » —, marqueur « produit » que le plugin rend « → 40 »
#     et exclut du calcul de faisabilité. Une quantité positive resterait une
#     exigence et plafonnerait le compte au stock de flèches déjà possédées.
#
# Ce fichier désigne les objets par AegisName : il faut la table nom -> id, qui
# vit dans les item_db du serveur.
id_by_aegis = {}
db_dir = SERVER / "db"
db_files = sorted(db_dir.glob("pre-re/item_db*.yml")) + sorted(db_dir.glob("item_db*.yml"))
cur_id = None
for path in db_files:
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"\s*-?\s*Id:\s*(\d+)", line)
        if m:
            cur_id = int(m.group(1))
            continue
        m = re.match(r"\s*AegisName:\s*(\S+)", line)
        if m and cur_id is not None:
            id_by_aegis.setdefault(m.group(1), cur_id)
            cur_id = None
print(f"Total AegisName connus : {len(id_by_aegis)}")

arrow_recipes = []      # [(source_id, [(item_id, amount), ...])]
arrow_missing = set()
arrow_path = db_dir / "create_arrow_db.yml"
if arrow_path.exists():
    source_aegis = None
    made = []
    pending_item = None

    def flush_arrow():
        """Clôt l'entrée courante. Nommée plutôt qu'inline : le YAML se ferme
        aussi bien sur une nouvelle Source que sur la fin du fichier."""
        if source_aegis is None:
            return
        sid = id_by_aegis.get(source_aegis)
        if sid is None:
            arrow_missing.add(source_aegis)
            return
        if made:
            arrow_recipes.append((sid, list(made)))

    for line in arrow_path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"\s*-\s*Source:\s*(\S+)", line)
        if m:
            flush_arrow()
            source_aegis, made, pending_item = m.group(1), [], None
            continue
        m = re.match(r"\s*-\s*Item:\s*(\S+)", line)
        if m:
            pending_item = m.group(1)
            continue
        m = re.match(r"\s*Amount:\s*(\d+)", line)
        if m and pending_item:
            amount = int(m.group(1))
            iid = id_by_aegis.get(pending_item)
            if iid is None:
                arrow_missing.add(pending_item)
            elif amount > 0:       # 0 = entrée retirée par un import
                made.append((iid, amount))
            pending_item = None
    flush_arrow()
print(f"{len(arrow_recipes)} recettes de fleches")

# ── Écriture ───────────────────────────────────────────────────────────────────
out_lines = []
for product, itemlv, req_skill, req_skill_lv, mats in recipes:
    out_lines.append(f"{product}#")
    for mat_id, amount in mats:
        name = name_by_id.get(mat_id, f"Item {mat_id}")
        # La quantité 0 EST le marqueur « doit être possédé, pas consommé » —
        # celui de produce_db, qu'on se contente de recopier. Une première version
        # écrivait « 1 … (non consomme) » pour que la ligne se lise seule ; c'était
        # une erreur de couche : le fichier porte la DONNÉE, le libellé appartient
        # à l'interface (qui affiche « requis, non consommé »). Et le suffixe
        # cassait l'extraction de l'id, qui exigeait « ] » en fin de ligne.
        out_lines.append(f"{amount} {name} [{mat_id}]")
    out_lines.append("#")

# 🔴 Les recettes de flèches vivent dans un ESPACE DE CLÉS SÉPARÉ : `id + 1000000`.
# Raison : un même id est souvent à la fois produit de `produce_db` ET source de
# flèches — Phracon, Coal, Elunium, Green Live… La table du client est indexée par
# id, donc l'un écrasait l'autre, et la liste du Hunter affichait des recettes de
# FORGE (⏱ constaté en jeu : Phracon montrait « 45 Spawn, 40 Glass Bead »).
# Un simple garde-fou de collision ne réglait rien : il choisissait un perdant.
# La clé biaisée permet de garder les DEUX. Le natif ne consulte jamais ces clés
# (il n'interroge que l'id du produit visé dans sa fenêtre 80), et les ids d'objets
# plafonnent bien en dessous du million : aucun risque de recouvrement.
ARROW_KEY_BIAS = 1000000

for source_id, made in arrow_recipes:
    src_name = name_by_id.get(source_id, f"Item {source_id}")
    out_lines.append(f"{source_id + ARROW_KEY_BIAS}#")
    # Le matériau est consommé à raison de UN par fabrication
    # (`pc_delitem(sd, j, 1, ...)`) : c'est lui qui borne la faisabilité.
    out_lines.append(f"1 {src_name} [{source_id}]")
    for item_id, amount in made:
        out_lines.append(f"-{amount} {name_by_id.get(item_id, f'Item {item_id}')} "
                         f"[{item_id}]")
    out_lines.append("#")

# ⚠ newline="" est OBLIGATOIRE. Sans lui, Path.write_text ouvre en mode TEXTE et
# Windows traduit chaque "\n" en "\r\n" : nos "\r\n" devenaient "\r\r\n", et
# l'éditeur comptait le CR isolé comme un saut de ligne supplémentaire — d'où une
# ligne vide entre chaque ligne du fichier généré.
# On écrit donc les fins de ligne à la main, sans traduction.
with OUT.open("w", encoding="cp949", errors="replace", newline="") as out_file:
    out_file.write("\r\n".join(out_lines) + "\r\n")

print(f"\n{len(recipes)} recettes + {len(arrow_recipes)} fleches ecrites dans {OUT}")

# ── Sortie YAML : le fichier de Bourgeon ───────────────────────────────────────
# Deux fichiers, deux publics, et c'est volontaire :
#  - MetalProcessItemList.txt sert le CLIENT NATIF (fenêtre 80). Son format est
#    celui de Gravity : des lignes de texte, une clé par produit, rien d'autre.
#    Les joueurs qui n'activent pas l'interface moderne en profitent — le plugin
#    est opt-in, ce fichier reste donc utile.
#  - bourgeon_recipes.yaml sert l'interface ImGui. Il porte ce que le premier ne
#    peut PAS exprimer : itemlv, compétence requise, rendements des flèches, et
#    surtout un INDEX PAR COMPÉTENCE — sans lequel on ne peut pas énumérer les
#    formules d'un métier (le getteur natif est une lookup par clé, la table n'est
#    pas parcourable).
#
# Il va dans SystemEN\, auprès d'itemInfoMerged.lua : c'est de la donnée de patch,
# livrée aux joueurs, pas un réglage utilisateur.
YAML_OUT = Path(sys.argv[2]) if len(sys.argv) > 2 else OUT.with_name("bourgeon_recipes.yaml")

# ── Refine : chances et niveaux d'arme ─────────────────────────────────────────
# Le taux de refine est le SEUL entièrement calculable côté client :
#   per = Rate/100 + (classe 3 ? +10 : (job_level - 50) / 2)   puis  per > rnd()%100
# donc la probabilité EST `per` %, sans terme aléatoire dans le calcul (contrairement
# à `make_per` de la forge, qui contient un rnd_value(1,100)*10).
# Il manque deux choses au client, qu'on embarque ici :
#   - le Rate par (niveau d'arme, refine VISÉ), type « Normal » ;
#   - le niveau d'arme de chaque objet — absent du paquet ZC 0x0221 comme de
#     l'itemInfo.
refine_weapon = {}      # {weapon_level: {target_refine: rate}}
group = None
wlevel = None
rlevel = None
in_chances = False
chance_type = None
for line in (SERVER / "db/pre-re/refine.yml").read_text(
        encoding="utf-8", errors="replace").splitlines():
    m = re.match(r"\s*-\s*Group:\s*(\S+)", line)
    if m:
        group, wlevel, rlevel = m.group(1), None, None
        continue
    m = re.match(r"(\s*)-\s*Level:\s*(\d+)", line)
    if m:
        # L'indentation distingue le niveau d'ARME du niveau de REFINE : le YAML
        # réutilise la clé « Level » aux deux étages.
        indent = len(m.group(1))
        if indent <= 8:
            wlevel, rlevel = int(m.group(2)), None
        else:
            rlevel = int(m.group(2))
        in_chances = False
        continue
    if re.match(r"\s*Chances:", line):
        in_chances = True
        continue
    m = re.match(r"\s*-\s*Type:\s*(\S+)", line)
    if m:
        chance_type = m.group(1)
        continue
    m = re.match(r"\s*Rate:\s*(\d+)", line)
    if m and group == "Weapon" and in_chances and chance_type == "Normal" \
            and wlevel and rlevel:
        refine_weapon.setdefault(wlevel, {})[rlevel] = int(m.group(1))

# ── Réglages de bataille qui MULTIPLIENT les chances de fabrication ───────────
# 🔴 Sans eux, aucun calcul de chance n'est juste côté client, et l'erreur est
# silencieuse : `weapon_produce_rate: 500` multiplie `make_per` par CINQ
# (skill.cpp : `make_per = make_per * battle_config.wp_rate / 100`).
#
# ⚠ L'ORDRE COMPTE : `conf/import/battle_conf.txt` SURCHARGE `conf/battle/items.conf`.
# Sur ce serveur, items.conf annonce 100 et l'import 500 — lire le premier seulement
# donnerait un chiffre cinq fois trop bas.
#
# `oridecon_research_fix` conditionne le bonus BS_ORIDEOCON (armes de niveau >= 3) ;
# il vaut `no` ici, donc ce terme ne s'applique pas — mais on l'embarque pour que le
# client n'ait pas à le supposer.
battle_rates = {"weapon_produce": 100, "potion_produce": 100,
                "oridecon_research_fix": 0}
_rate_keys = {"weapon_produce_rate": "weapon_produce",
              "potion_produce_rate": "potion_produce"}
for conf in ("conf/battle/items.conf", "conf/battle/skill.conf",
             "conf/import/battle_conf.txt"):
    path = SERVER / conf
    if not path.exists():
        continue
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = line.split("//", 1)[0].strip()
        m = re.match(r"(\w+)\s*:\s*(\S+)", line)
        if not m:
            continue
        key, raw = m.group(1), m.group(2)
        if key in _rate_keys:
            battle_rates[_rate_keys[key]] = int(raw)
        elif key == "oridecon_research_fix":
            battle_rates["oridecon_research_fix"] = \
                1 if raw.lower() in ("yes", "1", "true", "on") else 0
print("Taux de bataille : " + ", ".join(f"{k}={v}" for k, v in battle_rates.items()))

# ── Kits de cuisine : itemId -> niveau (`cooking N;` dans le script d'objet) ────
#
# 🔴 Le niveau du kit N'EST DANS AUCUN PAQUET. Le serveur le range dans
# `menuskill_val` et ne l'envoie jamais ; le client ne voit qu'un `mk_type = 1`
# identique pour les cinq kits. Or c'est le terme DOMINANT de la réussite d'un plat
# (`1200 * (val - 10)`, soit +12 % par palier), et à `val >= 15` la réussite est
# GARANTIE (`make_per = 10000`). Sans cette table, la fenêtre ne peut rien annoncer.
#
# Le client retrouve le kit par l'objet CONSOMMÉ, qu'il observe sur CZ_USE_ITEM.
#
# ⚠ L'import est lu APRÈS le db de base et l'écrase — mêmes règles que le serveur.
# ⚠ Un `cooking N;` avec N hors [11,20] n'est PAS de la cuisine : le serveur teste
# `menuskill_val > 10 && <= 20` avant d'appliquer la formule, et retombe sinon sur
# `make_per = 5000`. Cas réel : 12849 Combination Kit fait `cooking 30;`, pour
# lequel produce_db n'a AUCUNE recette d'itemlv 30 — liste toujours vide. On les
# embarque quand même, marqués par leur niveau réel : c'est au client de dire
# « ce kit ne sert à rien ici » plutôt que de l'ignorer en silence.
cooking_kits = {}
cur_id = None
for rel in ("pre-re/item_db_usable.yml", "import/items/item_db_usable.yml",
            "import/item_db_usable.yml"):
    path = db_dir / rel
    if not path.exists():
        continue
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"\s*-?\s*Id:\s*(\d+)", line)
        if m:
            cur_id = int(m.group(1))
            continue
        m = re.search(r"cooking\s+(\d+)\s*;", line)
        if m and cur_id is not None:
            cooking_kits[cur_id] = int(m.group(1))
print(f"Cuisine : {len(cooking_kits)} kits "
      + ", ".join(f"{name_by_id.get(i, i)}={lv}"
                  for i, lv in sorted(cooking_kits.items())))

weapon_lv = {}
cur_id = None
for path in sorted(db_dir.glob("pre-re/item_db_equip.yml")):
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"\s*-?\s*Id:\s*(\d+)", line)
        if m:
            cur_id = int(m.group(1))
            continue
        m = re.match(r"\s*WeaponLevel:\s*(\d+)", line)
        if m and cur_id is not None:
            weapon_lv[cur_id] = int(m.group(1))
print(f"Refine : {sum(len(v) for v in refine_weapon.values())} taux, "
      f"{len(weapon_lv)} armes avec niveau")

by_skill = {}
for product, itemlv, req_skill, _lv, _mats in recipes:
    by_skill.setdefault(req_skill, []).append(product)

def pairs(seq):
    """Paires en style FLOW : « [[7472, 0], [940, 5]] ». Un fichier de 400 recettes
    en style bloc ferait quatre mille lignes pour la même information."""
    return "[" + ", ".join(f"[{a}, {b}]" for a, b in seq) + "]"

y = []
y.append("# GÉNÉRÉ par moonlight/tools/gen_metalprocess.py — NE PAS ÉDITER À LA MAIN.")
y.append("# Source : db/pre-re/produce_db.txt + db/create_arrow_db.yml + item_db*.yml")
y.append("version: 1")
y.append("")
y.append("# Une entrée par produit fabricable.")
y.append("#   lv       : itemlv de produce_db — 1-3 arme, 11-20 nourriture, sinon exact")
y.append("#   skill    : compétence requise (0 = fabrication par script d'objet)")
y.append("#   mats     : [id, quantité] ; quantité 0 = à POSSÉDER, non consommé (guides)")
y.append("recipes:")
for product, itemlv, req_skill, req_skill_lv, mats in recipes:
    name = name_by_id.get(product, f"Item {product}")
    y.append(f"  # {name}")
    y.append(f"  - {{id: {product}, lv: {itemlv}, skill: {req_skill}, "
             f"skill_lv: {req_skill_lv}, mats: {pairs(mats)}}}")
y.append("")
y.append("# Fabrication de flèches (db/create_arrow_db.yml). ⚠ La clé est le MATÉRIAU")
y.append("# consommé (un par fabrication), et « yields » ce qu'on obtient.")
y.append("arrows:")
for source_id, made in arrow_recipes:
    y.append(f"  # {name_by_id.get(source_id, f'Item {source_id}')}")
    y.append(f"  - {{src: {source_id}, yields: {pairs(made)}}}")
y.append("")
y.append("# Index compétence -> produits. C'est LUI qui rend l'Atlas possible : la")
y.append("# table du client n'est interrogeable que par clé, jamais parcourable.")
y.append("by_skill:")
for skill_id in sorted(by_skill):
    ids = ", ".join(str(i) for i in by_skill[skill_id])
    y.append(f"  {skill_id}: [{ids}]")
y.append("")
y.append("# ── Réglages serveur indispensables au calcul des chances ─────────────")
y.append("# 🔴 `weapon_produce` / `potion_produce` MULTIPLIENT make_per :")
y.append("#     make_per = make_per * rate / 100     (skill.cpp)")
y.append("# À 500, les chances de forge sont donc quintuplées. Un client qui l'ignore")
y.append("# annonce un chiffre cinq fois trop bas — d'où leur présence ici.")
y.append("# `oridecon_research_fix` conditionne le bonus BS_ORIDEOCON (armes lv >= 3).")
y.append("# ⚠ Lus dans items.conf PUIS conf/import/battle_conf.txt, qui surcharge.")
y.append("rates:")
for key in ("weapon_produce", "potion_produce", "oridecon_research_fix"):
    y.append(f"  {key}: {battle_rates[key]}")
y.append("")
y.append("# ── Kits de cuisine : itemId -> niveau (`cooking N;`) ─────────────────")
y.append("# 🔴 Le niveau du kit n'est dans AUCUN paquet (le serveur le garde dans")
y.append("# menuskill_val), et c'est le terme dominant : +12 % par palier, et")
y.append("# réussite GARANTIE à partir de 15. Le client retrouve le kit par l'objet")
y.append("# consommé, observé sur CZ_USE_ITEM.")
y.append("# ⚠ Un niveau hors [11,20] n'est pas de la cuisine : le serveur retombe")
y.append("# alors sur un 50 % plat. (12849 Combination Kit -> 30, et produce_db n'a")
y.append("# aucune recette d'itemlv 30 : sa liste est toujours vide.)")
y.append("cooking_kits:")
for item_id in sorted(cooking_kits):
    y.append(f"  {item_id}: {cooking_kits[item_id]}"
             f"   # {name_by_id.get(item_id, f'Item {item_id}')}")
y.append("")
y.append("# ── Refine ────────────────────────────────────────────────────────────")
y.append("# `refine_weapon[niveau d'arme][refine VISÉ] = Rate` (type Normal, /100 = %).")
y.append("# Le client calcule alors : per = Rate/100 + (classe 3 ? 10 : (job_lv-50)/2),")
y.append("# et la probabilité EST per % — le tirage est `per > rnd()%100`.")
y.append("refine_weapon:")
for wl in sorted(refine_weapon):
    inner = ", ".join(f"{r}: {refine_weapon[wl][r]}"
                      for r in sorted(refine_weapon[wl]))
    y.append(f"  {wl}: {{{inner}}}")
y.append("")
y.append("# Niveau d'arme par objet — ABSENT du paquet ZC 0x0221 comme de l'itemInfo,")
y.append("# et pourtant indispensable pour choisir la bonne ligne ci-dessus.")
y.append("# Les ids consécutifs de même niveau sont COMPRESSÉS en plages « a-b: lv » :")
y.append("# les armes se suivent par familles (1101-1109 = épées lv 1), donc 707")
y.append("# entrées se réduisent à quelques dizaines. `1101-1109` reste un scalaire")
y.append("# YAML valide — le tiret n'est un problème qu'en tête de clé suivi d'une")
y.append("# espace.")
y.append("weapon_lv:")
sorted_ids = sorted(weapon_lv)
start = 0
while start < len(sorted_ids):
    end = start
    while (end + 1 < len(sorted_ids)
           and sorted_ids[end + 1] == sorted_ids[end] + 1
           and weapon_lv[sorted_ids[end + 1]] == weapon_lv[sorted_ids[start]]):
        end += 1
    level = weapon_lv[sorted_ids[start]]
    if end > start:
        y.append(f"  {sorted_ids[start]}-{sorted_ids[end]}: {level}")
    else:
        y.append(f"  {sorted_ids[start]}: {level}")
    start = end + 1

with YAML_OUT.open("w", encoding="utf-8", newline="") as yaml_file:
    yaml_file.write("\n".join(y) + "\n")
print(f"{len(recipes)} recettes, {len(arrow_recipes)} fleches, "
      f"{len(by_skill)} competences -> {YAML_OUT}")
if arrow_missing:
    print(f"ATTENTION {len(arrow_missing)} AegisName introuvables : "
          + ", ".join(sorted(arrow_missing)[:12]))
if missing_names:
    print(f"ATTENTION {len(missing_names)} materiaux sans nom dans l'itemInfo : "
          + ", ".join(str(i) for i in sorted(missing_names)[:20]))
