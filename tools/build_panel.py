"""Panneau « Build client » de MoonlightAdmin.

Bâtit le client moonclient depuis son dépôt, étape par étape, en enchaînant
comme `&&` : la première étape qui échoue arrête la chaîne. Les étapes se
cochent :

  - contrôles du dépôt : format (scripts/check_format.py) et hygiène
    (scripts/check_hygiene.py), ceux que rejoue le crochet pre-push ;
  - configurer : `cmake --preset <configuration>`, au besoin `--fresh`
    (cache CMake effacé). Inutile à chaque rebuild : la bâtisse reconfigure
    seule quand un CMakeLists.txt change, et relève le tampon de révision.
    Elle s'ajoute d'office à un dossier qui n'a pas encore de cache ;
  - bâtir : `cmake --build --preset <build>`, au besoin `--clean-first` ;
  - tester : `ctest --preset <essais>` ;
  - archiver les symboles : scripts/archive_symbols.py ;
  - déposer dans l'installation : scripts/deploy_client.py.

Les presets sont lus dans CMakePresets.json et CMakeUserPresets.json du dépôt.
Un preset de build ou d'essais vaut pour un preset de configuration s'il nomme
ce preset ou l'un de ses ancêtres qui bâtit dans le même dossier : c'est ainsi
qu'un preset utilisateur qui hérite de `windows` se bâtit par `windows-release`.

Les chemins (dépôt, installation, Python) sont ceux du panneau Tables : une
seule saisie pour les deux fenêtres. Le reste se retient dans la section
[build] de MoonlightAdmin.ini.
"""
import json
import os
import queue
import subprocess
import sys
import time
import tkinter as tk
from dataclasses import dataclass
from datetime import datetime
from tkinter import filedialog, messagebox, scrolledtext, ttk

from tables_panel import CONSOLE_COLORS, DRAIN_MS, ProcessRunner

SETTINGS_SECTION = "build"
PRESET_FILES = ("CMakePresets.json", "CMakeUserPresets.json")
SOURCE_DIR_MACRO = "${sourceDir}"
PRESET_NAME_MACRO = "${presetName}"
# Sous Windows, seuls les presets du générateur Visual Studio se bâtissent ici.
WINDOWS_GENERATOR_PREFIX = "Visual Studio"
CMAKE_CACHE = "CMakeCache.txt"
DEFAULT_CONFIGURATION = "Release"
SYMBOLS_CONFIGURATION = "Release"
# Une ligne de la sortie MSBuild qui contient l'un de ces motifs passe en rouge.
ERROR_MARKERS = (": error ", ": fatal error ", "CMake Error", "FAILED:")
WARNING_MARKERS = (": warning ", "CMake Warning")

BOOL_OPTIONS = {
    # clé .ini : (défaut, libellé)
    "check_format": (False, "Contrôle du format"),
    "check_hygiene": (False, "Contrôle d'hygiène"),
    "configure": (False, "Configurer (d'office si le dossier n'a pas de cache)"),
    "fresh": (False, "à neuf (--fresh : cache CMake effacé)"),
    "build": (True, "Bâtir"),
    "clean_first": (False, "Clean first (--clean-first : tout recompiler)"),
    "verbose": (False, "Détaillé (--verbose)"),
    "test": (False, "Lancer les essais"),
    "archive": (True, "Archiver les symboles"),
    "deploy": (True, "Déposer dans l'installation"),
    "draft": (False, "brouillon (--brouillon : sans exiger les symboles)"),
}
CHOICE_OPTIONS = ("configure_preset", "build_preset", "test_preset")


@dataclass(frozen=True)
class Step:
    label: str
    argv: tuple
    cwd: str


@dataclass
class Presets:
    configure: dict   # nom -> preset de configuration (dict brut)
    build: dict       # nom -> preset de build
    test: dict        # nom -> preset d'essais
    errors: list      # fichiers illisibles


def load_presets(repo):
    """Les presets des deux fichiers du dépôt, fusionnés par nom."""
    presets = Presets({}, {}, {}, [])
    for name in PRESET_FILES:
        path = os.path.join(repo, name)
        if not os.path.isfile(path):
            continue
        try:
            with open(path, encoding="utf-8") as f:
                data = json.load(f)
        except (OSError, ValueError) as exc:
            presets.errors.append(f"{name} : {exc}")
            continue
        for key, target in (("configurePresets", presets.configure),
                            ("buildPresets", presets.build),
                            ("testPresets", presets.test)):
            for preset in data.get(key, []):
                if "name" in preset:
                    target[preset["name"]] = preset
    return presets


