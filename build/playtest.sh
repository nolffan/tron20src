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
# Tron's health bar, green, 210..110 pixels left of the screen's centre and
# 21..15 pixels above its bottom at every resolution (the bar is not scaled:
# HUDHealthEnergy.cpp; x 110..210, y 461..466 at 640x480; NOLF2's is red,
# lower left). The adapted copy is written to
# out/play/playtest-tron20.sh each run (written to a new file, then
# renamed: runs that start at the same time, as in make sweep, do not
# read a half-written copy); ext/tools/ is not changed.
#
# One more setting: PLAYTEST_AUDIO=<prefix> (an absolute path) records what
# the game plays where ext/tools/playtest.sh plays it into ALSA's null
# device (no sound card: the container). ALSA's file plugin in front of the
# null device writes each stream, raw, to <prefix>-<rate>-<channels>-
# <format>.raw (the streams of one format go into one file). The null device
# takes data faster than a card plays it, so the recording holds what was
# played, not when. (avp2src's setting; MODERN.md X-1, X-2.)
#
# skipcine also presses 1 (Space, Return, 1 in turn) when there is no HUD
# after 20 s: a decision window of Tron 2.0 (M01-01's "Continue basic
# training") takes its answer from the number keys, and while it is up
# over the bottom of the screen the health bar is not seen, so the HUD
# test fails until it is answered. Not sooner: in play, 1 selects a
# weapon, and build/playtest-save.txt's quick load in the legacy build
# then crashed in CHUDWeapon::SetWeapon (the modern build did not).
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
    -e 's#local x=$((WX + 30\*WW/642)) y=$((WY + 448\*WH/482)) w=$((30\*WW/642)) h=$((4\*WH/482 + 1))#local x=$((WX + WW/2 - 210)) y=$((WY + WH - 21)) w=100 h=6#' \
    -e "s#exit !(\$1 > 0.3 \&\& \$1 > 2.5 \* \$2)#exit !(\$2 > 0.6 \&\& \$2 > 1.8 * \$1)#" \
    -e 's#\[ $k = space \] \&\& k=Return || k=space#if [ $k = space ]; then k=Return; elif [ $k = Return ] \&\& [ $(( $(date +%s) - st )) -ge 20 ]; then k=1; else k=space; fi#' \
    -e 's#^STOPPED=""; NOHUD=""$#STOPPED=""; NOHUD=""; HUDUP=""#' \
    -e 's#^            \*)     log "  unknown command#            nohud) N=$((N+1)); f=$(printf '"'"'shots/%02d-%s.png'"'"' $N "$1"); grab "$f"; log "  -> $f"\
                   if hud; then HUDUP="line $LINE"; log "  nohud: the HUD is up"; else log "  nohud: no HUD, as expected"; fi ;;\
&#' \
    -e 's#^if alive \&\& \[ -n "$NOHUD" \]; then$#if alive \&\& [ -n "$HUDUP" ]; then\
    echo "HUD UP: alive, but the HUD was up where the script expects none (nohud at $HUDUP)" > result.txt\
    wineserver -k; sleep 2\
elif alive \&\& [ -n "$NOHUD" ]; then#' \
    -e '/pcm\.!default { type null }/a\    [ -z "${PLAYTEST_AUDIO:-}" ] || printf "</usr/share/alsa/alsa.conf>\\npcm.!default { type file slave.pcm null file \\"%s-%%r-%%c-%%f.raw\\" format raw truncate false }\\n" "$PLAYTEST_AUDIO" > asound.conf' \
    -e 's#^source "$(dirname "${BASH_SOURCE\[0\]}")/layout.sh"$#REPO="'"$REPO"'"; TOOLS="'"$REPO"'/ext/tools"#' \
    "$REPO/ext/tools/playtest.sh" > "$OUT.$$" && mv -f "$OUT.$$" "$OUT"
exec bash "$OUT" "$@"
