"""Garde d'encodage des scripts de moon/ : refuse ce qu'un editeur a casse.

Les scripts sont en Windows-1252, avec des ilots CP949 (noms de fichiers de la
GRF dans les cutin et soundeffect, noms de PNJ). Un editeur qui les lit en UTF-8
les abime de deux facons, toutes deux refusees ici :

  1. un octet qu'il ne sait pas lire devient U+FFFD (EF BF BD) a l'enregistrement :
     le caractere d'origine est perdu ;
  2. le fichier entier repart en UTF-8 : chaque accent 1252 devient deux octets
     (« e » accent aigu E9 -> C3 A9), et le client affiche « Ã© ».

Un BOM UTF-8 en tete est refuse aussi : rAthena le lirait comme du script.

Usage :
  python tools/util/check_npc_encoding.py            fichiers indexes (crochet pre-commit)
  python tools/util/check_npc_encoding.py --tout     tout moon/ sur le disque
Sortie 1 s'il reste un fichier refuse.
"""
import pathlib
import re
import subprocess
import sys

REPLACEMENT_CHAR = b'\xef\xbf\xbd'
UTF8_BOM = b'\xef\xbb\xbf'
# Un caractere Latin-1 (U+0080-U+00FF) ecrit en UTF-8 : la trace d'un 1252 transcode.
LATIN1_IN_UTF8 = re.compile(rb'[\xc2\xc3][\x80-\xbf]')
SCRIPT_SUFFIXES = ('.npc', '.txt')
SCRIPT_ROOT = 'moon/'


def problems(data):
    found = []
    if REPLACEMENT_CHAR in data:
        line = data[:data.index(REPLACEMENT_CHAR)].count(b'\n') + 1
        found.append(f'caractere de remplacement U+FFFD (ligne {line}) : '
                     'un editeur a lu le fichier dans le mauvais encodage')
    if data.startswith(UTF8_BOM):
        found.append('BOM UTF-8 en tete')
    if LATIN1_IN_UTF8.search(data):
        try:
            data.decode('utf-8')
            found.append('fichier transcode en UTF-8 : ses accents doivent etre en Windows-1252')
        except UnicodeDecodeError:
            pass
    return found


def staged_scripts():
    names = subprocess.run(
        ['git', 'diff', '--cached', '--name-only', '--diff-filter=ACMR', '-z'],
        capture_output=True, check=True).stdout.split(b'\0')
    for raw in names:
        name = raw.decode('utf-8', 'replace')
        if name.startswith(SCRIPT_ROOT) and name.endswith(SCRIPT_SUFFIXES):
            yield name, subprocess.run(['git', 'show', ':' + name],
                                       capture_output=True, check=True).stdout


def disk_scripts():
    for path in sorted(pathlib.Path(SCRIPT_ROOT).rglob('*')):
        if path.suffix in SCRIPT_SUFFIXES and path.is_file():
            yield path.as_posix(), path.read_bytes()


def main():
    source = disk_scripts() if '--tout' in sys.argv else staged_scripts()
    refused = 0
    for name, data in source:
        for problem in problems(data):
            refused += 1
            print(f'{name} : {problem}', file=sys.stderr)
    if refused:
        print('\nRouvrir le fichier en Windows-1252 (barre d\'etat > Reopen with Encoding),\n'
              'ou repartir de la version de git : git restore -- <fichier>', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
