#!/usr/bin/env bash
# The test of the modern SndDrv.dll's MP3 codec (MODERN.md X-2), outside
# the game: src/tron20src/mp3/mp3acmtest.cpp, compiled with Zig's C
# compiler against the codec's source, run under Wine on every WAV file of
# the retail Sound.rez (3,404 of them are MP3; about 7 minutes). Wine's own
# MP3 codecs are switched off for the run (WINEDLLOVERRIDES), so every
# conversion is the codec's. Prints the failures and a summary; exit
# status 0 when no file fails.
#
#   build/mp3acmtest.sh           (in the build container: make shell)
set -e
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO"
source build/retail.sh
RETAIL="$(retail_dir)"
WORK="$REPO/out/mp3acmtest"
rm -rf "$WORK"
mkdir -p "$WORK/wav"
export ZIG_GLOBAL_CACHE_DIR="$WORK/zig-cache" ZIG_LOCAL_CACHE_DIR="$WORK/zig-cache"
zig cc -x c++ -fno-exceptions -fno-rtti -target x86-windows-gnu -O2 -w \
    -I src/tron20src/mp3 src/tron20src/mp3/mp3acmtest.cpp \
    -o "$WORK/mp3acmtest.exe" -lmsacm32 -lwinmm -luser32
python3 -I ext/tools/rezextract.py x "$RETAIL/Sound.rez" "$WORK/wav" .wav > /dev/null
( cd "$WORK" && find wav -iname '*.wav' | sort > list.txt )
echo "== $(wc -l < "$WORK/list.txt") WAV files from Sound.rez"
rc=0
( cd "$WORK" && WINEDEBUG=-all WINEDLLOVERRIDES="l3codeca.acm=d;winemp3.acm=d" \
    wine mp3acmtest.exe list.txt ) || rc=$?
rm -rf "$WORK/wav" "$WORK/zig-cache"
exit $rc
