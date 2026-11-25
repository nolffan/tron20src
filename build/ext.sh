#!/usr/bin/env bash
# The modern build's open-source middleware, built from ext/ into out/ext/
# (MODERN.md X-1). build/all.sh step 2 runs it when TRON20SRC_MIDDLEWARE=
# open, which build/variant.sh sets for modern and modern-debug:
#
#   out/ext/binkw32.dll   our Bink player (src/modern/binkw32/) on FFmpeg's
#                         Bink demuxer and decoders (ext/ffmpeg, linked
#                         statically, only what Bink needs), in place of
#                         RAD's binkw32.dll
#
# Nothing else: Tron 2.0 has no Miles. RAD's binkw32.dll names mss32.dll
# only for its optional Miles sound output, and no retail binary of the game
# imports or loads mss32.dll. The engine's sound is its own driver,
# SndDrv.dll (src/Engine/snddrv/, which decodes MP3 itself in the modern
# build, MODERN.md X-2), and DirectMusic.
#
# The same script as nolf2src's and cjacksrc's (their X-1), with the same
# FFmpeg (the submodule at n8.1.3). FFmpeg is configured as LGPL-2.1-or-
# later: no GPL or non-free part is enabled. It is compiled with Zig's C
# compiler (`zig cc`: clang with its own mingw-w64 headers and libraries,
# cross-compiling for 32-bit Windows; the build image has it,
# docker/Dockerfile). Nothing is written into ext/. Each part is built
# again only when its inputs change (stamps in out/ext/). FFmpeg's object
# files are removed when its libraries are installed (about 100 MB).
#
#   build/ext.sh          (in the build container; build/all.sh runs it)
set -e
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO"
OUT="${TRON20SRC_EXT_OUT:-$REPO/out/ext}"
TARGET=x86-windows-gnu
export ZIG_GLOBAL_CACHE_DIR="$OUT/zig-cache" ZIG_LOCAL_CACHE_DIR="$OUT/zig-cache"

if [ ! -f ext/ffmpeg/LICENSE.md ]; then
    echo "error: ext/ffmpeg is empty -- run: git submodule update --init ext/ffmpeg" >&2
    exit 1
fi
if ! command -v zig >/dev/null 2>&1; then
    echo "error: no zig here -- run this in the build container (make shell)" >&2
    exit 1
fi
mkdir -p "$OUT"

# fresh <name> <inputs...>: true (nothing to do) when <name>'s inputs are
# what they were at its last build. A directory (the submodule) counts by
# the names, sizes and times of its files -- read without git, which the
# image does not have, so that a submodule checkout and a worktree's
# hard-linked copy of one (ext/tools/agentwt.sh) are the same.
inputs() { for f in "$@"; do
               if [ -d "$f" ]; then
                   echo "$f $(find -L "$f" -name .git -prune -o -type f -printf '%P %s %T@\n' | sort | sha256sum)"
               else sha256sum "$f"; fi
           done; zig version; }
fresh()  { local s="$OUT/.$1.stamp"; shift
           [ -f "$s" ] && [ "$(inputs "$@" | sha256sum)" = "$(cat "$s")" ]; }
stamp()  { local s="$OUT/.$1.stamp"; shift; inputs "$@" | sha256sum > "$s"; }

# --- FFmpeg -> out/ext/ffmpeg/ (static libraries) -----------------------------
if fresh ffmpeg ext/ffmpeg build/ext.sh; then
    echo "== FFmpeg: up to date ($OUT/ffmpeg)"
else
    echo "== FFmpeg: ext/ffmpeg -> $OUT/ffmpeg (the Bink decoders only, static)"
    rm -rf "$OUT/ffmpeg-build" "$OUT/ffmpeg"
    mkdir -p "$OUT/ffmpeg-build"
    ( cd "$OUT/ffmpeg-build" &&
      "$REPO/ext/ffmpeg/configure" --prefix="$OUT/ffmpeg" \
          --enable-cross-compile --target-os=mingw32 --arch=x86 \
          --cc="zig cc -target $TARGET" --ar="zig ar" --ranlib="zig ranlib" \
          --nm=nm --disable-stripping \
          --disable-everything --disable-autodetect --disable-programs \
          --disable-doc --disable-network --disable-x86asm \
          --disable-avdevice --disable-avfilter --disable-swresample \
          --enable-static --disable-shared --enable-w32threads \
          --enable-decoder=bink,binkaudio_rdft,binkaudio_dct \
          --enable-demuxer=bink --enable-protocol=file \
          > configure.log 2>&1 ||
          { tail -20 configure.log; [ -f ffbuild/config.log ] && tail -40 ffbuild/config.log; exit 1; }
      make -j"${TRON20SRC_JOBS:-$(nproc)}" > make.log 2>&1 || { tail -40 make.log; exit 1; }
      make install > install.log 2>&1 || { tail -20 install.log; exit 1; } )
    rm -rf "$OUT/ffmpeg-build"
    stamp ffmpeg ext/ffmpeg build/ext.sh
fi

# --- our Bink player -> out/ext/binkw32.dll ----------------------------------
B=src/modern/binkw32
if fresh binkw32 ext/ffmpeg build/ext.sh "$B/binkw32.c" "$B/binkw32.def"; then
    echo "== binkw32.dll: up to date ($OUT/binkw32.dll)"
else
    echo "== binkw32.dll: $B/ + FFmpeg -> $OUT/binkw32.dll"
    zig cc -target $TARGET -O2 -s -shared -o "$OUT/binkw32.dll" \
        -I"$OUT/ffmpeg/include" "$B/binkw32.c" "$B/binkw32.def" \
        -L"$OUT/ffmpeg/lib" -lavformat -lavcodec -lswscale -lavutil \
        -lwinmm -lbcrypt
    rm -f "$OUT/binkw32.lib" "$OUT/binkw32.pdb"
    stamp binkw32 ext/ffmpeg build/ext.sh "$B/binkw32.c" "$B/binkw32.def"
fi
