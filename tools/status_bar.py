"""Bandeau d'état de MoonlightAdmin : pc-serv d'un coup d'œil.

Toutes les `intervalle_s` secondes (section [etat] de MoonlightAdmin.ini),
ssh lance tools/server_health.py dans le dépôt du serveur. Le bandeau montre
une pastille par service — vert en marche, rouge arrêté, gris inconnu —, puis
la charge, la mémoire, les disques et l'âge de la dernière sauvegarde. Un
clic sur une pastille ouvre le menu ./start de ce service, lancé dans la
fenêtre « Serveur (SSH) ».

Alertes : un service qui tombe, un disque plein, une sauvegarde trop vieille,
pc-serv injoignable. Chacune s'écrit une fois au journal, quand l'état
change, et non à chaque relevé ; elle sonne et fait clignoter la barre des
tâches si la case « Alertes » de la console est cochée.
"""
import json
import queue
import subprocess
import threading
import tkinter as tk
from datetime import datetime
from tkinter import ttk

from ssh_panel import START_ACTIONS, start_command
from tables_panel import NO_WINDOW

SETTINGS_SECTION = "etat"
DEFAULT_INTERVAL_S = 30
MIN_INTERVAL_S = 10
HEALTH_SCRIPT = "tools/server_health.py"
HEALTH_MISSING_CODE = 3
SSH_TIMEOUT_S = 25
DRAIN_MS = 300
MS_PER_S = 1000
# Relevés manqués d'affilée avant de déclarer pc-serv injoignable : un seul
# peut tomber pendant un redémarrage du réseau.
UNREACHABLE_AFTER = 2
DISK_WARN_PCT = 85
DISK_ALERT_PCT = 92
MEM_WARN_PCT = 90
LOAD_WARN_PER_CPU = 1.0
# /home/dump.sh tourne chaque nuit : au-delà d'un jour et deux heures, une
# nuit a été manquée.
BACKUP_ALERT_H = 26
MINUTES_PER_HOUR = 60
GIB_PER_TIB = 1024
HOURS_PER_DAY = 24
COLORS = {"up": "#3b3", "down": "#e44", "unknown": "#888", "warn": "#d90", "alert": "#e44", "ok": ""}
SERVICES = ("login", "char", "map", "web", "voice", "moonbot", "survivors")


def format_size(gib):
    """« 34 Gio », ou « 3,0 To » au-delà de mille Gio, virgule décimale."""
    if gib >= GIB_PER_TIB:
        return f"{gib / GIB_PER_TIB:.1f} To".replace(".", ",")
    return f"{gib:.0f} Gio"


def format_age(hours):
    if hours < 1:
        return f"{int(hours * MINUTES_PER_HOUR)} min"
    if hours < HOURS_PER_DAY:
        whole = int(hours)
        return f"{whole} h {int((hours - whole) * MINUTES_PER_HOUR):02d}"
    return f"{hours / HOURS_PER_DAY:.1f} j".replace(".", ",")