def _parents(preset):
    inherits = preset.get("inherits", [])
    return [inherits] if isinstance(inherits, str) else list(inherits)


def resolve_field(presets, name, field, seen=None):
    """Valeur de `field` pour le preset de configuration `name`, héritage
    compris (le premier parent qui la porte l'emporte, comme chez CMake)."""
    seen = seen if seen is not None else set()
    preset = presets.configure.get(name)
    if preset is None or name in seen:
        return None
    seen.add(name)
    if field in preset:
        return preset[field]
    for parent in _parents(preset):
        value = resolve_field(presets, parent, field, seen)
        if value is not None:
            return value
    return None


def binary_dir(presets, repo, name):
    raw = resolve_field(presets, name, "binaryDir")
    if raw is None:
        return None
    path = raw.replace(SOURCE_DIR_MACRO, repo).replace(PRESET_NAME_MACRO, name)
    return os.path.normpath(path)


def ancestors(presets, name):
    """`name` puis ses ancêtres, en largeur, sans doublon."""
    order, todo = [], [name]
    while todo:
        current = todo.pop(0)
        if current in order or current not in presets.configure:
            continue
        order.append(current)
        todo.extend(_parents(presets.configure[current]))
    return order


def usable_configure_presets(presets):
    names = [n for n, p in presets.configure.items() if not p.get("hidden")]
    if sys.platform == "win32":
        names = [n for n in names
                 if str(resolve_field(presets, n, "generator") or "").startswith(WINDOWS_GENERATOR_PREFIX)]
    return names


def presets_for(presets, kind, configure_name, repo):
    """Les presets de build ou d'essais (`kind`) qui bâtissent dans le dossier
    du preset de configuration `configure_name`."""
    table = presets.build if kind == "build" else presets.test
    own_dir = binary_dir(presets, repo, configure_name)
    matching = {a for a in ancestors(presets, configure_name)
                if binary_dir(presets, repo, a) == own_dir}
    return [n for n, p in table.items()
            if not p.get("hidden") and p.get("configurePreset") in matching]


def preset_configuration(presets, kind, name):
    table = presets.build if kind == "build" else presets.test
    return (table.get(name) or {}).get("configuration") or DEFAULT_CONFIGURATION


