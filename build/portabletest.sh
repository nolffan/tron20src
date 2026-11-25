#!/usr/bin/env bash
# The portable test, for the modern build (docs/MODERN.md APPS-2, APPS-3):
# the launcher, the direct start and the dedicated server keep their
# settings beside the executable, not in the registry or in the user's
# folders (APPDATA), and the launcher stays until the game stops.
#
#   build/portabletest.sh [gamedir]     (in the build container;
#                                        `make portabletest` runs it)
#
# gamedir: default out/tron20-modern-release. One fresh Wine prefix (a
# copy of the container's, in /tmp, without registry values of the game
# and without a CD-ROM drive) for all the runs:
#   1. TronLauncher.exe (build/playtest-launcher-portable.txt): Play, the
#      detail setting, the game to the main menu, then Quit. Out:
#      out/play/<name>-launcher/
#   2. Tron.exe, the direct start (build/playtest-launcher-portable-
#      direct.txt): the game to the main menu, then Quit. Out:
#      out/play/<name>-direct/
#   3. TRONSrv.exe in run 2's directory: it hosts a LAN Disc Arena game
#      until its world is loaded (its console, tronsrv.log), then stops.
# While runs 1 and 2 go, a monitor writes the processes each second to
# procs.txt in the run directory. Before and after all the runs the
# prefix's system.reg, user.reg and userdef.reg (the key names) and the
# list of files under drive_c/users/ are kept; out/play/<name>/ gets:
#   reg-diff.txt     the keys that the runs added (Wine adds its own)
#   users-diff.txt   the files that the runs added under drive_c/users/
#   result.txt       PASS or FAIL, and why
# The test fails when:
#   - a key or a file that the runs added names the game (tron,
#     monolith, lithtech, gamespy, the game's files), or a file was added
#     under AppData (Roaming or Local, but not Temp: the engine copies
#     the game DLLs that it loads from the rez files there, and deletes
#     them; the game DLLs are not in the rez files here, so none should
#     appear);
#   - the launcher (run 1) or Tron.exe (run 2) stopped before the game, or
#     did not stop within 15 s after it;
#   - the game did not get to its main menu in run 1 or run 2, or the
#     server did not load its world;
#   - tron20.ini (beside the executables) does not have the launcher's
#     values (Num Launcher Runs) and the game's (ProfileName) after run 1.
# $PLAYTEST_NAME: <name>, default test-portable.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
abs() { case "$1" in /*) echo "$1" ;; *) echo "$REPO/$1" ;; esac; }
GAME="$(abs "${1:-out/tron20-modern-release}")"
NAME="${PLAYTEST_NAME:-test-portable}"
OUT="$REPO/out/play/$NAME"
rm -rf "$OUT"; mkdir -p "$OUT/before" "$OUT/after"
for f in TronLauncher.exe Tron.exe TRONSrv.exe Lithtech.exe; do
    [ -f "$GAME/$f" ] || { echo "portabletest.sh: no $GAME/$f (the modern build: make modern)"; exit 2; }
done

SRC_PREFIX="${WINEPREFIX:-$HOME/.wine}"
P="$(mktemp -d "${TMPDIR:-/tmp}/portable-prefix.XXXXXX")" || exit 2
MON=""; XV=""
cleanup() {
    [ -n "$MON" ] && kill $MON 2>/dev/null
    [ -n "$XV" ] && kill $XV 2>/dev/null
    WINEPREFIX="$P" wineserver -k 2>/dev/null
    rm -rf "$P"
}
trap cleanup EXIT
if [ -f "$SRC_PREFIX/system.reg" ]; then
    cp -a "$SRC_PREFIX/." "$P/" || exit 2
    rm -rf "$P"/drive_c/users/*/Temp/* "$P"/drive_c/users/*/AppData/Local/Temp/*
fi
export WINEPREFIX="$P" WINEDEBUG=-all
wineboot -u >/dev/null 2>&1; wineserver -w

# the key names of the registry files (the values change with each start:
# times, window positions), and the files under drive_c/users/
snapshot() {
    local d="$1" f
    for f in system.reg user.reg userdef.reg; do
        [ -f "$P/$f" ] && grep '^\[' "$P/$f" | sed -E 's/\] [0-9]+$/]/' | sort > "$d/$f.keys"
        [ -f "$P/$f" ] && grep -v '^\[\|^#\|^$' "$P/$f" | sort > "$d/$f.values"
    done
    (cd "$P/drive_c/users" && find . -mindepth 1 | sort) > "$d/users.txt"
}
snapshot "$OUT/before"

# monitor <rundir>: the processes, each second (the Wine ones)
monitor() {
    while :; do
        printf '%s ' "$(date +%s)"
        # each process's program: the first word of its command line, without
        # its directory
        pgrep -fia 'tronlauncher\.exe|tron\.exe|lithtech\.exe|tronsrv\.exe' | cut -d' ' -f2- |
            awk '{ print $1 }' | sed -E 's/.*[\\\/]//' | tr 'A-Z' 'a-z' |
            grep -x -E 'tronlauncher\.exe|tron\.exe|lithtech\.exe|tronsrv\.exe' | sort -u | tr '\n' ' '
        echo
        sleep 1
    done > "$1/procs.txt" 2>/dev/null
}

FAIL=""
fail() { FAIL="$FAIL
- $*"; }

# lifetime <procs.txt> <launcher name>: the launcher was up while the game
# was, and stopped within 15 s after it
lifetime() {
    python3 -I - "$1" "$2" <<'PY'
import sys
path, exe = sys.argv[1], sys.argv[2].lower()
rows = []
for line in open(path):
    p = line.split()
    if not p:
        continue
    rows.append((int(p[0]), set(p[1:])))
game = [t for t, s in rows if 'lithtech.exe' in s]
both = [t for t, s in rows if 'lithtech.exe' in s and exe in s]
if not game:
    print('FAIL no Lithtech.exe seen'); sys.exit(1)
first, last = game[0], game[-1]
gone = [t for t, s in rows if t > last and exe not in s]
gap = (gone[0] - last) if gone else None
# the launcher was there at every sample of the game
missing = [t for t, s in rows if first <= t <= last and exe not in s]
msg = '%s: game up %d s, launcher up with it %d of %d samples' % (exe, last - first, len(both), len(game))
if missing:
    print('FAIL', msg, '- launcher missing at', len(missing), 'samples'); sys.exit(1)
if gap is None:
    print('FAIL', msg, '- launcher still up at the end'); sys.exit(1)
if gap > 15:
    print('FAIL', msg, '- launcher stopped %d s after the game' % gap); sys.exit(1)
print('OK', msg, '- launcher stopped %d s after the game' % gap)
PY
}

# runs 1 and 2: the launcher and the direct start, to the main menu and Quit
run() {   # run <suffix> <exe> <script>
    local r="$REPO/out/play/$NAME-$1"
    # (no run directory of an earlier run: the monitor waits for this one's)
    rm -rf "$r"
    PLAYTEST_NAME="$NAME-$1" PLAYTEST_PREFIX="$P" PLAYTEST_LAUNCH="$2" \
        PLAYTEST_SCREEN=800x600 PLAYTEST_RES=800x600 PLAYTEST_SCRIPT="$3" \
        bash "$REPO/build/playtest.sh" "$GAME" - 20 > "$OUT/playtest-$1.txt" 2>&1 &
    local pt=$!
    # the monitor starts once playtest.sh has made the run directory
    local n=0; while [ ! -f "$r/runner.sh" ] && [ $n -lt 60 ]; do sleep 1; n=$((n+1)); done
    mkdir -p "$r"; monitor "$r" & MON=$!
    wait $pt
    kill $MON 2>/dev/null; wait $MON 2>/dev/null; MON=""
    WINEPREFIX="$P" wineserver -w
    echo "== $1 ($2): $(cat "$OUT/playtest-$1.txt" | head -1)"
    grep -q 'menu' "$r/script.log" 2>/dev/null && [ -f "$r/shots/"*-menu.png ] || fail "$1: no shot of the main menu"
    local lt; lt="$(lifetime "$r/procs.txt" "$(echo "$2" | tr 'A-Z' 'a-z')")" || fail "$1: $lt"
    echo "   $lt"
    echo "$lt" > "$r/lifetime.txt"
    cp "$r/tron20.ini" "$OUT/tron20-$1.ini" 2>/dev/null
}
run launcher TronLauncher.exe build/playtest-launcher-portable.txt
grep -qi '^Num Launcher Runs=1' "$OUT/tron20-launcher.ini" 2>/dev/null || fail "launcher: tron20.ini has no Num Launcher Runs=1"
grep -qi '^ProfileName=' "$OUT/tron20-launcher.ini" 2>/dev/null || fail "launcher: tron20.ini has no ProfileName"
run direct Tron.exe build/playtest-launcher-portable-direct.txt

# run 3: the dedicated server in run 2's directory, a LAN game
SRV="$REPO/out/play/$NAME-direct"
mkdir -p "$SRV/ServerData"
printf '[Settings]\r\nServerName = "portable"\r\nPassword = ""\r\nSCMDPassword = "portable"\r\nGameType = 5\r\nPort = 27888\r\nBandwidth = 3\r\nBandwidthCustom = 256\r\nLocked = 0\r\nDedicated = 1\r\nLANOnly = 1\r\nAllowSCMD = 0\r\n\r\n[DiscArena]\r\nMaxPlayers = 8\r\nMap00 = "Worlds\\RetailMultiPlayer\\DiscArena\\DiscArena01"\r\n' > "$SRV/ServerData/portable.txt"
rm -f "$SRV/tronsrv.log"
Xvfb :79 -screen 0 640x480x24 -nolisten tcp > "$OUT/xvfb-srv.log" 2>&1 & XV=$!
sleep 2
(cd "$SRV" && DISPLAY=:79 wine TRONSrv.exe -config portable +errorlog 1 +alwaysflushlog 1 > "$OUT/wine-srv.log" 2>&1) & S=$!
up=""
for i in $(seq 1 120); do
    if grep -q -i -E 'world.*(load|start)|loaded world|Server started' "$SRV/tronsrv.log" 2>/dev/null; then up=1; break; fi
    kill -0 $S 2>/dev/null || break
    sleep 1
done
sleep 5
DISPLAY=:79 import -window root "$OUT/server.png" 2>/dev/null
if [ -n "$up" ]; then echo "== server: world loaded"; else echo "== server: no world"; fail "server: no world loaded (tronsrv.log)"; fi
cp "$SRV/tronsrv.log" "$OUT/tronsrv.log" 2>/dev/null
WINEPREFIX="$P" wineserver -k; wait $S 2>/dev/null
kill $XV 2>/dev/null; XV=""
WINEPREFIX="$P" wineserver -w

snapshot "$OUT/after"
PAT='tron|monolith|lithtech|gamespy|tron20\.ini|launchcmds|autoexec|\.rez|\.lto|cshell|cres|sres|clientfx|object\.lto|profiles|save'
: > "$OUT/reg-diff.txt"
for f in system.reg user.reg userdef.reg; do
    for k in keys values; do
        [ -f "$OUT/after/$f.$k" ] || continue
        comm -13 "$OUT/before/$f.$k" "$OUT/after/$f.$k" | sed "s|^|$f: |" >> "$OUT/reg-diff.txt"
    done
done
comm -13 "$OUT/before/users.txt" "$OUT/after/users.txt" > "$OUT/users-diff.txt"
if grep -i -E "$PAT" "$OUT/reg-diff.txt" > "$OUT/reg-game.txt"; then
    fail "registry: the runs wrote the game's values (reg-game.txt)"
fi
if grep -i -E "$PAT" "$OUT/users-diff.txt" > "$OUT/users-game.txt" ||
   grep -i -E '/AppData/(Roaming|Local)/' "$OUT/users-diff.txt" | grep -v -i '/Temp' >> "$OUT/users-game.txt"; then
    [ -s "$OUT/users-game.txt" ] && fail "users: the runs wrote files of the game (users-game.txt)"
fi
echo "== registry: $(grep -c . "$OUT/reg-diff.txt") new key or value lines (reg-diff.txt), $(grep -c . "$OUT/reg-game.txt") of the game's"
echo "== users/: $(grep -c . "$OUT/users-diff.txt") new files (users-diff.txt), $(grep -c . "$OUT/users-game.txt" 2>/dev/null || echo 0) of the game's"
echo "== tron20.ini after the launcher run:"; tr -d '\r' < "$OUT/tron20-launcher.ini" 2>/dev/null | sed 's/^/   /'
if [ -z "$FAIL" ]; then echo "PASS" > "$OUT/result.txt"; else printf 'FAIL%s\n' "$FAIL" > "$OUT/result.txt"; fi
cat "$OUT/result.txt"
[ -z "$FAIL" ]
