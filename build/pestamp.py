#!/usr/bin/env python3
"""Give the build's binaries, .pdb and .map files a fixed time.

  build/pestamp.py <epoch> <file>...

VC6's linker writes the time of the link into each binary: the COFF
header's TimeDateStamp, the export and resource directories' TimeDateStamp,
each debug directory entry's TimeDateStamp and, in the CodeView NB10
record of a debug build, the signature of its .pdb (also a time; the .pdb
has the same value in its PDB stream). The .map has a "Timestamp is" line.
Those times are of the machine that built it. build/all.sh (step 4) and
build/variant.sh run this on the game directory's copies, so that every
build of the same source gives the same files, whenever and wherever it
runs. The binaries in src/**/built/, which `make progress` scores, are not
changed. (ext/tools/buildcmp.py blanks the same fields to compare builds.)

For each file, by its content:

  PE image   those fields set to <epoch>, and the optional header's
             CheckSum computed again if it is not 0
  .pdb       (VC6's "program database 2.00") the name of the linker's
             temporary file of the resources (lnkXXXX.tmp, after its
             process id) as lnk0000.tmp; the signature in its PDB
             stream set to <epoch>, if that is the signature that the
             binary beside it (given in the same run) had: a .pdb is
             changed with its binary, and only together do they still
             match
  .map       the "Timestamp is" line given <epoch>

A file that already has <epoch> is not written again.
"""
import os
import re
import struct
import sys
import time

TMPNAME = re.compile(rb"(?i)[a-z]:\\windows\\temp\\lnk([0-9a-f]{4})\.tmp")
PDB2 = b"Microsoft C/C++ program database 2.00\r\n\x1aJG\0\0"


def rva_to_off(sections, rva):
    for va, vsize, raw, rawsize in sections:
        if va <= rva < va + max(vsize, rawsize):
            return raw + (rva - va)
    return None


def put(buf, off, value, changed):
    if 0 <= off and off + 4 <= len(buf) and struct.unpack_from("<I", buf, off)[0] != value:
        struct.pack_into("<I", buf, off, value)
        changed.append(off)


def resource_dirs(buf, base, off, epoch, changed, depth=0):
    if depth > 4 or off + 16 > len(buf):
        return
    put(buf, off + 4, epoch, changed)
    named, ids = struct.unpack_from("<HH", buf, off + 12)
    for i in range(named + ids):
        e = off + 16 + 8 * i
        if e + 8 > len(buf):
            return
        _, target = struct.unpack_from("<II", buf, e)
        if target & 0x80000000:
            resource_dirs(buf, base, base + (target & 0x7FFFFFFF), epoch, changed, depth + 1)


def checksum(buf, at):
    """The PE checksum (imagehlp's CheckSumMappedFile) of buf, whose
    CheckSum field is at `at`."""
    data = bytes(buf[:at]) + b"\0\0\0\0" + bytes(buf[at + 4:])
    if len(data) % 2:
        data += b"\0"
    s = 0
    for (w,) in struct.iter_unpack("<H", data):
        s += w
        s = (s & 0xFFFF) + (s >> 16)
    s = (s & 0xFFFF) + (s >> 16)
    return (s + len(buf)) & 0xFFFFFFFF


