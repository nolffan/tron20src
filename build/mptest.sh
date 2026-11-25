#!/usr/bin/env bash
# The multiplayer runtime test, for Tron 2.0: a server hosts a game and two
# clients join it, under Wine.
#
#   build/mptest.sh [gamedir] [seconds]     (in the build container;
#                                            `make mptest` runs it)
#
# gamedir: a game directory as build/all.sh assembles it (default
#          out/tron20-legacy-release); seconds: how long the clients play
#          after the last one joined (default 60).
#
# Settings, from the environment:
#   MPTEST_MODE=dedicated  (the default) the dedicated server, TRONSrv.exe,
#                          hosts the game; clients 1 and 2 join it
#   MPTEST_MODE=listen     client 1 hosts the game itself, from its menus
#                          as a player does: Multiplayer, Host (on the
#                          configuration below, which its profile names),
#                          Launch; client 2 joins it. (The command line's
#                          "+host <world>" stops at the loading screen, with
#                          the retail game DLLs too: the control)
#   MPTEST_TYPE=arena      the game type: arena (Disc Arena, the default),
#                          tourn (Disc Tournament), lc (Light Cycles)
#   MPTEST_WORLD=<world>   the world (default: the first of the type)
#   MPTEST_CLIENTS=2       how many clients join (1 or 2)
#   MPTEST_KEYS1/2="..."   what each client plays once it is in: keysyms,
#                          wN (wait N seconds), fire (mouse button 1),
#                          turn (the mouse to the right), click:X,Y (a
#                          click on the 640x480 screen); default: F1, a
#                          team (client 1 the first, client 2 the second),
#                          then a short walk, turns and discs thrown
#   MPTEST_LANONLY=0       a game that is not LAN only: the server reports
#                          to GameSpy's master server (gone: what it
#                          tries goes to wine-*.log with
#                          MPTEST_WINEDEBUG=+winsock)
#   MPTEST_WINEDEBUG=...   the WINEDEBUG of every process
#   MPTEST_SRVEXE=<file>  the dedicated server to run (default the game
#                          directory's TRONSrv.exe)
#   MPTEST_NAME=<name>     the run directory, out/play/<name>/ (default
#                          <gamedir's basename>-mp-<mode>-<type>)
#
# Each program runs in a directory of its own in out/play/<name>/ (srv/,
# c1/, c2/: binaries copied, data linked, as build/playtest.sh does), with
# a Wine prefix of its own (a copy of the container's, in /tmp, removed at
# the end: one game's DLLs, which it takes out of the rez files into the
# Temp folder, must not be written over by another's) and an X display of
# its own (Xvfb, 640x480: the client runs full screen and wants a display
# mode of its window's size). The registry gets what the game's setup and
# Update 2 write (HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0:
# InstallDir, NetRegionCode EN, UpdateNum 2, UpdateCommandLine0/1 "-rez
# gamep3.rez", "-rez gamep4.rez"); the dedicated server builds its rez stack
# from it (and puts custom/ last). The game is LAN only, so no client needs
# a CD key (the server checks none on a LAN game). The server's
# configuration is the file the host screen writes, ServerData/mptest.txt.
# Writes, in out/play/<name>/:
#   srv/tronsrv.log         the dedicated server's console (+errorlog)
#   c1/error.log, c2/...    each client's console (+ErrorLog 1)
#   wine-*.log              what Wine printed for each program
#   shots/                  each display after each step; shots.png
#   result.txt              per program ALIVE or EXITED; per client JOINED
#                           (its console has its player connected) and
#                           SEES <player> (its console has the others)
# and prints result.txt. Exit status 0 only when every program is alive
# and every client joined.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
for t in wine Xvfb xdotool convert montage; do
    command -v $t >/dev/null 2>&1 || { echo "mptest.sh: no $t here -- run it in the build container"; exit 2; }
