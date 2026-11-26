#!/usr/bin/env bash
# The run scripts' test: linux-run.sh and linux-server.sh
# (build/game-scripts/) in a game directory installed from a public
# archive with its linux-install.sh, as a player runs them.
#
#   build/scripttest.sh <game directory> [run|server|both]
#                                       (in the build container;
#                                        `make scripttest` runs it)
#
# Each step runs with a new, empty HOME and no WINEPREFIX, so that the
# scripts make their default prefixes ($HOME/.wine-tron20,
# $HOME/.wine-tron20-server) from nothing, as on a first run.
#   run     ./linux-run.sh on a virtual display (Xvfb: 800x600 for the
#           legacy build, whose window must have a display mode of its size;
#           1280x720 for the modern build; SCRIPTTEST_SCREEN=WxH). The
#           legacy build's launcher opens: Play. The detail setting (both
#           builds, first run): Medium. Then the engine must reach the main
#           menu (a screenshot 45 s after the engine starts, which you look
#           at; windows.txt: the windows on the display, their places and
#           sizes).
#   server  ./linux-server.sh with no display (headless: xvfb-run), on the
#           configuration it writes, with +MasterDomain 127.0.0.1 (the
#           modern build's default configuration is an Internet game: no
#           report to OpenSpy from a test); its console must have the world
#           loaded. Then ./linux-run.sh -gogame +join 127.0.0.1:27888
#           +gametype DiscArena +playername ScriptTest +ErrorLog 1 (in the
#           prefix of the run step, which has had its first run) must join:
#           the client's console (error.log) has its player connected.
# The game directory is the player's: the scripts write their settings,
# the prefixes and ServerData/ there and in HOME, as they do for a player.
# Writes, in out/play/<name>/ ($SCRIPTTEST_NAME, default
# scripttest-<game directory's name>): run.log, server.log, join.log
# (what the scripts and Wine printed), tronsrv.log (the server's console),
# client-error.log (the joined client's console),
# shots/ (the display after each step), display.cfg, registry.txt (the
# legacy build: the game's key in the prefix), drives.txt (the prefix's
# drives) and result.txt. Exit status 0 when every step passed.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
for t in wine Xvfb xvfb-run xdotool convert; do
    command -v $t >/dev/null 2>&1 || { echo "scripttest.sh: no $t here -- run it in the build container"; exit 2; }
