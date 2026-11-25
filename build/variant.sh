#!/usr/bin/env bash
# The game built from the same source with project defines that the release
# build never has -- never for measuring: progress is only ever scored on the
# release build (`make`, `make progress`).
#
#   build/variant.sh debug         make debug          -> out/tron20-legacy-debug/
#   build/variant.sh modern        make modern         -> out/tron20-modern-release/
#   build/variant.sh modern-debug  make modern-debug   -> out/tron20-modern-debug/
#
# (on the host the make targets run this in the build container; inside
# `make shell` run it directly). The result is a game directory like
# out/tron20-legacy-release/, made by the same build/all.sh.
#
#   debug    TRON20SRC_DEBUG and symbols, for finding out why something goes
#            wrong. Compile + /Z7 /Oy- /D TRON20SRC_DEBUG (debug information,
#            frame pointers kept so that a backtrace can walk the stack;
#            every other flag, the optimisation too, is the release one);
#            resource scripts + /d TRON20SRC_DEBUG; link + /DEBUG /OPT:REF
#            /INCREMENTAL:NO (a .pdb for each binary, the same removal of
#            unreferenced code as the release link). The .pdb and the .map
#            of each binary go beside it in the game directory.
#   modern   TRON20SRC_MODERN: the fixes and features of the modern build
#            (docs/MODERN.md). Compile + /D TRON20SRC_MODERN, resource
#            scripts + /d TRON20SRC_MODERN. build/build.mk then links the
#            launcher twice: TronLauncher.exe and Tron.exe (MODERN.md APPS-1).
#   modern-debug   both.
#
# The middleware: TRON20SRC_MIDDLEWARE=open (the default of modern and
# modern-debug) puts our Bink player on FFmpeg in the game directory in
# place of RAD's binkw32.dll (build/ext.sh, built once into the checkout's
# out/ext/ for every variant; MODERN.md X-1), and the Microsoft runtime
# from the retail game. `retail` (the default of debug, and what the
# release build does) puts RAD's Bink there too. Either can be asked for:
# `make modern TRON20SRC_MIDDLEWARE=retail`.
#
# The flags reach every compile and link through build/env.sh's
# TRON20SRC_CFLAGS / TRON20SRC_LDFLAGS / TRON20SRC_RCFLAGS, which are empty
# for every release command -- and which build/env.sh refuses outside the
# trees this script makes, so they cannot get into the release build by
# accident.
#
# It never touches the release build. The source is copied (without any
# build output) into out/build-<variant>/ and built there by the same
# build/all.sh, so src/*/built -- what `make progress` scores --,
# out/release-tree/ and out/tron20-legacy-release/ stay exactly as they
# were. The copy keeps its own build outputs from one run to the next, and
# only changed sources are written again into it, so a rebuild does again
# only what changed. The Wine prefix of the build tools is a hard-linked copy
# of out/release-tree/'s (if there is one; Wine writes its registry to a new
# file and renames it, so a link is never written through). The game data
# goes into out/<name>/ on the first run (hard links to the retail files
# where possible) and is kept after that.
#
# With TRON20SRC_NODISCS=1 (make NODISCS=1 <variant>: without the retail
# game, build/all.sh) the game directory is out/<name>-nodiscs/, which does
# not play.
set -e
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO"

VARIANT="${1:-}"
case "$VARIANT" in
    debug)        CFLAGS="/Z7 /Oy- /D TRON20SRC_DEBUG"
                  LDFLAGS="/DEBUG /OPT:REF /INCREMENTAL:NO"
                  RCFLAGS="/d TRON20SRC_DEBUG"
                  NAME=tron20-legacy-debug ;;
    modern)       CFLAGS="/D TRON20SRC_MODERN"
                  LDFLAGS=""
                  RCFLAGS="/d TRON20SRC_MODERN"
                  NAME=tron20-modern-release ;;
    modern-debug) CFLAGS="/Z7 /Oy- /D TRON20SRC_DEBUG /D TRON20SRC_MODERN"
                  LDFLAGS="/DEBUG /OPT:REF /INCREMENTAL:NO"
                  RCFLAGS="/d TRON20SRC_DEBUG /d TRON20SRC_MODERN"
                  NAME=tron20-modern-debug ;;
    *) echo "usage: build/variant.sh debug|modern|modern-debug" >&2; exit 2 ;;
