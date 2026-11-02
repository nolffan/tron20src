"""What the scoreboard scores: Tron 2.0's binaries, and where each one's own
code, and ours, is.

tools/progress.py and tools/ratchet.py read this file and nothing else
about the project, so they know no binary by name: a binary is added here,
not in the tools. Plain data, with the evidence for every address beside
it (the CSVs in this directory are the rest of the evidence).

TARGETS, one dict per shipped binary, in the README's order:

  label      the binary's name, as the README and functions.csv show it
  what       what it is, for the README
  kind       "image": scored function by function against the shipped
             image's code; "resources": a resource-only image, scored
             resource by resource and left out of the total
  shipped    the retail binary, extracted by build/reference.sh
  stem       the prefix of its files here (<stem>-screen-exclude.csv, ...)
             and in out/reference/ (<stem>-functions.csv)
  built      our build of it: a path or glob under the repository, or None
             while it has no source. Its link is read from the response
             file beside it (<built>.rsp, and <built>.cmd for the
             directory the link ran in): every object and library of ours
             it names is scored, and its .map places our functions. A
             target without a build scores as all unwritten
  optional   True: a missing build is expected (no source yet, or another
             project's output still to come), not an error
  app        [(lo, hi)]: the image's own code -- everything the scoreboard
             counts. Above it is the C runtime (the static parts of
             msvcrt.lib, the import thunks) and the exception-handling
             funclets of .text$x: status `crt`, listed, not counted. A
             range after the first is code of the image's own that the
             linker placed above the runtime
  middleware [(lo, hi)]: third-party code linked inside `app`: status
             `middleware`, listed, not counted
  encrypted  [(lo, hi)]: code the retail image keeps encrypted (SecuROM):
             status `encrypted`, listed, not counted -- nothing can be
             compared there
  third_party  link inputs of ours (file names) that build `middleware`:
             left out of our side, so they neither match nor count as
             unplaced
  cites      True when src/ cites this image's addresses in comments: a
             cited address of an unmatched function is `attempted`
  cited_src  the source trees whose comments cite it (default CITED_SRC):
             per target, because the DLLs share the 0x10000000 range
  purecall   the shipped address of the CRT's `_purecall` (every abstract
             vtable's filler), where tools/vtabcheck.py cannot find its
             `jmp [IAT]` thunk itself

The reconstruction tools (tools/tgt.py) read the same entries: a tool is
given a target by label or stem (`Lithtech`, `server`), and its image,
build, link inputs and regions come from here.

Optional per target, for what NOLF's scoreboard needs and this one does
not yet: icl (Intel C/C++ padding and /Qip names), linked_oracle (compare
relocation-free functions against our linked image too), lib_range,
lib_block, own_names / own_symbols (names that say a function's shipped
address) -- see tools/progress.py.

How the ranges were found: `tools/libscan.py <image> <lib>...` puts each
library's functions in the image (here msvc-6.00-8168's msvcrt.lib,
msvcprt.lib, msvcirt.lib, libcmt.lib and libcpmt.lib, and MFC's
mfcs42.lib). The image's own code ends where the runtime's first member
starts: as in NOLF2's binaries, the same compiler generation, that is an
import-thunk run (`jmp [IAT]`, 6 bytes each) or msvcprt's `operator
delete` thunk (delop_s.obj: `push [esp+4]; call; pop ecx; ret`, packed
without padding), then MSVCIRT's thunks, then msvcrt.lib's own objects.
The image's own /O2 code ends padded to 16 bytes; the runtime's members
are packed. Until a binary has a link of ours, its boundary is found that
way: marked "provisional" below, to be pinned from our .map (which names
the library of every function) once there is one, as nolf2src did. All of
Tron 2.0 links the C runtime as a DLL (MSVCRT.dll, with MSVCIRT.dll and
MFC42.DLL where used), except ClientFx.fxd and the resource DLLs, which
link libcmt.lib statically.
"""

REFERENCE = "out/reference"     # build/reference.sh writes it
OUT = "out/progress"            # each run's functions.csv and summary.json
DATA = "progress"               # the recorded baseline and the evidence

# Every hex address cited in these source trees marks the address
# `attempted` in a target with `cites` (an address a reconstruction has
# been written for), unless the target names its own `cited_src`.
CITED_SRC = []

# No prebuilt static library ships with Tron 2.0: every library is built
# from source and scored inside the binaries that link it.
LIBS = []
SHIPPED_LIB = None
SHIPPED_AS = {}
SHIPPED_ENV = None

DRIVER_OBJS = set()

