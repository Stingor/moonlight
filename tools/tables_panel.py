"""Panneau « Tables moonclient » de MoonlightAdmin.

Le staff pose en jeu des bons de travail que le client moonclient écrit dans
<installation>/savedata/ ; quatre scripts du dépôt moonclient les portent dans
les tables (et, pour deux d'entre eux, dans la base du serveur). Ce panneau :

  - montre, par script, combien de bons attendent dans son delta ;
  - lance chaque script à blanc (« Voir ») ou pour de vrai (« Appliquer »,
    après confirmation) ;
  - lance scripts/push_mirror.ps1, à blanc ou pour de vrai (après
    confirmation : ce qui part sur le miroir part chez tous les joueurs) ;
  - affiche la sortie du processus en direct, puis son code de sortie.

Un seul processus tourne à la fois. Les chemins se règlent dans le panneau et
sont retenus dans la section [tables] de MoonlightAdmin.ini.
"""
import glob
import os
import queue
import re
import shutil
import subprocess
import sys
import threading
import time
import tkinter as tk
from dataclasses import dataclass
from datetime import datetime
from tkinter import filedialog, messagebox, scrolledtext, ttk

try:
    import yaml
except ImportError:  # le compte de secours suffit à dire « en attente »
    yaml = None

SETTINGS_SECTION = "tables"
DEFAULT_PATHS = {
    "repo": r"G:\GitHub\moonclient",
    "server": r"G:\GitHub\moonlight",
    "installation": r"E:\Moonlight-Destiny",
    "python": "python",
    "pwsh": "",          # vide : cherché sur le PATH puis sous Program Files
    "mirror_ssh": "",    # vide : la variable MOONCLIENT_MIRROR_SSH du script
}
PATH_FIELDS = (
    ("repo", "Dépôt moonclient", "dir"),
    ("server", "Dépôt serveur", "dir"),
    ("installation", "Installation du jeu", "dir"),
    ("python", "Python", "file"),
    ("pwsh", "PowerShell 7 (pwsh.exe)", "file"),
    ("mirror_ssh", "Compte SSH du miroir", None),
)
SAVE_FOLDER = "savedata"
PUSH_MIRROR = os.path.join("scripts", "push_mirror.ps1")


@dataclass(frozen=True)
class DeltaScript:
    label: str
    script: str        # relatif au dépôt moonclient
    delta_file: str    # sous <installation>/savedata/
    delta_key: str     # la clé dont la liste ou la table porte les bons
    needs_server: bool
    legacy_file: str = ""  # lu par le script à défaut de delta_file

    def source_file(self, save_dir):
        """Le delta que le script lira : delta_file s'il existe, sinon l'ancien."""
        current = os.path.join(save_dir, self.delta_file)
        if self.legacy_file and not os.path.exists(current):
            legacy = os.path.join(save_dir, self.legacy_file)
            if os.path.exists(legacy):
                return legacy
        return current


DELTA_SCRIPTS = (
    DeltaScript("Fiches d'objet", "scripts/yaml_tables/apply_item_info_delta.py",
                "item_info_delta.yml", "item_info_delta", False),
    DeltaScript("item_db serveur", "scripts/yaml_tables/apply_item_db_delta.py",
                "item_db_delta.yml", "item_db_delta", True),
    DeltaScript("Albums de cartes", "scripts/yaml_tables/apply_card_album_delta.py",
                "card_album_delta.yml", "card_album_delta", True),
    DeltaScript("Journal des quêtes", "scripts/yaml_tables/apply_quest_delta.py",
                "quest_delta.yml", "quests", False, legacy_file="quest_npc_delta.yml"),
)

STATE_POLL_MS = 3000
DRAIN_MS = 80
CONSOLE_COLORS = {"out": "#ccc", "err": "#e88", "cmd": "#6af", "ok": "#5c5", "fail": "#f55"}
# Sans fenêtre console : MoonlightAdmin.exe est bâti --windowed.
NO_WINDOW = getattr(subprocess, "CREATE_NO_WINDOW", 0)
TOP_KEY_RE = re.compile(r"^([A-Za-z_][\w-]*):\s*(.*?)\s*(#.*)?$")


