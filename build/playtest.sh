#!/usr/bin/env bash
# The runtime test, for Tron 2.0: ext/tools/playtest.sh (shared with
# nolf2src, whose game names it assumes) run on a Tron 2.0 game directory.
#
#   build/playtest.sh [gamedir] [world] [seconds]     (in the build container;
#                                                       `make test` runs it)
#
# Everything is ext/tools/playtest.sh's (its header documents the arguments,
# the PLAYTEST_* settings, the input scripts and the results in
# out/play/<name>/), with Tron 2.0's names in place of NOLF2's: the command
# file tron20cmds.txt (build/tron20cmds.sh), the game directory
# out/tron20-legacy-release, the first single-player world
# Worlds\RetailSinglePlayer\M01-01. The adapted copy is written to
# out/play/playtest-tron20.sh each run; ext/tools/ is not changed.
set -u
# a game directory built without the retail game (NODISCS=1) does not play
for a in "$@"; do
    [ ! -f "$a/NOT-PLAYABLE.txt" ] || { cat "$a/NOT-PLAYABLE.txt"; exit 2; }
done
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$REPO/out/play"
OUT="$REPO/out/play/playtest-tron20.sh"
sed -e 's/nolf2cmds\.txt/tron20cmds.txt/g' \
    -e 's/NOLF2SRC_PADTEST/TRON20SRC_PADTEST/g' \
    -e 's#out/nolf2-legacy-release#out/tron20-legacy-release#g' \
    -e 's#Worlds\\\\RetailSinglePlayer\\\\C01S01#Worlds\\\\RetailSinglePlayer\\\\M01-01#' \
    -e 's#^source "$(dirname "${BASH_SOURCE\[0\]}")/layout.sh"$#REPO="'"$REPO"'"; TOOLS="'"$REPO"'/ext/tools"#' \
    "$REPO/ext/tools/playtest.sh" > "$OUT"
exec bash "$OUT" "$@"
