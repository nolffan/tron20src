#!/usr/bin/env bash
# The world sweep: load each single-player world of the retail game
# (build/sweep-worlds.txt) in the game directory, one playtest each
# (build/playtest.sh, in its own build container), and write one summary.
#
#   build/sweep.sh [gamedir] [filter]      (on the host; `make sweep` runs it)
#
# gamedir: the game directory (default out/tron20-legacy-release)
# filter:  only the worlds whose path contains this text (no case; e.g.
#          m05, LightCycle); default all
#
# Each world gets an input script from its kind (build/sweep-worlds.txt):
#   sp    skipcine (until the health bar is drawn: the player is in
#         control), a short move, a look round, a disc throw, then about
#         60 s with a shot every 20 s
#   sub   the first level of a mission: the opening cinematic (Space skips
#         it), then the subroutine screen (a shot; Escape closes it),
#         then as sp
#   cine  a key that skips nothing (x: the "Press any key to continue"
#         after the load), then about 60 s with no input, a shot every 20 s
#   lc, splc  (Space skips an opening) a turn left and a turn right,
#         about 60 s, a shot every 20 s; the cycle hits a wall in time and
#         the game asks "Retry light cycle?": that is correct
# From the environment (the Makefile sets them):
#   SWEEP_DOCKER   the docker command, without the container name and the
#                  image (default: `docker run --rm --init` with this
#                  checkout at /repo, as you)
#   IMAGE          the build image (default tron20src-build)
#   CONTAINER      the stem of the container names (default
#                  tron20src-build): one container for each world,
#                  <stem>-sweep-<world>
#   SWEEP_NAME     the stem of the run names (default sweep): each world
#                  plays in out/play/<stem>-<world>/
#   SWEEP_JOBS     worlds played at the same time (default 4)
#   SWEEP_SECS     seconds of play after the opening (default 60)
# Writes out/play/<stem>-summary.txt (a line for each world: the kind,
# the world, the result of the run, the mean brightness of the last shot,
# the last shot) and out/play/<stem>-sheet.png (the last shot of each
# world). The result is ALIVE / EXITED / CRASHED / NO HUD (playtest.sh);
# a brightness below 0.02 is marked DARK (a black screen?). Look at the
# shots: a world that runs but draws wrong is a failure too.
# Exit status 0 only if every world ends ALIVE.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO" || exit 2
GAME="${1:-out/tron20-legacy-release}"
FILTER="${2:-}"
IMAGE="${IMAGE:-tron20src-build}"
CONTAINER="${CONTAINER:-tron20src-build}"
STEM="${SWEEP_NAME:-sweep}"
JOBS="${SWEEP_JOBS:-4}"
SECS="${SWEEP_SECS:-60}"
DOCKER="${SWEEP_DOCKER:-docker run --rm --init -u $(id -u):$(id -g) -e HOME=/tmp/home -v $REPO:/repo -w /repo}"
[ -f "$GAME/Lithtech.exe" ] || { echo "error: no $GAME/ -- build it first (make)" >&2; exit 2; }

SCRIPTS="out/play/$STEM-scripts"
rm -rf "$SCRIPTS"; mkdir -p "$SCRIPTS"
LIST="$SCRIPTS/worlds.txt"
grep -vE '^[[:space:]]*(#|$)' build/sweep-worlds.txt |
    { if [ -n "$FILTER" ]; then grep -iF -- "$FILTER"; else cat; fi; } > "$LIST"
[ -s "$LIST" ] || { echo "error: no world matches '$FILTER'" >&2; exit 2; }

