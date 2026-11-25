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
# Worlds\RetailSinglePlayer\M01-01, and the HUD test of `skipcine`:
# Tron's health bar, green, at x 110..210, y 461..466 of the 640x480 view
# (NOLF2's is red, lower left). The adapted copy is written to
# out/play/playtest-tron20.sh each run (written to a new file, then
# renamed: runs that start at the same time, as in make sweep, do not
# read a half-written copy); ext/tools/ is not changed.
#
# One more script command, for Tron 2.0's scripts only:
#   nohud <name>          the HUD must not be up now (for example, the
#                         post-load screen waits for a key); takes a shot
#                         like `shot <name>`. If the HUD is up, the run is
#                         judged HUD UP, a failure.
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
    -e 's#local x=$((WX + 30\*WW/642)) y=$((WY + 448\*WH/482)) w=$((30\*WW/642)) h=$((4\*WH/482 + 1))#local x=$((WX + 110*WW/642)) y=$((WY + 461*WH/482)) w=$((100*WW/642)) h=$((5*WH/482 + 1))#' \
    -e "s#exit !(\$1 > 0.3 \&\& \$1 > 2.5 \* \$2)#exit !(\$2 > 0.6 \&\& \$2 > 1.8 * \$1)#" \
    -e 's#^STOPPED=""; NOHUD=""$#STOPPED=""; NOHUD=""; HUDUP=""#' \
    -e 's#^            \*)     log "  unknown command#            nohud) N=$((N+1)); f=$(printf '"'"'shots/%02d-%s.png'"'"' $N "$1"); grab "$f"; log "  -> $f"\
                   if hud; then HUDUP="line $LINE"; log "  nohud: the HUD is up"; else log "  nohud: no HUD, as expected"; fi ;;\
&#' \
    -e 's#^if alive \&\& \[ -n "$NOHUD" \]; then$#if alive \&\& [ -n "$HUDUP" ]; then\
    echo "HUD UP: alive, but the HUD was up where the script expects none (nohud at $HUDUP)" > result.txt\
    wineserver -k; sleep 2\
elif alive \&\& [ -n "$NOHUD" ]; then#' \
    -e 's#^source "$(dirname "${BASH_SOURCE\[0\]}")/layout.sh"$#REPO="'"$REPO"'"; TOOLS="'"$REPO"'/ext/tools"#' \
    "$REPO/ext/tools/playtest.sh" > "$OUT.$$" && mv -f "$OUT.$$" "$OUT"
exec bash "$OUT" "$@"
