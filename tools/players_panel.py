"""Fenêtre « Joueurs en ligne » de MoonlightAdmin.

Le tableau des personnages connectés, relu toutes les `intervalle_s` secondes
(section [joueurs] de MoonlightAdmin.ini) par la commande PLAYERS du canal
admin du map-server (src/map/admin.cpp), tant que la fenêtre est ouverte.
Le relevé passe en silence : il n'écrit rien au journal principal.

Un clic sur un en-tête trie ; le filtre cherche dans le nom, la carte et son
nom lisible, la guilde, le groupe et le métier. Une carte d'instance se montre
par la carte qu'elle copie ; le survol d'une carte en donne le nom lisible
(map_titles.py). Un clic droit sur une ligne ouvre les actions sur ce joueur
(player_actions.py), qui elles passent au journal.
"""
import tkinter as tk
import unicodedata
from dataclasses import dataclass
from datetime import datetime
from tkinter import ttk

from map_titles import MapTitles
from player_actions import add_player_menu
from tabs_host import PageSpec

SETTINGS_SECTION = "joueurs"
PAGE = PageSpec("players", "Joueurs", "Moonlight - Joueurs en ligne", 1000, 420)
DEFAULT_INTERVAL_S = 5
MIN_INTERVAL_S = 2
MS_PER_S = 1000
FIELD_SEPARATOR = "\t"
# Au-delà, un joueur est dit inactif (affiché en minutes).
IDLE_SHOWN_AFTER_S = 300
SECONDS_PER_MINUTE = 60
UNKNOWN_COMMAND_REPLY = "ERR unknown command"
NOT_CONNECTED_PREFIX = "ERR pas connecté"


@dataclass(frozen=True)
class Player:
    char_id: int
    account_id: int
    name: str
    base_level: int
    job_level: int
    job: str
    group_id: int
    map: str
    x: int
    y: int
    party: str
    guild: str
    vending: bool
    autotrade: bool
    idle_s: int
    # La carte dont une carte d'instance est la copie ; vide hors instance.
    source_map: str = ""

    @classmethod
    def parse(cls, line):
        """Une ligne de PLAYERS, ou None si elle n'en a pas la forme : les champs
        dans l'ordre d'admin_players. Un map-server qui ne donne pas encore la
        carte source s'arrête un champ plus tôt."""
        f = line.split(FIELD_SEPARATOR)
        if len(f) not in (len(PLAYER_FIELDS), len(PLAYER_FIELDS) - 1):
            return None
        try:
            return cls(int(f[0]), int(f[1]), f[2], int(f[3]), int(f[4]), f[5], int(f[6]), f[7],
                       int(f[8]), int(f[9]), f[10], f[11], f[12] == "1", f[13] == "1", int(f[14]),
                       f[15] if len(f) > SOURCE_MAP_FIELD else "")
        except ValueError:
            return None

    def shown_map(self):
        """La carte telle qu'on la lit : une carte d'instance (« 0979#000040 »)
        par la carte qu'elle copie et le numéro de l'instance."""
        if not self.source_map:
            return self.map
        instance = self.map.rpartition(INSTANCE_MAP_SEPARATOR)[2].lstrip("0") or "0"
        return f"{self.source_map} (instance {instance})"

    def real_map(self):
        """La carte qui porte un nom lisible : la source pour une instance."""
        return self.source_map or self.map

    def state(self):
        parts = []
        if self.autotrade:
            parts.append("autotrade")
        elif self.vending:
            parts.append("boutique")
        if self.idle_s >= IDLE_SHOWN_AFTER_S:
            parts.append(f"inactif {self.idle_s // SECONDS_PER_MINUTE} min")
        return ", ".join(parts)


PLAYER_FIELDS = ("char_id", "account_id", "name", "base_level", "job_level", "job", "group_id", "map",
                 "x", "y", "party", "guild", "vending", "autotrade", "idle_s", "source_map")
SOURCE_MAP_FIELD = PLAYER_FIELDS.index("source_map")
# Une carte d'instance se nomme « <numéro de carte>#<numéro d'instance> ».
INSTANCE_MAP_SEPARATOR = "#"


def text_key(text):
    """Clé de tri d'un texte : sans casse ni accents, « Éloïse » avec les E."""
    decomposed = unicodedata.normalize("NFD", text.casefold())
    return "".join(c for c in decomposed if not unicodedata.combining(c))


