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
# (the build container has it), and the retail game, Tron 2.0 (USA) with
# Update 2 (v1.030), as build/discs.py installs it from your own disc images
# and the update (out/retail/tron2-usa-1.030/, build/retail.sh; `make
# retail`). This script only reads that directory.
#
# TRON20SRC_NODISCS=1 builds without the retail game (`make NODISCS=1
# game`). The binaries are the same: the art they are compiled with is
# committed (assets/art/). Steps 1 and 2 are then skipped, and the game
# directory is out/tron20-legacy-release-nodiscs/, with NOT-PLAYABLE.txt in
# it: without the .rez archives it does not play.
#
# The steps, in order:
#   1. resources          the game's data from the retail game
#                         (ext/tools/retailextract.py, with the layout of
#                         build/retail.py): the .rez archives, hard-linked
#                         where they hold no binary; GameDLL.rez,
#                         Gamep3.rez and Gamep4.rez, which hold the retail
#                         game DLLs, copied with those entries zeroed. Also
#                         the movies (hard links) and the shipped
#                         autoexec.cfg and Profiles/Default.prf (as
#                         *.retail copies). No binary.
#   2. third-party runtime  Bink (binkw32.dll) and the Microsoft C, C++ and
#                         MFC 4.2 runtime DLLs (MFC42.DLL, MFC42U.DLL,
#                         MSVCP60.DLL, Msvcrt.dll), from the retail game:
#                         licensed binaries with no source. The game ships
#                         no DirectX files.
#   3. compile            every workspace build/build.mk knows of
#                         (build/compile.sh: one job per CPU,
#                         TRON20SRC_JOBS=<n> to change it), and only what
#                         changed since the last build.
#   4. assemble           the built binaries into the game directory: the
#                         game DLLs in the custom/ rez directory, last in the
#                         rez stack (tron20cmds.txt, build/tron20cmds.sh);
#                         the engine, the launcher and the dedicated server
#                         in the game's root.
#   5. audit              every PE file in the game directory listed with
#                         its origin; the build fails if one is unaccounted
#                         for, if a .rez archive still holds one, or if a
#                         binary we build is missing.
#
# The packages of the game directory (-private / -public archives) are
# docs/PLAN.md's package 10.
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

mkdir -p "$OUT/custom" "$OUT/Save" "$OUT/Profiles"
if [ -n "$NODISCS" ]; then
    step "1. resources: none (built without the retail game) -> $OUT"
    cat > "$OUT/NOT-PLAYABLE.txt" <<EOF
This directory was built without the retail game (NODISCS=1). It has only
the binaries built from our source: no game data (no .rez archives, no
movies) and no Bink or Microsoft runtime. It does not play. Build with the
retail game (make) for a game directory that plays.
EOF
    step "2. third-party runtime: none (built without the retail game)"
else
    step "1. resources: game data from $RETAIL -> $OUT"
    rm -f "$OUT/NOT-PLAYABLE.txt"
    python3 ext/tools/retailextract.py resources "$RETAIL" "$OUT"
    # autoexec.cfg and the default profile start as the shipped ones (the
    # game rewrites them); a rebuild keeps the player's.
    [ -f "$OUT/autoexec.cfg" ] || cp "$OUT/autoexec.cfg.retail" "$OUT/autoexec.cfg"
    [ -f "$OUT/Profiles/Default.prf" ] || cp "$OUT/Default.prf.retail" "$OUT/Profiles/Default.prf"

    step "2. THIRD-PARTY RUNTIME: from $RETAIL: Bink and the Microsoft runtime"
    python3 ext/tools/retailextract.py thirdparty "$RETAIL" "$OUT"
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
    mv -f "$OUT/${BUILT[i+1]}.part" "$OUT/${BUILT[i+1]}"
    printf 'built %s\t%s\n' "${BUILT[i]}" "${BUILT[i+1]}" >> "$OUT/MANIFEST.txt"
    echo "   ${BUILT[i+1]}  <-  ${BUILT[i]}"
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

if [ -n "$NODISCS" ]; then
    step "done: $OUT ($(( $(date +%s) - T0 ))s) -- built without the retail game: it does not play (NOT-PLAYABLE.txt)"
else
    step "done: $OUT ($(( $(date +%s) - T0 ))s); make test runs it"
fi
