"""Section « Débogage (gdb) » du panneau SSH de MoonlightAdmin.

Deux lignes, qui passent par tools/server_debug.py sur pc-serv :

  - Plantages : les cores que ./start a rangés sous log/ à la mort d'un
    service. « Analyser » lance gdb en lot sur celui choisi, sans toucher
    au service qui tourne ; la pile s'écrit dans la sortie du panneau. Un
    binaire recompilé depuis le plantage est signalé : sa pile peut mentir.
  - Pile à chaud : gdb s'attache au service choisi, relève la pile de tous
    ses fils et se détache. Le service est suspendu une seconde ou deux, d'où
    la confirmation. C'est l'outil d'un serveur figé ou à 100 % de CPU.

Un gdb interactif passe par « Terminal… » : attaché, il arrête le service
tant qu'on ne le relance pas, ce qu'aucun bouton ne doit faire à l'aveugle.
"""
import json
import queue
import shlex
import subprocess
import threading
from tkinter import messagebox, ttk

from tables_panel import NO_WINDOW

DEBUG_SCRIPT = "tools/server_debug.py"
DEBUGGABLE_SERVICES = ("map", "char", "login", "web", "voice")
DEFAULT_SNAPSHOT_SERVICE = "map"
LIST_TIMEOUT_S = 25
DRAIN_MS = 200
HOURS_PER_DAY = 24


def core_label(core):
    age = core["age_h"]
    when = f"il y a {age:.0f} h" if age < HOURS_PER_DAY else f"il y a {age / HOURS_PER_DAY:.0f} j"
    note = "  ·  binaire recompilé depuis" if core["binary_newer"] else ""
    return f"{core['file']}  ·  {core['size_mib']:.0f} Mio  ·  {when}{note}"


class DebugSection:
    def __init__(self, parent, ssh_window, make_command):
        self.ssh = ssh_window
        self._command = make_command  # RemoteCommand, passé pour éviter l'import circulaire
        self._results = queue.Queue()
        self._listing = False
        self._cores = {}  # libellé -> chemin du core

        frame = ttk.LabelFrame(parent, text="Débogage (gdb)  — gdb interactif : Terminal…")
        frame.pack(fill="x", padx=8, pady=4)
        crash = ttk.Frame(frame)
        crash.pack(fill="x", padx=4, pady=(4, 2))
        ttk.Label(crash, text="Plantages").pack(side="left", padx=(0, 4))
        self.core_box = ttk.Combobox(crash, state="readonly", width=60)
        self.core_box.pack(side="left", padx=2, fill="x", expand=True)
        self.buttons = [ttk.Button(crash, text="Analyser", command=self._analyse),
                        ttk.Button(crash, text="Supprimer…", command=self._delete)]
        for button in self.buttons:
            button.pack(side="left", padx=2)
        ttk.Button(crash, text="↻", width=3, command=self.refresh).pack(side="left", padx=(2, 0))

        live = ttk.Frame(frame)
        live.pack(fill="x", padx=4, pady=(2, 4))
        ttk.Label(live, text="Pile à chaud").pack(side="left", padx=(0, 4))
        self.service_box = ttk.Combobox(live, values=DEBUGGABLE_SERVICES, state="readonly", width=8)
        self.service_box.set(DEFAULT_SNAPSHOT_SERVICE)
        self.service_box.pack(side="left", padx=2)
        ttk.Button(live, text="Relever la pile…", command=self._snapshot).pack(side="left", padx=2)
        ttk.Label(live, text="suspend le service une seconde ou deux : pour un serveur figé ou à 100 % de CPU",
                  foreground="#888").pack(side="left", padx=8)
        parent.after(DRAIN_MS, lambda: self._drain(parent))

    # ---- liste des cores --------------------------------------------------------
    def refresh(self):
        if self._listing:
            return
        self._listing = True
        argv = self.ssh.ssh_argv(f"if [ -f {DEBUG_SCRIPT} ]; then python3 {DEBUG_SCRIPT} cores --json; "
                                 f"else echo '[]'; fi")

        def worker():
            try:
                done = subprocess.run(argv, capture_output=True, stdin=subprocess.DEVNULL,
                                      timeout=LIST_TIMEOUT_S, creationflags=NO_WINDOW)
                found = json.loads(done.stdout) if done.returncode == 0 else None
            except (OSError, subprocess.TimeoutExpired, ValueError):
                found = None
            self._results.put(found)

        threading.Thread(target=worker, daemon=True).start()

    def _drain(self, widget):
        try:
            found = self._results.get_nowait()
        except queue.Empty:
            pass
        else:
            self._listing = False
            self._apply(found)
        widget.after(DRAIN_MS, lambda: self._drain(widget))

    def _apply(self, found):
        if found is None:
            self.core_box.config(values=())
            self.core_box.set("relevé impossible")
            return
        self._cores = {core_label(c): c["file"] for c in found}
        labels = list(self._cores)
        self.core_box.config(values=labels)
        self.core_box.set(labels[0] if labels else "aucun plantage enregistré")

    def _selected_core(self):
        return self._cores.get(self.core_box.get())

    # ---- actions ------------------------------------------------------------
    def _analyse(self):
        core = self._selected_core()
        if core is None:
            return
        self.ssh.run_chain([self._command(f"analyse {core}", f"python3 {DEBUG_SCRIPT} analyse {shlex.quote(core)}")])

    def _delete(self):
        core = self._selected_core()
        if core is None:
            return
        if not messagebox.askyesno("Supprimer le core", f"Effacer {core} ?\n\nSa pile ne pourra plus "
                                   "être relue.", parent=self.core_box, icon="warning", default="no"):
            return
        self.ssh.run_chain([self._command(f"delete {core}", f"python3 {DEBUG_SCRIPT} delete {shlex.quote(core)}")])

    def _snapshot(self):
        service = self.service_box.get()
        if not messagebox.askyesno(
                "Relever la pile", f"gdb va s'attacher à {service} et le suspendre le temps du relevé, "
                "une seconde ou deux : les joueurs sentiront un à-coup.\n\nRelever la pile ?",
                parent=self.service_box, icon="warning", default="no"):
            return
        self.ssh.run_chain([self._command(f"pile de {service}",
                                          f"python3 {DEBUG_SCRIPT} snapshot {shlex.quote(service)}")])
