#!/usr/bin/env python3
"""Where each retail file's bytes are in your two disc images.

  build/retailways.py write [file ...]   write build/retail-ways.txt
  build/retailways.py check [file ...]   check build/retail-ways.txt

build/discs.py installs the retail game with unshield: it copies the
InstallShield cabinets out of the disc images (and the update, for the
reference of the checks), and unpacks them. The install scripts of a
public archive (build/mkinstall.py) cannot use unshield (it is not on
Windows, nor on every Linux). They read the bytes of each file directly
from the images. This script finds those bytes, from the InstallShield 6
header (data1.hdr) of the discs, and writes them into
build/retail-ways.txt. That list holds no game data: only offsets, sizes
and SHA-256 values.

The files are those of build/retail-files.txt that are on the discs (the
install scripts need only the discs: Update 2's data ships with the
release, assets/README.md; its binaries are built here), and the images
are found as build/discs.py finds them (the arguments, or
$TRON20SRC_ISOS, or isos/). `write` reads each file from its ranges, and
writes the list only if every file has the size and SHA-256 that
build/retail-files.txt gives. `check` does the same reads and compares the
result with the committed list.

The list's lines (the fields are separated by one TAB):

  W <path> copy|is <id>:<offset>:<length> ...
      One retail file (its path in build/retail-files.txt), and how to read
      it: those byte ranges, joined, are the file (copy), or the
      InstallShield 6 cabinet data that holds it (is: a run of chunks, each
      a 16-bit length and a raw deflate stream). A file split across two
      cabinets has one range in each. <id> is a disc ("1", "2": the offset
      is in its 2048-byte-sector image, the .iso).
"""
import hashlib
import os
import struct
import sys
import zlib

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "build"))
import discs  # noqa: E402

WAYS = os.path.join(REPO, "build", "retail-ways.txt")
HEADER = """\
## Where the bytes of each retail file are, in your own two disc images
## (build/retailways.py writes and checks this list). The install scripts
## of a public archive read the game's data from these byte ranges
## (build/mkinstall.py). They need no update: Update 2's data ships with
## the release (assets/README.md). This list holds no game data: only
## offsets, sizes and SHA-256 values.
##
## W <path> copy|is <id>:<offset>:<length> ...
##   A retail file of the discs (build/retail-files.txt), and the byte
##   ranges that are the file (copy), or that hold its InstallShield 6
##   cabinet data (is: a run of chunks, each a 16-bit length and a raw
##   deflate stream). <id> is a disc (its .iso image).
## The fields are separated by one TAB.
"""


class Cabinets:
    """An InstallShield 6 cabinet set: data1.hdr, and where each dataN.cab
    is (its image id and offset there)."""
    def __init__(self, hdr, volumes, read):
        self.volumes, self.read = volumes, read
        sig, ver, _vi, cdo, _cds = struct.unpack_from("<5I", hdr, 0)
        if sig != 0x28635349 or ver >> 24 != 1 or (ver >> 12) & 0xF != 6:
            sys.exit(f"retailways: not an InstallShield 6 header (version {ver:#x})")
        fto, = struct.unpack_from("<I", hdr, cdo + 0x0C)
        ndirs, = struct.unpack_from("<I", hdr, cdo + 0x1C)
        nfiles, fto2 = struct.unpack_from("<II", hdr, cdo + 0x28)
        ft = cdo + fto
        cstr = lambda o: hdr[o:hdr.index(b"\0", o)].decode("latin-1")
        dirs = [cstr(ft + struct.unpack_from("<I", hdr, ft + 4 * i)[0]) for i in range(ndirs)]
        self.files = []
        for i in range(nfiles):
            p = ft + fto2 + i * 0x57
            flags, = struct.unpack_from("<H", hdr, p)
            usize, csize, off = struct.unpack_from("<3Q", hdr, p + 2)
            name_off, d = struct.unpack_from("<IH", hdr, p + 58)
            volume, = struct.unpack_from("<H", hdr, p + 85)
            path = (dirs[d] + "/" if dirs[d] else "") + cstr(ft + name_off)
            self.files.append((i, path.replace("\\", "/"), flags, usize, csize, off, volume))
        self.vh = {}
        for v, (ident, at) in volumes.items():
            h = read(ident, at, 84)
            self.vh[v] = struct.unpack_from("<16I", h, 20)

    def ranges(self, f):
        """[(image id, offset, length)] of file f's stored data."""
        i, _path, flags, usize, csize, off, v = f
        if flags & 2:
            sys.exit(f"retailways: {f[1]} is obfuscated: not known")
        if not flags & 1:                  # in one volume
            ident, at = self.volumes[v]
            return [(ident, at + off, csize)]
        out, left = [], csize
        while left:
            h = self.vh[v]
            ident, at = self.volumes[v]
            if h[3] == i and h[14]:        # the last file of this volume
                o, n = h[10], h[14]
            elif h[2] == i:                # the first file of this volume
                o, n = h[4], h[8]
            else:
                sys.exit(f"retailways: {f[1]}: split, but not at volume {v}'s ends")
            out.append((ident, at + o, n))
            left -= n
            v += 1
        return out


