#!/usr/bin/env bash
# THE build: Tron 2.0 in out/tron20-legacy-release/, with every binary
# compiled here and the data taken from the retail game.
#
#   make                    (on the host: builds the docker image, installs
#                            and checks the retail game, runs this in a
#                            container)
#   build/all.sh            (inside the build container, e.g. `make shell`)
#
# Needs: the toolchain (ext/toolchains/, see ext/toolchains/README.md), Wine
# and CMake (the build container has them), the lithtools submodule
# (ext/lithtools/: lithrez, step 1b), and the retail game, Tron 2.0 (USA)
# with Update 2 (v1.030), as build/discs.py installs it from your own disc
# images and the update (out/retail/tron2-usa-1.030/, build/retail.sh;
# `make retail`). This script only reads that directory. The game
# directory takes only the discs' files from it: Update 2's data is
# committed in assets/ (step 1b), and its binaries are built here.
#
# TRON20SRC_NODISCS=1 builds without the retail game (`make NODISCS=1
# game`). The binaries are the same: the art they are compiled with is
# committed (assets/art/). Steps 1 and 2 are then skipped (step 1b is not:
# it needs no retail game), and the game directory is
# out/tron20-legacy-release-nodiscs/, with NOT-PLAYABLE.txt in it: without
# the discs' .rez archives it does not play.
#
# The steps, in order:
#   1. resources          the game's data from the retail game
#                         (ext/tools/retailextract.py, with the layout of
#                         build/retail.py): the .rez archives, hard-linked
#                         where they hold no binary; GameDLL.rez, which
#                         holds the retail game DLLs, copied with those
#                         entries zeroed and renamed (build/rezhide.py:
#                         custom/ gives the DLLs whatever the rez order).
#                         Also the movies (hard links), the manual and
#                         the shipped autoexec.cfg and
#                         Profiles/Default.prf (as *.retail copies). Only
#                         the discs' files. No binary.
#   1b. assets            Update 2's data, committed in assets/
#                         (assets/README.md): assets/rez/ packed into
#                         tron20src.rez by lithtools' lithrez
#                         (build/lithrez.sh) with a fixed time, and the
#                         update's readme (assets/game/). Also without
#                         the retail game; with it, assets/ is checked
#                         against the retail update first
#                         (build/assets.py).
#   2. third-party runtime  Bink (binkw32.dll) and the Microsoft C, C++ and
#                         MFC 4.2 runtime DLLs (MFC42.DLL, MFC42U.DLL,
#                         MSVCP60.DLL, Msvcrt.dll), from the retail game:
#                         licensed binaries with no source. The game ships
#                         no DirectX files. With TRON20SRC_MIDDLEWARE=open
#                         (build/variant.sh sets it for the modern builds)
#                         only the Microsoft runtime comes from the retail
#                         game, and binkw32.dll is ours: a Bink player on
#                         FFmpeg, built from src/modern/binkw32/ and the
#                         ext/ffmpeg submodule by build/ext.sh (MODERN.md
#                         X-1).
#   3. compile            every workspace build/build.mk knows of
#                         (build/compile.sh: one job per CPU,
#                         TRON20SRC_JOBS=<n> to change it), and only what
#                         changed since the last build.
#   4. assemble           the built binaries into the game directory: the
#                         game DLLs in the custom/ rez directory, last in the
#                         rez stack (tron20cmds.txt, build/tron20cmds.sh);
#                         the engine, the launcher and the dedicated server
#                         in the game's root; and the run scripts
#                         linux-run.sh and linux-server.sh
#                         (build/game-scripts/). Each copy gets the fixed time
#                         TRON20SRC_EPOCH (2000-01-01) in place of the time
#                         of its link (build/pestamp.py); src/**/built/
#                         keeps the linker's output.
#   5. audit              every PE file in the game directory listed with
#                         its origin; the build fails if one is unaccounted
#                         for, if a .rez archive still holds one, or if a
#                         binary we build is missing.
#
#   6. package            out/tron20-legacy-release-private.tar.gz: the game
#                         directory, retail data and all; and
#                         -public.tar.gz: only what we build, with install
#                         scripts that copy the rest from the player's own
#                         two discs (build/package.sh,
#                         build/mkinstall.py, checked by build/pubcheck.py).
#                         Built without the retail game: the public one
#                         only, the same archive. TRON20SRC_PACKAGE=0 skips
#                         it (build/variant.sh packages its game directory
#                         itself, once its symbols are in place).
set -e
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Wine and the toolchain live in the build container: on the host, use make.
if ! command -v wine >/dev/null 2>&1; then
    echo "error: no wine here -- run \`make\` from the repository root (it runs this" >&2
    echo "       script in the build container), or run it inside \`make shell\`." >&2
    exit 1
