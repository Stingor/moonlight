"""Fenêtre « Atelier NPC » de MoonlightAdmin.

Ce qui a changé dans le dépôt de pc-serv, fichier par fichier : état git,
encodage et, pour un script de moon/, la déclaration qui le charge au
démarrage. tools/npc_check.py, lancé sur le serveur, fournit ces trois
relevés. Depuis la liste :

  - voir le diff d'un fichier (double-clic) ;
  - convertir en Windows-1252 ce qui se convertit sans perte ;
  - recharger en jeu par le canal admin : reloadnpcfile pour un script de
    moon/, unloadnpcfile pour un script supprimé. Le serveur ne recharge
    ainsi qu'un fichier de moon/ : un script de npc/ attend reloadscript ;
  - commiter les fichiers cochés, après le même contrôle d'encodage, puis au
    besoin pull --rebase et push. La chaîne tourne dans la fenêtre « Serveur
    (SSH) », qui en montre la sortie.

Voir, convertir et recharger agissent sur la sélection, à défaut sur les
fichiers cochés ; le commit, toujours sur les cochés.
"""
import json
import queue
import shlex
import subprocess
import threading
import tkinter as tk
from tkinter import messagebox, scrolledtext, ttk

from ssh_panel import GIT_PUSH, NPC_CHECK, PULL_REBASE, RemoteCommand, npc_check_command
from tables_panel import DRAIN_MS, NO_WINDOW

SETTINGS_SECTION = "atelier_npc"
SCRIPT_SUFFIXES = (".npc", ".txt")
MOON_PREFIX = "moon/"
NPC_PREFIX = "npc/"
STATUS_MISSING_CODE = 3
DIFF_CONTEXT_LINES = 3
CAPTURE_TIMEOUT_S = 60
CHECKED, UNCHECKED = "☑", "☐"
ENCODING_LABELS = {
    "fffd": "U+FFFD", "bom": "BOM", "utf8": "UTF-8", "symbols": "symboles UTF-8",
    "mixed": "accent UTF-8 collé", "undefined": "octet inconnu",
}
STARTUP_LABELS = {"déclaré": "déclaré", "commenté": "commenté", "absent": "non déclaré"}
ROW_COLORS = {"erreur": "#e55", "avertissement": "#c90"}
OUTPUT_COLORS = {"add": "#5c5", "del": "#e66", "hunk": "#6af", "head": "#c8f",
                 "ok": "#5c5", "fail": "#f55", "info": "#999", "out": "#ccc"}
# Ce que l'atcommande écrit quand le chargement échoue ; la cause (ligne du
# script en défaut) part dans la console du map-server.
RELOAD_FAILURE_MARKERS = ("could not be loaded", "failed", "File not found")
# Réponse de MoonlightAdmin quand le canal admin n'est pas ouvert.
NOT_CONNECTED_REPLY = "ERR pas connecté"


def decode(raw):
    """Une ligne de sortie : UTF-8 (git, python), à défaut Windows-1252, celui
    des scripts que montre un diff."""
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError:
        return raw.decode("cp1252", "replace")


def is_script(path):
    return path.endswith(SCRIPT_SUFFIXES)


def quote_all(paths):
    return " ".join(shlex.quote(p) for p in paths)