# one input script for each world
step=$((SECS / 3))
while read -r kind world; do
    slug="$(basename "${world//\\//}")"
    {
        echo "# $kind: $world"
        echo "window 240"
        case "$kind" in
            sp)   echo "skipcine 180"; echo "shot start"
                  echo "hold w 0.3"; echo "turn 400 0"; echo "click 1 0.3"; echo "turn -400 0" ;;
            sub)  for n in 1 2 3 4 5; do echo "wait 4"; echo "key space"; done
                  echo "wait 6"; echo "shot subroutines"; echo "key Escape"; echo "wait 3"
                  echo "skipcine 120"; echo "shot start"
                  echo "hold w 0.3"; echo "turn 400 0"; echo "click 1 0.3"; echo "turn -400 0" ;;
            cine) echo "wait 5"; echo "key x"; echo "shot start" ;;
            lc|splc) for n in 1 2 3; do echo "wait 4"; echo "key space"; done
                  echo "wait 3"; echo "shot start"; echo "hold a 0.3"; echo "wait 2"; echo "hold d 0.3" ;;
        esac
        for n in 1 2 3; do echo "wait $step"; echo "shot $((n * step))s"; done
    } > "$SCRIPTS/$slug.txt"
done < "$LIST"

play() {   # play <kind> <world>
    local kind="$1" world="$2" slug
    slug="$(basename "${world//\\//}")"
    $DOCKER --name "$CONTAINER-sweep-$slug" "$IMAGE" \
        env PLAYTEST_NAME="$STEM-$slug" PLAYTEST_SCRIPT="$SCRIPTS/$slug.txt" \
        bash build/playtest.sh "$GAME" "$world" 2 > "$SCRIPTS/$slug.log" 2>&1
    echo "$slug: $(tail -1 "$SCRIPTS/$slug.log")"
}
export -f play; export DOCKER CONTAINER IMAGE STEM SCRIPTS GAME
echo "sweep: $(wc -l < "$LIST") worlds, $JOBS at a time, runs in out/play/$STEM-<world>/"
# (each line: "<kind> <world>"; the world's backslashes are kept)
tr '\n' '\0' < "$LIST" | xargs -0 -P "$JOBS" -I{} bash -c 'set -- $1; play "$1" "$2"' _ {}

# the summary and the contact sheet (ImageMagick, in the build image)
SUMMARY="out/play/$STEM-summary.txt"
$DOCKER --name "$CONTAINER-sweep-summary" "$IMAGE" bash -c '
    list="$1"; stem="$2"; summary="$3"; shots=()
    printf "%-5s %-40s %-9s %-6s %s\n" kind world result bright "last shot" > "$summary"
    while read -r kind world; do
        slug="$(basename "${world//\\//}")"; run="out/play/$stem-$slug"
        res="$(head -1 "$run/result.txt" 2>/dev/null || echo "NO RESULT")"
        shot="$(ls "$run"/shots/*.png 2>/dev/null | tail -1)"
        if [ -n "$shot" ]; then
            b="$(convert "$shot" -colorspace Gray -format "%[fx:mean]" info: 2>/dev/null)"
            b="$(printf "%.3f" "${b:-0}")"
            awk -v b="$b" "BEGIN { exit !(b < 0.02) }" && res="$res (DARK)"
            shots+=("$shot")
        else b="-"; shot="-"; fi
        case "$res" in "NO HUD"*) r="NO-HUD" ;; "NO RESULT"*) r="NONE" ;; *) r="${res%% *}" ;; esac
        printf "%-5s %-40s %-9s %-6s %s\n" "$kind" "$world" "$r" "$b" "$shot" >> "$summary"
        printf "      %s\n" "$res" >> "$summary"
    done < "$list"
    [ ${#shots[@]} -gt 0 ] && montage -label "%d" -pointsize 9 "${shots[@]}" \
        -tile 6x -geometry 256x192+2+2 "out/play/$stem-sheet.png" 2>/dev/null
' _ "$LIST" "$STEM" "$SUMMARY"
cat "$SUMMARY"
! grep -E '^(sp|sub|cine|lc|splc) ' "$SUMMARY" | awk '$3 != "ALIVE"' | grep -q .
