# Source this to get the VC6-under-Wine build environment. It runs inside the
# build container (docker/Dockerfile; `make shell` gives you one), where Wine
# is on PATH and the toolchain is ext/toolchains/ in this checkout
# (ext/toolchains/README.md). The compile and link command lines themselves are
# build/build.mk's.

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
for f in msvc-6.00-8168/BIN/cl.exe msvc-6.00-9782/Common/MSDev98/Bin/RC.EXE dxsdk-8.1/include/d3d8.h; do
    if [ ! -f "$REPO/ext/toolchains/$f" ]; then
        echo "error: ext/toolchains/$f not found -- the toolchain is missing (ext/toolchains/README.md)" >&2
        return 1 2>/dev/null || exit 1
    fi
done
if ! command -v wine >/dev/null 2>&1; then
    echo "error: wine is not on PATH -- run this inside the build container (make shell)" >&2
    return 1 2>/dev/null || exit 1
fi
export WINEDEBUG=-all
export WINEDLLOVERRIDES="mscoree,mshtml="
# the variant macros ext/tools/releasesrc.py resolves, and the project's own
# header directory ext/tools/mkdeps.py follows
export RELEASESRC_VARIANTS="TRON20_MODERN TRON20_DEBUG"
export MKDEPS_PROJECT_INCLUDE=src/tron20src

# Extra flags for every compile and every link, for the debug and modern
# builds (build/variant.sh sets them). Empty otherwise -- and the release
# build's command lines are then exactly what the project files say.
export TRON20SRC_CFLAGS="${TRON20SRC_CFLAGS:-}"
export TRON20SRC_LDFLAGS="${TRON20SRC_LDFLAGS:-}"
export TRON20SRC_RCFLAGS="${TRON20SRC_RCFLAGS:-}"    # resource scripts: /d NAME
# They are only allowed in the trees build/variant.sh makes (out/build-*/):
# anywhere else they would put TRON20_DEBUG or TRON20_MODERN code into the
# objects `make progress` scores, which must only ever be the release build.
if [ -n "$TRON20SRC_CFLAGS$TRON20SRC_LDFLAGS$TRON20SRC_RCFLAGS" ]; then
    case "$REPO" in
        */out/build-*) ;;
        *) echo "error: TRON20SRC_CFLAGS / LDFLAGS / RCFLAGS are set, but this is not a" >&2
           echo "       build/variant.sh tree ($REPO). Unset them; use make debug/modern." >&2
           return 1 2>/dev/null || exit 1 ;;
    esac
fi

# tron20src_tree_layout <tree>: build/ and the tools copied into the release
# tree, the tools at <tree>/ext/tools/ so that they take <tree> for the
# project (as their layout.py works it out), and the other submodules of
# ext/ (the compilers, the retail game where build/retail.sh looks for it)
# links to this checkout's. A tree made before the submodules moved to ext/
# had tools/, toolchains and rez at its top: those go first.
tron20src_tree_layout() {
    local tree="$1" d n
    rm -rf "$tree/tools"
    rm -f "$tree/toolchains" "$tree/rez"
    mkdir -p "$tree/ext"
    rm -rf "$tree/build" "$tree/ext/tools"
    tar -C "$REPO" -cf - --exclude='__pycache__' --exclude=.git build ext/tools | tar -C "$tree" -xf - || return 1
    for d in "$REPO"/ext/*; do
        n="$(basename "$d")"
        [ "$n" = tools ] || [ -L "$tree/ext/$n" ] || ln -s "../../../ext/$n" "$tree/ext/$n" || return 1
    done
}

# The release build compiles src/ WITHOUT its TRON20_MODERN /
# TRON20_DEBUG blocks, not merely with them switched off: a block the
# compiler skips still moves everything below it, and VC6 lets the file's
# layout decide where some out-of-line inline functions go, so enough
# skipped blocks can change the release binary (ext/tools/releasesrc.py).
# build/compile.sh calls this first, with its own arguments: in a checkout it
# brings out/release-tree/ up to date -- src/ with those blocks resolved as
# the release build sees them, the same build/ and ext/tools/, this checkout's
# build outputs -- runs the script there, copies the build outputs back into
# this checkout's src/ (where `make progress` and the checks read them) and
# exits with the script's status. In a build/variant.sh tree (out/build-*/,
# which must see the blocks), or inside the release tree, it does nothing.
tron20src_release_tree() {
    [ -n "${TRON20SRC_IN_RELEASE_TREE:-}" ] && return 0
    case "$REPO" in */out/build-*) return 0 ;; esac
    local tree="$REPO/out/release-tree" script rc=0
    script="$(basename "${BASH_SOURCE[1]}")"
    mkdir -p "$tree"
    python3 "$REPO/ext/tools/releasesrc.py" sync "$REPO/src" "$tree/src" || exit 1
    python3 "$REPO/ext/tools/releasesrc.py" outputs "$REPO/src" "$tree/src" || exit 1
    tron20src_tree_layout "$tree" || exit 1
    ( cd "$tree" && TRON20SRC_IN_RELEASE_TREE=1 bash "build/$script" "$@" ) || rc=$?
    python3 "$REPO/ext/tools/releasesrc.py" outputs "$tree/src" "$REPO/src" || exit 1
    exit $rc
}