class BuildWindow:
    """Fenêtre « Build client », créée une fois, masquée à la fermeture."""

    def __init__(self, root, settings, bind_copy_menu, path_vars):
        self.root = root
        self.events = queue.Queue()
        self.runner = ProcessRunner(self.events)
        self.paths = path_vars  # StringVar partagées avec le panneau Tables
        self.flags = {k: tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, k, default))
                      for k, (default, _label) in BOOL_OPTIONS.items()}
        self.choices = {k: tk.StringVar(value=settings.get(SETTINGS_SECTION, k, ""))
                        for k in CHOICE_OPTIONS}
        self._geometry = settings.get(SETTINGS_SECTION, "geometry")
        self._bind_copy_menu = bind_copy_menu
        self._presets = Presets({}, {}, {}, [])
        self._queue = []          # étapes restantes de la chaîne en cours
        self._results = []        # (libellé, code, secondes) des étapes passées
        self._step_started = None
        self._chain_started = None
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        data = {k: v.get() for k, v in self.flags.items()}
        data.update({k: v.get() for k, v in self.choices.items()})
        data["geometry"] = self._current_geometry()
        return data

    def _current_geometry(self):
        try:
            if self.win is not None and self.win.winfo_exists():
                return self.win.geometry()
        except tk.TclError:
            pass
        return self._geometry

    def busy(self):
        return self.runner.busy or bool(self._queue)

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        if self.win is None:
            self._build()
        self.win.deiconify()
        self.win.lift()
        self._reload_presets()

    def _hide(self):
        self._geometry = self.win.geometry()
        self.win.withdraw()

    def _build(self):
        self.win = tk.Toplevel(self.root)
        self.win.title("Moonlight - Build client")
        self.win.minsize(720, 600)
        if self._geometry:
            try:
                self.win.geometry(self._geometry)
            except tk.TclError:
                pass
        self.win.protocol("WM_DELETE_WINDOW", self._hide)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)

        paths = ttk.LabelFrame(body, text="Chemins  (partagés avec « Tables moonclient »)")
        paths.pack(fill="x", padx=8, pady=(8, 4))
        paths.columnconfigure(1, weight=1)
        for row, (key, label) in enumerate((("repo", "Dépôt moonclient"),
                                            ("installation", "Installation (--dest)"),
                                            ("python", "Python"))):
            ttk.Label(paths, text=label).grid(row=row, column=0, padx=4, pady=2, sticky="w")
            ttk.Entry(paths, textvariable=self.paths[key]).grid(row=row, column=1, padx=4, sticky="ew")
            ttk.Button(paths, text="…", width=3,
                       command=lambda k=key: self._browse(k)).grid(row=row, column=2, padx=4)

        presets = ttk.LabelFrame(body, text="Presets")
        presets.pack(fill="x", padx=8, pady=4)
        self.combos = {}
        for col, (key, label) in enumerate((("configure_preset", "Configuration"),
                                            ("build_preset", "Build"),
                                            ("test_preset", "Essais"))):
            ttk.Label(presets, text=label).grid(row=0, column=2 * col, padx=4, pady=4, sticky="w")
            combo = ttk.Combobox(presets, textvariable=self.choices[key], state="readonly", width=24)
            combo.grid(row=0, column=2 * col + 1, padx=4, pady=4, sticky="w")
            self.combos[key] = combo
        self.combos["configure_preset"].bind("<<ComboboxSelected>>", lambda e: self._refresh_dependent())
        self.combos["build_preset"].bind("<<ComboboxSelected>>", lambda e: self._refresh_info())
        ttk.Button(presets, text="Relire", command=self._reload_presets).grid(row=0, column=6, padx=4)
        self.info_lbl = ttk.Label(presets, text="", foreground="#888")
        self.info_lbl.grid(row=1, column=0, columnspan=7, padx=4, pady=(0, 4), sticky="w")

        steps = ttk.LabelFrame(body, text="Étapes  (enchaînées : la première qui échoue arrête tout)")
        steps.pack(fill="x", padx=8, pady=4)
        layout = (("check_format", "check_hygiene"),
                  ("configure", "fresh"),
                  ("build", "clean_first", "verbose"),
                  ("test",),
                  ("archive",),
                  ("deploy", "draft"))
        for row, keys in enumerate(layout):
            for col, key in enumerate(keys):
                ttk.Checkbutton(steps, text=BOOL_OPTIONS[key][1], variable=self.flags[key],
                                command=self._refresh_info).grid(
                    row=row, column=col, padx=(4 if col == 0 else 16, 4), pady=1, sticky="w")

        actions = ttk.Frame(body)
        actions.pack(fill="x", padx=8, pady=4)
        self.run_btn = ttk.Button(actions, text="▶ Lancer", command=self._run_chain)
        self.run_btn.pack(side="left", padx=(0, 4))
        self.stop_btn = ttk.Button(actions, text="■ Arrêter", state="disabled", command=self._stop)
        self.stop_btn.pack(side="left", padx=4)
        self.tool_btns = [
            ttk.Button(actions, text="Symboles archivés", command=self._list_symbols),
            ttk.Button(actions, text="Vérifier les tables générées", command=self._verify_tables),
        ]
        for b in self.tool_btns:
            b.pack(side="left", padx=4)
        ttk.Button(actions, text="Dossier de build", command=self._open_build_dir).pack(side="right", padx=4)
        ttk.Button(actions, text="Effacer la sortie", command=self._clear).pack(side="right", padx=4)

        out = ttk.LabelFrame(body, text="Sortie")
        out.pack(fill="both", expand=True, padx=8, pady=(4, 8))
        self.run_lbl = ttk.Label(out, text="aucun processus")
        self.run_lbl.pack(anchor="w", padx=4, pady=(4, 0))
        self.console = scrolledtext.ScrolledText(out, height=18, state="disabled", wrap="none",
                                                 background="#111", foreground="#ccc",
                                                 insertbackground="#ccc", font=("Consolas", 9))
        self.console.pack(fill="both", expand=True, padx=4, pady=4)
        for tag, color in CONSOLE_COLORS.items():
            self.console.tag_config(tag, foreground=color)
        self.console.tag_config("warn", foreground="#db3")
        self.console.tag_config("step", foreground="#fff", font=("Consolas", 9, "bold"))
        self._bind_copy_menu(self.console)

        self.win.after(DRAIN_MS, self._drain)

    def _browse(self, key):
        current = self.paths[key].get().strip()
        if key == "python":
            chosen = filedialog.askopenfilename(parent=self.win, filetypes=[("Exécutable", "*.exe")],
                                                initialdir=os.path.dirname(current) or None)
        else:
            chosen = filedialog.askdirectory(parent=self.win, initialdir=current or None)
        if chosen:
            self.paths[key].set(os.path.normpath(chosen))
            if key == "repo":
                self._reload_presets()

    # ---- presets ------------------------------------------------------------
    def _repo(self):
        return self.paths["repo"].get().strip()

    def _reload_presets(self):
        self._presets = load_presets(self._repo())
        names = usable_configure_presets(self._presets)
        self.combos["configure_preset"]["values"] = names
        if self.choices["configure_preset"].get() not in names:
            self.choices["configure_preset"].set(names[0] if names else "")
        for error in self._presets.errors:
            self._write("Presets illisibles — " + error, "fail")
        self._refresh_dependent()

    def _refresh_dependent(self):
        configure = self.choices["configure_preset"].get()
        for kind, key in (("build", "build_preset"), ("test", "test_preset")):
            names = presets_for(self._presets, kind, configure, self._repo()) if configure else []
            self.combos[key]["values"] = names
            if self.choices[key].get() not in names:
                fallback = [n for n in names if preset_configuration(self._presets, kind, n)
                            == DEFAULT_CONFIGURATION]
                self.choices[key].set((fallback or names or [""])[0])
        self._refresh_info()

    def _build_dir(self):
        configure = self.choices["configure_preset"].get()
        return binary_dir(self._presets, self._repo(), configure) if configure else None

    def _configuration(self):
        return preset_configuration(self._presets, "build", self.choices["build_preset"].get())

    def _refresh_info(self):
        if self.win is None:
            return
        parts = [f"dossier : {self._build_dir() or '?'}", f"configuration : {self._configuration()}"]
        if (self.flags["archive"].get() or self.flags["deploy"].get()) \
                and self._configuration() != SYMBOLS_CONFIGURATION:
            parts.append(f"⚠ archiver et déposer visent une construction {SYMBOLS_CONFIGURATION}")
        self.info_lbl.config(text="   ·   ".join(parts))

    # ---- chaîne d'étapes ----------------------------------------------------
    def _steps(self):
        repo = self._repo()
        python = self.paths["python"].get().strip() or "python"
        f = {k: v.get() for k, v in self.flags.items()}
        c = {k: v.get() for k, v in self.choices.items()}
        build_dir = self._build_dir()
        configuration = self._configuration()
        script = lambda name: os.path.join(repo, "scripts", name)  # noqa: E731
        steps = []
        if f["check_format"]:
            steps.append(Step("Format", (python, script("check_format.py")), repo))
        if f["check_hygiene"]:
            steps.append(Step("Hygiène", (python, script("check_hygiene.py")), repo))
        # `cmake --build` reconfigure seul quand un CMakeLists.txt change : la
        # configuration explicite ne sert qu'à un dossier neuf ou à --fresh.
        unconfigured = not (build_dir and os.path.isfile(os.path.join(build_dir, CMAKE_CACHE)))
        if f["configure"] or f["fresh"] or (f["build"] and unconfigured):
            argv = ["cmake", "--preset", c["configure_preset"]]
            if f["fresh"]:
                argv.append("--fresh")
            steps.append(Step("Configuration", tuple(argv), repo))
        if f["build"]:
            argv = ["cmake", "--build", "--preset", c["build_preset"]]
            if f["clean_first"]:
                argv.append("--clean-first")
            if f["verbose"]:
                argv.append("--verbose")
            steps.append(Step("Build", tuple(argv), repo))
        if f["test"]:
            steps.append(Step("Essais", ("ctest", "--preset", c["test_preset"]), repo))
        if f["archive"]:
            steps.append(Step("Symboles", (python, script("archive_symbols.py"),
                                           "--build", build_dir, "--config", configuration), repo))
        if f["deploy"]:
            argv = [python, script("deploy_client.py"), "--dest", self.paths["installation"].get().strip(),
                    "--build", build_dir, "--config", configuration]
            if f["draft"]:
                argv.append("--brouillon")
            steps.append(Step("Dépôt", tuple(argv), repo))
        return steps

    def _validate(self, steps):
        repo = self._repo()
        if not repo or not os.path.isdir(repo):
            return f"Dépôt moonclient introuvable : {repo or '(vide)'}"
        if not steps:
            return "Aucune étape cochée."
        labels = {s.label for s in steps}
        c = {k: v.get() for k, v in self.choices.items()}
        if not c["configure_preset"]:
            return "Aucun preset de configuration."
        if "Build" in labels and not c["build_preset"]:
            return f"Aucun preset de build pour « {c['configure_preset']} »."
        if "Essais" in labels and not c["test_preset"]:
            return f"Aucun preset d'essais pour « {c['configure_preset']} »."
        if labels & {"Symboles", "Dépôt"} and not self._build_dir():
            return "Dossier de build inconnu : binaryDir absent du preset."
        if "Dépôt" in labels:
            dest = self.paths["installation"].get().strip()
            if not dest or not os.path.isdir(dest):
                return f"Installation introuvable : {dest or '(vide)'}"
        return None

    def _run_chain(self):
        if self.busy():
            self._write("Une chaîne tourne déjà.", "fail")
            return
        self._reload_presets()
        steps = self._steps()
        problem = self._validate(steps)
        if problem:
            self._write(problem, "fail")
            return
        if self.flags["deploy"].get() and not messagebox.askyesno(
                "Déposer le client",
                f"La chaîne se termine par le dépôt du client dans\n"
                f"{self.paths['installation'].get().strip()}\n\n"
                f"Le client ne doit pas y tourner pendant le dépôt.\nLancer ?",
                parent=self.win, icon="question"):
            return
        self._queue = steps
        self._results = []
        self._chain_started = time.monotonic()
        self._write(f"══ {datetime.now():%H:%M:%S}  chaîne : "
                    + " → ".join(s.label for s in steps), "step")
        self._next_step()

    def _run_single(self, step):
        if self.busy():
            self._write("Une chaîne tourne déjà.", "fail")
            return
        if not os.path.isdir(step.cwd):
            self._write(f"Dépôt moonclient introuvable : {step.cwd}", "fail")
            return
        self._queue = [step]
        self._results = []
        self._chain_started = time.monotonic()
        self._next_step()

    def _next_step(self):
        step = self._queue[0]
        self._write(f"── {step.label}  > {subprocess.list2cmdline(step.argv)}", "cmd")
        try:
            self.runner.start(list(step.argv), step.cwd)
        except OSError as exc:
            self._write(f"Lancement impossible : {exc}", "fail")
            self._finish(failed_label=step.label)
            return
        self._step_started = time.monotonic()
        self.run_lbl.config(text=f"en cours : {step.label}  ({len(self._results) + 1}/"
                                 f"{len(self._results) + len(self._queue)})")
        self._update_buttons()

    def _on_exit(self, code):
        step = self._queue.pop(0)
        seconds = time.monotonic() - self._step_started
        self._results.append((step.label, code, seconds))
        self._write(f"   {step.label} : code {code}  ({seconds:.1f} s)", "ok" if code == 0 else "fail")
        if code != 0:
            self._finish(failed_label=step.label)
        elif self._queue:
            self._next_step()
        else:
            self._finish()

    def _finish(self, failed_label=None):
        skipped = [s.label for s in self._queue]
        self._queue = []
        total = time.monotonic() - (self._chain_started or time.monotonic())
        if failed_label:
            text = f"══ ÉCHEC à « {failed_label} »  ({total:.1f} s)"
            if skipped:
                text += "  — non lancées : " + ", ".join(skipped)
            self._write(text, "fail")
            self.run_lbl.config(text=f"échec : {failed_label}")
            self.win.bell()
        else:
            self._write(f"══ Réussi  ({total:.1f} s)", "ok")
            self.run_lbl.config(text="réussi")
        self._update_buttons()

    def _stop(self):
        if self.runner.busy:
            self._write("Arrêt demandé.", "fail")
            self.runner.stop()

    def _update_buttons(self):
        state = "disabled" if self.busy() else "normal"
        self.run_btn.config(state=state)
        for b in self.tool_btns:
            b.config(state=state)
        self.stop_btn.config(state="normal" if self.busy() else "disabled")

    # ---- outils ponctuels ---------------------------------------------------
    def _list_symbols(self):
        python = self.paths["python"].get().strip() or "python"
        repo = self._repo()
        self._run_single(Step("Symboles archivés",
                              (python, os.path.join(repo, "scripts", "archive_symbols.py"), "--lister"), repo))

    def _verify_tables(self):
        python = self.paths["python"].get().strip() or "python"
        repo = self._repo()
        self._run_single(Step("Tables générées",
                              (python, os.path.join(repo, "scripts", "regen_all.py"), "--verifier"), repo))

    def _open_build_dir(self):
        path = self._build_dir()
        if path and os.path.isdir(path):
            os.startfile(path)
        else:
            self._write(f"Dossier de build absent : {path or '?'}", "fail")

    # ---- sortie -------------------------------------------------------------
    def _clear(self):
        self.console.configure(state="normal")
        self.console.delete("1.0", "end")
        self.console.configure(state="disabled")

    def _write(self, text, tag="out"):
        at_bottom = self.console.yview()[1] >= 0.999
        self.console.configure(state="normal")
        self.console.insert("end", text + "\n", tag)
        self.console.configure(state="disabled")
        if at_bottom:
            self.console.see("end")

    def _drain(self):
        try:
            for _ in range(500):  # une rafale de MSBuild ne gèle pas la fenêtre
                try:
                    kind, payload = self.events.get_nowait()
                except queue.Empty:
                    break
                if kind == "exit":
                    self._on_exit(payload)
                else:
                    self._write(payload, classify(payload, kind))
        finally:
            self.win.after(DRAIN_MS, self._drain)


