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
make tests      # the scripted tests: menus, a level, saving and loading, a quick load with the weapon chooser open, a race
make launchertest  # the launcher, Tron.exe: its dialogs, then Play to the main menu
make portabletest  # the modern build: no registry or APPDATA use, the launcher waits for the game
make mp3acmtest # the modern SndDrv.dll's MP3 codec on every MP3 file of Sound.rez
make mptest     # multiplayer: the dedicated server and two clients (some minutes)
make scripttest SCRIPTTEST_GAME=<dir>  # linux-run.sh and linux-server.sh in an installed public archive
make sweep      # every single-player world for a minute each (slow); out/play/sweep-summary.txt
make package    # the archives of a game directory again (PACKAGE_GAME=<dir>): below
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
not ours or third-party. On Linux, start the game with `./linux-run.sh` in
that directory (below: "Playing on Linux"). Or start it with
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
With the modern build, `MPTEST_MASTER=local` runs a local stand-in for
the master server (`build/gamespy.py`), and client 2 joins through the
Internet server list (`docs/MODERN.md` GS-1 to GS-4).
It is slow, so `make tests` does not run it.

### The archives: private and public

Each build (`make`, `make debug`, `make modern`, `make modern-debug`)
packages its game directory last (`build/package.sh`; `make package
PACKAGE_GAME=<dir>` does it again):