# Compile and link through build/build.mk, in this tree (run it after
# tron20src_release_tree): every listed target, TRON20SRC_JOBS at a time
# (default: one per CPU), each job's output kept together.
tron20src_make() {
    make -C "$REPO" -f build/build.mk --no-print-directory -Otarget \
        -j"${TRON20SRC_JOBS:-$(nproc)}" "$@"
}

# ---------------------------------------------------------------------------
# The tools by hand, for the one-object loop and experiments
# (build/engine.sh, ext/tools/guides/02, 10): the compiler, linker, librarian
# and resource compiler build/build.mk uses, in the same environment --
# src/ as drive S:, ext/toolchains/ as T:, the DirectX 8.1 SDK first on the
# INCLUDE and LIB paths, src/tron20src last -- so a command line copied out of
# an object's <object>.cmd gives the same object. (A compile's .cmd also
# starts with `cd <dir> &&`: run it from that directory.) They use their own
# Wine prefix, out/tools-wine, so they never touch the build's.
#
#   run_cl /nologo /O2 /c foo.cpp       cl 12.00.8168 (VC6), unless
#                                       CL_REV=<build> names another
#                                       ext/toolchains/msvc-6.00-<build>
#   run_link, run_lib, run_rc           link/lib 6.00.8168, RC from SP6
#   tron20src_wpath <path>               the Windows path the tools see
TC="$REPO/ext/toolchains"
VC6="$TC/msvc-6.00-8168"
VC9782="$TC/msvc-6.00-9782"
DXSDK="$TC/dxsdk-8.1"
tron20src_wpath() {
    local p
    p="$(realpath -m "$1")"
    case "$p" in
        "$REPO/src"|"$REPO/src"/*)      p="s:${p#"$REPO/src"}" ;;
        "$TC"|"$TC"/*)                  p="t:${p#"$TC"}" ;;
        *)                              p="z:$p" ;;
    esac
    echo "${p//\//\\}"
}
tron20src_tools_env() {   # the prefix, made the first time, with S: and T:
    export WINEPREFIX="$REPO/out/tools-wine"
    if [ ! -f "$WINEPREFIX/.ready" ]; then
        mkdir -p "$WINEPREFIX"
        wine wineboot -i >/dev/null 2>&1 || true
        wineserver -w || true
        touch "$WINEPREFIX/.ready"
    fi
    ln -sfn "$REPO/src" "$WINEPREFIX/dosdevices/s:"
    ln -sfn "$TC" "$WINEPREFIX/dosdevices/t:"
    local v d
    v="$(tron20src_wpath "$VC6")"; d="$(tron20src_wpath "$DXSDK")"
    export WINEPATH="$v\\BIN;$(tron20src_wpath "$VC9782")\\Common\\MSDev98\\Bin"
    export INCLUDE="$d\\include;$v\\INCLUDE;$v\\MFC\\INCLUDE;$(tron20src_wpath "$REPO/src/tron20src")"
    export LIB="$d\\lib;$v\\LIB;$v\\MFC\\LIB"
}
# The variant flags go after the command's own (/Oy- must follow the /O2
# that implies it), but before a /link, after which the line is the linker's.
tron20src_cl() {
    local cl="$VC6/BIN/cl.exe"
    if [ -n "${CL_REV:-}" ]; then
        cl="$TC/msvc-6.00-$CL_REV/BIN/cl.exe"
        [ -f "$cl" ] || { echo "error: CL_REV=$CL_REV: no $cl" >&2; return 1; }
    fi
    echo "$cl"
}
run_cl() {
    local a linked=0 args=() cl
    cl="$(tron20src_cl)" || return 1
    for a in "$@"; do
        if [ $linked = 0 ] && [ "${a,,}" = "/link" ]; then
            args+=($TRON20SRC_CFLAGS); linked=1
        fi
        args+=("$a")
    done
    [ $linked = 1 ] || args+=($TRON20SRC_CFLAGS)
    ( tron20src_tools_env && wine "$cl" "${args[@]}" )
}
run_link() { ( tron20src_tools_env && wine "$VC6/BIN/link.exe" $TRON20SRC_LDFLAGS "$@" ); }
run_lib()  { ( tron20src_tools_env && wine "$VC6/BIN/lib.exe" "$@" ); }
run_rc()   { ( tron20src_tools_env && wine "$VC9782/Common/MSDev98/Bin/RC.EXE" $TRON20SRC_RCFLAGS "$@" ); }