def classify(line, kind):
    """Couleur d'une ligne de sortie : les erreurs et avertissements de
    compilation ressortent même quand ils arrivent sur stdout."""
    if any(m in line for m in ERROR_MARKERS):
        return "fail"
    if any(m in line for m in WARNING_MARKERS):
        return "warn"
    return kind


def _self_test():
    import tempfile
    configure = [
        {"name": "base", "hidden": True, "generator": "Visual Studio 18 2026",
         "binaryDir": "${sourceDir}/build/${presetName}"},
        {"name": "windows", "inherits": "base", "binaryDir": "${sourceDir}/build/windows"},
        {"name": "windows-asan", "inherits": "windows", "binaryDir": "${sourceDir}/build/windows-asan"},
        {"name": "linux", "generator": "Ninja", "binaryDir": "${sourceDir}/build/linux"},
    ]
    build = [
        {"name": "windows-release", "configurePreset": "windows", "configuration": "Release"},
        {"name": "windows-debug", "configurePreset": "windows", "configuration": "Debug"},
        {"name": "windows-asan", "configurePreset": "windows-asan", "configuration": "Debug"},
    ]
    user = [{"name": "windows-jeu", "inherits": "windows"}]
    with tempfile.TemporaryDirectory() as repo:
        with open(os.path.join(repo, "CMakePresets.json"), "w", encoding="utf-8") as f:
            json.dump({"configurePresets": configure, "buildPresets": build}, f)
        with open(os.path.join(repo, "CMakeUserPresets.json"), "w", encoding="utf-8") as f:
            json.dump({"configurePresets": user}, f)
        p = load_presets(repo)
        assert not p.errors
        assert binary_dir(p, repo, "windows-jeu") == os.path.normpath(os.path.join(repo, "build/windows"))
        assert binary_dir(p, repo, "base") == os.path.normpath(os.path.join(repo, "build/base"))
        assert sorted(presets_for(p, "build", "windows-jeu", repo)) == ["windows-debug", "windows-release"]
        assert presets_for(p, "build", "windows-asan", repo) == ["windows-asan"], "autre dossier"
        usable = usable_configure_presets(p)
        assert "base" not in usable, "caché"
        if sys.platform == "win32":
            assert "linux" not in usable and "windows-jeu" in usable
        assert preset_configuration(p, "build", "windows-debug") == "Debug"
    assert classify("x.cpp(3): error C2065: 'y'", "out") == "fail"
    assert classify("x.cpp(3): warning C4100: 'y'", "out") == "warn"
    assert classify("  moonclient.vcxproj -> x.exe", "out") == "out"
    print("build_panel : essais réussis")


if __name__ == "__main__":
    if "--self-test" in sys.argv:
        _self_test()
