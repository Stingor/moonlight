"""Onglets détachables de MoonlightAdmin.

Chaque panneau vit dans une page : un cadre Tk qui est soit un onglet de la
fenêtre principale, soit une fenêtre à lui. Passer de l'un à l'autre ne
reconstruit rien (`wm manage` / `wm forget`) : la saisie en cours, la sortie
d'une console et une commande ssh qui tourne restent en place.

  - détacher : glisser l'onglet hors de la fenêtre, double-clic dessus, ou
    clic droit → « Détacher » ;
  - réattacher : le bouton en tête de la fenêtre détachée, ou sa croix. La
    croix ne fait rien disparaître : la page redevient onglet.

Une page est « visible » quand elle est détachée, ou qu'elle est l'onglet
affiché d'une fenêtre principale qui n'est pas masquée. Le panneau en est
prévenu (`on_page_shown`, `on_page_hidden`) : c'est là qu'il se construit la
première fois, et qu'un relevé périodique démarre ou s'arrête.

La fenêtre principale peut être masquée tant qu'une page est détachée : les
connexions et le bandeau d'état y vivent toujours. La disposition (pages
détachées, leur géométrie, l'onglet affiché) se retient dans la section
[onglets] de MoonlightAdmin.ini.
"""
import tkinter as tk
from dataclasses import dataclass
from tkinter import ttk

SETTINGS_SECTION = "onglets"
LIST_SEPARATOR = ","
GEOMETRY_KEY_PREFIX = "geometry_"
# Une page détachée sans géométrie retenue s'ouvre décalée de la fenêtre
# principale, ou du point où l'onglet a été lâché.
DETACH_OFFSET_PX = 40
DROP_OFFSET_PX = 20
# Un glisser commence au-delà de ce déplacement : en deçà, c'est un clic.
DRAG_THRESHOLD_PX = 8
DRAG_CURSOR = "fleur"


@dataclass(frozen=True)
class PageSpec:
    key: str        # nom dans l'ini et pour --panneau
    tab: str        # texte de l'onglet
    title: str      # titre de la fenêtre détachée
    min_width: int
    min_height: int


@dataclass
class _Page:
    spec: PageSpec
    panel: object       # on_page_shown(), on_page_hidden()
    frame: tk.Frame     # le cadre déplacé : onglet ou fenêtre
    bar: ttk.Frame      # tête de la fenêtre détachée, masquée en onglet
    body: ttk.Frame     # ce que le panneau remplit
    detached: bool = False
    geometry: str = ""  # dernière géométrie détachée