fi
export WINEDEBUG=-all

cd "$REPO"
source build/retail.sh
OUT="${TRON20SRC_GAME_OUT:-out/tron20-legacy-release}"
NODISCS=
if [ "${TRON20SRC_NODISCS:-0}" = 1 ]; then
    NODISCS=1
    OUT="${TRON20SRC_GAME_OUT:-out/tron20-legacy-release-nodiscs}"
else
    RETAIL="$(retail_dir)"
    retail_verify
fi
T0=$(date +%s)
step() { echo; echo "######## $* ($(( $(date +%s) - T0 ))s)"; }
B=src/Game/built/Final
E=src/Engine/built/Final
# Every binary of the game: each one is ours (docs/PLAN.md, the table of
# "The retail game"). No retail binary stands in for one of them.
BUILT=(
    "$B/ClientShellDLL/TRON/CShell.dll"  custom/CShell.dll
    "$B/ObjectDll/TRON/Object.lto"       custom/Object.lto
    "$B/ClientFxDll/ClientFx.fxd"        custom/ClientFx.fxd
    "$B/ClientRes/TRON/CRes.dll"         custom/CRes.dll
    "$B/ServerRes/TRON/SRes.dll"         custom/SRes.dll
    "$B/Launcher/Tron.exe"               Tron.exe
    "$B/ServerApp/TRONSrv.exe"           TRONSrv.exe
    "$E/Lithtech/Lithtech.exe"           Lithtech.exe
    "$E/server/server.dll"               server.dll
    "$E/snddrv/SndDrv.dll"               SndDrv.dll
    "$E/ltmsg/ltmsg.dll"                 ltmsg.dll
)
# The modern build links the launcher twice (build/build.mk): Tron.exe
# starts the game, TronLauncher.exe is the launcher (MODERN.md APPS-1)
case " ${TRON20SRC_CFLAGS:-} " in *" /D TRON20SRC_MODERN "*)
    BUILT+=("$B/Launcher/TronLauncher.exe" TronLauncher.exe) ;;
esac

