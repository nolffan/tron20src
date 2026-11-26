#!/bin/sh
# linux-server.sh -- run a Tron 2.0 dedicated server (TRONSrv.exe, built
# from source) with Wine, from this directory (the game directory: run
# ./linux-install.sh first).
#
#   ./linux-server.sh [options] [TRONSrv.exe args...]
#
# Options:
#   --config <name>  host with the server configuration ServerData/<name>.txt
#                    (the game type, the worlds, the players, the port, LAN
#                    only, the password). The game's Multiplayer > Host
#                    screens save a configuration there, under the
#                    profile's name (the game writes Default.txt, with no
#                    worlds, on its first run). The default: server, which
#                    this script writes if it is not there (below).
#   --choose         (on a display) TRONSrv.exe's own dialog: choose one of
#                    the configurations in ServerData/
#   --headless       run without a display: under xvfb-run, a virtual X
#                    server (the default when $DISPLAY is not set)
#   --help           this text
# Anything else goes to TRONSrv.exe, after this script's arguments:
#   -port <port>             the UDP port (replaces the configuration's)
#   -name <name>             the server's name in the server lists
#   +MasterDomain <domain>   (the modern build) the master server's domain
#                            (default openspy.net; MODERN.md GS-1)
#   +errorlogfile <file>     another log file
# For example:
#   ./linux-server.sh --config server -name "My server" -port 27888
#
# The default configuration (ServerData/server.txt, when the script writes
# it): Disc Arena, 8 players, port 27888, the five Disc Arena worlds, no
# password, no remote server control. In the modern build it is an
# Internet game (LANOnly = 0: the server reports to the master server,
# OpenSpy by default, and plays on the LAN too; MODERN.md GS-1, GS-4). In
# the legacy build it is LAN only (LANOnly = 1): the retail server reports
# to GameSpy's master server, which is gone, and checks each player's CD
# key with GameSpy. Players join it with its address (the game's
# Multiplayer > Join, or ./linux-run.sh +join <address>:27888 +gametype
# DiscArena). Change the file with a text editor, or make other
# configurations with the game's Host screens.
#
# Environment:
#   WINEPREFIX   the Wine prefix (default: $HOME/.wine-tron20, the prefix of
#                linux-run.sh; headless, $HOME/.wine-tron20-server: the
#                Wine desktop of a prefix is on one display, so a game in a
#                prefix whose programs are on the server's virtual display
#                would open there)
#   WINE         the wine binary (default: wine)
#   WINESERVER   the wineserver binary (default: wineserver)
#   WINEDEBUG    Wine's log channels (default: -all)
#
# The server runs in this directory and writes its console to tronsrv.log
# (emptied each start: this script gives +errorlog 1 +alwaysflushlog 1).
# The legacy build's TRONSrv.exe takes the archives of Update 2 from the
# registry values that the retail setup and Update 2 wrote, and stops
# without them: this script writes them into the prefix, as linux-run.sh
# does (HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0: InstallDir,
# UpdateNum 1, UpdateCommandLine0 "-rez tron20src.rez": Update 2's data, in
# the place of the update's two archives). The modern build's needs none
# (MODERN.md APPS-3). The server answers on UDP port 27888 (or the
# configuration's, or -port): open it in a firewall for players on other
# machines, and for an Internet game forward it on your router to this
# machine. The server runs until you click Stop Server, or you stop the
# process (Ctrl+C here).
set -e
cd "$(dirname "$0")"
# a game directory built without the retail game (NODISCS=1) does not play
if [ -f NOT-PLAYABLE.txt ]; then cat NOT-PLAYABLE.txt >&2; exit 1; fi

CONFIG=
CHOOSE=
HEADLESS=
case "${DISPLAY:-}" in "") HEADLESS=1 ;; esac
[ -z "${TRON20SRC_XVFB:-}" ] || HEADLESS=1     # (this script again, under xvfb-run)
while [ $# -gt 0 ]; do
    case "$1" in
        --config)   CONFIG="${2:?--config needs a configuration name}"; CONFIG="${CONFIG%.txt}"; shift ;;
        --headless) HEADLESS=1 ;;
        --choose)   CHOOSE=1 ;;
        --help|-h)  sed -n '2,/^set -e$/{/^set -e$/d;p}' "./${0##*/}" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)          break ;;
    esac
    shift
done
[ -z "$CHOOSE" ] || [ -z "$HEADLESS" ] || { echo "linux-server.sh: --choose needs a display: name a configuration with --config" >&2; exit 1; }

if [ -n "$HEADLESS" ]; then
    WINEPREFIX="${WINEPREFIX:-$HOME/.wine-tron20-server}"
else
    WINEPREFIX="${WINEPREFIX:-$HOME/.wine-tron20}"