done
abs() { case "$1" in /*) echo "$1" ;; *) echo "$REPO/$1" ;; esac; }
GAME="$(abs "${1:?usage: build/scripttest.sh <game directory> [run|server|both]}")"
WHAT="${2:-both}"
[ -x "$GAME/linux-run.sh" ] && [ -x "$GAME/linux-server.sh" ] || { echo "scripttest.sh: no executable linux-run.sh and linux-server.sh in $GAME"; exit 2; }
[ -f "$GAME/Game.rez" ] || { echo "scripttest.sh: $GAME has no game data: install it with its linux-install.sh first"; exit 2; }
MODERN=
[ -f "$GAME/TronLauncher.exe" ] && MODERN=1
NAME="${SCRIPTTEST_NAME:-scripttest-$(basename "$GAME")}"
RUN="$REPO/out/play/$NAME"
rm -rf "$RUN"; mkdir -p "$RUN/shots"
H="$(mktemp -d "${TMPDIR:-/tmp}/scripttest-home.XXXXXX")" || exit 2
unset WINEPREFIX
export HOME="$H" WINEDEBUG=-all
if [ ! -e /dev/snd ] && [ -f /usr/share/alsa/alsa.conf ]; then
    printf '</usr/share/alsa/alsa.conf>\npcm.!default { type null }\n' > "$RUN/asound.conf"
    export ALSA_CONFIG_PATH="$RUN/asound.conf"
fi
if [ -n "$MODERN" ]; then SCREEN="${SCRIPTTEST_SCREEN:-1280x720}"; else SCREEN="${SCRIPTTEST_SCREEN:-800x600}"; fi
SW=${SCREEN%x*} SH=${SCREEN#*x}
D=:83 XPID=
cleanup() {
    for p in "$H/.wine-tron20" "$H/.wine-tron20-server"; do
        [ -d "$p" ] && WINEPREFIX="$p" wineserver -k 2>/dev/null
    done
    [ -n "$XPID" ] && kill $XPID 2>/dev/null
    sleep 1; rm -rf "$H"
}
trap cleanup EXIT
N=0
shot() { N=$((N+1)); DISPLAY=$D xwd -root -silent 2>/dev/null | convert xwd:- "$(printf '%s/shots/%02d-%s.png' "$RUN" $N "$1")" 2>/dev/null; }
# waitwin <regex> <secs>: a visible window whose name matches
waitwin() {
    local i
    for i in $(seq 1 "$2"); do
        DISPLAY=$D xdotool search --onlyvisible --name "$1" 2>/dev/null | tail -1 | grep . && return 0
        sleep 1
    done
    return 1
}
running() { pgrep -if "$1" >/dev/null 2>&1; }
R=0
say() { echo "$*" | tee -a "$RUN/result.txt"; }

if [ "$WHAT" = run ] || [ "$WHAT" = both ] || [ "$WHAT" = server ]; then
    Xvfb $D -screen 0 "${SCREEN}x24" -nolisten tcp > "$RUN/xvfb.log" 2>&1 & XPID=$!
    sleep 2
fi

if [ "$WHAT" = run ] || [ "$WHAT" = both ]; then
    (cd "$GAME" && DISPLAY=$D exec ./linux-run.sh) > "$RUN/run.log" 2>&1 &
    if [ -z "$MODERN" ]; then
        # the launcher (a fresh prefix takes a while), then Play
        if w=$(waitwin 'TRON' 180); then
            sleep 3; shot launcher
            DISPLAY=$D xdotool windowfocus $w 2>/dev/null
            DISPLAY=$D xdotool mousemove --window $w 460 38 click 1
            sleep 4
        else
            say "RUN: no launcher window"
        fi
    else
        sleep 5; waitwin . 180 >/dev/null
        sleep 3
    fi
    # the detail setting, centred: Medium
    shot detail
    DISPLAY=$D xdotool mousemove $((SW / 2)) $((SH / 2 + 52)) click 1
    st=
    for i in $(seq 1 120); do running 'Lithtech\.exe' && { st=1; break; }; sleep 1; done
    if [ -n "$st" ]; then
        sleep 45; shot menu
        # the windows on the display: name, place and size
        for w in $(DISPLAY=$D xdotool search --onlyvisible --name . 2>/dev/null); do
            echo "$(DISPLAY=$D xdotool getwindowname $w 2>/dev/null): $(DISPLAY=$D xdotool getwindowgeometry $w 2>/dev/null | tr '\n' ' ')"
        done > "$RUN/windows.txt"
        if running 'Lithtech\.exe'; then say "RUN ALIVE: the engine runs; its window: $(grep -a '^TRON 2.0:' "$RUN/windows.txt" | tail -1)"
        else say "RUN EXITED: the engine stopped"; R=1; fi
        running '[Tt]ron\.exe' && say "RUN: Tron.exe waits for the engine" || say "RUN: Tron.exe has stopped"
    else
        shot nogame; say "RUN: the engine did not start"; R=1
    fi
    cp "$GAME/display.cfg" "$RUN/" 2>/dev/null
    cp "$GAME/launchcmds.txt" "$RUN/" 2>/dev/null
    export WINEPREFIX="$H/.wine-tron20"
    wine reg query 'HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0' /reg:32 > "$RUN/registry.txt" 2>&1
    ls -l "$WINEPREFIX/dosdevices/" > "$RUN/drives.txt" 2>&1
    wineserver -k 2>/dev/null; sleep 2
    unset WINEPREFIX
fi

if [ "$WHAT" = server ] || [ "$WHAT" = both ]; then
    rm -f "$GAME/tronsrv.log"
    (cd "$GAME" && unset DISPLAY && exec ./linux-server.sh +MasterDomain 127.0.0.1) > "$RUN/server.log" 2>&1 &
    up=
    for i in $(seq 1 180); do
        grep -aqiE "world.*(load|start)|loaded world|Server started" "$GAME/tronsrv.log" 2>/dev/null && { up=1; break; }
        sleep 1
    done
    [ -n "$up" ] && say "SERVER UP: its world is loaded" || { say "SERVER: no world after 180 s"; R=1; }
    running 'TRONSrv\.exe' || { say "SERVER EXITED"; R=1; }
    rm -f "$GAME/error.log"
    (cd "$GAME" && DISPLAY=$D exec ./linux-run.sh -gogame +join 127.0.0.1:27888 +gametype DiscArena \
        +playername ScriptTest +ErrorLog 1 +AlwaysFlushLog 1) > "$RUN/join.log" 2>&1 &
    joined=
    for i in $(seq 1 180); do
        grep -aq "MID_PLAYER_CONNECTED.*(ScriptTest)" "$GAME/error.log" 2>/dev/null && { joined=1; break; }
        sleep 1
    done
    sleep 20; shot joined
    cp "$GAME/tronsrv.log" "$RUN/" 2>/dev/null
    cp "$GAME/error.log" "$RUN/client-error.log" 2>/dev/null
    [ -n "$joined" ] && say "JOINED: the client's console has its player connected (ScriptTest)" ||
        { say "NOJOIN: the client's console has no MID_PLAYER_CONNECTED (ScriptTest)"; R=1; }
    running 'TRONSrv\.exe' && say "SERVER ALIVE" || { say "SERVER EXITED"; R=1; }
fi
montage -label '%f' -pointsize 9 "$RUN"/shots/*.png -tile 4x -geometry 400x300+2+2 "$RUN/shots.png" 2>/dev/null
exit $R
