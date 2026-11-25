#!/usr/bin/env python3
"""fpscheck.py -- compare the frame-rate test runs (docs/MODERN.md EN-45).

    python3 build/fpscheck.py walk <run dir or poslog.txt>...
    python3 build/fpscheck.py race <run dir or poslog.txt>...

Each run is a modern build run with "PosLog 1" (build/fps-*.cfg): its
poslog.txt has one line a frame (CMoveMgr::ModernPosLog): the time, the
frame time, the local player object's position and velocity, the server
object's position and the physics model.

walk: each stretch of movement (build/playtest-fps-walk.txt): its time,
      its distance on the ground, its top speed on the ground, how high it
      rose (a jump).
race: the light cycle (build/playtest-fps-race.txt): from its first move,
      the distance at 1, 2, ... 10 seconds, and the turns (the time after
      the first move and where).
The frame rate of each run is printed too. Runs of the same script at two
frame rates must give the same numbers, within the timing of the keys
(a few percent for the walk).
"""
import math
import os
import sys


def rows_of(path):
    if os.path.isdir(path):
        path = os.path.join(path, 'poslog.txt')
    rows = []
    with open(path) as f:
        for line in f:
            if line.startswith('#'):
                continue
            r = line.split()
            if len(r) < 12:
                continue
            rows.append([float(x) for x in r[:11]] + [int(r[11])])
    return path, rows


def fps(rows):
    if len(rows) < 2 or rows[-1][0] <= rows[0][0]:
        return 0.0
    return len(rows) / (rows[-1][0] - rows[0][0])


def walk(path):
    path, rows = rows_of(path)
    print('%s: %d frames, %.0f frames a second' % (path, len(rows), fps(rows)))
    segs, seg = [], None
    for i in range(1, len(rows)):
        a, b = rows[i - 1], rows[i]
        moving = math.hypot(b[2] - a[2], b[4] - a[4]) > 0.001 or abs(b[3] - a[3]) > 0.001
        if moving and seg is None:
            seg = [i - 1, i]
        elif moving:
            seg[1] = i
        elif seg is not None and b[0] - rows[seg[1]][0] > 0.3:
            segs.append(seg)
            seg = None
    if seg:
        segs.append(seg)
    for s0, s1 in segs:
        a, b = rows[s0], rows[s1]
        if b[0] - a[0] < 0.05:
            continue
        dist = sum(math.hypot(rows[j][2] - rows[j - 1][2], rows[j][4] - rows[j - 1][4])
                   for j in range(s0 + 1, s1 + 1))
        top = max(math.hypot(rows[j][5], rows[j][7]) for j in range(s0, s1 + 1))
        rise = max(rows[j][3] for j in range(s0, s1 + 1)) - a[3]
        print('  %7.2f s, %.2f s long: distance %6.1f, top speed %5.1f, rise %5.2f'
              % (a[0], b[0] - a[0], dist, top, rise))


def race(path):
    path, rows = rows_of(path)
    rows = [r for r in rows if r[11] != 0]
    print('%s: %d frames, %.0f frames a second' % (path, len(rows), fps(rows)))
    start = None
    for i in range(1, len(rows)):
        if (rows[i][2], rows[i][4]) != (rows[i - 1][2], rows[i - 1][4]):
            start = i
            break
    if start is None:
        print('  the cycle does not move')
        return
    t0 = rows[start - 1][0]
    dist, last, turns, marks, out = 0.0, None, [], list(range(1, 11)), []
    for i in range(start, len(rows)):
        a, b = rows[i - 1], rows[i]
        dx, dz = b[2] - a[2], b[4] - a[4]
        d = abs(dx) + abs(dz)
        dist += d
        if d > 0:
            way = ('x', dx > 0) if abs(dx) > abs(dz) else ('z', dz > 0)
            if last and way != last:
                turns.append('%.2f s at (%.0f, %.0f)' % (b[0] - t0, b[2], b[4]))
            last = way
        while marks and b[0] - t0 >= marks[0]:
            out.append('%d s %.0f' % (marks[0], dist))
            marks.pop(0)
    print('  distance: ' + ', '.join(out))
    print('  turns: ' + (', '.join(turns) if turns else 'none'))


def main():
    if len(sys.argv) < 3 or sys.argv[1] not in ('walk', 'race'):
        print(__doc__)
        sys.exit(2)
    for p in sys.argv[2:]:
        (walk if sys.argv[1] == 'walk' else race)(p)


if __name__ == '__main__':
    main()
