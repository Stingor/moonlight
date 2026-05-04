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

    # Pattern : toutes les variantes de position head costume :
    # - valeurs uniques  : Upper, Middle, Lower, Top, Mid, Bot, Low, Bottom
    # - séparées virgule : "Position: Upper, Middle", "Position: Upper, Middle, Lower"
    # - séparées slash   : "Position: Upper / Middle / Lower" (idempotent)
    HEAD_WORD = rb'(?:Upper|Middle|Lower|Top|Mid|Bot|Low|Bottom)'
    POSITION_PATTERN = HEAD_WORD + rb'(?:[,/]\s*' + HEAD_WORD + rb')*'
    POSITION_PATTERN = rb'"Position: ' + POSITION_PATTERN + rb'"'

    def replace_in_costume_block(match):
        block = match.group(0)
        if b"costume = true" in block:
            block = re.sub(
                POSITION_PATTERN,
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

    def count_positions(data):
        return len(re.findall(POSITION_PATTERN, data))

    before = count_positions(content)
    after = count_positions(new_content)

    with open(path, "wb") as f:
        f.write(new_content)

    print(f"{path}: {before - after} remplacés, {after} headgears normaux conservés")


if __name__ == "__main__":
    for file in FILES:
        process_file(file)
