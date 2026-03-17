# -*- coding: utf-8 -*-
from Npp import editor

# Recupere tout le texte
text = editor.getText().strip()

# Separe les lignes et garde uniquement les nombres
xp_values = [line.strip() for line in text.split("\n") if line.strip().isdigit()]

yaml_lines = []
yaml_lines.append("Header:")
yaml_lines.append("  Type: HOMUN_EXP_DB")
yaml_lines.append("  Version: 1")
yaml_lines.append("")
yaml_lines.append("Body:")

level = 1
for xp in xp_values:
    yaml_lines.append("  - Level: {0}".format(level))
    yaml_lines.append("    Exp: {0}".format(xp))
    level += 1

# Remplace tout le contenu du fichier
editor.setText("\n".join(yaml_lines))
