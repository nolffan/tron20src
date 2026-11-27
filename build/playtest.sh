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
# lower left). The bar is bright green as far as the health goes and dark
# green (0, 32, 0) after it: the test takes green over red and blue, at
# any health (a new game from the menus starts M01-01 with 50 of 100, the
# retail game DLLs too: the control; +runworld with 100). The adapted copy is written to
# out/play/playtest-tron20.sh each run (written to a new file, then
# renamed: runs that start at the same time, as in make sweep, do not
# read a half-written copy); ext/tools/ is not changed.
#
# The run directory is build/rundir.py's, in the place of the links of
# ext/tools/playtest.sh: the binaries copied, a link to each other file
# that MANIFEST.txt names, and real directories. So the game never writes
# into the game directory: not into a linked directory (LightCycleGameData/,
# ServerData/), and not through a link to a file that the game itself
# wrote there earlier (the ban list bl.dat). The files that are not in
# MANIFEST.txt (what was played in the game directory) are not taken.
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
# test fails until it is answered. Not sooner: in play, 1 opens the
# weapon chooser, and a later step of the script can then find it open.
# (A quick load with the chooser open crashed in CHUDWeapon::SetWeapon;
# that is fixed, and build/playtest-weapon.txt does it on purpose.)
#
# One more script command, for Tron 2.0's scripts only:
#   nohud <name>          the HUD must not be up now (for example, the
#                         post-load screen waits for a key); takes a shot
#                         like `shot <name>`. If the HUD is up, the run is
#                         judged HUD UP, a failure.
#   lit <name>            takes a shot like `shot <name>`, and keeps the
#                         percentage of black pixels (below 3% grey) in the
#                         middle of the window (60% of its width, 50% of
#                         its height). Only a shot of a cinematic counts:
#                         its top 12% is black (the letterbox; not a menu
#                         or a load screen), and its middle is less than
#                         90% black (not a fade or a cut to black)
#   litcheck [max] [min]  the median of the percentages of the `lit` shots
#                         that count must be <max> or less (default 10).
#                         If not, the run is judged DARK, a failure: the
#                         models are black (models lit by DirLights, the
#                         intro's people: build/playtest-intro.txt). The
#                         median, not the mean: a shot or two of a darker
#                         scene, as the timing of the cinematic changes
#                         from run to run, do not change it. Fewer than
#                         <min> shots that count (default 5): the run is
#                         judged NO CINEMATIC, a failure (the cinematic
#                         was skipped or not yet on the screen: nothing
#                         was tested)
#   disc <name> in|out    takes a shot like `shot <name>`: the disc must be
#                         in the player's hand (in) or not (out). The test
#                         is the mean grey of the lower middle of the view
#                         (x 300..441, y 380..428 of the shot scaled to
#                         640x480), where the held disc is drawn: dark
#                         (about 60%) with the disc, the bright floor of
#                         M01-01's start (about 98%) without it, so it
#                         needs that view (build/playtest-recall.txt).
#                         Below 80%: in. If not as expected, the run is
#                         judged DISC, a failure
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
    -e "s#-format '%\[fx:mean.r\] %\[fx:mean.g\]' info:#-format '%[fx:mean.r] %[fx:mean.g] %[fx:mean.b]' info:#" \
    -e "s#exit !(\$1 > 0.3 \&\& \$1 > 2.5 \* \$2)#exit !(\$2 > 0.1 \&\& \$2 > 1.8 * \$1 \&\& \$2 > 1.8 * \$3)#" \
    -e 's#\[ $k = space \] \&\& k=Return || k=space#if [ $k = space ]; then k=Return; elif [ $k = Return ] \&\& [ $(( $(date +%s) - st )) -ge 20 ]; then k=1; else k=space; fi#' \
    -e 's#^STOPPED=""; NOHUD=""$#STOPPED=""; NOHUD=""; HUDUP=""; DARK=""; NOCINE=""; LITS=""; DISCBAD=""#' \
    -e 's#^            \*)     log "  unknown command#            nohud) N=$((N+1)); f=$(printf '"'"'shots/%02d-%s.png'"'"' $N "$1"); grab "$f"; log "  -> $f"\
                   if hud; then HUDUP="line $LINE"; log "  nohud: the HUD is up"; else log "  nohud: no HUD, as expected"; fi ;;\
&#' \
    -e 's#^            nohud) #            lit) N=$((N+1)); f=$(printf "shots/%02d-%s.png" $N "$1"); grab "$f"\
                   d=$(convert "$f" -gravity center -crop 60%x50%+0+0 -colorspace gray -threshold 3% -format "%[fx:round(100*(1-mean))]" info: 2>/dev/null)\
                   t=$(convert "$f" -gravity north -crop 100%x12%+0+0 +repage -colorspace gray -format "%[fx:round(1000*mean)]" info: 2>/dev/null)\
                   if [ "${t:-1000}" -gt 20 ]; then log "  -> $f (black: ${d:-?}%; no letterbox: not the cinematic, not counted)"\
                   elif [ "${d:-100}" -ge 90 ]; then log "  -> $f (black: ${d:-?}%: a fade or a cut, not counted)"\
                   else LITS="$LITS $d"; log "  -> $f (black: $d%)"; fi ;;\
            litcheck) n=$(echo $LITS | wc -w); s=$(echo $LITS | tr " " "\\n" | sort -n | awk "{ v[NR] = \\$1 } END { print (NR ? v[int((NR + 1) / 2)] : 0) }")\
                   if [ $n -lt ${2:-5} ]; then NOCINE="line $LINE: $n counted, fewer than ${2:-5}"; log "  litcheck: $n shots of the cinematic counted: NO CINEMATIC"\
                   elif [ $s -gt ${1:-10} ]; then DARK="line $LINE: median $s% black of $n shots, more than ${1:-10}%"; log "  litcheck: median $s% black of $n shots: DARK"\
                   else log "  litcheck: median $s% black of $n shots, as expected"; fi ;;\
&#' \
    -e 's#^            lit) #            disc) N=$((N+1)); f=$(printf "shots/%02d-%s.png" $N "$1"); grab "$f"\
                   g=$(convert "$f" -resize "640x480!" -crop 141x48+300+380 +repage -colorspace gray -format "%[fx:round(100*mean)]" info: 2>/dev/null)\
                   if [ "${g:-100}" -lt 80 ]; then d=in; else d=out; fi\
                   if [ "$d" = "${2:-in}" ]; then log "  -> $f (grey ${g:-?}%: the disc is $d, as expected)"\
                   else DISCBAD="line $LINE: the disc is $d, not ${2:-in}"; log "  -> $f (grey ${g:-?}%: the disc is $d, NOT ${2:-in})"; fi ;;\
&#' \
    -e 's#^if alive \&\& \[ -n "$NOHUD" \]; then$#if alive \&\& [ -n "$DISCBAD" ]; then\
    echo "DISC: alive, but the disc is not where the script expects it ($DISCBAD)" > result.txt\
    wineserver -k; sleep 2\
elif alive \&\& [ -n "$HUDUP" ]; then\
    echo "HUD UP: alive, but the HUD was up where the script expects none (nohud at $HUDUP)" > result.txt\
    wineserver -k; sleep 2\
elif alive \&\& [ -n "$DARK" ]; then\
    echo "DARK: alive, but the lit shots are black ($DARK)" > result.txt\
    wineserver -k; sleep 2\
elif alive \&\& [ -n "$NOCINE" ]; then\
    echo "NO CINEMATIC: alive, but too few lit shots show the cinematic ($NOCINE)" > result.txt\
    wineserver -k; sleep 2\
elif alive \&\& [ -n "$NOHUD" ]; then#' \
    -e '/pcm\.!default { type null }/a\    [ -z "${PLAYTEST_AUDIO:-}" ] || printf "</usr/share/alsa/alsa.conf>\\npcm.!default { type file slave.pcm null file \\"%s-%%r-%%c-%%f.raw\\" format raw truncate false }\\n" "$PLAYTEST_AUDIO" > asound.conf' \
    -e 's#^for f in "$GAME"/\*; do$#python3 -I "$REPO/build/rundir.py" "$GAME" "$RUN" autoexec.cfg Save Profiles tron20cmds.txt linux-run.sh linux-run-launcher.sh run-cmds.txt launchcmds.txt || exit 2\
for f in; do#' \
    -e 's#^source "$(dirname "${BASH_SOURCE\[0\]}")/layout.sh"$#REPO="'"$REPO"'"; TOOLS="'"$REPO"'/ext/tools"#' \
    "$REPO/ext/tools/playtest.sh" > "$OUT.$$" || exit 2
# (if ext/tools/playtest.sh changes so that an edit above does not apply,
# stop: the run directory must be build/rundir.py's, and the HUD test and
# the lit shots Tron's)
for m in 'build/rundir.py' 'mean.b' '1.8 \* \$3' 'NO CINEMATIC' 'HUD UP' 'k=1;' 'DISC:'; do
    grep -q "$m" "$OUT.$$" || { rm -f "$OUT.$$"; echo "playtest.sh: ext/tools/playtest.sh changed: an edit of build/playtest.sh does not apply ($m)"; exit 2; }
done
mv -f "$OUT.$$" "$OUT"
exec bash "$OUT" "$@"
