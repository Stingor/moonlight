"""Le nom lisible des cartes, pour les consoles de MoonlightAdmin.

La table est celle que moonclient livre à ses joueurs
(dist/data/yaml_files/map_names.yml) : carte du serveur -> nom montré au
joueur. Elle se lit dans le dépôt moonclient du panneau Tables ; absente, ou
sans PyYAML, les cartes se montrent par leur seul nom de fichier.
"""
import os

try:
    import yaml
except ImportError:
    yaml = None

MAP_NAMES_RELATIVE_PATH = os.path.join("dist", "data", "yaml_files", "map_names.yml")


class MapTitles:
    def __init__(self, title_by_map):
        self._title_by_map = title_by_map

    @classmethod
    def load(cls, moonclient_repo):
        """La table du dépôt moonclient ; vide si elle manque ou se lit mal."""
        if yaml is None or not moonclient_repo:
            return cls({})
        path = os.path.join(moonclient_repo, MAP_NAMES_RELATIVE_PATH)
        try:
            with open(path, encoding="utf-8") as f:
                maps = yaml.safe_load(f)["maps"]
        except (OSError, yaml.YAMLError, KeyError, TypeError):
            return cls({})
        if not isinstance(maps, dict):
            return cls({})
        return cls({str(name): entry["name"] for name, entry in maps.items()
                    if isinstance(entry, dict) and entry.get("name")})

    def title(self, map_name):
        """Le nom lisible de la carte, ou "" quand la table ne la nomme pas."""
        return self._title_by_map.get(map_name, "")