class NpcWindow:
    """Fenêtre « Atelier NPC », créée une fois, masquée à la fermeture."""

    def __init__(self, root, settings, bind_copy_menu, ssh_window, send_admin):
        self.root = root
        self.ssh = ssh_window
        self.send_admin = send_admin
        self._bind_copy_menu = bind_copy_menu
        self._geometry = settings.get(SETTINGS_SECTION, "geometry")
        self._files = {}       # chemin -> relevé de npc_check.py --status
        self._included = {}    # chemin -> coché pour le commit
        self._results = queue.Queue()
        self._capturing = False
        self._reload_queue = []
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        return {"geometry": self._current_geometry()}

    def _current_geometry(self):
        try:
            if self.win is not None and self.win.winfo_exists():
                return self.win.geometry()
        except tk.TclError:  # fenêtre principale déjà détruite
            pass
        return self._geometry

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        if self.win is None:
            self._build()
        self.win.deiconify()
        self.win.lift()
        self.refresh()

    def _hide(self):
        self._geometry = self.win.geometry()
        self.win.withdraw()

    def _build(self):
        self.win = tk.Toplevel(self.root)
        self.win.title("Moonlight - Atelier NPC")
        self.win.minsize(820, 600)
        if self._geometry:
            try:
                self.win.geometry(self._geometry)
            except tk.TclError:
                pass
        self.win.protocol("WM_DELETE_WINDOW", self._hide)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)
        self.action_widgets = []

        head = ttk.Frame(body)
        head.pack(fill="x", padx=8, pady=(8, 2))
        self.summary_lbl = ttk.Label(head, text="…")
        self.summary_lbl.pack(side="left")
        self._action(ttk.Button(head, text="Actualiser", command=self.refresh)).pack(side="right")

        listing = ttk.Frame(body)
        listing.pack(fill="both", expand=True, padx=8, pady=2)
        columns = ("inclus", "etat", "chemin", "encodage", "demarrage")
        self.tree = ttk.Treeview(listing, columns=columns, show="headings", height=12,
                                 selectmode="extended")
        for column, text, width, stretch in (
                ("inclus", "Commit", 60, False), ("etat", "État", 80, False),
                ("chemin", "Fichier", 360, True), ("encodage", "Encodage", 150, False),
                ("demarrage", "Au démarrage", 110, False)):
            self.tree.heading(column, text=text)
            self.tree.column(column, width=width, stretch=stretch, anchor="w")
        scroll = ttk.Scrollbar(listing, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scroll.set)
        self.tree.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
        for verdict, color in ROW_COLORS.items():
            self.tree.tag_configure(verdict, foreground=color)
        self.tree.bind("<Button-1>", self._on_click)
        self.tree.bind("<Double-1>", lambda _e: self._show_diff())
        self.tree.bind("<<TreeviewSelect>>", lambda _e: self._show_details())

        self.details_lbl = ttk.Label(body, text="", foreground="#888", justify="left", wraplength=780)
        self.details_lbl.pack(fill="x", padx=10, pady=(2, 0))

        actions = ttk.Frame(body)
        actions.pack(fill="x", padx=8, pady=4)
        for text, command in (("Tout cocher", lambda: self._check_all(True)),
                              ("Rien cocher", lambda: self._check_all(False))):
            ttk.Button(actions, text=text, command=command).pack(side="left", padx=2)
        ttk.Separator(actions, orient="vertical").pack(side="left", fill="y", padx=8)
        for text, command in (("Voir le diff", self._show_diff),
                              ("Convertir en ANSI", self._convert),
                              ("Recharger en jeu", self._reload)):
            self._action(ttk.Button(actions, text=text, command=command)).pack(side="left", padx=2)
        ttk.Label(actions, text="sélection, à défaut les cochés", foreground="#888").pack(
            side="left", padx=8)

        commit = ttk.LabelFrame(body, text="Commit des fichiers cochés  (contrôle d'encodage d'abord)")
        commit.pack(fill="x", padx=8, pady=4)
        ttk.Label(commit, text="Message").pack(side="left", padx=(4, 2), pady=4)
        self.message_var = tk.StringVar()
        entry = ttk.Entry(commit, textvariable=self.message_var)
        entry.pack(side="left", fill="x", expand=True, padx=2)
        entry.bind("<Return>", lambda _e: self._commit(sync=False))
        self._action(ttk.Button(commit, text="Commiter",
                                command=lambda: self._commit(sync=False))).pack(side="left", padx=2)
        self._action(ttk.Button(commit, text="Commiter + pull --rebase + push",
                                command=lambda: self._commit(sync=True))).pack(side="left", padx=(2, 4))

        out = ttk.LabelFrame(body, text="Sortie")
        out.pack(fill="both", expand=True, padx=8, pady=(4, 8))
        self.output = scrolledtext.ScrolledText(out, height=12, state="disabled", wrap="none",
                                                background="#111", foreground="#ccc",
                                                insertbackground="#ccc")
        self.output.pack(fill="both", expand=True, padx=4, pady=4)
        for tag, color in OUTPUT_COLORS.items():
            self.output.tag_config(tag, foreground=color)
        self._bind_copy_menu(self.output)

        self.win.after(DRAIN_MS, self._drain)

    def _action(self, widget):
        """Un bouton qui interroge le serveur : grisé tant qu'une requête tourne."""
        self.action_widgets.append(widget)
        return widget

    def _update_buttons(self):
        state = "disabled" if self._capturing else "normal"
        for w in self.action_widgets:
            w.config(state=state)

    # ---- requêtes ssh ---------------------------------------------------------
    def _capture(self, shell, on_done):
        """`shell` dans le dossier du dépôt ; `on_done(code, sortie, erreurs)`
        rappelé dans le fil de l'interface, sorties en octets."""
        if self._capturing:
            self._write("Une requête tourne déjà.", "fail")
            return
        self._capturing = True
        self._update_buttons()
        argv = self.ssh.ssh_argv(shell)

        def worker():
            try:
                done = subprocess.run(argv, capture_output=True, stdin=subprocess.DEVNULL,
                                      timeout=CAPTURE_TIMEOUT_S, creationflags=NO_WINDOW)
                result = (done.returncode, done.stdout, done.stderr)
            except (OSError, subprocess.TimeoutExpired) as exc:
                result = (-1, b"", str(exc).encode())
            self._results.put((on_done, result))

        threading.Thread(target=worker, daemon=True).start()

    def _drain(self):
        try:
            while True:
                on_done, result = self._results.get_nowait()
                self._capturing = False
                self._update_buttons()
                on_done(*result)
        except queue.Empty:
            pass
        self.win.after(DRAIN_MS, self._drain)

    # ---- liste ----------------------------------------------------------------
    def refresh(self):
        if self._capturing:
            return
        self.summary_lbl.config(text="relevé en cours…")
        # Le fetch donne l'avance et le retard du moment ; hors réseau, il
        # échoue sans rien empêcher.
        self._capture(f"git fetch --quiet 2>/dev/null; if [ -f {NPC_CHECK} ]; then "
                      f"python3 {NPC_CHECK} --status --json; else exit {STATUS_MISSING_CODE}; fi",
                      self._on_status)

    def _on_status(self, code, stdout, stderr):
        if code == STATUS_MISSING_CODE:
            self.summary_lbl.config(text=f"{NPC_CHECK} absent sur le serveur : git pull d'abord, "
                                         f"depuis la fenêtre Serveur (SSH).")
            return
        try:
            data = json.loads(stdout)
        except ValueError:
            self.summary_lbl.config(text="relevé illisible, voir la sortie")
            self._write(decode(stderr or stdout).strip(), "fail")
            return
        previous = self._included
        self._files = {f["path"]: f for f in data["files"]}
        self._included = {p: previous.get(p, f["tracked"]) for p, f in self._files.items()}
        self._render()
        self._summarize(data["branch"])

    def _render(self):
        selected = set(self.tree.selection())
        self.tree.delete(*self.tree.get_children())
        for path, f in self._files.items():
            label = f["state"] + (f" ← {f['origin']}" if f["origin"] else "")
            values = (CHECKED if self._included[path] else UNCHECKED, label, path,
                      self._encoding_label(f), STARTUP_LABELS.get(f["startup"], ""))
            tags = (f["verdict"],) if f["verdict"] in ROW_COLORS else ()
            self.tree.insert("", "end", iid=path, values=values, tags=tags)
        keep = [p for p in selected if p in self._files]
        if keep:
            self.tree.selection_set(keep)
        self._show_details()

    @staticmethod
    def _encoding_label(f):
        if not f["checked"]:
            return "—"
        encoding = [p for p in f["problems"] if p["code"] in ENCODING_LABELS]
        if not encoding:
            return "OK"
        worst = next((p for p in encoding if p["level"] == "erreur"), encoding[0])
        mark = "✖" if worst["level"] == "erreur" else "⚠"
        fix = " (corrigible)" if any(p["fixable"] for p in encoding) else ""
        return f"{mark} {ENCODING_LABELS[worst['code']]}{fix}"

    def _summarize(self, branch):
        errors = sum(f["verdict"] == "erreur" for f in self._files.values())
        parts = [f"{len(self._files)} fichier(s) modifié(s)" if self._files else "rien de modifié"]
        if errors:
            parts.append(f"{errors} en erreur d'encodage")
        state = f"branche {branch['name']}"
        if branch["upstream"]:
            moves = [f"{branch['ahead']} à pousser" if branch["ahead"] else "",
                     f"{branch['behind']} à recevoir" if branch["behind"] else ""]
            moves = [m for m in moves if m]
            state += " : " + (", ".join(moves) if moves else f"à jour avec {branch['upstream']}")
        parts.append(state)
        self.summary_lbl.config(text="  ·  ".join(parts))

    def _show_details(self):
        lines = []
        for path in self.tree.selection():
            f = self._files.get(path)
            if f is None:
                continue
            for problem in f["problems"]:
                lines.append(f"{path} : {problem['message']}")
            if f["startup_conf"] and not any(p["code"] == "startup" for p in f["problems"]):
                lines.append(f"{path} : {f['startup']} dans {f['startup_conf']}")
        self.details_lbl.config(text="\n".join(lines))

    def _on_click(self, event):
        """Un clic dans la colonne Commit coche ou décoche la ligne."""
        if self.tree.identify_region(event.x, event.y) != "cell":
            return None
        if self.tree.identify_column(event.x) != "#1":
            return None
        path = self.tree.identify_row(event.y)
        if path:
            self._included[path] = not self._included[path]
            self.tree.set(path, "inclus", CHECKED if self._included[path] else UNCHECKED)
        return "break"

    def _check_all(self, value):
        for path in self._included:
            self._included[path] = value
            self.tree.set(path, "inclus", CHECKED if value else UNCHECKED)

    def _targets(self):
        selected = list(self.tree.selection())
        return selected or [p for p, on in self._included.items() if on]

    # ---- actions ------------------------------------------------------------
    def _show_diff(self):
        targets = self._targets()
        if not targets:
            return
        f = self._files[targets[0]]
        path = shlex.quote(f["path"])
        if not f["tracked"]:
            # Fichier neuf : comparé à rien, git le montre en entier.
            shell = f"git diff --no-color --no-index -- /dev/null {path}"
        elif f["origin"]:
            shell = f"git diff --no-color -M HEAD -- {shlex.quote(f['origin'])} {path}"
        else:
            shell = f"git diff --no-color -U{DIFF_CONTEXT_LINES} HEAD -- {path}"
        self._clear()
        self._capture(shell, self._on_diff)

    def _on_diff(self, _code, stdout, stderr):
        if not stdout:
            self._write(decode(stderr).strip() or "aucune différence", "info")
            return
        for raw in stdout.split(b"\n"):
            line = decode(raw)
            if line.startswith(("diff --git", "index ", "--- ", "+++ ", "new file", "deleted file",
                                "similarity", "rename ")):
                tag = "head"
            elif line.startswith("@@"):
                tag = "hunk"
            elif line.startswith("+"):
                tag = "add"
            elif line.startswith("-"):
                tag = "del"
            else:
                tag = "out"
            self._write(line, tag, scroll=False)
        self.output.see("1.0")

    def _convert(self):
        fixable = [p for p in self._targets()
                   if any(pr["fixable"] for pr in self._files[p]["problems"])]
        if not fixable:
            self._write("Rien à convertir sans perte dans ces fichiers.", "info")
            return
        if not messagebox.askyesno(
                "Convertir en ANSI",
                "Réécrire en Windows-1252 (BOM retiré, accents UTF-8 convertis) :\n\n"
                + "\n".join(fixable) + "\n\nLa conversion ne perd aucun caractère.",
                parent=self.win, icon="question"):
            return
        self._clear()
        self._capture(f"python3 {NPC_CHECK} --fix -- {quote_all(fixable)}", self._on_converted)

    def _on_converted(self, code, stdout, stderr):
        for raw in (stdout + stderr).split(b"\n"):
            line = decode(raw)
            if line:
                self._write(line, "fail" if line.startswith("ERREUR") else "out")
        self._write("Conversion faite." if code == 0 else "Il reste des erreurs.", "ok" if code == 0 else "fail")
        self.refresh()

    def _reload(self):
        commands, notes = [], []
        for path in self._targets():
            f = self._files[path]
            if not is_script(path):
                continue
            if not path.startswith(MOON_PREFIX):
                if path.startswith(NPC_PREFIX):
                    notes.append(f"{path} : le serveur ne recharge un par un que les scripts de moon/ ; "
                                 f"RELOAD script (reloadscript) recharge tout.")
                continue
            if f["state"] == "supprimé":
                commands.append((path, f"unloadnpcfile {path}", "déchargé"))
                continue
            commands.append((path, f"reloadnpcfile {path}", "rechargé"))
            if f["verdict"] == "erreur":
                notes.append(f"{path} : erreur d'encodage, ses accents s'afficheront mal en jeu.")
            if f["startup"] in ("absent", "commenté"):
                notes.append(f"{path} : rechargé, mais il ne se chargera pas au prochain démarrage "
                             f"({STARTUP_LABELS[f['startup']]}).")
        self._clear()
        for note in notes:
            self._write(note, "info")
        if not commands:
            self._write("Aucun script de moon/ à recharger.", "info")
            return
        self._reload_queue = commands
        self._next_reload()

    def _next_reload(self):
        if not self._reload_queue:
            return
        path, atcmd, done_label = self._reload_queue.pop(0)
        self._write(f"> {atcmd}", "hunk")
        self.send_admin(atcmd, lambda output, reply: self._on_reloaded(path, done_label, output, reply))

    def _on_reloaded(self, path, done_label, output, reply):
        if reply.startswith(NOT_CONNECTED_REPLY):
            self._reload_queue = []
            self._write("  Pas connecté : connecter d'abord la fenêtre principale au canal admin.", "fail")
            return
        failed = not reply.startswith("OK") or any(
            m in line for line in output for m in RELOAD_FAILURE_MARKERS)
        for line in output:
            self._write("  " + line, "fail" if failed else "out")
        if failed:
            self._write(f"  {path} : échec, la ligne en défaut est dans la console du map-server.", "fail")
        else:
            self._write(f"  {path} : {done_label}", "ok")
        self._next_reload()

    def _commit(self, sync):
        message = self.message_var.get().strip()
        paths = [p for p, on in self._included.items() if on]
        if not message:
            self._write("Message de commit vide : rien n'est lancé.", "fail")
            return
        if not paths:
            self._write("Aucun fichier coché.", "fail")
            return
        blocked = [p for p in paths if self._files[p]["verdict"] == "erreur"]
        if blocked:
            messagebox.showwarning(
                "Erreurs d'encodage",
                "Le contrôle refusera ce commit :\n\n" + "\n".join(blocked)
                + "\n\n« Convertir en ANSI » corrige ce qui se corrige sans perte ; "
                  "le reste se reprend à la main, ou se décoche.", parent=self.win)
            return
        # `git commit -- <chemins>` engage modifications, suppressions et
        # renommages, l'ancien nom compris ; seul un fichier neuf doit d'abord
        # être ajouté, git ne connaissant pas encore son chemin.
        pathspec = paths + [self._files[p]["origin"] for p in paths if self._files[p]["origin"]]
        untracked = [p for p in paths if not self._files[p]["tracked"]]
        # Sans chemin, le contrôle porterait sur tout ce que commit -a
        # emporterait : un commit de suppressions seules s'en passe.
        to_check = [p for p in paths if self._files[p]["state"] != "supprimé" and is_script(p)]
        steps = [npc_check_command(to_check)] if to_check else []
        if untracked:
            steps.append(RemoteCommand("git add (neufs)", "git add -- " + quote_all(untracked)))
        steps.append(RemoteCommand(f"commit « {message} »",
                                   f"git commit -m {shlex.quote(message)} -- {quote_all(pathspec)}"))
        if sync:
            steps += [PULL_REBASE, GIT_PUSH]
        if self.ssh.run_chain(steps, on_done=self._on_committed):
            self._write(f"Commit lancé dans la fenêtre Serveur (SSH) : {len(paths)} fichier(s).", "info")

    def _on_committed(self, succeeded):
        if succeeded:
            self.message_var.set("")
            self._write("Commit fait.", "ok")
        else:
            self._write("Le commit n'a pas abouti : voir la fenêtre Serveur (SSH).", "fail")
        if self.win is not None and self.win.winfo_exists():
            self.refresh()

    # ---- sortie -------------------------------------------------------------
    def _clear(self):
        self.output.configure(state="normal")
        self.output.delete("1.0", "end")
        self.output.configure(state="disabled")

    def _write(self, text, tag="out", scroll=True):
        self.output.configure(state="normal")
        self.output.insert("end", text + "\n", tag)
        self.output.configure(state="disabled")
        if scroll:
            self.output.see("end")