def count_pending(path, key):
    """Nombre de bons sous `key` dans le delta `path` : (nombre, None) ou
    (None, raison). Un fichier absent, vide ou vidé compte zéro."""
    try:
        with open(path, encoding="utf-8") as f:
            text = f.read()
    except FileNotFoundError:
        return 0, None
    except OSError as exc:
        return None, str(exc)
    if yaml is not None:
        try:
            data = yaml.safe_load(text)
        except yaml.YAMLError as exc:
            return None, f"YAML illisible : {exc.__class__.__name__}"
        if data is None:
            return 0, None
        if not isinstance(data, dict):
            return None, "la racine n'est pas une table"
        entries = data.get(key)
        if entries is None:
            return 0, None
        if isinstance(entries, (list, dict)):
            return len(entries), None
        return None, f"« {key} » n'est ni une liste ni une table"
    return _count_pending_by_lines(text, key)


def _count_pending_by_lines(text, key):
    """Compte de secours, sans PyYAML : les enfants directs de `key` en forme
    de blocs, telle que le client l'écrit."""
    lines = text.splitlines()
    for i, line in enumerate(lines):
        m = TOP_KEY_RE.match(line)
        if not m or m.group(1) != key:
            continue
        inline = m.group(2)
        if inline:
            return (0, None) if inline in ("[]", "{}") else (None, "forme en ligne")
        child_indent = None
        is_list = False
        count = 0
        for child in lines[i + 1:]:
            stripped = child.strip()
            if not stripped or stripped.startswith("#"):
                continue
            indent = len(child) - len(child.lstrip(" "))
            if indent == 0 and not stripped.startswith("- "):
                break
            if child_indent is None:
                if stripped in ("[]", "{}"):
                    return 0, None
                child_indent, is_list = indent, stripped.startswith("- ")
            if indent == child_indent and (stripped.startswith("- ") if is_list else ":" in stripped):
                count += 1
        return count, None
    return 0, None


def find_pwsh():
    """pwsh.exe sur le PATH, sinon la plus récente sous Program Files."""
    found = shutil.which("pwsh")
    if found:
        return found
    base = os.environ.get("ProgramFiles", r"C:\Program Files")
    candidates = sorted(glob.glob(os.path.join(base, "PowerShell", "*", "pwsh.exe")), reverse=True)
    return candidates[0] if candidates else "pwsh"


def ps_quote(text):
    return "'" + text.replace("'", "''") + "'"


def mirror_argv(paths, publish):
    """pwsh qui lance scripts/push_mirror.ps1 du dépôt moonclient : à blanc
    (-DryRun) ou pour de bon. `paths` : les chemins du panneau Tables, dont
    le compte SSH du miroir. Partagé avec la fenêtre Build client."""
    call = "& " + ps_quote(os.path.join(paths["repo"], PUSH_MIRROR))
    if paths["mirror_ssh"]:
        call += " -SshTarget " + ps_quote(paths["mirror_ssh"])
    if not publish:
        call += " -DryRun"
    # Write-Host suit l'encodage de la console : UTF-8, comme le lit le panneau.
    command = ("[Console]::OutputEncoding = [Text.Encoding]::UTF8; "
               f"try {{ {call}; exit $LASTEXITCODE }} "
               "catch { [Console]::Error.WriteLine($_.ToString()); exit 1 }")
    return [paths["pwsh"] or find_pwsh(), "-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", command]


def confirm_mirror_publish(parent):
    return messagebox.askyesno(
        "Publier sur le miroir",
        "Envoyer lang/, dist/data/ et MoonClient.exe sur le miroir de fichiers ?\n\n"
        "Ce qui part sur le miroir part chez TOUS les joueurs à leur prochain lancement, "
        "et rien n'y est effacé ensuite.\n\n"
        "L'essai à blanc dit d'abord ce qui partirait.",
        parent=parent, icon="warning", default="no")


class ProcessRunner:
    """Un processus enfant dont stdout et stderr arrivent ligne à ligne dans
    `events` : ("out", ligne), ("err", ligne), puis ("exit", code)."""

    def __init__(self, events):
        self.events = events
        self.proc = None

    @property
    def busy(self):
        return self.proc is not None and self.proc.poll() is None

    def start(self, argv, cwd):
        env = dict(os.environ, PYTHONIOENCODING="utf-8", PYTHONUTF8="1", PYTHONUNBUFFERED="1")
        self.proc = subprocess.Popen(argv, cwd=cwd, env=env, stdin=subprocess.DEVNULL,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                     creationflags=NO_WINDOW)
        readers = [threading.Thread(target=self._pump, args=(stream, kind), daemon=True)
                   for stream, kind in ((self.proc.stdout, "out"), (self.proc.stderr, "err"))]
        for r in readers:
            r.start()
        proc = self.proc

        def waiter():
            for r in readers:
                r.join()
            self.events.put(("exit", proc.wait()))

        threading.Thread(target=waiter, daemon=True).start()

    def _pump(self, stream, kind):
        for raw in iter(stream.readline, b""):
            self.events.put((kind, decode_line(raw).rstrip("\r\n")))
        stream.close()

    def stop(self):
        """Arrête le processus ET ses descendants : cmake --build laisse sinon
        MSBuild et les compilateurs tourner derrière lui."""
        if not self.busy:
            return
        if sys.platform == "win32":
            subprocess.call(["taskkill", "/T", "/F", "/PID", str(self.proc.pid)],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                            creationflags=NO_WINDOW)
        else:
            self.proc.terminate()


