#!/usr/bin/env python3
"""The launcher's, the dedicated server's and CRes.dll's art: assets/art/.

  build/appres.py write   <retail game directory> <art directory>
  build/appres.py check   <retail game directory> <art directory>
  build/appres.py install <art directory> <output directory>

Tron 2.0's launcher (Tron.exe), dedicated server (TRONSrv.exe) and client
resources (CRes.dll) carry art in their resources: the launcher's
backgrounds and buttons (bitmaps), its sounds (8 waves), its icon and
cursor; the server's splash screen and two icons; the client resources'
one cursor. None of it was ever released as source (Build 69's
Launcher/TRON/RES has only its .rc2). The retail art is committed,
unchanged, in assets/art/ (assets/README.md), so that the binaries compile
the same with or without the retail game. Each file is the file the
resource compiler reads -- a bitmap with its BITMAPFILEHEADER, a wave as
it is, an icon or cursor group as an .ico or .cur file -- in
<art>/Launcher/, <art>/ServerApp/ and <art>/ClientRes/. Compiled again,
each resource is the retail one byte for byte. (cjacksrc's
build/appres.py, for Contract J.A.C.K.'s, is the model.)

  write    writes each one out of the retail image into <art> again (make art)
  check    fails unless <art> holds exactly what `write` would write (make
           art-check; build/compile.sh does it when it has the retail game)
  install  copies <art> into <output>, where the projects' resource scripts
           find them (their /i paths): build/compile.sh does it into
           src/Game/built/retail/ before the compile. It writes only what is
           missing or differs, so a rebuild recompiles nothing.
"""
import os
import struct
import sys

RT_CURSOR, RT_BITMAP, RT_ICON, RT_GROUP_CURSOR, RT_GROUP_ICON = 1, 2, 3, 12, 14

# Each image, and the file name its resource script gives each resource of
# it (named resources are written under their own name). The names are
# NOLF2's source release's for the same resources.
IMAGES = {
    "Tron.exe": ("Launcher", {
        (RT_BITMAP, 130): "MainAppBackground.bmp",
        (RT_BITMAP, 164): "OptionsBackground.bmp",
        (RT_BITMAP, 219): "DisplayBackground.bmp",
        (RT_BITMAP, 283): "BoxBackground.bmp",
        (RT_BITMAP, 284): "DetailSettingsBackground.bmp",
        (RT_BITMAP, 296): "Information.bmp",
        (RT_BITMAP, 297): "Error.bmp",
        (RT_BITMAP, 298): "Warning.bmp",
        (RT_BITMAP, 320): "DemoMainAppBackground.bmp",
        (RT_BITMAP, 323): "CustomBackground.bmp",
        ("WAVE", 225): "Select.wav",
        ("WAVE", 226): "Click.wav",
        ("WAVE", 278): "buttondown.wav",
        ("WAVE", 279): "type1.wav",
        ("WAVE", 280): "type2.wav",
        ("WAVE", 281): "type3.wav",
        ("WAVE", 282): "TypeBack.wav",
        ("WAVE", 294): "Intro.wav",
        (RT_GROUP_ICON, 128): "Launcher.ico",
        (RT_GROUP_CURSOR, 308): "WebHand.cur",
    }),
    # CRES.dll is not installed as a file: Update 2 ships it in Gamep3.rez
    # (build/retail.py), whence it is read.
    "Gamep3.rez:CRES.dll": ("ClientRes", {
        (RT_GROUP_CURSOR, 125): "arrow.cur",
    }),
    "TRONSrv.exe": ("ServerApp", {
        (RT_BITMAP, 535): "splash.bmp",
        (RT_GROUP_ICON, 524): "ServerApp.ico",
        (RT_GROUP_ICON, 526): "selected.ico",
    }),
}


def image_bytes(game, image):
    """The image's bytes: a file of the game directory, or, for
    "<archive>:<file>", a file at the root of one of its .rez archives."""
    if ":" not in image:
        path = os.path.join(game, image)
        if not os.path.isfile(path):
            sys.exit(f"appres: {path} not found (the retail game, build/retail.sh)")
        return open(path, "rb").read()
    archive, name = image.split(":", 1)
    hits = [f for f in os.listdir(game) if f.lower() == archive.lower()]
    if not hits:
        sys.exit(f"appres: {os.path.join(game, archive)} not found (the retail game, build/retail.sh)")
    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ext", "tools"))
    from rezextract import Rez
    rez = Rez(os.path.join(game, hits[0]))
    try:
        for path, pos, size in rez.walk():
            if path.lower() == name.lower():
                rez.f.seek(pos)
                return rez.f.read(size)
    finally:
        rez.f.close()
    sys.exit(f"appres: no {name} in {os.path.join(game, hits[0])}")