mkdir -p "$OUT/custom" "$OUT/Save" "$OUT/Profiles"
# Step 1b, with or without the retail game: after step 1
assets_step() {
    step "1b. assets: Update 2's data (assets/) -> $OUT/tron20src.rez"
    # Update 2's data ships with the release (assets/README.md): what its
    # Gamep4.rez holds besides the game DLLs, packed into tron20src.rez by
    # lithtools' lithrez (build/lithrez.sh) with a fixed time (the time of
    # Gamep4.rez, 2003-10-03), so that the archive is the same bytes on every
    # build; and the update's readme. The game loads tron20src.rez where the
    # update's setup put Gamep3.rez and Gamep4.rez (build/tron20cmds.sh).
    # build/retail.py keeps the retail archives of the update out. With the
    # retail game, assets/ is first checked against the retail update
    # (build/assets.py; make art-check).
    [ -n "$NODISCS" ] || python3 build/assets.py check "$RETAIL"
    LITHREZ="$(bash build/lithrez.sh)"
    rm -f "$OUT/Gamep3.rez" "$OUT/Gamep4.rez"     # an older build's, from the retail game
    "$LITHREZ" c "$OUT/tron20src.rez.part" assets/rez --time 1065224897 --compat 1.02 >/dev/null
    # (the same bytes as the last build's: that file is kept)
    if cmp -s "$OUT/tron20src.rez.part" "$OUT/tron20src.rez"; then
        rm -f "$OUT/tron20src.rez.part"
    else
        mv -f "$OUT/tron20src.rez.part" "$OUT/tron20src.rez"
    fi
    printf 'asset assets/rez (lithrez)\ttron20src.rez\n' >> "$OUT/MANIFEST.txt"
    echo "   tron20src.rez  <-  assets/rez/ ($(find assets/rez/ -type f | wc -l) files)"
    for f in assets/game/*; do
        n="$(basename "$f")"
        cmp -s "$f" "$OUT/$n" || { cp "$f" "$OUT/$n.part"; mv -f "$OUT/$n.part" "$OUT/$n"; }
        printf 'asset %s\t%s\n' "$f" "$n" >> "$OUT/MANIFEST.txt"
        echo "   $n  <-  $f"
    done
}

if [ -n "$NODISCS" ]; then
    step "1. resources: none (built without the retail game) -> $OUT"
    cat > "$OUT/NOT-PLAYABLE.txt" <<EOF
This directory was built without the retail game (NODISCS=1). It has only
the binaries built from our source: no game data (no .rez archives, no
movies) and no Bink or Microsoft runtime. It does not play. Build with the
retail game (make) for a game directory that plays.
EOF
    assets_step
    if [ "${TRON20SRC_MIDDLEWARE:-retail}" = open ]; then
        # our Bink player is ours to build (the modern build's public
        # archive has it): only the retail Microsoft runtime is left out
        step "2. open runtime: our Bink player on FFmpeg (build/ext.sh); no retail runtime (built without the retail game)"
        bash build/ext.sh
        python3 ext/tools/retailextract.py openruntime "${TRON20SRC_EXT_OUT:-out/ext}" "$OUT"
    else
        step "2. third-party runtime: none (built without the retail game)"
    fi
else
    step "1. resources: game data from $RETAIL -> $OUT"
    rm -f "$OUT/NOT-PLAYABLE.txt"
    python3 ext/tools/retailextract.py resources "$RETAIL" "$OUT"
    # The zeroed game DLLs of the discs' archive renamed, so that no rez
    # order gives the engine those in place of ours (build/rezhide.py)
    python3 build/rezhide.py "$OUT/GameDLL.rez"
    # autoexec.cfg and the default profile start as the shipped ones (the
    # game rewrites them); a rebuild keeps the player's.
    [ -f "$OUT/autoexec.cfg" ] || cp "$OUT/autoexec.cfg.retail" "$OUT/autoexec.cfg"
    [ -f "$OUT/Profiles/Default.prf" ] || cp "$OUT/Default.prf.retail" "$OUT/Profiles/Default.prf"
    assets_step

    # TRON20SRC_MIDDLEWARE (build/variant.sh): retail, the default, RAD's
    # Bink; open, the modern builds' default, our Bink player on FFmpeg
    # (MODERN.md X-1)
    case "${TRON20SRC_MIDDLEWARE:-retail}" in
        retail)
            step "2. THIRD-PARTY RUNTIME: from $RETAIL: Bink and the Microsoft runtime"
            python3 ext/tools/retailextract.py thirdparty "$RETAIL" "$OUT" ;;
        open)
            step "2. open runtime: our Bink player on FFmpeg (build/ext.sh); from $RETAIL: the Microsoft runtime"
            bash build/ext.sh
            python3 ext/tools/retailextract.py thirdparty "$RETAIL" "$OUT" open
            python3 ext/tools/retailextract.py openruntime "${TRON20SRC_EXT_OUT:-out/ext}" "$OUT" ;;
        *) echo "error: TRON20SRC_MIDDLEWARE=$TRON20SRC_MIDDLEWARE: retail or open" >&2; exit 1 ;;
    esac
fi

bash build/tron20cmds.sh > "$OUT/tron20cmds.txt"

step "3. compile: the workspaces (build/compile.sh: build/build.mk, ${TRON20SRC_JOBS:-$(nproc)} jobs)"
bash build/compile.sh all

step "4. assemble: the built binaries -> $OUT"
missing=()
for ((i = 0; i < ${#BUILT[@]}; i += 2)); do
    if [ ! -f "${BUILT[i]}" ]; then
        missing+=("${BUILT[i+1]}")
        continue
    fi
    cp "${BUILT[i]}" "$OUT/${BUILT[i+1]}.part"
    # the link's times set to TRON20SRC_EPOCH (build/pestamp.py): the same
    # files from every build of the same source
    python3 build/pestamp.py "${TRON20SRC_EPOCH:-946684800}" "$OUT/${BUILT[i+1]}.part"
    mv -f "$OUT/${BUILT[i+1]}.part" "$OUT/${BUILT[i+1]}"
    printf 'built %s\t%s\n' "${BUILT[i]}" "${BUILT[i+1]}" >> "$OUT/MANIFEST.txt"
    echo "   ${BUILT[i+1]}  <-  ${BUILT[i]}"
done
# The run scripts (build/game-scripts/; usage in their headers, --help):
# linux-run.sh starts the game with the host's Wine through Tron.exe, and
# linux-server.sh the dedicated server, TRONSrv.exe, headless under
# xvfb-run without a display. Each finds the build by its files
# (TronLauncher.exe: the modern build), so both builds have the same
# scripts. Both archives carry them (MANIFEST.txt: ours).
for s in linux-run.sh linux-server.sh; do
    cp "build/game-scripts/$s" "$OUT/$s.part"
    chmod 755 "$OUT/$s.part"
    mv -f "$OUT/$s.part" "$OUT/$s"
    printf 'ours build/game-scripts/%s\t%s\n' "$s" "$s" >> "$OUT/MANIFEST.txt"
    echo "   $s  <-  build/game-scripts/$s"
done
# one row per file, of the files that are here
python3 - "$OUT/MANIFEST.txt" <<'PY'
import os, sys
p = sys.argv[1]
d = os.path.dirname(p)
rows = {}
for line in open(p).read().splitlines():
    if "\t" in line and not line.startswith("#"):
        origin, path = line.split("\t", 1)
        if os.path.exists(os.path.join(d, path)):
            rows[path] = origin
with open(p, "w") as f:
    f.write("# origin<TAB>path, one row per file build/all.sh put here\n")
    for path in sorted(rows, key=str.lower):
        f.write(f"{rows[path]}\t{path}\n")
PY

step "5. audit: every PE file in $OUT"
python3 ext/tools/retailextract.py audit "$OUT"
if [ ${#missing[@]} -gt 0 ]; then
    echo "error: not built (see the compile above): ${missing[*]}" >&2
    exit 1
fi

if [ "${TRON20SRC_PACKAGE:-1}" = 1 ]; then
    if [ -n "$NODISCS" ]; then
        step "6. package: $OUT -> $(dirname "$OUT")/$(basename "$OUT" -nodiscs)-public.tar.gz (no private archive without the retail game)"
        bash build/package.sh "$OUT" "$(basename "$OUT" -nodiscs)"
    else
        step "6. package: $OUT -> $OUT-private.tar.gz, -public.tar.gz"
        bash build/package.sh "$OUT"
    fi
fi

if [ -n "$NODISCS" ]; then
    step "done: $OUT ($(( $(date +%s) - T0 ))s) -- built without the retail game: it does not play (NOT-PLAYABLE.txt); its public archive, installed from the discs, does"
else
    step "done: $OUT ($(( $(date +%s) - T0 ))s); make test runs it"
fi