def decode_line(raw):
    """UTF-8 d'abord ; à défaut, la page de code OEM de la console, dans
    laquelle écrivent MSBuild et le compilateur MSVC."""
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError:
        return raw.decode("oem" if sys.platform == "win32" else "latin-1", "replace")


class TablesWindow:
    """Fenêtre « Tables moonclient », créée une fois, masquée à la fermeture."""

    def __init__(self, root, settings, bind_copy_menu):
        self.root = root
        self.events = queue.Queue()
        self.runner = ProcessRunner(self.events)
        self.vars = {k: tk.StringVar(value=settings.get(SETTINGS_SECTION, k, v))
                     for k, v in DEFAULT_PATHS.items()}
        self._geometry = settings.get(SETTINGS_SECTION, "geometry")
        self._bind_copy_menu = bind_copy_menu
        self._started_at = None
        self._pending = {}  # DeltaScript -> nombre en attente (None = illisible)
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        data = {k: v.get().strip() for k, v in self.vars.items()}
        data["geometry"] = self._current_geometry()
        return data

    def _current_geometry(self):
        try:
            if self.win is not None and self.win.winfo_exists():
                return self.win.geometry()
        except tk.TclError:  # fenêtre principale déjà détruite
            pass
        return self._geometry

    def busy(self):
        return self.runner.busy

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        if self.win is None:
            self._build()
        self.win.deiconify()
        self.win.lift()
        self._refresh_state()

    def _hide(self):
        self._geometry = self.win.geometry()
        self.win.withdraw()

    def _build(self):
        self.win = tk.Toplevel(self.root)
        self.win.title("Moonlight - Tables moonclient")
        self.win.minsize(640, 520)
        if self._geometry:
            try:
                self.win.geometry(self._geometry)
            except tk.TclError:
                pass
        self.win.protocol("WM_DELETE_WINDOW", self._hide)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)

        paths = ttk.LabelFrame(body, text="Chemins")
        paths.pack(fill="x", padx=8, pady=(8, 4))
        paths.columnconfigure(1, weight=1)
        for row, (key, label, browse) in enumerate(PATH_FIELDS):
            ttk.Label(paths, text=label).grid(row=row, column=0, padx=4, pady=2, sticky="w")
            ttk.Entry(paths, textvariable=self.vars[key]).grid(row=row, column=1, padx=4, sticky="ew")
            if browse:
                ttk.Button(paths, text="…", width=3,
                           command=lambda k=key, b=browse: self._browse(k, b)).grid(row=row, column=2, padx=4)
        ttk.Label(paths, text="Python et PowerShell vides : cherchés sur le PATH. "
                              "Compte SSH vide : variable MOONCLIENT_MIRROR_SSH.",
                  foreground="#888").grid(row=len(PATH_FIELDS), column=0, columnspan=3,
                                          padx=4, pady=(0, 4), sticky="w")

        deltas = ttk.LabelFrame(body, text="Bons de travail du staff  (<installation>/savedata/)")
        deltas.pack(fill="x", padx=8, pady=4)
        deltas.columnconfigure(2, weight=1)
        self.state_lbls = {}
        self.view_btns = []
        self.apply_btns = {}
        self.file_lbls = {}
        for row, delta in enumerate(DELTA_SCRIPTS):
            ttk.Label(deltas, text=delta.label).grid(row=row, column=0, padx=4, pady=2, sticky="w")
            file_lbl = ttk.Label(deltas, text=delta.delta_file, foreground="#888")
            file_lbl.grid(row=row, column=1, padx=4, sticky="w")
            self.file_lbls[delta] = file_lbl
            lbl = ttk.Label(deltas, text="…")
            lbl.grid(row=row, column=2, padx=8, sticky="w")
            self.state_lbls[delta] = lbl
            view = ttk.Button(deltas, text="Voir", command=lambda d=delta: self._run_delta(d, False))
            view.grid(row=row, column=3, padx=2)
            apply = ttk.Button(deltas, text="Appliquer…", command=lambda d=delta: self._run_delta(d, True))
            apply.grid(row=row, column=4, padx=(2, 4))
            self.view_btns.append(view)
            self.apply_btns[delta] = apply
        ttk.Button(deltas, text="Actualiser", command=self._refresh_state).grid(
            row=len(DELTA_SCRIPTS), column=3, columnspan=2, padx=4, pady=4, sticky="e")

        mirror = ttk.LabelFrame(body, text="Miroir de fichiers  (scripts/push_mirror.ps1)")
        mirror.pack(fill="x", padx=8, pady=4)
        self.mirror_btns = [
            ttk.Button(mirror, text="Voir (à blanc)", command=lambda: self._run_mirror(False)),
            ttk.Button(mirror, text="Publier…", command=lambda: self._run_mirror(True)),
        ]
        for b in self.mirror_btns:
            b.pack(side="left", padx=4, pady=4)
        ttk.Label(mirror, text="lang/, dist/data/ et MoonClient.exe partent chez tous les joueurs.",
                  foreground="#888").pack(side="left", padx=8)

        out = ttk.LabelFrame(body, text="Sortie")
        out.pack(fill="both", expand=True, padx=8, pady=(4, 8))
        bar = ttk.Frame(out)
        bar.pack(fill="x", padx=4, pady=(4, 0))
        self.run_lbl = ttk.Label(bar, text="aucun processus")
        self.run_lbl.pack(side="left")
        self.stop_btn = ttk.Button(bar, text="Arrêter", state="disabled", command=self.runner.stop)
        self.stop_btn.pack(side="right")
        self.console = scrolledtext.ScrolledText(out, height=14, state="disabled", wrap="word",
                                                 background="#111", foreground="#ccc",
                                                 insertbackground="#ccc")
        self.console.pack(fill="both", expand=True, padx=4, pady=4)
        for tag, color in CONSOLE_COLORS.items():
            self.console.tag_config(tag, foreground=color)
        self._bind_copy_menu(self.console)

        self.win.after(DRAIN_MS, self._drain)
        self.win.after(STATE_POLL_MS, self._poll_state)

    def _browse(self, key, kind):
        current = self.vars[key].get().strip()
        if kind == "dir":
            chosen = filedialog.askdirectory(parent=self.win, initialdir=current or None)
        else:
            chosen = filedialog.askopenfilename(parent=self.win, filetypes=[("Exécutable", "*.exe")],
                                                initialdir=os.path.dirname(current) or None)
        if chosen:
            self.vars[key].set(os.path.normpath(chosen))
            self._refresh_state()

    # ---- état « en attente » ------------------------------------------------
    def _poll_state(self):
        try:
            if self.win.state() != "withdrawn":
                self._refresh_state()
        finally:
            self.win.after(STATE_POLL_MS, self._poll_state)

    def _refresh_state(self):
        save_dir = os.path.join(self.vars["installation"].get().strip(), SAVE_FOLDER)
        for delta in DELTA_SCRIPTS:
            source = delta.source_file(save_dir)
            self.file_lbls[delta].config(text=os.path.basename(source))
            count, reason = count_pending(source, delta.delta_key)
            self._pending[delta] = count
            if count is None:
                text, color = f"illisible : {reason}", "#e55"
            elif count == 0:
                text, color = "rien en attente", "#888"
            else:
                text, color = f"{count} en attente", "#db3"
            self.state_lbls[delta].config(text=text, foreground=color)
        self._update_buttons()

    def _update_buttons(self):
        busy = self.runner.busy
        idle = "disabled" if busy else "normal"
        for b in self.view_btns + self.mirror_btns:
            b.config(state=idle)
        for delta, b in self.apply_btns.items():
            b.config(state="disabled" if busy or self._pending.get(delta) == 0 else "normal")
        self.stop_btn.config(state="normal" if busy else "disabled")

    # ---- lancements ---------------------------------------------------------
    def _paths(self):
        return {k: v.get().strip() for k, v in self.vars.items()}

    def _check_dir(self, path, what):
        if path and os.path.isdir(path):
            return True
        self._write(f"{what} introuvable : {path or '(vide)'}", "fail")
        return False

    def _run_delta(self, delta, apply):
        p = self._paths()
        script = os.path.join(p["repo"], delta.script)
        if not (self._check_dir(p["repo"], "Dépôt moonclient")
                and self._check_dir(p["installation"], "Installation")
                and (not delta.needs_server or self._check_dir(p["server"], "Dépôt serveur"))):
            return
        if not os.path.isfile(script):
            self._write(f"Script introuvable : {script}", "fail")
            return
        argv = [p["python"] or "python", script]
        if delta.needs_server:
            argv += ["--serveur", p["server"]]
        argv += ["--installation", p["installation"]]
        if apply:
            warning = ("\n\nLa base visée sert le serveur de production ; ensuite : "
                       "@reloaditemdb, puis relancer le client.") if delta.needs_server else \
                      "\n\nEnsuite : relancer le client."
            if not messagebox.askyesno(
                    "Appliquer les bons",
                    f"Appliquer pour de vrai les bons « {delta.label} » ({delta.delta_file}) ?\n\n"
                    f"Le script écrit les tables, les recopie dans l'installation, "
                    f"puis vide le delta.{warning}", parent=self.win, icon="warning"):
                return
            argv.append("--apply")
        self._start(argv, p["repo"], os.path.basename(script))

    def _run_mirror(self, publish):
        p = self._paths()
        if not self._check_dir(p["repo"], "Dépôt moonclient"):
            return
        script = os.path.join(p["repo"], PUSH_MIRROR)
        if not os.path.isfile(script):
            self._write(f"Script introuvable : {script}", "fail")
            return
        if publish and not confirm_mirror_publish(self.win):
            return
        self._start(mirror_argv(p, publish), p["repo"], os.path.basename(script))

    def _start(self, argv, cwd, label):
        if self.runner.busy:
            self._write("Un processus tourne déjà.", "fail")
            return
        shown = subprocess.list2cmdline(argv)
        self._write(f"[{datetime.now():%H:%M:%S}] > {shown}", "cmd")
        try:
            self.runner.start(argv, cwd)
        except OSError as exc:
            self._write(f"Lancement impossible : {exc}", "fail")
            return
        self._started_at = time.monotonic()
        self.run_lbl.config(text="en cours : " + label)
        self._update_buttons()

    # ---- sortie -------------------------------------------------------------
    def _write(self, text, tag="out"):
        at_bottom = self.console.yview()[1] >= 0.999
        self.console.configure(state="normal")
        self.console.insert("end", text + "\n", tag)
        self.console.configure(state="disabled")
        if at_bottom:
            self.console.see("end")

    def _drain(self):
        try:
            while True:
                try:
                    kind, payload = self.events.get_nowait()
                except queue.Empty:
                    break
                if kind == "exit":
                    seconds = time.monotonic() - (self._started_at or time.monotonic())
                    self._write(f"— fin : code {payload}  ({seconds:.1f} s)",
                                "ok" if payload == 0 else "fail")
                    self.run_lbl.config(text=f"dernier code de sortie : {payload}")
                    self._refresh_state()
                else:
                    self._write(payload, kind)
        finally:
            self.win.after(DRAIN_MS, self._drain)