esac
DIR="$NAME"
[ "${TRON20SRC_NODISCS:-0}" = 1 ] && DIR="$NAME-nodiscs"

if ! command -v wine >/dev/null 2>&1; then
    echo "error: no wine here -- run \`make $VARIANT\` from the repository root, or" >&2
    echo "       run this inside \`make shell\`." >&2
    exit 1
fi
# (the checkout's environment: the toolchain check, tron20src_tree_layout)
source "$REPO/build/env.sh"

TREE="$REPO/out/build-$VARIANT"
echo "== $VARIANT tree: src/ build/ -> out/build-$VARIANT/ (sources only; its outputs kept)"
mkdir -p "$TREE"
python3 ext/tools/releasesrc.py mirror "$REPO/src" "$TREE/src"
# build/, assets/ and the tools copied, the compilers linked (as for the
# release tree)
tron20src_tree_layout "$TREE"
# the Wine prefix of build/build.mk: hard links to the release tree's, the
# first time
PFX_FROM="$REPO/out/release-tree/.wine/drives"
if [ ! -f "$TREE/.wine/drives/.ready" ] && [ -f "$PFX_FROM/.ready" ]; then
    echo "== Wine prefix: hard links to out/release-tree/.wine/"
    rm -rf "$TREE/.wine/drives"
    mkdir -p "$TREE/.wine"
    cp -al "$PFX_FROM" "$TREE/.wine/drives"
fi
# the retail game: the checkout's (build/retail.sh), checked already by
# `make retail`
source "$REPO/build/retail.sh"
TRON20SRC_RETAIL="$(retail_root)"
export TRON20SRC_RETAIL TRON20SRC_SKIP_RETAIL_CHECK=1

export TRON20SRC_CFLAGS="$CFLAGS"
export TRON20SRC_LDFLAGS="$LDFLAGS"
export TRON20SRC_RCFLAGS="$RCFLAGS"
export TRON20SRC_GAME_OUT="$REPO/out/$DIR"
case "$VARIANT" in
    modern*) export TRON20SRC_MIDDLEWARE="${TRON20SRC_MIDDLEWARE:-open}" ;;
    *)       export TRON20SRC_MIDDLEWARE="${TRON20SRC_MIDDLEWARE:-retail}" ;;
esac
export TRON20SRC_EXT_OUT="$REPO/out/ext"
bash "$TREE/build/all.sh"

case "$VARIANT" in *debug)
    echo
    echo "== symbols: the .pdb (and .map) of each built binary, beside it in out/$DIR/"
    # MANIFEST.txt has `built <source>\t<installed path>` for each binary
    # we built; its .pdb has the name of the link output, with .pdb, and
    # the debugger looks for it by that name in the binary's directory.
    grep '^built ' "$TRON20SRC_GAME_OUT/MANIFEST.txt" | sort -u |
    while IFS=$'\t' read -r src dest; do
        src="${src#built }"
        d="$TRON20SRC_GAME_OUT/$(dirname "$dest")"
        pdb="$TREE/${src%.*}.pdb"
        if [ -f "$pdb" ]; then
            cp "$pdb" "$d/"
            echo "   $(dirname "$dest")/$(basename "$pdb")"
        else
            echo "   (no .pdb for $dest)"
        fi
        map="$TREE/${src%.*}.map"
        [ -f "$map" ] && cp "$map" "$d/"
    done ;;
esac
echo "== done: out/$DIR/"
