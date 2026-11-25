# tron20src

*Tron 2.0* (Monolith, 2003), built from source.

> [!WARNING]
> **Work in progress.** The reconstruction is far from complete: much of the
> game code does not match retail yet (see [`progress/README.md`](progress/README.md)).
> The legacy build runs under Wine to the main menu, through the first
> single-player level, saves and loads, and a light-cycle race (`make
> tests`), but it is not tested further. It is not ready to play.

Monolith never released Tron 2.0's source as it shipped, as it did for *No
One Lives Forever 2*. This project reconstructs it: the game code from
NOLF2's source release (Tron 2.0 was built from the same TO2 tree) and the
Tron game code merged into the 2006 LithTech Jupiter distribution
(`Game/*/TRON`), and the engine from nolf2src's reconstruction of NOLF2's
engine and the Jupiter engine source, each brought to what the retail
binaries are, function by function. As for
[nolfsrc](https://github.com/nolffan/nolfsrc),
[nolf2src](https://github.com/nolffan/nolf2src) and
[cjacksrc](https://github.com/nolffan/cjacksrc), the aim is two builds:

- **legacy**: Tron 2.0 (USA) at its last update, Update 2 (v1.030), rebuilt
  so that it compiles back to the retail binaries byte for byte, as far as
  that can be done -- with the compiler Monolith used, Visual C++ 6.0
  (12.00.8168), under Wine;
- **modern**: the same source with fixes and new features (`TRON20SRC_MODERN`).

Compiling back to the retail bytes means compiling the retail game's bugs as
well; their fixes go behind `TRON20SRC_MODERN`, into the modern build only, so
the legacy build keeps matching the retail one.

## Progress

How much of each binary compiles to exactly the bytes of the retail game,
function by function: the table, and how the scoring works and what it
leaves out, are in [`progress/README.md`](progress/README.md). Which
compiler built each binary, and the evidence, is `progress/COMPILERS.md`.

## Building

Needs docker and the submodules (`git submodule update --init`):
`ext/toolchains/` (the compilers) and `ext/tools/` (the reconstruction
tools), and for `make modern`, `ext/ffmpeg` (FFmpeg n8.1.3, which the
modern build's Bink player uses; `build/ext.sh`, `docs/LICENSE_NOTES.md`).

### What you need

Bring your own discs. The build takes the retail game's binaries and data
from your own copy of Tron 2.0 (USA) and its Update 2 (v1.030). Put these
three files in `isos/` at the root of the repository (a hard link is good:
`ln <file> isos/`), or set `TRON20SRC_ISOS=<directory>`:

| What | Redump name of the image | Recognised by |
|---|---|---|
| disc 1 | `Tron 2.0 (USA) (Disc 1)` | volume label `TRON`, 333178 blocks |
| disc 2 | `Tron 2.0 (USA) (Disc 2)` | volume label `TRON`, 139556 blocks |
| Update 2, v1.030, the "complete" update | `tron-update_v1x2_complete.exe` | 32479660 bytes, its SHA-256 |

A disc image can be a `.iso`, a `.bin` with its `.cue`, or a `.zip` that
holds one of them. The update can be the `.exe` or a `.zip` that holds it.
`make retail` (every build does it first) installs the game from these
files once, into `out/retail/tron2-usa-1.030/` (`build/discs.py`), and
checks each file against `build/retail-files.txt`. This list holds only
the path, size, SHA-256 and place of each file, and no game data. Any
correct copy of the discs works. Without the files, `make` stops and tells
you what to supply.

```sh
make            # the game, from our binaries and the retail data, in out/tron20-legacy-release/
make debug      # the same with symbols (.pdb, .map), in out/tron20-legacy-debug/
make modern     # the modern build (TRON20SRC_MODERN, docs/MODERN.md), in out/tron20-modern-release/
make modern-debug  # both, in out/tron20-modern-debug/
make test       # run it under Xvfb and Wine to the main menu (out/play/test/shot.png)
make tests      # the scripted tests: menus, a level, saving and loading, a race
make launchertest  # the launcher, Tron.exe: its dialogs, then Play to the main menu
make portabletest  # the modern build: no registry or APPDATA use, the launcher waits for the game
make mp3acmtest # the modern SndDrv.dll's MP3 codec on every MP3 file of Sound.rez
make mptest     # multiplayer: the dedicated server and two clients (some minutes)
make sweep      # every single-player world for a minute each (slow); out/play/sweep-summary.txt
make reference  # the retail binaries and their function tables, in out/reference/
make progress   # score the build against them
make help       # everything else
```

`make` (`build/all.sh`) puts every binary that we build into the game
directory: the engine (`Lithtech.exe`, `server.dll`, `SndDrv.dll`,
`ltmsg.dll`), the launcher (`Tron.exe`), the dedicated server
(`TRONSrv.exe`), and the game DLLs (`CShell.dll`, `Object.lto`,
`ClientFx.fxd`, `CRes.dll`, `SRes.dll`) in `custom/`, last in the rez stack
(`tron20cmds.txt`). The data is the retail game's: hard links to its
`.rez` archives and movies, and copies of the three archives that hold the
retail game DLLs, with those DLLs set to zeros and their names changed
(`build/rezhide.py`), so that the retail launcher's rez order also gives
the engine our DLLs. The only retail binaries
are the third-party runtime that we have no source for: Bink
(`binkw32.dll`) and the Microsoft C, C++ and MFC runtime. The modern build
has our own `binkw32.dll` (a Bink player on FFmpeg) in place of RAD's, and
its `SndDrv.dll` decodes MP3 itself (`docs/MODERN.md`, X-1 and X-2). The last step
lists every executable file with its origin, and stops the build if one is
not ours or third-party. Start the game with
`wine Lithtech.exe -cmdfile tron20cmds.txt` in that directory, or with the
launcher, `Tron.exe`, which reads the registry values of the game's setup
(`HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0`: `InstallDir`,
`UpdateNum` and `UpdateCommandLine0/1`, as `build/launchertest.sh` writes
them) and wants disc 1 in a drive, as retail's does. The modern build's
launcher (`TronLauncher.exe`, and `Tron.exe`, which starts the game at
once) needs neither: the game directory is the install, and the launcher,
the game and the dedicated server keep their settings in `tron20.ini`
beside the executables (`docs/MODERN.md` APPS-3).

`make mptest` (`build/mptest.sh`) runs a multiplayer game on the loopback:
`MPTEST_MODE=dedicated` (the default: `TRONSrv.exe` hosts, two clients
join) or `listen` (client 1 hosts from the multiplayer menu, client 2
joins), and `MPTEST_TYPE=arena` (Disc Arena, the default), `tourn` (Disc
Tournament) or `lc` (Light Cycles). Each program has its own Xvfb display
and Wine prefix. The results are in `out/play/<game>-mp-<mode>-<type>/`.
It is slow, so `make tests` does not run it.

### Without the retail game: the CI build

`make NODISCS=1 compile` compiles and links every binary without the
retail game. The binaries are the same as with it: the only retail data
that they contain is the art of the launcher, the dedicated server and
`CRes.dll`, which is committed in `assets/art/` (`assets/README.md`).
`make progress-ci` then checks the build against `progress/hashes/`
without the retail binaries (`progress/README.md`). `make ci` does both;
the CI (`.github/workflows/ci.yml`) runs them on every push.

The retail game's bugs, as they are found, are listed in
[`docs/RETAIL_BUGS.md`](docs/RETAIL_BUGS.md); what the modern build changes
is [`docs/MODERN.md`](docs/MODERN.md).