done
abs() { case "$1" in /*) echo "$1" ;; *) echo "$REPO/$1" ;; esac; }
GAME="$(abs "${1:-out/tron20-legacy-release}")"
SECS="${2:-60}"
[ ! -f "$GAME/NOT-PLAYABLE.txt" ] || { cat "$GAME/NOT-PLAYABLE.txt"; exit 2; }
# MPTEST_SRVEXE=<file>: the dedicated server to run instead of the game
# directory's (the control, out/tron20-control, has none: retail's then)
SRVEXE="${MPTEST_SRVEXE:+$(abs "$MPTEST_SRVEXE")}"; SRVEXE="${SRVEXE:-$GAME/TRONSrv.exe}"
[ -f "$SRVEXE" ] || { echo "mptest.sh: no $SRVEXE"; exit 2; }
for f in Lithtech.exe server.dll tron20cmds.txt autoexec.cfg.retail Default.prf.retail; do
    [ -f "$GAME/$f" ] || { echo "mptest.sh: $GAME/$f missing"; exit 2; }
done
MODE="${MPTEST_MODE:-dedicated}"
TYPE="${MPTEST_TYPE:-arena}"
NCLIENTS="${MPTEST_CLIENTS:-2}"
# The game type: its number (NetDefs.h), its name (+gametype), its section
# in the configuration and its first world.
case "$TYPE" in
    arena) GT=5; GTNAME=DiscArena;              SEC=DiscArena;      DEFWORLD='Worlds\RetailMultiPlayer\DiscArena\DiscArena01' ;;
    tourn) GT=6; GTNAME=DiscTournament;         SEC=DiscTournament; DEFWORLD='Worlds\RetailMultiPlayer\DiscArena\DiscArena01' ;;
    lc)    GT=7; GTNAME=MultiplayerLightcycles; SEC=Lightcycles;    DEFWORLD='Worlds\RetailLightCycle\LC_01S01' ;;
    *)     echo "mptest.sh: MPTEST_TYPE is arena, tourn or lc"; exit 2 ;;
esac
case "$MODE" in dedicated|listen) ;; *) echo "mptest.sh: MPTEST_MODE is dedicated or listen"; exit 2 ;; esac
WORLD="${MPTEST_WORLD:-$DEFWORLD}"
NAME="${MPTEST_NAME:-$(basename "$GAME")-mp-$MODE-$TYPE}"
RUN="$REPO/out/play/$NAME"
rm -rf "$RUN"; mkdir -p "$RUN/shots"
REL="$(python3 -c 'import os,sys; print(os.path.relpath(sys.argv[1], sys.argv[2]))' "$GAME" "$RUN/srv")"

# The configuration the host screen writes (ServerGameOptions::SaveToBute):
# LAN only, port 27888, the one world. Light cycles: retail's light-cycle
# section is "Lightcycles" for the players and maps, "LightCycles" for the
# rest; ButeMgr reads both alike.
config() {
    printf '[Settings]\r\nServerName = "mptest"\r\nPassword = ""\r\nSCMDPassword = "mptest"\r\n'
    printf 'GameType = %s\r\nPort = 27888\r\nBandwidth = 3\r\nBandwidthCustom = 256\r\n' "$GT"
    printf 'Locked = 0\r\nDedicated = %s\r\nLANOnly = %s\r\nAllowSCMD = 0\r\n\r\n' "$1" "${MPTEST_LANONLY:-1}"
    printf '[%s]\r\nMaxPlayers = 8\r\nMap00 = "%s"\r\n' "$SEC" "$WORLD"
}
setup() {   # setup <dir>: binaries copied, data linked, the shipped settings
    mkdir -p "$1/Save" "$1/Profiles" "$1/ServerData"
    for f in "$GAME"/*; do
        b=$(basename "$f")
        case "$b" in autoexec.cfg|Save|Profiles|ServerData|tron20cmds.txt|launchcmds.txt|MANIFEST.txt) continue ;; esac
        if [ -f "$f" ] && [ "$(head -c 2 "$f" | tr -d '\0')" = MZ ]; then cp "$f" "$1/$b"
        else ln -s "$REL/$b" "$1/$b"; fi
    done
    cp "$GAME/autoexec.cfg.retail" "$1/autoexec.cfg"
    cp "$GAME/Default.prf.retail" "$1/Profiles/Default.prf"
}
CMDS="$(tr -d '\r\n' < "$GAME/tron20cmds.txt") ++screenwidth 640 ++screenheight 480 ++bitdepth 32 ++windowed 0 +DisableMovies 1 +ErrorLog 1 +AlwaysFlushLog 1"
setup "$RUN/srv"; cp "$SRVEXE" "$RUN/srv/TRONSrv.exe"; config 1 > "$RUN/srv/ServerData/mptest.txt"
for n in 1 2; do
    setup "$RUN/c$n"; config 0 > "$RUN/c$n/ServerData/mptest.txt"
done
if [ "$MODE" = listen ]; then
    printf '%s +playername Host\r\n' "$CMDS" > "$RUN/c1/tron20cmds.txt"
else
    printf '%s +playername Player1 +join 127.0.0.1:27888 +gametype %s\r\n' "$CMDS" "$GTNAME" > "$RUN/c1/tron20cmds.txt"
fi
printf '%s +playername Player2 +join 127.0.0.1:27888 +gametype %s\r\n' "$CMDS" "$GTNAME" > "$RUN/c2/tron20cmds.txt"
# The host's configuration is the one its profile names (Profiles/
# Default.prf, [Multiplayer] ServerConfig, "Default" as shipped): ours.
sed -i 's/^ServerConfig = "Default"/ServerConfig = "mptest"/' "$RUN/c1/Profiles/Default.prf"

if [ ! -e /dev/snd ] && [ -f /usr/share/alsa/alsa.conf ]; then
    printf '</usr/share/alsa/alsa.conf>\npcm.!default { type null }\n' > "$RUN/asound.conf"
    export ALSA_CONFIG_PATH="$RUN/asound.conf"
fi
export WINEDEBUG="${MPTEST_WINEDEBUG:--all,+seh,+msgbox}" WINEDLLOVERRIDES="winedbg.exe=d"
SRC_PREFIX="${WINEPREFIX:-$HOME/.wine}"
TMPP="$(mktemp -d "${TMPDIR:-/tmp}/mptest.XXXXXX")" || exit 2
PIDS=""
cleanup() {
    for p in srv c1 c2; do
        [ -d "$TMPP/$p" ] && WINEPREFIX="$TMPP/$p" wineserver -k 2>/dev/null
    done
    for p in $PIDS; do kill $p 2>/dev/null; done
    sleep 1; rm -rf "$TMPP"
}
trap cleanup EXIT
# prefix <who> <display>: the prefix and display of srv, c1 or c2
D_srv=:71 D_c1=:72 D_c2=:73
for p in srv c1 c2; do
    mkdir -p "$TMPP/$p"
    if [ -f "$SRC_PREFIX/system.reg" ]; then
        cp -a "$SRC_PREFIX/." "$TMPP/$p/" || exit 2
        rm -rf "$TMPP/$p"/drive_c/users/*/Temp/*
    fi
    eval "d=\$D_$p"
    Xvfb $d -screen 0 640x480x24 -nolisten tcp > "$RUN/xvfb-$p.log" 2>&1 & PIDS="$PIDS $!"
done
sleep 2
w() {   # w <who> <command...>: run in that prefix, display and directory
    local p=$1; shift; eval "local d=\$D_$p"
    (cd "$RUN/$p" && WINEPREFIX="$TMPP/$p" DISPLAY=$d "$@")
}
for p in srv c1 c2; do
    w $p wineboot -i >/dev/null 2>&1
    K='HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0'
    w $p wine reg add "$K" /v InstallDir /t REG_SZ /d "Z:$(echo "$RUN/$p" | tr / '\\')" /f >/dev/null 2>&1
    w $p wine reg add "$K" /v NetRegionCode /t REG_SZ /d EN /f >/dev/null 2>&1
    w $p wine reg add "$K" /v UpdateNum /t REG_DWORD /d 2 /f >/dev/null 2>&1
    w $p wine reg add "$K" /v UpdateCommandLine0 /t REG_SZ /d "-rez gamep3.rez" /f >/dev/null 2>&1
    w $p wine reg add "$K" /v UpdateCommandLine1 /t REG_SZ /d "-rez gamep4.rez" /f >/dev/null 2>&1
done

T0=$(date +%s); N=0
log() { echo "$(( $(date +%s) - T0 ))s $*" >> "$RUN/steps.log"; }
shot() {   # shot <who> <name>
    eval "local d=\$D_$1"; N=$((N+1))
    DISPLAY=$d xwd -root -silent 2>/dev/null | convert xwd:- "$(printf '%s/shots/%02d-%s-%s.png' "$RUN" $N "$1" "$2")" 2>/dev/null
}
alive() { kill -0 "$1" 2>/dev/null; }
# waitlog <file> <regex> <secs> <pid>: until the regex is in the file
waitlog() {
    local i
    for i in $(seq 1 "$3"); do
        grep -aqiE "$2" "$1" 2>/dev/null && return 0
        alive "$4" || return 1
        sleep 1
    done
    return 1
}
# play <who> <keys>: keys to that client's window
play() {
    local p=$1 k win; eval "local d=\$D_$p"
    win=$(DISPLAY=$d xdotool search --onlyvisible --name '^LithTech$' 2>/dev/null | tail -1)
    for k in $2; do
        case $k in
            w[0-9]*) sleep ${k#w} ;;
            click:*) local xy=${k#click:}
               DISPLAY=$d xdotool mousemove ${xy%,*} ${xy#*,}; sleep 0.3
               DISPLAY=$d xdotool click 1; sleep 3; shot $p menu ;;
            fire) DISPLAY=$d xdotool mousedown 1; sleep 0.3; DISPLAY=$d xdotool mouseup 1; sleep 1 ;;
            turn) DISPLAY=$d xdotool mousemove_relative -- 80 0; sleep 0.5 ;;
            *) [ -n "$win" ] && DISPLAY=$d xdotool windowfocus $win 2>/dev/null
               DISPLAY=$d xdotool keydown $k; sleep 0.4; DISPLAY=$d xdotool keyup $k; sleep 0.5 ;;
        esac
    done
    shot $p played-$p
}

S=""; C1=""; C2=""
if [ "$MODE" = dedicated ]; then
    w srv wine TRONSrv.exe -config mptest +errorlog 1 +alwaysflushlog 1 > "$RUN/wine-srv.log" 2>&1 & S=$!
    log "server started"
    # the world is loaded once the server's console says so
    waitlog "$RUN/srv/tronsrv.log" "world.*(load|start)|loaded world|Server started" 120 $S \
        && log "server: world up" || log "server: no world after 120s"
    sleep 5; shot srv up
    HOSTLOG="$RUN/srv/tronsrv.log"
fi
w c1 wine Lithtech.exe -cmdfile tron20cmds.txt > "$RUN/wine-c1.log" 2>&1 & C1=$!
log "client 1 started ($MODE)"
if [ "$MODE" = listen ]; then
    # the main menu, then Multiplayer, Host, Launch (the menus' 640x480 page)
    sleep 40; shot c1 menu
    play c1 "click:248,218 click:30,268 click:565,456"
    log "client 1: host launched"
fi
sleep 60; shot c1 in
if [ "$NCLIENTS" -ge 2 ]; then
    w c2 wine Lithtech.exe -cmdfile tron20cmds.txt > "$RUN/wine-c2.log" 2>&1 & C2=$!
    log "client 2 started"
    sleep 60; shot c2 in; shot c1 c2-in
fi
# Each player joins a team (F1: the team screen; client 1 the first team,
# client 2 the second), waits for the match to start, then moves, turns
# and throws discs
K1="${MPTEST_KEYS1:-F1 w3 click:40,100 w30 w a w d fire turn fire turn fire w a fire}"
K2="${MPTEST_KEYS2:-F1 w3 click:40,142 w30 s w fire turn fire turn turn fire d fire}"
alive "$C1" && play c1 "$K1" & P1=$!
[ -n "$C2" ] && { alive "$C2" && play c2 "$K2" & P2=$!; }
wait $P1 ${P2:-} 2>/dev/null
log "played"
for i in $(seq 1 "$SECS"); do
    sleep 1
    for p in $S $C1 $C2; do alive $p || break 2; done
done
shot c1 end; [ -n "$C2" ] && shot c2 end; [ -n "$S" ] && shot srv end
# Tab: the scores, on each client
for p in c1 c2; do
    eval "pid=\$C${p#c}"; [ -n "$pid" ] && alive $pid || continue
    eval "d=\$D_$p"
    win=$(DISPLAY=$d xdotool search --onlyvisible --name '^LithTech$' 2>/dev/null | tail -1)
    [ -n "$win" ] && DISPLAY=$d xdotool windowfocus $win 2>/dev/null
    DISPLAY=$d xdotool keydown Tab; sleep 1; shot $p scores; DISPLAY=$d xdotool keyup Tab
done
{
    [ -n "$S" ] && { alive $S && echo "SERVER ALIVE" || echo "SERVER EXITED"; }
    alive $C1 && echo "CLIENT1 ALIVE" || echo "CLIENT1 EXITED"
    [ -n "$C2" ] && { alive $C2 && echo "CLIENT2 ALIVE" || echo "CLIENT2 EXITED"; }
} > "$RUN/result.txt"
montage -label '%f' -pointsize 9 "$RUN"/shots/*.png -tile 4x -geometry 320x240+2+2 "$RUN/shots.png" 2>/dev/null
# Joined: each client's console says that its player came in
# (MID_PLAYER_CONNECTED) and names the other player (MID_PLAYER_INFO)
J=1
for n in $(seq 1 "$NCLIENTS"); do
    me=Player$n; [ "$MODE" = listen ] && [ $n = 1 ] && me=Host
    if grep -aq "MID_PLAYER_CONNECTED.*($me)" "$RUN/c$n/error.log" 2>/dev/null; then
        echo "CLIENT$n JOINED as $me" >> "$RUN/result.txt"
    else
        echo "CLIENT$n NOJOIN" >> "$RUN/result.txt"; J=0
    fi
    for o in $(seq 1 "$NCLIENTS"); do
        [ $o = $n ] && continue
        other=Player$o; [ "$MODE" = listen ] && [ $o = 1 ] && other=Host
        grep -aq "MID_PLAYER_INFO.*($other)" "$RUN/c$n/error.log" 2>/dev/null \
            && echo "CLIENT$n SEES $other" >> "$RUN/result.txt" \
            || { echo "CLIENT$n DOES NOT SEE $other" >> "$RUN/result.txt"; J=0; }
    done
done
grep -h 'msgbox\|EXCEPTION_ACCESS\|c0000005\|Unhandled' "$RUN"/wine-*.log 2>/dev/null | head -5 >> "$RUN/result.txt"
cat "$RUN/result.txt"
[ $J = 1 ] && ! grep -q EXITED "$RUN/result.txt"