def resources(d):
    """{(type, name): data} for the first language of every resource of the
    image d (its bytes); a type or name is an int or an upper-case string,
    as the image has it."""
    pe = struct.unpack_from("<I", d, 0x3C)[0]
    nsec = struct.unpack_from("<H", d, pe + 6)[0]
    optsz = struct.unpack_from("<H", d, pe + 20)[0]
    for i in range(nsec):
        s = pe + 24 + optsz + i * 40
        if d[s:s + 8].rstrip(b"\0") == b".rsrc":
            va, rp = struct.unpack_from("<II", d, s + 12)[0], struct.unpack_from("<I", d, s + 20)[0]
            break
    else:
        sys.exit("appres: an image has no resources")

    def entries(off):
        nn, ni = struct.unpack_from("<HH", d, rp + off + 12)
        for i in range(nn + ni):
            name, data = struct.unpack_from("<II", d, rp + off + 16 + i * 8)
            if name & 0x80000000:
                o = rp + (name & 0x7FFFFFFF)
                n = struct.unpack_from("<H", d, o)[0]
                name = d[o + 2:o + 2 + n * 2].decode("utf-16le")
            yield name, data

    out = {}
    for t, td in entries(0):
        for n, nd in entries(td & 0x7FFFFFFF):
            _lang, ld = next(entries(nd & 0x7FFFFFFF))
            drva, size = struct.unpack_from("<II", d, rp + ld)
            out[(t, n)] = d[rp + drva - va:rp + drva - va + size]
    return out


def bitmap_file(dib):
    """A .bmp: the BITMAPFILEHEADER the resource compiler strips again."""
    hsize, _w, _h, _planes, bits, _comp, _isize, _xr, _yr, used = struct.unpack_from("<IiiHHIIiiI", dib)
    colours = used or (1 << bits if bits <= 8 else 0)
    return b"BM" + struct.pack("<IHHI", 14 + len(dib), 0, 0, 14 + hsize + 4 * colours) + dib


def group_file(res, group, kind):
    """An .ico (kind 1) or .cur (kind 2) of a group's images, in the group's
    order. A cursor's image starts with its hot spot, which the file keeps
    in the directory entry instead."""
    _r, _t, n = struct.unpack_from("<HHH", group)
    images, dirents = [], []
    off = 6 + 16 * n
    for i in range(n):
        e = group[6 + 14 * i:6 + 14 * (i + 1)]
        if kind == 1:
            w, h, colours, rsv, planes, bits, size, rid = struct.unpack("<BBBBHHIH", e)
            img = res[(RT_ICON, rid)]
            dirents.append(struct.pack("<BBBBHHII", w, h, colours, rsv, planes, bits, len(img), off))
        else:
            w, h2, planes, bits, size, rid = struct.unpack("<HHHHIH", e)
            raw = res[(RT_CURSOR, rid)]
            hx, hy = struct.unpack_from("<HH", raw)
            img = raw[4:]
            dirents.append(struct.pack("<BBBBHHII", w & 0xFF, (h2 // 2) & 0xFF, 0, 0, hx, hy, len(img), off))
        images.append(img)
        off += len(img)
    return struct.pack("<HHH", 0, kind, n) + b"".join(dirents) + b"".join(images)


def extract(game):
    """{relative path: bytes} of every art file, out of the retail images."""
    out = {}
    for image, (sub, names) in IMAGES.items():
        res = resources(image_bytes(game, image))
        files = {}
        for (t, n), data in res.items():
            if t == RT_BITMAP:
                files[names.get((t, n)) or f"{n}.bmp"] = bitmap_file(data)
            elif (t, n) in names and t == "WAVE":
                files[names[(t, n)]] = data
            elif t in (RT_GROUP_ICON, RT_GROUP_CURSOR):
                files[names[(t, n)]] = group_file(res, data, 1 if t == RT_GROUP_ICON else 2)
        missing = [v for (t, n), v in names.items() if v not in files]
        if missing:
            sys.exit(f"appres: {image} lacks {', '.join(missing)}")
        for name, data in files.items():
            out[f"{sub}/{name}"] = data
    return out


def tree(d):
    """{relative path: bytes} of the files under d."""
    out = {}
    for root, _dirs, names in os.walk(d):
        for n in names:
            p = os.path.join(root, n)
            out[os.path.relpath(p, d).replace(os.sep, "/")] = open(p, "rb").read()
    return out


def put(files, d):
    """Write each file under d, unless it is there already; the count."""
    written = 0
    for rel, data in sorted(files.items()):
        p = os.path.join(d, rel)
        if os.path.isfile(p) and open(p, "rb").read() == data:
            continue
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as f:
            f.write(data)
        written += 1
    return written


def main():
    if len(sys.argv) != 4 or sys.argv[1] not in ("write", "check", "install"):
        sys.exit(__doc__)
    mode, src, dest = sys.argv[1:]
    if mode == "install":
        if not os.path.isdir(src):
            sys.exit(f"appres: {src} not found (the committed retail art, assets/README.md)")
        n = put(tree(src), dest)
        if n:
            print(f"   appres: {n} file(s) of the launcher's, the server's and the client resources' art from {src}")
        return
    want = extract(src)
    if mode == "write":
        have = tree(dest) if os.path.isdir(dest) else {}
        for rel in sorted(set(have) - set(want)):
            os.unlink(os.path.join(dest, rel))
        n = put(want, dest)
        print(f"   appres: {len(want)} art files in {dest} ({n} written)")
        return
    have = tree(dest) if os.path.isdir(dest) else {}
    bad = [f"missing  {r}" for r in sorted(set(want) - set(have))]
    bad += [f"extra    {r}" for r in sorted(set(have) - set(want))]
    bad += [f"changed  {r}" for r in sorted(set(want) & set(have)) if want[r] != have[r]]
    for line in bad:
        print(f"   {line}")
    if bad:
        sys.exit(f"appres: {dest} is not the retail art of {src}: run make art")
    print(f"   appres: all {len(want)} art files in {dest} are the retail art")


if __name__ == "__main__":
    main()