def inflate(img, ranges):
    buf = b""
    gen = (b for ident, off, n in ranges for b in img[ident].chunks(off, n))
    for b in gen:
        buf += b
        while len(buf) >= 2:
            n, = struct.unpack_from("<H", buf)
            if len(buf) < 2 + n:
                break
            yield zlib.decompress(buf[2:2 + n], -15)
            buf = buf[2 + n:]
    if buf:
        sys.exit("retailways: compressed data ends inside a chunk")


def digest(img, kind, ranges):
    h, size = hashlib.sha256(), 0
    gen = inflate(img, ranges) if kind == "is" else \
        (b for ident, off, n in ranges for b in img[ident].chunks(off, n))
    for b in gen:
        h.update(b)
        size += len(b)
    return size, h.hexdigest()


def build(args):
    dlist, update, cabs, files = discs.load()
    found, _exe = discs.identify(dlist, update, discs.candidates(args), want_update=False)
    if len(found) < len(dlist):
        print(discs.missing_message(dlist, update, found, None, want_update=False),
              file=sys.stderr)
        sys.exit(3)
    img = {str(n): i for n, i in found.items()}
    lines = []

    def read(ident, off, n):
        return img[ident].read(off, n)

    bycab = {c.name.lower(): c for c in cabs}
    hdr = bycab["data1.hdr"]
    disc_set = Cabinets(read(str(hdr.disc), hdr.off, hdr.size),
                        {int(c.name[4]): (str(c.disc), c.off) for c in cabs
                         if c.name.lower().endswith(".cab")}, read)

    for f in sorted(files, key=lambda f: f.path.lower()):
        if f.source == "update":
            continue               # the update's: not installed from the discs
        if f.disc:
            ways = [("copy", [(str(f.disc), f.off, f.size)])]
        else:
            name = f.gpath.lower()
            ways = []
            for c in disc_set.files:
                if c[1].lower() == name and c[3] == f.size and not c[2] & 8:
                    ways.append(("is" if c[2] & 4 else "copy", disc_set.ranges(c)))
        for kind, ranges in ways:
            if digest(img, kind, ranges) == (f.size, f.sha):
                lines.append(f"W\t{f.path}\t{kind}\t" +
                             " ".join(f"{i}:{o}:{n}" for i, o, n in ranges))
                print(f"   {f.path}: {kind}, {len(ranges)} range(s)")
                break
        else:
            sys.exit(f"retailways: {f.path}: no bytes of the images make it")
    return HEADER + "".join(line + "\n" for line in lines)


def main():
    a = sys.argv[1:]
    if a[:1] == ["write"]:
        text = build(a[1:])
        with open(WAYS, "w", newline="\n") as out:
            out.write(text)
        print(f"   wrote {os.path.relpath(WAYS, REPO)}")
        return 0
    if a[:1] == ["check"]:
        text = build(a[1:])
        if open(WAYS).read() != text:
            print("error: build/retail-ways.txt is not what the images give "
                  "(build/retailways.py write)", file=sys.stderr)
            return 1
        print("   build/retail-ways.txt agrees with the images")
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
