#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
check_cell.py
=============
Dit si une cellule est praticable, en lisant le map cache du serveur.

A utiliser avant d'ecrire un warp vers une coordonnee choisie a la main
(warp agent, point de repli d'instance...) : pc_setpos ne relocalise que
les coordonnees HORS carte, une cellule bloquee en plein milieu laisse le
joueur coince.

Usage :
  python tools/util/check_cell.py rachel 174 140
  python tools/util/check_cell.py rachel 174,140 172,138 120,125

Format du cache (src/map/map.cpp) :
  header  : uint32 file_size, uint16 map_count  -> sizeof == 8 (padding)
  par map : char name[12], int16 xs, int16 ys, int32 len, puis len octets zlib
  cellule : un octet gat ; 1 (mur) et 5 (fosse) ne sont pas praticables.
"""
import argparse
import io
import os
import struct
import sys
import zlib

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
CACHE = os.path.join(ROOT, 'db', 'pre-re', 'map_cache.dat')   # db/map_cache.dat est un stub
GAT = {0: 'sol', 1: 'MUR', 2: 'sol', 3: 'eau', 4: 'sol', 5: 'FOSSE', 6: 'sol'}


def load_cache(path):
    data = open(path, 'rb').read()
    _, count = struct.unpack_from('<IH', data, 0)
    pos, maps = 8, {}
    for _ in range(count):
        name, xs, ys, ln = struct.unpack_from('<12shhi', data, pos)
        pos += 20
        maps[name.split(b'\0')[0].decode('latin-1')] = (xs, ys, data[pos:pos + ln])
        pos += ln
    return maps


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('map')
    ap.add_argument('coords', nargs='+', help='x y  ou  x,y x,y ...')
    a = ap.parse_args()

    if len(a.coords) == 2 and ',' not in a.coords[0]:
        pairs = [(int(a.coords[0]), int(a.coords[1]))]
    else:
        pairs = [tuple(int(v) for v in c.split(',')) for c in a.coords]

    maps = load_cache(CACHE)
    if a.map not in maps:
        print('%s : absente du cache (%d cartes)' % (a.map, len(maps)))
        return 2
    xs, ys, blob = maps[a.map]
    gat = zlib.decompress(blob)
    assert len(gat) == xs * ys
    print('%s (%dx%d)' % (a.map, xs, ys))
    rc = 0
    for x, y in pairs:
        if not (0 <= x < xs and 0 <= y < ys):
            print('  (%3d,%3d)  HORS CARTE' % (x, y))
            rc = 1
            continue
        g = gat[y * xs + x]
        ok = g not in (1, 5)
        rc |= 0 if ok else 1
        print('  (%3d,%3d)  gat=%d %-5s -> %s' % (x, y, g, GAT.get(g, '?'),
                                                 'praticable' if ok else '** BLOQUEE **'))
    return rc


if __name__ == '__main__':
    sys.exit(main())