# (colonne, en-tête, largeur, clé de tri)
COLUMNS = (
    ("name", "Nom", 150, lambda p: text_key(p.name)),
    ("level", "Niv.", 65, lambda p: (p.base_level, p.job_level)),
    ("job", "Métier", 115, lambda p: text_key(p.job)),
    ("map", "Carte", 135, lambda p: (p.shown_map(), p.x, p.y)),
    ("party", "Groupe", 100, lambda p: text_key(p.party)),
    ("guild", "Guilde", 110, lambda p: text_key(p.guild)),
    ("state", "État", 150, lambda p: (p.autotrade, p.vending, p.idle_s)),
    ("group", "GM", 40, lambda p: p.group_id),
    ("ids", "CID / AID", 115, lambda p: p.char_id),
)
ROW_COLORS = {"gm": "#c8f", "away": "#888"}
# Treeview.identify_column numérote les colonnes « #1 », « #2 »…
MAP_COLUMN_ID = f"#{[c[0] for c in COLUMNS].index('map') + 1}"
# Décalage de la bulle sous le pointeur, en pixels.
TOOLTIP_OFFSET_PX = 16
TOOLTIP_COLORS = {"background": "#ffffe0", "foreground": "#000"}


class PlayersWindow:
    """Page « Joueurs en ligne » (tabs_host.py), construite à sa première apparition ;
    hors de vue, elle ne relit plus rien."""

    def __init__(self, root, host, settings, send_quiet, send_admin, moonclient_repo):
        self.root = root
        self.moonclient_repo = moonclient_repo  # () -> dossier du dépôt moonclient (panneau Tables)
        self.map_titles = MapTitles({})
        self._tooltip = None
        self._tooltip_row = None
        self.host = host
        self.send_quiet = send_quiet   # (ligne, rappel) : PLAYERS, sans journal
        self.send_admin = send_admin   # (atcommande) : les actions, au journal
        try:
            interval = int(settings.get(SETTINGS_SECTION, "intervalle_s", DEFAULT_INTERVAL_S))
        except ValueError:
            interval = DEFAULT_INTERVAL_S
        self.interval_s = max(MIN_INTERVAL_S, interval)
        self.filter_var = tk.StringVar()
        self.hide_shops_var = tk.BooleanVar(value=settings.get_bool(SETTINGS_SECTION, "hide_autotrade", False))
        self._players = []
        self._sort_key, self._sort_reverse = "name", False
        self._waiting = False
        self._job = None
        self._shown = False
        self.win = None

    # ---- réglages -----------------------------------------------------------
    def settings_data(self):
        return {"intervalle_s": self.interval_s, "hide_autotrade": self.hide_shops_var.get()}

    # ---- fenêtre ------------------------------------------------------------
    def show(self):
        self.host.reveal(PAGE.key)

    def on_page_shown(self):
        if self.win is None:
            self._build()
        self._shown = True
        self.refresh()

    def on_page_hidden(self):
        self._shown = False
        if self._job is not None:
            self.win.after_cancel(self._job)
            self._job = None

    def _build(self):
        self.map_titles = MapTitles.load(self.moonclient_repo())
        self.win = self.host.body(PAGE.key)
        body = ttk.Frame(self.win)
        body.pack(fill="both", expand=True)

        bar = ttk.Frame(body)
        bar.pack(fill="x", padx=8, pady=(8, 4))
        ttk.Label(bar, text="Filtre").pack(side="left")
        entry = ttk.Entry(bar, textvariable=self.filter_var, width=28)
        entry.pack(side="left", padx=4)
        entry.bind("<Escape>", lambda _e: self.filter_var.set(""))
        self.filter_var.trace_add("write", lambda *_: self._render())
        ttk.Checkbutton(bar, text="masquer les boutiques en autotrade", variable=self.hide_shops_var,
                        command=self._render).pack(side="left", padx=8)
        ttk.Button(bar, text="↻", width=3, command=self.refresh).pack(side="right")
        self.summary_lbl = ttk.Label(bar, text="")
        self.summary_lbl.pack(side="right", padx=8)

        listing = ttk.Frame(body)
        listing.pack(fill="both", expand=True, padx=8, pady=(0, 8))
        self.tree = ttk.Treeview(listing, columns=[c[0] for c in COLUMNS], show="headings",
                                 selectmode="browse")
        for key, title, width, _sort in COLUMNS:
            self.tree.heading(key, text=title, command=lambda k=key: self._sort_by(k))
            self.tree.column(key, width=width, anchor="w", stretch=key in ("name", "map", "guild"))
        scroll = ttk.Scrollbar(listing, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scroll.set)
        self.tree.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
        for tag, color in ROW_COLORS.items():
            self.tree.tag_configure(tag, foreground=color)
        self.tree.bind("<Button-3>", self._row_menu)
        self.tree.bind("<Motion>", self._hover)
        self.tree.bind("<Leave>", lambda _e: self._hide_tooltip())

    # ---- relevé -------------------------------------------------------------
    def refresh(self):
        if self._waiting or not self._shown:
            return
        self._waiting = True
        self.send_quiet("PLAYERS", self._on_players)

    def _on_players(self, output, reply):
        self._waiting = False
        if reply.startswith("OK"):
            players = [p for p in (Player.parse(line) for line in output) if p is not None]
            self._players = players
            self._render()
            shops = sum(p.vending or p.autotrade for p in players)
            gms = sum(p.group_id > 0 for p in players)
            self.summary_lbl.config(text=f"{len(players)} en ligne · {shops} boutique(s) · {gms} GM"
                                         f"  ·  relevé {datetime.now():%H:%M:%S}", foreground="")
        elif reply.startswith(UNKNOWN_COMMAND_REPLY):
            self.summary_lbl.config(text="le map-server ne connaît pas encore PLAYERS : make server, "
                                         "puis redémarrage", foreground="#d90")
        elif reply.startswith(NOT_CONNECTED_PREFIX):
            self.summary_lbl.config(text="canal admin non connecté (fenêtre principale)", foreground="#d90")
        else:
            self.summary_lbl.config(text=reply, foreground="#e55")
        if self._shown:
            self._job = self.win.after(self.interval_s * MS_PER_S, self._scheduled)

    def _scheduled(self):
        self._job = None
        self.refresh()

    # ---- tableau ------------------------------------------------------------
    def _visible(self):
        needle = self.filter_var.get().strip().lower()
        shown = []
        for p in self._players:
            if self.hide_shops_var.get() and p.autotrade:
                continue
            haystack = text_key(" ".join((p.name, p.map, p.source_map, self.map_titles.title(p.real_map()),
                                          p.guild, p.party, p.job)))
            if needle and text_key(needle) not in haystack:
                continue
            shown.append(p)
        sort = dict((c[0], c[3]) for c in COLUMNS)[self._sort_key]
        return sorted(shown, key=sort, reverse=self._sort_reverse)

    def _render(self):
        if self.win is None:
            return
        selected = self.tree.selection()
        self._hide_tooltip()
        self.tree.delete(*self.tree.get_children())
        for p in self._visible():
            values = (p.name, f"{p.base_level} / {p.job_level}", p.job, f"{p.shown_map()} {p.x},{p.y}",
                      p.party, p.guild, p.state(), p.group_id or "", f"{p.char_id} / {p.account_id}")
            tags = ("gm",) if p.group_id > 0 else ("away",) if p.autotrade else ()
            self.tree.insert("", "end", iid=str(p.char_id), values=values, tags=tags)
        keep = [i for i in selected if self.tree.exists(i)]
        if keep:
            self.tree.selection_set(keep)

    def _sort_by(self, key):
        if self._sort_key == key:
            self._sort_reverse = not self._sort_reverse
        else:
            self._sort_key, self._sort_reverse = key, False
        self._render()

    def _row_menu(self, event):
        row = self.tree.identify_row(event.y)
        if not row:
            return
        self.tree.selection_set(row)
        player = next((p for p in self._players if str(p.char_id) == row), None)
        if player is None:
            return
        menu = tk.Menu(self.win, tearoff=0)
        add_player_menu(menu, player.name, self.send_admin, self.win)
        menu.add_command(label="Copier le nom", command=lambda: self._copy(player.name))
        menu.tk_popup(event.x_root, event.y_root)

    # ---- bulle du nom de carte ---------------------------------------------
    def _map_tooltip_text(self, player):
        title = self.map_titles.title(player.real_map())
        if player.source_map:
            # Le nom brut reste lisible ici : c'est lui que prennent les atcommandes.
            return f"{title} — {player.map}" if title else player.map
        return title

    def _hover(self, event):
        row = self.tree.identify_row(event.y)
        on_map = row and self.tree.identify_column(event.x) == MAP_COLUMN_ID
        player = next((p for p in self._players if str(p.char_id) == row), None) if on_map else None
        text = self._map_tooltip_text(player) if player is not None else ""
        if not text:
            self._hide_tooltip()
            return
        x, y = event.x_root + TOOLTIP_OFFSET_PX, event.y_root + TOOLTIP_OFFSET_PX
        if self._tooltip is not None and self._tooltip_row == row:
            self._tooltip.geometry(f"+{x}+{y}")
            return
        self._hide_tooltip()
        self._tooltip = tk.Toplevel(self.win)
        self._tooltip.wm_overrideredirect(True)
        self._tooltip.geometry(f"+{x}+{y}")
        tk.Label(self._tooltip, text=text, relief="solid", borderwidth=1, padx=4, pady=2,
                 **TOOLTIP_COLORS).pack()
        self._tooltip_row = row

    def _hide_tooltip(self):
        if self._tooltip is not None:
            self._tooltip.destroy()
        self._tooltip = None
        self._tooltip_row = None

    def _copy(self, text):
        self.win.clipboard_clear()
        self.win.clipboard_append(text)