class StatusBar:
    """Le bandeau, placé dans `parent` sous le widget `after`."""

    def __init__(self, parent, after, settings, ssh_window, on_alert):
        self.ssh = ssh_window
        self.on_alert = on_alert  # (texte, urgent) -> None
        try:
            interval = int(settings.get(SETTINGS_SECTION, "intervalle_s", DEFAULT_INTERVAL_S))
        except ValueError:
            interval = DEFAULT_INTERVAL_S
        self.interval_s = max(MIN_INTERVAL_S, interval)
        self._results = queue.Queue()
        self._polling = False
        self._job = None         # prochain relevé planifié : un seul à la fois
        self._failures = 0
        self._running = {}       # service -> en marche au relevé précédent
        self._alerted = set()    # alertes en cours, pour ne les écrire qu'une fois

        # Deux lignes : les services, cliquables, puis les mesures.
        self.frame = ttk.LabelFrame(parent, text="pc-serv  (clic sur un service : ./start)")
        self.frame.pack(fill="x", padx=8, pady=(0, 4), after=after)
        services = ttk.Frame(self.frame)
        services.pack(fill="x", padx=4, pady=(2, 0))
        self.pills = {}
        for name in SERVICES:
            pill = ttk.Label(services, text=f"● {name}", foreground=COLORS["unknown"], cursor="hand2")
            pill.pack(side="left", padx=(0, 8))
            pill.bind("<Button-1>", lambda e, n=name: self._service_menu(e, n))
            self.pills[name] = pill
        ttk.Button(services, text="↻", width=3, command=self.poll_now).pack(side="right")
        self.stamp_lbl = ttk.Label(services, text="", foreground=COLORS["unknown"])
        self.stamp_lbl.pack(side="right", padx=6)
        metrics = ttk.Frame(self.frame)
        metrics.pack(fill="x", padx=4, pady=(0, 2))
        self.metrics = {}
        for key in ("load", "memory", "disks", "backup"):
            lbl = ttk.Label(metrics, text="")
            lbl.pack(side="left", padx=(0, 14))
            self.metrics[key] = lbl

        self.frame.after(DRAIN_MS, self._drain)
        self.frame.after(MS_PER_S, self.poll_now)

    def settings_data(self):
        return {"intervalle_s": self.interval_s}

    # ---- relevé -------------------------------------------------------------
    def _scheduled_poll(self):
        self._job = None
        self.poll_now()

    def poll_now(self):
        if self._polling:
            return
        self._polling = True
        argv = self.ssh.ssh_argv(f"if [ -f {HEALTH_SCRIPT} ]; then python3 {HEALTH_SCRIPT}; "
                                 f"else exit {HEALTH_MISSING_CODE}; fi")

        def worker():
            try:
                done = subprocess.run(argv, capture_output=True, stdin=subprocess.DEVNULL,
                                      timeout=SSH_TIMEOUT_S, creationflags=NO_WINDOW)
                result = (done.returncode, done.stdout)
            except (OSError, subprocess.TimeoutExpired):
                result = (-1, b"")
            self._results.put(result)

        threading.Thread(target=worker, daemon=True).start()

    def _drain(self):
        try:
            code, stdout = self._results.get_nowait()
        except queue.Empty:
            pass
        else:
            self._polling = False
            self._apply(code, stdout)
            if self._job is not None:
                self.frame.after_cancel(self._job)
            self._job = self.frame.after(self.interval_s * MS_PER_S, self._scheduled_poll)
        self.frame.after(DRAIN_MS, self._drain)

    def _apply(self, code, stdout):
        if code == HEALTH_MISSING_CODE:
            self._set_unknown(f"{HEALTH_SCRIPT} absent sur le serveur : git pull")
            return
        try:
            data = json.loads(stdout)
        except ValueError:
            self._failures += 1
            self._set_unknown("pc-serv injoignable" if self._failures >= UNREACHABLE_AFTER else "relevé manqué")
            if self._failures >= UNREACHABLE_AFTER:
                self._raise("unreachable", "pc-serv injoignable par ssh.", urgent=True)
            return
        if self._failures >= UNREACHABLE_AFTER:
            self.on_alert("pc-serv répond de nouveau.", False)
        self._failures = 0
        self._clear("unreachable")
        self._show_services(data["services"])
        self._show_metrics(data)
        self.stamp_lbl.config(text=f"relevé {datetime.now():%H:%M:%S}", foreground=COLORS["unknown"])

    def _set_unknown(self, reason):
        for pill in self.pills.values():
            pill.config(foreground=COLORS["unknown"])
        self.stamp_lbl.config(text=reason, foreground=COLORS["warn"])

    # ---- affichage ------------------------------------------------------------
    def _show_services(self, services):
        seen = {s["name"]: s for s in services}
        for name, pill in self.pills.items():
            service = seen.get(name)
            if service is None:
                pill.config(foreground=COLORS["unknown"])
                continue
            running = service["running"]
            pill.config(foreground=COLORS["up"] if running else COLORS["down"])
            before = self._running.get(name)
            if before and not running:
                if self.ssh.service_action_recent():
                    self.on_alert(f"Service {name} arrêté (arrêt demandé depuis MoonlightAdmin).", False)
                else:
                    self.on_alert(f"Service {name} arrêté.", True)
            elif before is False and running:
                self.on_alert(f"Service {name} reparti.", False)
            self._running[name] = running

    def _show_metrics(self, data):
        load = data["load"]
        per_cpu = load["load_1m"] / load["cpus"]
        self.metrics["load"].config(text=f"charge {load['load_1m']:.2f}",
                                    foreground=COLORS["warn"] if per_cpu > LOAD_WARN_PER_CPU else COLORS["ok"])
        memory = data["memory"]["used_pct"]
        self.metrics["memory"].config(text=f"mém {memory:.0f} %",
                                      foreground=COLORS["warn"] if memory > MEM_WARN_PCT else COLORS["ok"])

        # Le disque du serveur et du site ; deux s'ils venaient à se séparer.
        parts, level = [], "ok"
        for disk in data["disks"]:
            free = format_size(disk["free_gib"])
            parts.append(f"{disk['label']} : {free} libres ({disk['used_pct']:.0f} %)")
            if disk["used_pct"] > DISK_ALERT_PCT:
                level = "alert"
                self._raise(f"disk:{disk['label']}", f"Disque {disk['label']} plein à "
                            f"{disk['used_pct']:.0f} % ({free} libres).", urgent=True)
            else:
                self._clear(f"disk:{disk['label']}")
                if disk["used_pct"] > DISK_WARN_PCT and level == "ok":
                    level = "warn"
        self.metrics["disks"].config(text=", ".join(parts), foreground=COLORS[level])

        # Deux causes, deux alertes : une nuit manquée, une destination vide
        # (la clé USB démontée, le plus souvent).
        ages = [b["age_h"] for b in data["backups"] if b["age_h"] is not None]
        missing = [b["label"] for b in data["backups"] if b["age_h"] is None]
        oldest = max(ages) if ages else None
        late = oldest is not None and oldest > BACKUP_ALERT_H
        text = f"sauvegarde il y a {format_age(oldest)}" if ages else "aucune sauvegarde"
        if missing and ages:
            text += f" (aucune sur : {', '.join(missing)})"
        if late:
            self._raise("backup:late", f"Dernière sauvegarde il y a {format_age(oldest)}.", urgent=True)
        else:
            self._clear("backup:late")
        if missing:
            self._raise("backup:missing:" + ",".join(missing),
                        f"Aucune sauvegarde trouvée sur : {', '.join(missing)}.", urgent=True)
        else:
            for key in [k for k in self._alerted if k.startswith("backup:missing:")]:
                self._clear(key)
        self.metrics["backup"].config(text=text, foreground=COLORS["alert" if late or missing else "ok"])

    def _raise(self, key, text, urgent):
        if key not in self._alerted:
            self._alerted.add(key)
            self.on_alert(text, urgent)

    def _clear(self, key):
        self._alerted.discard(key)

    # ---- menu d'un service ------------------------------------------------------
    def _service_menu(self, event, name):
        menu = tk.Menu(self.frame, tearoff=0)
        for action, confirm in START_ACTIONS:
            menu.add_command(label=f"./start {action} {name}",
                             command=lambda a=action, c=confirm: self._run_start(a, name, c))
        menu.tk_popup(event.x_root, event.y_root)

    def _run_start(self, action, name, confirm):
        self.ssh.run_chain([start_command(action, name, confirm)], on_done=lambda _ok: self.poll_now())
