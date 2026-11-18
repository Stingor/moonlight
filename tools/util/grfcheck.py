import sys, struct, zlib, os

def grf_names(path):
    with open(path, 'rb') as f:
        hdr = f.read(46)
        assert hdr[:15] in (b'Master of Magic', b'Event Horizon\x00c'), path
        ver = struct.unpack_from('<I', hdr, 42)[0]
        if ver == 0x300:
            off = struct.unpack_from('<Q', hdr, 30)[0]
            f.seek(off + 46)
            f.read(4)  # unknown
        else:
            off = struct.unpack_from('<I', hdr, 30)[0]
            f.seek(off + 46)
        csize, usize = struct.unpack('<II', f.read(8))
        table = zlib.decompress(f.read(csize))
    names = []
    p = 0
    entry = 21 if ver == 0x300 else 17
    while p < len(table):
        e = table.index(b'\0', p)
        names.append(table[p:e])
        p = e + 1 + entry
    return ver, names

cuts = [l.strip() for l in open(sys.argv[1], encoding='ascii') if l.strip()]
found = {}
for g in sys.argv[2:]:
    ver, names = grf_names(g)
    print(f'{os.path.basename(g)}: version {ver:#x}, {len(names)} entrées')
    for n in names:
        s = n.decode('cp949', 'replace').replace('/', '\\').lower()
        if '\\illust\\' in s:
            base = s.rsplit('\\', 1)[1]
            found.setdefault(base, []).append((os.path.basename(g), s))
missing = []
for c in cuts:
    stem = c.lower().rsplit('.', 1)[0]
    hits = [k for k in found if k.rsplit('.', 1)[0] == stem]
    exact = c.lower() in found
    if exact:
        print('OK  ', c, '->', found[c.lower()][0][0])
    elif hits:
        print('EXT ', c, '-> autre extension :', hits, found[hits[0]][0][0])
    else:
        print('MISS', c)
        missing.append(c)
print('Manquantes :', len(missing))
dirs = {}
for c in cuts:
    for g, s in found.get(c.lower(), []):
        dirs.setdefault(s.rsplit('\\', 1)[0], set()).add(g)
print(dirs)