TARGETS = [
    # ---- the engine ------------------------------------------------------
    dict(label="Lithtech.exe", what="the engine", kind="image",
         shipped=REFERENCE + "/Lithtech.exe", stem="Lithtech",
         built="src/Engine/built/Final/Lithtech/Lithtech.exe", optional=True,
         cites=True, cited_src=["src/Engine/runtime"],
         # SecuROM-wrapped, as NOLF2's and Contract J.A.C.K.'s are: the
         # .ifkui and .khpb sections, nearly every import call redirected
         # (libscan.py finds one msvcrt.lib member, ullshr.obj's __aullshr
         # at 0x581e80), and .text 0x572000..0x577000 encrypted on disc
         # (five 4 KB pages of 7.2 to 7.9 bits of entropy a byte; no other
         # page of .text is above 7.0). Ordinary code resumes at 0x577000
         # and runs, padded and aligned, to 0x581e80; from there the
         # runtime, and the .text$x funclets (655 `mov eax, imm32; jmp`
         # in 0x581e80..0x58b000 against 14 below it). Provisional: the
         # boundary is that one hit.
         app=[(0x00401000, 0x00581e80)],
         encrypted=[(0x00572000, 0x00577000)]),
         # D3DX, statically linked from a DirectX 9 SDK's d3dx9.lib (the
         # Rich header's 110 C++ and 3 C objects of cl 13.00.9178, 6 of
         # masm 6.14.8444, and 9 C objects of cl 13.10 build 2067; the
         # engine imports d3d9.dll), is somewhere inside `app` until that
         # SDK is in toolchains/ and libscan.py can place it: then its
         # block becomes `middleware`, with third_party=["d3dx9.lib"].
    dict(label="server.dll", what="the engine's server", kind="image",
         shipped=REFERENCE + "/server.dll", stem="server",
         built="src/Engine/built/Final/server/server.dll", optional=True,
         cites=True, cited_src=["src/Engine/runtime"],
         # provisional: two import thunks at 0x1004f750, then msvcrt.lib
         # from atonexit.obj's __onexit (0x1004f75c)
         app=[(0x10001000, 0x1004f750)]),
    dict(label="SndDrv.dll", what="the sound driver", kind="image",
         shipped=REFERENCE + "/SndDrv.dll", stem="SndDrv",
         built="src/Engine/built/Final/snddrv/SndDrv.dll", optional=True,
         # provisional: msvcrt.lib from ulldiv.obj's __aulldiv (0x1000b040)
         # to crtdll.obj's _DllMainCRTStartup; the two padded wrappers
         # before it (0x1000b020, 0x1000b030) are compiled code of its own
         app=[(0x10001000, 0x1000b040)]),
    dict(label="ltmsg.dll", what="the engine's messages", kind="resources",
         shipped=REFERENCE + "/ltmsg.dll", stem="ltmsg",
         built="src/Engine/built/Final/ltmsg/ltmsg.dll", optional=True),
    # ---- the launcher and the dedicated server ------------------------
    dict(label="Tron.exe", what="the launcher", kind="image",
         shipped=REFERENCE + "/Tron.exe", stem="Tron",
         built="src/Game/built/Final/Launcher/Tron.exe", optional=True,
         # provisional: MFC42.DLL's import thunks from 0x40cb30, then
         # msvcrt.lib from __onexit (0x40cf2e) to crtexew.obj's
         # WinMainCRTStartup and mfcs42.lib's AfxInitialize (0x40d128).
         # Not linked /incremental: .text opens with code, not a jump table
         app=[(0x00401000, 0x0040cb30)]),
    dict(label="TRONSrv.exe", what="the dedicated server", kind="image",
         shipped=REFERENCE + "/TRONSrv.exe", stem="TRONSrv",
         built="src/Game/built/Final/ServerApp/TRONSrv.exe", optional=True,
         # provisional: a run of 185 import thunks from 0x416240, then
         # msvcrt.lib from __onexit (0x416696) and mfcs42.lib's
         # AfxInitialize (0x416928)
         app=[(0x00401000, 0x00416240)]),
    # ---- the game ------------------------------------------------------
    dict(label="CShell.dll", what="the game: client", kind="image",
         shipped=REFERENCE + "/CShell.dll", stem="CShell",
         built="src/Game/built/Final/ClientShellDLL/TRON/CShell.dll", optional=True,
         # provisional: msvcprt.lib's delete thunk at 0x101d0890, then
         # MSVCIRT.dll's and msvcprt's thunks, and msvcrt.lib from
         # ehvecdtr.obj (0x101d0908) to dllmain.obj
         app=[(0x10001000, 0x101d0890)]),
    dict(label="Object.lto", what="the game: server objects", kind="image",
         shipped=REFERENCE + "/Object.lto", stem="Object",
         built="src/Game/built/Final/ObjectDll/TRON/Object.lto", optional=True,
         # provisional: msvcprt.lib's delete thunk at 0x10277c80, then the
         # thunks, and msvcrt.lib from chkstk.obj (0x10277d00) to
         # dllmain.obj
         app=[(0x10001000, 0x10277c80)]),
    dict(label="ClientFx.fxd", what="the game: client effects", kind="image",
         shipped=REFERENCE + "/ClientFx.fxd", stem="ClientFx",
         built="src/Game/built/Final/ClientFxDll/ClientFx.fxd", optional=True,
         # links libcmt.lib statically; provisional: libcpmt.lib's delete
         # thunk at 0x10023080, then libcmt.lib from atox.obj's _atol
         # (0x100230e9; 229 of its members placed)
         app=[(0x10001000, 0x10023080)]),
    dict(label="CRes.dll", what="the game's client resources",
         kind="resources", shipped=REFERENCE + "/CRes.dll", stem="CRes",
         built="src/Game/built/Final/ClientRes/TRON/CRes.dll", optional=True),
    dict(label="SRes.dll", what="the game's server resources",
         kind="resources", shipped=REFERENCE + "/SRes.dll", stem="SRes",
         built="src/Game/built/Final/ServerRes/TRON/SRes.dll", optional=True),
]