| Archive | What it holds | For |
|---|---|---|
| `out/<name>-private.tar.gz` | the whole game directory, with the retail game's data | your own machines only. Do not give it to other persons |
| `out/<name>-public.tar.gz` | only what this project builds (and the `.pdb` and `.map` files of a debug build), `LICENSE_NOTES.md` and `licenses/` (FFmpeg's LGPL-2.1, minimp3's CC0, the GameSpy SDK's licence), `MANIFEST.txt`, `SHA256SUMS`, the run scripts `linux-run.sh` and `linux-server.sh`, and the install scripts `linux-install.sh`, `windows-install.bat` and `mac-install.command` | other persons |

Both archives have the run scripts (`build/game-scripts/`, below). The
install scripts copy the game's data and the third-party runtime
(Bink, the Microsoft C, C++ and MFC runtime) out of the player's own
images of the two discs and the executable of Update 2. They read only
byte ranges of those files, which three committed lists give
(`build/retail-files.txt`; `build/retail-ways.txt`, from
`build/retailways.py`; `build/retail-ranges.txt`). They do not run
anything from the discs. They check the SHA-256 of each file that they
write. Then the directory is the game directory as it was built, byte for
byte. The scripts need:

- Linux: bash, python3 and GNU coreutils;
- Windows: Windows PowerShell 5.1 (Windows 10 and 11 have it);
- macOS: the Perl that macOS has (`/usr/bin/perl`), for CrossOver or
  another Wine.

To install a public archive:

```sh
mkdir tron20 && tar -C tron20 --strip-components=1 -xzf out/tron20-modern-release-public.tar.gz
cd tron20 && ./linux-install.sh <disc 1 image> <disc 2 image> <tron-update_v1x2_complete.exe>
sha256sum -c SHA256SUMS
```

(Without arguments, the script looks for the files in its directory and
the directory above it.) On Windows, drop the three files onto
`windows-install.bat`. The legacy build's script also writes the registry
values that the retail launcher reads. On a Mac, double-click
`mac-install.command` with the three files in its folder (or give them to
it in Terminal). Then play the game with CrossOver or another Wine: the
modern build's `Tron.exe`, or, for the legacy build, `Lithtech.exe
-cmdfile tron20cmds.txt` (its `Tron.exe` is the retail launcher, which
wants the retail setup's registry values and disc 1 in a drive).
`mac-install.command` is tested with Perl on Linux (the same files as
`linux-install.sh`), but not yet on a Mac.

### Playing on Linux

The run scripts are in the game directory and in both archives
(`build/game-scripts/`; `build/all.sh` puts them there). Each prints its
usage with `--help`. They need Wine (with 32-bit support).

- `./linux-run.sh [arguments]` starts the game through `Tron.exe`. In the
  modern build, `Tron.exe` starts the game at once and stays until the
  game stops (Steam's overlay and Remote Play need that:
  `docs/MODERN.md` APPS-1, APPS-2); `./linux-run.sh --launcher` opens
  `TronLauncher.exe`. In the legacy build, `Tron.exe` is the retail
  launcher: click Play. The first run asks for the detail setting. The
  arguments go to the engine, for example `++windowed 0` (full screen),
  `+DisableSound 1`, `+join <address>:27888 +gametype DiscArena`.
- `./linux-server.sh [--config <name>] [--headless] [arguments]` runs the
  dedicated server, `TRONSrv.exe`, on a server configuration
  (`ServerData/<name>.txt`, which the game's Host screens write). The
  default is `ServerData/server.txt`, which it writes if it is not there
  (Disc Arena, five worlds, port 27888; LAN only in the legacy build, an
  Internet game on OpenSpy in the modern build). With no display it runs under
  `xvfb-run`. Its console is in `tronsrv.log`.

The scripts use their own Wine prefix, `~/.wine-tron20` (headless
server: `~/.wine-tron20-server`; `WINEPREFIX` changes it), so your own
prefix is left alone. The modern build needs nothing in the prefix: its
settings are in `tron20.ini` in the game directory. For the legacy build,
the scripts write the registry values of the retail setup and Update 2,
and `linux-run.sh` also gives the retail launcher a CD-ROM drive with an
empty `TRON.EXE` (the launcher only looks for that file; the data is in
the game directory). On a fresh install of the modern build (no
`display.cfg`), `linux-run.sh` starts the game in a borderless window at
the desktop's resolution, and on a Steam Deck (`SteamDeck=1`) full screen
at 1280x800; later runs keep what Options > Display sets. The legacy
build starts in a window (`++windowed 1`; its Display has no window
mode). Under Wine, the music (DirectMusic) is silent for now
(`docs/MODERN.md` X-3). `make scripttest` tests the scripts in the build
container: the game from a fresh prefix to its main menu, the headless
server, and a client that joins it. Not tested there: a real Linux
desktop (a window manager, XRandR modes, sound), a Steam Deck, and Steam
or Proton (with Proton, add `Tron.exe` to Steam: the script is not used).

The archives are reproducible. The files are in name order, every time is
2000-01-01 00:00 UTC (`TRON20SRC_EPOCH`), the owner is 0:0 without names,
and gzip stores no name or time. The link time in each binary (and the
signature of its `.pdb`, and the time in its `.map`) is set to the same
time (`build/pestamp.py`). Thus a build without the retail game makes the
same public archive, byte for byte. `build/pubcheck.py` checks each public
archive before it is written: no file may be a file of the retail game, or
an entry of one of its `.rez` archives (by SHA-256), each file must be one
that the build makes, and no file may contain a home directory, or the
user or host name of the machine that built it.

### Without the retail game: the CI build

`make NODISCS=1 compile` compiles and links every binary without the
retail game. The binaries are the same as with it: the only retail data
that they contain is the art of the launcher, the dedicated server and
`CRes.dll`, which is committed in `assets/art/` (`assets/README.md`).
`make progress-ci` then checks the build against `progress/hashes/`
without the retail binaries (`progress/README.md`). `make ci` does both.
`make NODISCS=1` (or `debug`, `modern`, `modern-debug`) also makes the
game directory `out/<name>-nodiscs/`, which does not play, and its public
archive, `out/<name>-public.tar.gz`, which is the same as with the retail
game.

The CI (`.github/workflows/ci.yml`) runs on each push and pull request:

| Job | What it does |
|---|---|
| `tron20-legacy-release` | `make NODISCS=1 game`, then `make progress-ci` (the byte check) |
| `tron20-legacy-debug` | `make NODISCS=1 debug` |
| `tron20-modern-release` | `make NODISCS=1 modern` (with FFmpeg, `ext/ffmpeg`) |
| `tron20-modern-debug` | `make NODISCS=1 modern-debug` |

Each job uploads its public archive as an artifact. The submodules
(`ext/tools`, `ext/toolchains`, and for the modern builds `ext/ffmpeg`)
are fetched over HTTPS at depth 1, at the commits that this repository
records: each of those commits must be on GitHub before the CI can pass.

### Releases

- A push to `master` whose four builds pass updates the pre-release
  `nightly`: its tag moves to the commit, and its files are the modern
  build's two public archives, `tron20src-nightly-<commit>.tar.gz` and
  `tron20src-debug-nightly-<commit>.tar.gz`.
- A tag `v*` (for example `v0.1.0-alpha`) makes a GitHub release with the
  modern build's public archives, `tron20src-<tag>.tar.gz` and
  `tron20src-debug-<tag>.tar.gz`. A tag with a hyphen is a pre-release.

The text of a release is `.github/release_notes.md` (and, for the
nightly, `.github/nightly_notes.md` first). The legacy archives are only
artifacts of the CI runs.

The retail game's bugs, as they are found, are listed in
[`docs/RETAIL_BUGS.md`](docs/RETAIL_BUGS.md); what the modern build changes
is [`docs/MODERN.md`](docs/MODERN.md).
