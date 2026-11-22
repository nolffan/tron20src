# tron20src

*Tron 2.0* (Monolith, 2003), built from source.

> [!WARNING]
> **Work in progress.** The reconstruction is far from complete: much of the
> game code does not match retail yet (see [`progress/README.md`](progress/README.md)),
> and the builds have not been run as a game. Nothing here is ready to play.

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
- **modern**: the same source with fixes and new features (`TRON20_MODERN`).

Compiling back to the retail bytes means compiling the retail game's bugs as
well; their fixes go behind `TRON20_MODERN`, into the modern build only, so
the legacy build keeps matching the retail one.

## Progress

How much of each binary compiles to exactly the bytes of the retail game,
function by function: the table, and how the scoring works and what it
leaves out, are in [`progress/README.md`](progress/README.md). Which
compiler built each binary, and the evidence, is `progress/COMPILERS.md`.

## Building

Needs docker and the two submodules (`git submodule update --init`):
`ext/toolchains/` (the compilers) and `ext/tools/` (the reconstruction
tools).

### What you need

Bring your own discs. The build takes the retail game's binaries and data
from your own copy of Tron 2.0 (USA) and its Update 2 (v1.030). Put these
three files in `isos/` at the root of the repository (a hard link is good:
`ln <file> isos/`), or set `TRON20_ISOS=<directory>`:

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
make reference  # the retail binaries and their function tables, in out/reference/
make progress   # score the build against them
make help       # everything else
```

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
