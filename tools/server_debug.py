#!/usr/bin/env python3
"""Débogage des services de pc-serv par gdb, pour le panneau SSH de MoonlightAdmin.

    python3 tools/server_debug.py cores [--json]
    python3 tools/server_debug.py analyse <core>
    python3 tools/server_debug.py snapshot <service>
    python3 tools/server_debug.py snapshot --pid <pid>
    python3 tools/server_debug.py delete <core>

cores : les cores que ./start range sous log/ (core.<service>.<date>), du
plus récent au plus ancien, avec leur taille.

analyse : gdb en lot sur un core, avec le binaire du service : la pile du fil
fautif avec ses variables (bt full), puis celle de tous les fils. Le service
qui tourne n'est pas touché. Un binaire recompilé depuis le plantage ne
correspond plus au core : la pile peut alors mentir, et l'analyse le dit en
tête.

snapshot : gdb s'attache au service en marche, relève la pile de tous ses
fils et se détache. Le service est suspendu le temps du relevé, une seconde
ou deux : les joueurs sentent un à-coup.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time

REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG_DIR = os.path.join(REPO_DIR, "log")
START = "./start"
SERVICE_BINARIES = {"login": "login-server", "char": "char-server", "map": "map-server",
                    "web": "web-server", "voice": "voice-server"}
CORE_RE = re.compile(r"^core\.(?P<service>[a-z]+)\.(?P<stamp>\d{8}-\d{6})$")
GDB = ["gdb", "-q", "-batch", "-nx", "-ex", "set pagination off", "-ex", "set print pretty on"]
START_TIMEOUT_S = 20
MIB = 1024 * 1024
SECONDS_PER_HOUR = 3600


def cores():
    found = []
    try:
        names = os.listdir(LOG_DIR)
    except OSError:
        return found
    for name in names:
        m = CORE_RE.match(name)
        if not m:
            continue
        path = os.path.join(LOG_DIR, name)
        stat = os.stat(path)
        binary = os.path.join(REPO_DIR, SERVICE_BINARIES.get(m["service"], ""))
        found.append({
            "file": f"log/{name}", "service": m["service"], "stamp": m["stamp"],
            "size_mib": round(stat.st_size / MIB, 1),
            "age_h": round((time.time() - stat.st_mtime) / SECONDS_PER_HOUR, 1),
            # Le binaire changé depuis : ses symboles ne collent plus au core.
            "binary_newer": os.path.isfile(binary) and os.path.getmtime(binary) > stat.st_mtime,
        })
    return sorted(found, key=lambda c: c["stamp"], reverse=True)


def core_path(arg):
    """Le chemin d'un core rangé par ./start, ou None : rien d'autre ne
    s'analyse ni ne s'efface par ce script."""
    name = os.path.basename(arg)
    if not CORE_RE.match(name):
        return None
    path = os.path.join(LOG_DIR, name)
    return path if os.path.isfile(path) else None


def analyse(arg):
    path = core_path(arg)
    if path is None:
        print(f"Pas un core de log/ : {arg}")
        return 2
    service = CORE_RE.match(os.path.basename(path))["service"]
    binary = os.path.join(REPO_DIR, SERVICE_BINARIES.get(service, ""))
    if not os.path.isfile(binary):
        print(f"Binaire du service {service} introuvable : {binary}")
        return 2
    if os.path.getmtime(binary) > os.path.getmtime(path):
        print(f"ATTENTION : {os.path.basename(binary)} a été recompilé après ce plantage ; "
              f"la pile peut nommer de mauvaises fonctions.")
    print(f"== {os.path.basename(path)}  ({os.path.basename(binary)})", flush=True)
    return subprocess.call(GDB + [
        "-ex", "echo \\n== fil fautif, avec ses variables\\n", "-ex", "bt full",
        "-ex", "echo \\n== tous les fils\\n", "-ex", "thread apply all bt",
        binary, path], cwd=REPO_DIR)


def service_pid(service):
    done = subprocess.run([START, "status", "--machine", service], cwd=REPO_DIR, capture_output=True,
                          text=True, timeout=START_TIMEOUT_S, check=False)
    for line in done.stdout.splitlines():
        fields = line.split("\t")
        if len(fields) == 3 and fields[0] == service and fields[1] == "running" and fields[2]:
            return int(fields[2].split()[0])
    return None


def snapshot(service, pid):
    if pid is None:
        if service not in SERVICE_BINARIES:
            print(f"Service sans binaire à déboguer : {service}  (connus : {', '.join(SERVICE_BINARIES)})")
            return 2
        pid = service_pid(service)
        if pid is None:
            print(f"{service} n'est pas en marche.")
            return 1
    print(f"== pile de p{pid}, relevée à {time.strftime('%H:%M:%S')} (processus suspendu le temps du relevé)",
          flush=True)
    return subprocess.call(GDB + ["-p", str(pid), "-ex", "thread apply all bt", "-ex", "detach"], cwd=REPO_DIR)


def delete(arg):
    path = core_path(arg)
    if path is None:
        print(f"Pas un core de log/ : {arg}")
        return 2
    os.remove(path)
    print(f"effacé : log/{os.path.basename(path)}")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    sub = parser.add_subparsers(dest="action", required=True)
    listing = sub.add_parser("cores")
    listing.add_argument("--json", action="store_true")
    sub.add_parser("analyse").add_argument("core")
    snap = sub.add_parser("snapshot")
    snap.add_argument("service", nargs="?")
    snap.add_argument("--pid", type=int)
    sub.add_parser("delete").add_argument("core")
    args = parser.parse_args()
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

    if args.action == "cores":
        found = cores()
        if args.json:
            print(json.dumps(found))
        elif not found:
            print("Aucun core sous log/.")
        else:
            for c in found:
                note = "  (binaire recompilé depuis)" if c["binary_newer"] else ""
                print(f"{c['file']}  {c['size_mib']} Mio  il y a {c['age_h']} h{note}")
        return 0
    if args.action == "analyse":
        return analyse(args.core)
    if args.action == "snapshot":
        if args.service is None and args.pid is None:
            parser.error("snapshot : un service ou --pid")
        return snapshot(args.service, args.pid)
    return delete(args.core)


if __name__ == "__main__":
    sys.exit(main())
