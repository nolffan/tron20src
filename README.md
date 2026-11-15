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

Needs docker and the three submodules (`git submodule update --init`):
`ext/toolchains/` (the compilers), `ext/rez/` (which fetches the USA discs from
archive.org and installs the game with Update 2) and `ext/tools/` (the
reconstruction tools).

```sh
make reference  # the retail binaries and their function tables, in out/reference/
make progress   # score the build against them
make help       # everything else
```

The retail game's bugs, as they are found, are listed in
[`docs/RETAIL_BUGS.md`](docs/RETAIL_BUGS.md); what the modern build changes
is [`docs/MODERN.md`](docs/MODERN.md).