class TabHost:
    def __init__(self, root, settings):
        self.root = root
        self.settings = settings
        self.notebook = ttk.Notebook(root)
        self.notebook.enable_traversal()  # Ctrl+Tab
        self._pages = {}      # clé -> _Page, dans l'ordre des onglets
        self._visible = set()
        self._background = None
        self._topmost = False
        self._drag = None     # (clé, x_root, y_root) du bouton pressé sur un onglet
        self._dragging = False
        self.notebook.bind("<<NotebookTabChanged>>", lambda _e: self._refresh_visibility())
        self.notebook.bind("<ButtonPress-1>", self._on_press, add="+")
        self.notebook.bind("<B1-Motion>", self._on_motion, add="+")
        self.notebook.bind("<ButtonRelease-1>", self._on_release, add="+")
        self.notebook.bind("<Double-Button-1>", self._on_double_click, add="+")
        self.notebook.bind("<Button-3>", self._on_right_click, add="+")

    # ---- pages ---------------------------------------------------------------
    def add(self, spec, panel):
        """Une page pour `panel`, en dernier onglet ; son cadre à remplir."""
        # tk.Frame et non ttk.Frame : seul le premier se laisse `wm manage`.
        frame = tk.Frame(self.root, name=f"page_{spec.key}")
        if self._background is not None:
            frame.configure(background=self._background)
        bar = ttk.Frame(frame)
        ttk.Button(bar, text="⇱ Réattacher", command=lambda: self.attach(spec.key)).pack(
            side="left", padx=4, pady=(4, 0))
        ttk.Button(bar, text="Fenêtre principale", command=self.show_main).pack(
            side="left", padx=(0, 4), pady=(4, 0))
        body = ttk.Frame(frame)
        body.pack(fill="both", expand=True)
        self._pages[spec.key] = _Page(spec, panel, frame, bar, body)
        self.notebook.add(frame, text=spec.tab)
        return body

    def body(self, key):
        return self._pages[key].body

    def keys(self):
        return list(self._pages)

    def is_visible(self, key):
        return key in self._visible

    def detached_keys(self):
        return [k for k, p in self._pages.items() if p.detached]

    def reveal(self, key):
        """Amène la page devant : sa fenêtre si elle est détachée, sinon son
        onglet, fenêtre principale montrée. Le panneau est prévenu même s'il
        était déjà visible : il se rafraîchit."""
        page = self._pages[key]
        was_visible = key in self._visible
        if page.detached:
            self.root.tk.call("wm", "deiconify", page.frame)
            self.root.tk.call("raise", page.frame)
        else:
            self.show_main()
            self.notebook.select(page.frame)
        self._refresh_visibility()  # prévient le panneau qui devient visible
        if was_visible:
            page.panel.on_page_shown()

    def show_main(self):
        if self.root.state() in ("withdrawn", "iconic"):
            self.root.deiconify()
        self.root.lift()
        self._refresh_visibility()

    def hide_main(self):
        self.root.withdraw()
        self._refresh_visibility()

    # ---- détacher, réattacher -----------------------------------------------
    def detach(self, key, x_root=None, y_root=None):
        page = self._pages[key]
        if page.detached:
            self.reveal(key)
            return
        width, height = page.frame.winfo_width(), page.frame.winfo_height()
        self.notebook.forget(page.frame)
        tk_call = self.root.tk.call
        tk_call("wm", "manage", page.frame)
        page.detached = True
        page.bar.pack(fill="x", side="top", before=page.body)
        tk_call("wm", "title", page.frame, page.spec.title)
        tk_call("wm", "minsize", page.frame, page.spec.min_width, page.spec.min_height)
        tk_call("wm", "protocol", page.frame, "WM_DELETE_WINDOW",
                self.root.register(lambda: self.attach(key)))
        if self._topmost:
            tk_call("wm", "attributes", page.frame, "-topmost", 1)
        geometry = page.geometry
        if x_root is not None:
            size = f"{width}x{height}" if width > 1 and height > 1 else ""
            geometry = f"{size}+{x_root - DROP_OFFSET_PX}+{y_root - DROP_OFFSET_PX}"
        elif not geometry:
            geometry = (f"+{self.root.winfo_rootx() + DETACH_OFFSET_PX}"
                        f"+{self.root.winfo_rooty() + DETACH_OFFSET_PX}")
        try:
            tk_call("wm", "geometry", page.frame, geometry)
        except tk.TclError:
            pass
        self._refresh_visibility()

    def attach(self, key):
        page = self._pages[key]
        if not page.detached:
            return
        page.geometry = self.root.tk.call("wm", "geometry", page.frame)
        self.root.tk.call("wm", "forget", page.frame)
        page.detached = False
        page.bar.pack_forget()
        self.notebook.insert(self._tab_position(key), page.frame, text=page.spec.tab)
        # Rien ne disparaît : la page revient dans une fenêtre qu'on voit.
        self.show_main()
        self.notebook.select(page.frame)
        self._refresh_visibility()

    def _tab_position(self, key):
        """La place de la page parmi les onglets présents : celle de l'ordre
        d'ajout, sans compter les pages détachées."""
        position = 0
        for other, page in self._pages.items():
            if other == key:
                break
            if not page.detached:
                position += 1
        return position if position < len(self.notebook.tabs()) else "end"

    # ---- visibilité -----------------------------------------------------------
    def _refresh_visibility(self):
        visible = {k for k, p in self._pages.items() if p.detached}
        if self.root.state() != "withdrawn":
            selected = self.notebook.select()
            visible |= {k for k, p in self._pages.items() if not p.detached and str(p.frame) == selected}
        gained, lost = visible - self._visible, self._visible - visible
        self._visible = visible
        for key in lost:
            self._pages[key].panel.on_page_hidden()
        for key in gained:
            self._pages[key].panel.on_page_shown()

    # ---- souris sur les onglets -------------------------------------------------
    def _tab_at(self, event):
        if self.notebook.identify(event.x, event.y) != "label":
            return None
        frame = self.notebook.tabs()[self.notebook.index(f"@{event.x},{event.y}")]
        return next((k for k, p in self._pages.items() if str(p.frame) == frame), None)

    def _on_press(self, event):
        key = self._tab_at(event)
        self._drag = (key, event.x_root, event.y_root) if key else None
        self._dragging = False

    def _on_motion(self, event):
        if self._drag is None:
            return
        _key, x0, y0 = self._drag
        if abs(event.x_root - x0) + abs(event.y_root - y0) > DRAG_THRESHOLD_PX:
            self._dragging = True
        if self._dragging:
            outside = self._outside_main(event.x_root, event.y_root)
            self.notebook.configure(cursor=DRAG_CURSOR if outside else "")

    def _on_release(self, event):
        drag, dragging = self._drag, self._dragging
        self._drag, self._dragging = None, False
        self.notebook.configure(cursor="")
        if drag and dragging and self._outside_main(event.x_root, event.y_root):
            self.detach(drag[0], event.x_root, event.y_root)

    def _outside_main(self, x_root, y_root):
        left, top = self.root.winfo_rootx(), self.root.winfo_rooty()
        return not (left <= x_root < left + self.root.winfo_width()
                    and top <= y_root < top + self.root.winfo_height())

    def _on_double_click(self, event):
        key = self._tab_at(event)
        if key:
            self.detach(key)

    def _on_right_click(self, event):
        key = self._tab_at(event)
        if not key:
            return
        menu = tk.Menu(self.notebook, tearoff=0)
        menu.add_command(label=f"Détacher « {self._pages[key].spec.tab} »", command=lambda: self.detach(key))
        menu.tk_popup(event.x_root, event.y_root)

    # ---- apparence ----------------------------------------------------------------
    def set_background(self, color):
        self._background = color
        for page in self._pages.values():
            page.frame.configure(background=color)

    def set_topmost(self, on):
        self._topmost = on
        for page in self._pages.values():
            if page.detached:
                self.root.tk.call("wm", "attributes", page.frame, "-topmost", int(on))

    # ---- réglages -------------------------------------------------------------------
    def settings_data(self):
        data = {"detached": LIST_SEPARATOR.join(self.detached_keys()),
                "selected": next((k for k, p in self._pages.items()
                                  if not p.detached and str(p.frame) == self.notebook.select()), "")}
        for key, page in self._pages.items():
            geometry = self.root.tk.call("wm", "geometry", page.frame) if page.detached else page.geometry
            if geometry:
                data[GEOMETRY_KEY_PREFIX + key] = geometry
        return data

    def restore(self, only=None):
        """Rétablit la disposition retenue. Avec `only`, ouvre cette seule page,
        détachée, fenêtre principale masquée (option --panneau)."""
        for key, page in self._pages.items():
            page.geometry = self.settings.get(SETTINGS_SECTION, GEOMETRY_KEY_PREFIX + key, "")
        selected = self.settings.get(SETTINGS_SECTION, "selected", "")
        if selected in self._pages:
            self.notebook.select(self._pages[selected].frame)
        stored = self.settings.get(SETTINGS_SECTION, "detached", "")
        detached = [only] if only else [k for k in stored.split(LIST_SEPARATOR) if k in self._pages]
        for key in detached:
            self.detach(key)
        if only:
            self.hide_main()
        self._refresh_visibility()
