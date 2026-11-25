#!/usr/bin/env bash
# The launcher test, for Tron 2.0: our Tron.exe on a Wine prefix with the
# registry values that the game's setup and Update 2 write; its buttons,
# its dialogs, and Launch, which must start our Lithtech.exe to the main
# menu.
#
#   build/launchertest.sh [gamedir] [script]   (in the build container;
#                                               `make launchertest` runs it)
#
# gamedir: default out/tron20-legacy-release; script: the input script
# (default build/playtest-launcher.txt). The run is build/playtest.sh's,
# with PLAYTEST_LAUNCH=Tron.exe on a display of 800x600 (the mode
# the Medium detail setting picks, so that the game gets it full screen), in
# out/play/<name>/ ($PLAYTEST_NAME, default test-launcher). The Wine prefix
# is a copy of the container's in /tmp, with
# HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0: InstallDir (the run
# directory: the launcher changes to it), NetRegionCode EN, UpdateNum 2,
# UpdateCommandLine0/1 "-rez gamep3.rez" / "-rez gamep4.rez", and a
# CD-ROM drive D: with disc 1's TRON.EXE (an empty file: the launcher only
# looks for it). Then writes,
# in the run directory:
#   registry.txt    that key after the run: what the launcher stored
#                   (the legacy launcher keeps retail's place for its
#                   settings, the registry)
#   launchcmds.txt  the command line the launcher gave the engine (its
#                   own file, as retail's)
# and prints playtest.sh's result.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
abs() { case "$1" in /*) echo "$1" ;; *) echo "$REPO/$1" ;; esac; }
GAME="$(abs "${1:-out/tron20-legacy-release}")"
SCRIPT="${2:-build/playtest-launcher.txt}"
export PLAYTEST_NAME="${PLAYTEST_NAME:-test-launcher}"
RUN="$REPO/out/play/$PLAYTEST_NAME"
SRC_PREFIX="${WINEPREFIX:-$HOME/.wine}"
P="$(mktemp -d "${TMPDIR:-/tmp}/launcher-prefix.XXXXXX")" || exit 2
trap 'WINEPREFIX="$P" wineserver -k 2>/dev/null; rm -rf "$P"' EXIT
if [ -f "$SRC_PREFIX/system.reg" ]; then
    cp -a "$SRC_PREFIX/." "$P/" || exit 2
    rm -rf "$P"/drive_c/users/*/Temp/*
fi
export WINEPREFIX="$P" WINEDEBUG=-all
K='HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0'
wine reg add "$K" /v InstallDir /t REG_SZ /d "Z:$(echo "$RUN" | tr / '\\')" /f >/dev/null 2>&1
wine reg add "$K" /v NetRegionCode /t REG_SZ /d EN /f >/dev/null 2>&1
wine reg add "$K" /v UpdateNum /t REG_DWORD /d 2 /f >/dev/null 2>&1
wine reg add "$K" /v UpdateCommandLine0 /t REG_SZ /d "-rez gamep3.rez" /f >/dev/null 2>&1
wine reg add "$K" /v UpdateCommandLine1 /t REG_SZ /d "-rez gamep4.rez" /f >/dev/null 2>&1
# Disc 1 in drive D: (the launcher's Play looks for \TRON.EXE on a CD-ROM
# drive, as retail's does): a directory, with an empty TRON.EXE, that Wine
# takes for a CD-ROM drive
mkdir -p "$P/disc1" && : > "$P/disc1/TRON.EXE"
ln -sfn "$P/disc1" "$P/dosdevices/d:"
wine reg add 'HKLM\Software\Wine\Drives' /v d: /t REG_SZ /d cdrom /f >/dev/null 2>&1
wineserver -w
# LAUNCHERTEST_RETAIL=1: the retail Tron.exe instead of ours, for the
# control (it has no copy protection; it runs from a directory of its own,
# and so starts the run directory's engine, ours: retail's Lithtech.exe does
# not run under Wine)
if [ -n "${LAUNCHERTEST_RETAIL:-}" ]; then
    source "$REPO/build/retail.sh"
    mkdir -p "$P/retail" && cp "$(retail_dir)/Tron.exe" "$P/retail/Tron.exe" || exit 2
    PLAYTEST_LAUNCH="$P/retail/Tron.exe"
fi
export PLAYTEST_PREFIX="$P" PLAYTEST_LAUNCH="${PLAYTEST_LAUNCH:-Tron.exe}" PLAYTEST_SCREEN="${PLAYTEST_SCREEN:-800x600}" \
    PLAYTEST_RES="${PLAYTEST_RES:-800x600}" PLAYTEST_SCRIPT="$SCRIPT"
bash "$REPO/build/playtest.sh" "$GAME" - 0; rc=$?
WINEDEBUG=-all wine reg query "$K" > "$RUN/registry.txt" 2>&1
# the launcher may have written settings anywhere else in the registry
grep -n -i "tron\|monolith" "$P/user.reg" > "$RUN/user-reg.txt" 2>/dev/null
ls -la "$P"/drive_c/users/*/AppData/Roaming "$P"/drive_c/users/*/Application\ Data 2>/dev/null > "$RUN/appdata.txt"
[ -f "$RUN/launchcmds.txt" ] && { echo "-- launchcmds.txt:"; cat "$RUN/launchcmds.txt"; echo; }
echo "-- registry:"; tr -d '\r' < "$RUN/registry.txt"
exit $rc