def _self_test():
    import tempfile
    cases = {
        "list": ("k:\n  - item: 1\n    tint: red\n  - {item: 2}\n", "k", 2),
        "emptied": ("# Appliqué — plus rien en attente.\nk: []\n", "k", 0),
        "next_line_empty": ("# c\nk:\n  []\n", "k", 0),
        "mapping": ("version: 1\nquests:\n  2148:\n    title: x\n    npc:\n      x: 1\n  7:\n    npc: {}\n",
                    "quests", 2),
        "emptied_map": ("version: 1\nquests: {}\n", "quests", 0),
        "no_key": ("version: 1\n", "quests", 0),
        "empty": ("", "k", 0),
    }
    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "d.yml")
        assert count_pending(path, "k") == (0, None), "fichier absent"
        for name, (text, key, expected) in cases.items():
            with open(path, "w", encoding="utf-8") as f:
                f.write(text)
            got = count_pending(path, key)
            assert got == (expected, None), (name, got)
            assert _count_pending_by_lines(text, key) == (expected, None), (name, "secours")
        quests = DeltaScript("q", "s.py", "neuf.yml", "quests", False, legacy_file="d.yml")
        assert quests.source_file(tmp) == path, "à défaut, l'ancien delta"
        open(os.path.join(tmp, "neuf.yml"), "w").close()
        assert quests.source_file(tmp) == os.path.join(tmp, "neuf.yml"), "le nouveau prime"
    print("tables_panel : essais réussis")


if __name__ == "__main__":
    if "--self-test" in sys.argv:
        _self_test()
