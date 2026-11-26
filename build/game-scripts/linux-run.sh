#!/bin/sh
# linux-run.sh -- start Tron 2.0 (built from source) with Wine, from this
# directory (the game directory: run ./linux-install.sh first).
#
#   ./linux-run.sh [game args...]           the game
#   ./linux-run.sh --launcher [args...]     (the modern build) the launcher
#   ./linux-run.sh --help                   this text
#
# The modern build: Tron.exe starts the game at once, with the launcher's
# saved settings, as its Play button does, and stays until Lithtech.exe
# stops (MODERN.md APPS-1, APPS-2). On the first run it asks for the
# detail setting once. --launcher starts TronLauncher.exe, the launcher
# with its Display and Options dialogs, in the same Wine prefix.
# The legacy build: Tron.exe is the retail launcher. Click Play (on the
# first run, choose a detail setting). Its Display and Options buttons set
# the renderer, the resolution and the engine's options.
#
# The launcher starts Lithtech.exe with the game's archives, and gives the
# arguments of this script to the engine after its own (later ones win).
# Arguments that the engine and the game know, for example:
#   ./linux-run.sh ++windowed 0                  full screen
#   ./linux-run.sh ++windowed 1                  a window
#   ./linux-run.sh ++windowed 1 ++WindowMode 2   (the modern build) a
#                                                borderless window over the
#                                                whole desktop
#   ./linux-run.sh +ScreenWidth 1280 +ScreenHeight 720   the resolution
#   ./linux-run.sh +DisableSound 1               no sound
#   ./linux-run.sh +DisableMusic 1               no music
#   ./linux-run.sh +DisableMovies 1              no logo movies at the start
#   ./linux-run.sh -gogame                       (the legacy build) start the
#                                                game at once, as Play does,
#                                                without the launcher window
#   ./linux-run.sh +join <address>:27888 +gametype DiscArena
#                  join a multiplayer game (the game type: DiscArena,
#                  DiscTournament or MultiplayerLightcycles; +password <x>
#                  for a game with a password)
#   ./linux-run.sh +MasterDomain <domain>        (the modern build) the
#                  master server's domain for the Internet list (default
#                  openspy.net; MODERN.md GS-1)
# With "++" the engine keeps the value in autoexec.cfg for the next runs;
# with "+" only for this run.
#
# The display. The modern build starts as it was last left (Options >
# Display or the launcher's Display: full screen, a window or a borderless
# window, and the resolution: display.cfg). On a fresh install (no
# display.cfg, and no Windowed in autoexec.cfg) the launcher has no
# window mode to give, and the engine would start full screen at the
# detail setting's resolution (800x600 for Medium), which changes the mode
# of the display. There this script adds "+Windowed 1 +WindowMode 2": a
# borderless window over the desktop, at the desktop's resolution, with no
# change of mode. On a Steam Deck (SteamDeck=1, which Steam sets) it adds
# full screen at the panel's 1280x800 ("+Windowed 0 +ScreenWidth 1280
# +ScreenHeight 800 +BitDepth 32"). The game writes its choice to
# display.cfg, so later runs keep it, and Options > Display changes it.
# Arguments that give Windowed or WindowMode replace this default.
# The legacy build's Display has no window mode: the script adds
# "++windowed 1" (a window at the resolution of Display or of the detail
# setting) before your arguments, so "./linux-run.sh ++windowed 0" still
# gives full screen.
#
# Environment:
#   WINEPREFIX   the Wine prefix (default: $HOME/.wine-tron20, made on the
#                first run, so that your own prefix is left alone: the
#                legacy build writes registry values and a disc drive into
#                it, and the game copies its DLLs into the prefix's Temp
#                folder)
#   WINE         the wine binary (default: wine)
#   WINESERVER   the wineserver binary (default: wineserver)
#   WINEDEBUG    Wine's log channels (default: fixme-all)
#
# The modern build reads nothing from the registry: its settings are in
# tron20.ini in this directory (MODERN.md APPS-3), and this script writes
# nothing into the prefix for it. The legacy build's Tron.exe is the
# retail launcher. It reads what the retail setup and Update 2 wrote into
# the registry, and it wants disc 1 in a CD-ROM drive. This script writes
# both into the prefix, each time:
#   * HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0: InstallDir (this
#     directory), NetRegionCode EN, UpdateNum 1 and UpdateCommandLine0
#     ("-rez tron20src.rez": Update 2's data, in the place of the update's
#     -rez gamep3.rez and -rez gamep4.rez; UpdateCommandLine1 removed),
#     and "CdRom Drive" (the drive below). In both registry views: a
#     32-bit program reads HKLM\SOFTWARE through the 32-bit one.
#   * A CD-ROM drive (the first free letter from T:) on the folder
#     tron20-disc1 in the prefix, with an empty TRON.EXE in it: the
#     launcher looks only for that file. The game data is in this
#     directory (linux-install.sh), not on the disc.
#
# Sound: the engine's SndDrv.dll plays through DirectSound, so it needs
# Wine's audio working on this machine (winecfg > Audio); +DisableSound 1
# runs without it. The speech and the music waves are MP3. The modern
# build's SndDrv.dll decodes MP3 itself (MODERN.md X-2). The legacy
# build's uses the MP3 codec of the system: under Wine, Wine's own
# (l3codeca.acm), which plays the speech (tested with Wine 9.0). If the
# speech of the legacy build plays wrong with your Wine (as a drone of
# its first samples: Contract J.A.C.K.'s retail sound driver did that
# with some lines), install Fraunhofer's codec, which Windows has, into
# the prefix once:
#   WINEPREFIX=~/.wine-tron20 winetricks l3codecx
# The music is DirectMusic: under Wine it is silent for now (MODERN.md
# X-3, open).
#
# Movies: the three logo movies at the start play with Bink (the legacy
# build: RAD's binkw32.dll, which plays them under Wine 9.0; the modern
# build: our player on FFmpeg, MODERN.md X-1). +DisableMovies 1 leaves
# them out.
set -e
cd "$(dirname "$0")"
# a game directory built without the retail game (NODISCS=1) does not play
if [ -f NOT-PLAYABLE.txt ]; then cat NOT-PLAYABLE.txt >&2; exit 1; fi
case "${1:-}" in
--help|-h) sed -n '2,/^set -e$/{/^set -e$/d;p}' "./${0##*/}" | sed 's/^# \{0,1\}//'; exit 0 ;;
esac
# The modern build has the launcher twice: TronLauncher.exe and Tron.exe
MODERN=
[ -f TronLauncher.exe ] && MODERN=1
EXE=Tron.exe
if [ "${1:-}" = --launcher ]; then
    shift
    [ -n "$MODERN" ] && EXE=TronLauncher.exe