fi
WINEDEBUG="${WINEDEBUG:--all}"
export WINEPREFIX WINEDEBUG
WINE="${WINE:-wine}"
WINESERVER="${WINESERVER:-wineserver}"
command -v "$WINE" >/dev/null 2>&1 || { echo "linux-server.sh: '$WINE' not found -- install Wine (32-bit support needed)" >&2; exit 1; }
[ -f Game.rez ] || { echo "linux-server.sh: the game's data is not here yet -- run ./linux-install.sh first" >&2; exit 1; }
if [ -n "$HEADLESS" ]; then
    command -v xvfb-run >/dev/null 2>&1 || { echo "linux-server.sh: no display, and no xvfb-run (install xvfb) to make one" >&2; exit 1; }
    # The whole script again on a virtual display: Wine's setup of the
    # prefix, the registry and the server all on it
    if [ -z "${TRON20SRC_XVFB:-}" ]; then
        [ -n "$CONFIG" ] && set -- --config "$CONFIG" "$@"
        TRON20SRC_XVFB=1; export TRON20SRC_XVFB
        exec xvfb-run -a "./${0##*/}" "$@"
    fi
fi
MODERN=
[ -f TronLauncher.exe ] && MODERN=1

# The configuration: named, or server (written now if it is not there);
# with --choose, none (TRONSrv.exe asks, which needs a display)
mkdir -p ServerData
if [ -n "$CHOOSE" ]; then
    CONFIG=
elif [ -z "$CONFIG" ]; then
    CONFIG=server
elif [ ! -f "ServerData/$CONFIG.txt" ] && [ "$CONFIG" != server ]; then
    echo "linux-server.sh: no configuration ServerData/$CONFIG.txt; there are:" >&2
    ls ServerData/*.txt 2>/dev/null | sed 's|^ServerData/||; s|\.txt$||; s|^|    |' >&2
    exit 1
fi
if [ -n "$CONFIG" ] && [ ! -f "ServerData/$CONFIG.txt" ]; then
    # (the format of the game's Host screens: ServerGameOptions::SaveToBute)
    LANONLY=1
    [ -n "$MODERN" ] && LANONLY=0
    printf '[Settings]\r\nServerName = "Tron 2.0 server"\r\nPassword = ""\r\nSCMDPassword = ""\r\n' > "ServerData/$CONFIG.txt.part"
    printf 'GameType = 5\r\nPort = 27888\r\nBandwidth = 3\r\nBandwidthCustom = 256\r\n' >> "ServerData/$CONFIG.txt.part"
    printf 'Locked = 0\r\nDedicated = 1\r\nLANOnly = %s\r\nAllowSCMD = 0\r\n\r\n' "$LANONLY" >> "ServerData/$CONFIG.txt.part"
    printf '[DiscArena]\r\nMaxPlayers = 8\r\n' >> "ServerData/$CONFIG.txt.part"
    for n in 1 2 3 4 5; do
        printf 'Map0%s = "Worlds\\RetailMultiPlayer\\DiscArena\\DiscArena0%s"\r\n' $((n - 1)) $n >> "ServerData/$CONFIG.txt.part"
    done
    mv "ServerData/$CONFIG.txt.part" "ServerData/$CONFIG.txt"
    echo "linux-server.sh: wrote ServerData/$CONFIG.txt (Disc Arena, port 27888$([ $LANONLY = 1 ] && echo ', LAN only'))" >&2
fi
[ -n "$CONFIG" ] && set -- -config "$CONFIG" "$@"

# a prefix to set up: make it now if it does not exist yet (without Wine's
# offers to install Mono and Gecko, which the game does not use)
[ -d "$WINEPREFIX/dosdevices" ] || WINEDLLOVERRIDES="mscoree,mshtml=" "$WINE" wineboot -i >/dev/null 2>&1 || true
# The legacy build's server stops without the setup's key, and takes
# Update 2's data, tron20src.rez, from its values (custom/ after it)
if [ -z "$MODERN" ]; then
    DIR="$("$WINE" winepath -w "$(pwd)" 2>/dev/null)"
    KEY='HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0'
    reg() { "$WINE" reg add "$@" /reg:32 /f >/dev/null 2>&1
            "$WINE" reg add "$@" /reg:64 /f >/dev/null 2>&1 || true; }
    unreg() { "$WINE" reg delete "$@" /reg:32 /f >/dev/null 2>&1 || true
              "$WINE" reg delete "$@" /reg:64 /f >/dev/null 2>&1 || true; }
    reg "$KEY" /v InstallDir         /t REG_SZ    /d "$DIR"
    reg "$KEY" /v NetRegionCode      /t REG_SZ    /d EN
    reg "$KEY" /v UpdateNum          /t REG_DWORD /d 1
    reg "$KEY" /v UpdateCommandLine0 /t REG_SZ    /d '-rez tron20src.rez'
    unreg "$KEY" /v UpdateCommandLine1
fi

exec "$WINE" TRONSrv.exe +errorlog 1 +alwaysflushlog 1 "$@"