def stamp_pe(buf, epoch):
    """Returns (the old NB10 signature or None, offsets changed)."""
    changed, old_sig = [], None
    pe = struct.unpack_from("<I", buf, 0x3C)[0]
    if buf[pe:pe + 4] != b"PE\0\0":
        return None, changed
    coff = pe + 4
    nsec, = struct.unpack_from("<H", buf, coff + 2)
    put(buf, coff + 4, epoch, changed)
    optsize, = struct.unpack_from("<H", buf, coff + 16)
    opt = coff + 20
    ndirs, = struct.unpack_from("<I", buf, opt + 92)
    dirs = [struct.unpack_from("<II", buf, opt + 96 + 8 * i) for i in range(min(ndirs, 16))]
    sections = []
    for i in range(nsec):
        s = opt + optsize + 40 * i
        vsize, va, rawsize, raw = struct.unpack_from("<IIII", buf, s + 8)
        sections.append((va, vsize, raw, rawsize))
    if len(dirs) > 0 and dirs[0][0]:
        o = rva_to_off(sections, dirs[0][0])
        if o is not None:
            put(buf, o + 4, epoch, changed)
    if len(dirs) > 2 and dirs[2][0]:
        o = rva_to_off(sections, dirs[2][0])
        if o is not None:
            resource_dirs(buf, o, o, epoch, changed)
    if len(dirs) > 6 and dirs[6][0]:
        o = rva_to_off(sections, dirs[6][0])
        if o is not None:
            for i in range(dirs[6][1] // 28):
                e = o + 28 * i
                put(buf, e + 4, epoch, changed)
                _size, _rva, ptr = struct.unpack_from("<III", buf, e + 16)
                if buf[ptr:ptr + 4] == b"NB10":
                    old_sig = struct.unpack_from("<I", buf, ptr + 8)[0]
                    put(buf, ptr + 8, epoch, changed)
    if changed:
        at = opt + 64
        if struct.unpack_from("<I", buf, at)[0]:
            struct.pack_into("<I", buf, at, checksum(buf, at))
    return old_sig, changed


def pdb_stream1(buf):
    """The file offset of the PDB stream's signature in a VC6 .pdb, or None."""
    if not buf.startswith(PDB2):
        return None
    page, = struct.unpack_from("<I", buf, len(PDB2))
    _fpm, _npages = struct.unpack_from("<HH", buf, len(PDB2) + 4)
    root_size, = struct.unpack_from("<I", buf, len(PDB2) + 8)
    npages = (root_size + page - 1) // page
    pages = struct.unpack_from(f"<{npages}H", buf, len(PDB2) + 16)
    root = b"".join(buf[p * page:(p + 1) * page] for p in pages)[:root_size]
    nstreams, = struct.unpack_from("<H", root, 0)
    sizes = [struct.unpack_from("<I", root, 4 + 8 * i)[0] for i in range(nstreams)]
    at = 4 + 8 * nstreams
    for i, size in enumerate(sizes):
        n = 0 if size == 0xFFFFFFFF else (size + page - 1) // page
        if i == 1:
            if n == 0:
                return None
            first, = struct.unpack_from("<H", root, at)
            return first * page + 4         # (version, signature, age)
        at += 2 * n
    return None


def binary_sig(pdb):
    """(NB10 signature, age) of the binary beside a .pdb with its name."""
    stem = os.path.splitext(os.path.basename(pdb))[0].lower()
    d = os.path.dirname(os.path.abspath(pdb))
    for n in os.listdir(d):
        if os.path.splitext(n)[0].lower() != stem or n.lower().endswith((".pdb", ".map")):
            continue
        b = open(os.path.join(d, n), "rb").read()
        i = b.find(b"NB10")
        while i >= 0:
            name = b[i + 16:b.find(b"\0", i + 16)]
            if os.path.basename(name.replace(b"\\", b"/")).lower() == os.path.basename(pdb).lower().encode():
                return struct.unpack_from("<II", b, i + 8)
            i = b.find(b"NB10", i + 4)
    return None


def stamp(path, epoch, sigs):
    with open(path, "rb") as f:
        buf = bytearray(f.read())
    what = None
    if buf[:2] == b"MZ" and len(buf) > 0x40:
        old, changed = stamp_pe(buf, epoch)
        if old is not None:
            sigs[os.path.splitext(os.path.abspath(path))[0].lower()] = old
        what = changed
    elif buf.startswith(PDB2):
        at = pdb_stream1(buf)
        if at is None:
            sys.exit(f"pestamp: {path}: no PDB stream")
        sig, age = struct.unpack_from("<II", buf, at)
        stem = os.path.splitext(os.path.abspath(path))[0].lower()
        mine = binary_sig(path)
        if mine is None:
            sys.exit(f"pestamp: {path}: no binary beside it with its name")
        # the binary's signature as this run found it (it is stamped in the
        # same run), or both stamped already
        ok = sig == sigs.get(stem) or (sig == epoch and mine[0] == epoch)
        if mine[1] != age or not ok:
            sys.exit(f"pestamp: {path}: its signature is not its binary's "
                     "(stamp a .pdb in the same run as its binary)")
        what = []
        put(buf, at, epoch, what)
        # the name of the linker's temporary object of the resources
        # (C:\windows\temp\lnkXXXX.tmp: its process id) as a fixed one
        # of the same length
        for m in TMPNAME.finditer(bytes(buf)):
            if m.group(1) != b"0000":
                buf[m.start(1):m.end(1)] = b"0000"
                what.append(m.start(1))
    elif path.lower().endswith(".map"):
        t = time.strftime("%a %b %d %H:%M:%S %Y", time.gmtime(epoch))
        new = re.sub(rb"(?m)^( *Timestamp is )[0-9a-f]{8} \([^)\r\n]*\)",
                     lambda m: m.group(1) + b"%08x (%s)" % (epoch, t.encode()), bytes(buf))
        what = [0] if new != bytes(buf) else []
        buf = bytearray(new)
    if what:
        with open(path + ".part", "wb") as f:
            f.write(buf)
        os.replace(path + ".part", path)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    epoch = int(sys.argv[1])
    sigs = {}
    # binaries first: a .pdb takes its binary's old signature
    paths = sorted(sys.argv[2:], key=lambda p: (p.lower().endswith((".pdb", ".map")), p))
    for p in paths:
        stamp(p, epoch, sigs)


if __name__ == "__main__":
    main()
