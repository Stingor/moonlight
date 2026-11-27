#!/usr/bin/env python3
"""État de pc-serv en une ligne JSON, pour le bandeau de MoonlightAdmin.

    python3 tools/server_health.py

Services (./start status --machine), charge, mémoire, disque du serveur et du
site, et âge de la
dernière sauvegarde de chaque dossier de sauvegarde. Lecture seule : rien
n'est lancé ni écrit.
"""
import json
import os
import re
import subprocess
import time

REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
START = "./start"
START_TIMEOUT_S = 20
# (libellé, dossier) : chaque nuit, /home/dump.sh y dépose un dossier daté.
BACKUP_DIRS = (("disque", "/home/dumps"), ("usb", "/mnt/usb/_backup-pcserv/dumps"))
# (libellé, dossier) dont on suit le disque ; deux dossiers sur le même disque
# n'en donnent qu'une mesure, « serveur et site ». La clé USB des sauvegardes
# se suit par l'âge de la sauvegarde qu'elle reçoit.
WATCHED_DIRS = (("serveur", REPO_DIR), ("site", "/srv/moonlight-web"))
DATED_DIR_RE = re.compile(r"^\d{4}-\d{2}-\d{2}$")
KIB = 1024
GIB = 1024 ** 3
SECONDS_PER_HOUR = 3600


def services():
    """[{nom, état, pid}] tels que les donne ./start ; vide s'il échoue."""
    try:
        done = subprocess.run([START, "status", "--machine"], cwd=REPO_DIR, capture_output=True,
                              text=True, timeout=START_TIMEOUT_S, check=False)
    except (OSError, subprocess.TimeoutExpired):
        return []
    found = []
    for line in done.stdout.splitlines():
        fields = line.split("\t")
        if len(fields) == 3:
            found.append({"name": fields[0], "running": fields[1] == "running", "pids": fields[2]})
    return found


def load():
    with open("/proc/loadavg") as f:
        one, five, fifteen = (float(v) for v in f.read().split()[:3])
    return {"load_1m": one, "load_5m": five, "load_15m": fifteen, "cpus": os.cpu_count() or 1}


def memory():
    values = {}
    with open("/proc/meminfo") as f:
        for line in f:
            key, rest = line.split(":", 1)
            values[key] = int(rest.split()[0]) * KIB
    total, available = values["MemTotal"], values["MemAvailable"]
    return {"used_pct": round(100 * (total - available) / total, 1),
            "total_gib": round(total / GIB, 1)}


def disks():
    """Un relevé par disque portant un dossier suivi, libellés réunis."""
    by_device = {}
    for label, folder in WATCHED_DIRS:
        try:
            device = os.stat(folder).st_dev
        except OSError:
            continue
        by_device.setdefault(device, (folder, []))[1].append(label)
    found = []
    for folder, labels in by_device.values():
        stat = os.statvfs(folder)
        total = stat.f_blocks * stat.f_frsize
        free = stat.f_bavail * stat.f_frsize
        found.append({"label": " et ".join(labels), "used_pct": round(100 * (total - free) / total, 1),
                      "free_gib": round(free / GIB, 1), "total_gib": round(total / GIB, 1)})
    return found


def backup(label, folder):
    """Âge en heures du dernier dossier daté ; None si le dossier est absent ou vide."""
    try:
        dated = sorted(name for name in os.listdir(folder) if DATED_DIR_RE.match(name))
    except OSError:
        dated = []
    if not dated:
        return {"label": label, "folder": folder, "latest": None, "age_h": None}
    mtime = os.path.getmtime(os.path.join(folder, dated[-1]))
    return {"label": label, "folder": folder, "latest": dated[-1],
            "age_h": round((time.time() - mtime) / SECONDS_PER_HOUR, 1)}


def uptime_h():
    with open("/proc/uptime") as f:
        return round(float(f.read().split()[0]) / SECONDS_PER_HOUR, 1)


def main():
    print(json.dumps({
        "services": services(),
        "load": load(),
        "memory": memory(),
        "disks": disks(),
        "backups": [backup(label, folder) for label, folder in BACKUP_DIRS],
        "uptime_h": uptime_h(),
    }))


if __name__ == "__main__":
    main()
