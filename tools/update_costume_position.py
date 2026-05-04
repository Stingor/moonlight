"""
update_costume_position.py

Remplace "Position: Upper/Middle/Lower" par "Position: Upper / Middle / Lower"
dans les blocs costume (costume = true) des fichiers itemInfo Lua.

Traite les fichiers en binaire pour préserver l'encodage EUC-KR.

Usage:
    python tools/update_costume_position.py
"""

import re

FILES = [
    "client/SystemEN/itemInfomoon.lua",
    "client/SystemEN/itemInfokro.lua",
]


def process_file(path):
    with open(path, "rb") as f:
        content = f.read()

    def replace_in_costume_block(match):
        block = match.group(0)
        if b"costume = true" in block:
            block = re.sub(
                rb'"Position: (?:Upper|Middle|Lower)"',
                b'"Position: Upper / Middle / Lower"',
                block,
            )
        return block

    new_content = re.sub(
        rb"\[\d+\]\s*=\s*\{.*?\n\t\}",
        replace_in_costume_block,
        content,
        flags=re.DOTALL,
    )

    before = (
        content.count(b'"Position: Upper"')
        + content.count(b'"Position: Middle"')
        + content.count(b'"Position: Lower"')
    )
    after = (
        new_content.count(b'"Position: Upper"')
        + new_content.count(b'"Position: Middle"')
        + new_content.count(b'"Position: Lower"')
    )

    with open(path, "wb") as f:
        f.write(new_content)

    print(f"{path}: {before - after} remplacés, {after} headgears normaux conservés")


if __name__ == "__main__":
    for file in FILES:
        process_file(file)
