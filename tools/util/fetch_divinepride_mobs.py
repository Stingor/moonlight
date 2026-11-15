"""Récupère des fiches de monstres sur l'API Divine Pride, avec cache.

La clé est lue dans ~/.divine_pride_key (jamais affichée ni écrite ailleurs).
Chaque réponse est gardée dans tools/util/cache/divinepride/monster_<id>.json,
hors du suivi git : un mob déjà lu n'est jamais redemandé.

Limites de l'API : une requête par seconde au plus, jamais en parallèle.
Le script est séquentiel, attend 1,5 s entre deux requêtes et s'arrête net
sur toute erreur autre que 404, 429 compris : jamais de nouvelle tentative.

Usage :
  python tools/util/fetch_divinepride_mobs.py 21513-21552
  python tools/util/fetch_divinepride_mobs.py 21526 21530
  python tools/util/fetch_divinepride_mobs.py --aegis EP19_   # Id commentés de db/re/mob_db.yml
  python tools/util/fetch_divinepride_mobs.py --server iRO 21974-21978

--server choisit le serveur (en-tête x-server, kROM par défaut) ; ses réponses
ont leur propre cache, monster_<id>_<serveur>.json.
"""
import json
import os
import re
import sys
import time
import urllib.error
import urllib.request

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CACHE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "cache", "divinepride")
KEY_FILE = os.path.join(os.path.expanduser("~"), ".divine_pride_key")
BASE = "https://www.divine-pride.net/api/database/Monster/"
DELAY = 1.5


def read_key():
    with open(KEY_FILE, encoding="utf-8") as f:
        key = f.readline().strip()
    if not key:
        sys.exit(f"clé vide dans {KEY_FILE}")
    return key


def ids_from_args(args):
    if args[:1] == ["--aegis"]:
        prefix = args[1]
        text = open(os.path.join(ROOT, "db", "re", "mob_db.yml"), encoding="utf-8").read()
        pat = rf"^#?\s*- Id: (\d+)\s*\n#?\s*AegisName: {re.escape(prefix)}\S*"
        return [int(x) for x in re.findall(pat, text, re.M)]
    ids = []
    for a in args:
        if "-" in a:
            lo, hi = map(int, a.split("-"))
            ids += range(lo, hi + 1)
        else:
            ids.append(int(a))
    return ids


SERVER = None


def cache_path(mob_id):
    suffix = f"_{SERVER}" if SERVER else ""
    return os.path.join(CACHE, f"monster_{mob_id}{suffix}.json")


def fetch(mob_id, key):
    headers = {"Accept-Language": "en", "User-Agent": "moonlight-tools"}
    if SERVER:
        headers["x-server"] = SERVER
    req = urllib.request.Request(f"{BASE}{mob_id}?apiKey={key}", headers=headers)
    with urllib.request.urlopen(req, timeout=30) as r:
        return r.read()


def main():
    global SERVER
    args = sys.argv[1:]
    if args[:1] == ["--server"]:
        SERVER, args = args[1], args[2:]
    ids = ids_from_args(args)
    if not ids:
        sys.exit("aucun Id")
    os.makedirs(CACHE, exist_ok=True)
    key = None
    last = 0.0
    done = cached = missing = 0
    for mob_id in ids:
        if os.path.exists(cache_path(mob_id)):
            cached += 1
            continue
        key = key or read_key()
        wait = DELAY - (time.monotonic() - last)
        if wait > 0:
            time.sleep(wait)
        last = time.monotonic()
        try:
            body = fetch(mob_id, key)
        except urllib.error.HTTPError as e:
            if e.code == 404:
                print(f"{mob_id} : absent (404)")
                missing += 1
                continue
            # 429 compris : l'API demande d'arrêter, pas de réessayer.
            retry = e.headers.get("Retry-After")
            hint = f", Retry-After {retry} s" if retry else ""
            print(f"lus {done}, déjà en cache {cached}, absents {missing}")
            sys.exit(f"{mob_id} : HTTP {e.code}{hint}, arrêt ; relancer plus tard reprend au cache")
        data = json.loads(body)
        tmp = cache_path(mob_id) + ".tmp"
        with open(tmp, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=1)
        os.replace(tmp, cache_path(mob_id))
        print(f"{mob_id} : {data.get('dbname') or data.get('name')}")
        done += 1
    print(f"lus {done}, déjà en cache {cached}, absents {missing}")


if __name__ == "__main__":
    main()