fi

WINEPREFIX="${WINEPREFIX:-$HOME/.wine-tron20}"
WINEDEBUG="${WINEDEBUG:-fixme-all}"
export WINEPREFIX WINEDEBUG
WINE="${WINE:-wine}"
WINESERVER="${WINESERVER:-wineserver}"
command -v "$WINE" >/dev/null 2>&1 || { echo "linux-run.sh: '$WINE' not found -- install Wine (32-bit support needed)" >&2; exit 1; }
[ -f Game.rez ] || { echo "linux-run.sh: the game's data is not here yet -- run ./linux-install.sh first" >&2; exit 1; }

# The shipped settings, if there are none (linux-install.sh writes them)
[ -f autoexec.cfg ] || cp autoexec.cfg.retail autoexec.cfg
[ -f Profiles/Default.prf ] || { mkdir -p Profiles; cp Default.prf.retail Profiles/Default.prf; }
mkdir -p Save

# a prefix to set up: make it now if it does not exist yet (without Wine's
# offers to install Mono and Gecko, which the game does not use)
[ -d "$WINEPREFIX/dosdevices" ] || WINEDLLOVERRIDES="mscoree,mshtml=" "$WINE" wineboot -i >/dev/null 2>&1 || true

if [ -z "$MODERN" ]; then
    # reg <args>: in both registry views (in a 64-bit prefix `wine reg`
    # writes the 64-bit one, while Tron.exe reads the 32-bit one,
    # Wow6432Node; in a 32-bit prefix there is one view, and the /reg:64
    # write fails, harmlessly)
    reg() { "$WINE" reg add "$@" /reg:32 /f >/dev/null 2>&1
            "$WINE" reg add "$@" /reg:64 /f >/dev/null 2>&1 || true; }
    unreg() { "$WINE" reg delete "$@" /reg:32 /f >/dev/null 2>&1 || true
              "$WINE" reg delete "$@" /reg:64 /f >/dev/null 2>&1 || true; }
    # Disc 1: a folder in the prefix with an empty TRON.EXE, as a CD-ROM
    # drive; the drive this script made before, else the first free letter
    DISC="$WINEPREFIX/tron20-disc1"
    mkdir -p "$DISC"
    [ -f "$DISC/TRON.EXE" ] || : > "$DISC/TRON.EXE"
    CD= NEWCD=
    for l in t u v w x y s r q p o n m l k j i h g f e d; do
        if [ "$(readlink "$WINEPREFIX/dosdevices/$l:" 2>/dev/null)" = "$DISC" ]; then CD=$l; break; fi
    done
    if [ -z "$CD" ]; then
        for l in t u v w x y s r q p o n m l k j i h g f e d; do
            if [ ! -e "$WINEPREFIX/dosdevices/$l:" ] && [ ! -L "$WINEPREFIX/dosdevices/$l:" ]; then CD=$l; break; fi
        done
        [ -n "$CD" ] || { echo "linux-run.sh: no free drive letter in $WINEPREFIX for the disc" >&2; exit 1; }
        ln -s "$DISC" "$WINEPREFIX/dosdevices/$CD:"
        NEWCD=1
    fi
    reg 'HKLM\Software\Wine\Drives' /v "$CD:" /t REG_SZ /d cdrom
    DIR="$("$WINE" winepath -w "$(pwd)" 2>/dev/null)"
    KEY='HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0'
    reg "$KEY" /v InstallDir         /t REG_SZ    /d "$DIR"
    reg "$KEY" /v NetRegionCode      /t REG_SZ    /d EN
    reg "$KEY" /v UpdateNum          /t REG_DWORD /d 1
    reg "$KEY" /v UpdateCommandLine0 /t REG_SZ    /d '-rez tron20src.rez'
    unreg "$KEY" /v UpdateCommandLine1
    reg "$KEY" /v 'CdRom Drive'      /t REG_SZ    /d "$CD:"
    # Wine takes a new drive's type when it starts again
    [ -z "$NEWCD" ] || "$WINESERVER" -w 2>/dev/null || true
fi

# The display (the header)
DISPLAYARGS=
if [ -z "$MODERN" ]; then
    DISPLAYARGS="++windowed 1"
elif [ ! -f display.cfg ] && ! grep -qi '"windowed"' autoexec.cfg 2>/dev/null; then
    case " $* " in
    *[Ww][Ii][Nn][Dd][Oo][Ww][Ee][Dd]*|*[Ww][Ii][Nn][Dd][Oo][Ww][Mm][Oo][Dd][Ee]*) ;;
    *)  if [ "${SteamDeck:-0}" = 1 ]; then
            DISPLAYARGS="+Windowed 0 +ScreenWidth 1280 +ScreenHeight 800 +BitDepth 32"
        else
            DISPLAYARGS="+Windowed 1 +WindowMode 2"
        fi ;;
    esac
fi

exec "$WINE" "$EXE" $DISPLAYARGS "$@"
