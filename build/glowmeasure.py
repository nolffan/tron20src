#!/usr/bin/env python3
"""glowmeasure.py -- measure the screen glow's halo shape (docs/MODERN.md CL-36).

    build/glowmeasure.py <shot.png>...     (in the build container: it reads
                                            the shots with ImageMagick)

The source is the yellow item marker of M01-01's start (build/playtest-glow.txt),
a round object of the same size relative to the screen height at every
screen shape wider than 4:3 (the engine keeps the vertical field of view,
EN-30; at 5:4 it keeps the horizontal one, and the object is smaller). For each
shot: its "yellowness" (min(R, G) - B) is found around the brightest yellow
area in the upper half of the screen, then measured along the horizontal
and the vertical line through that point:
  w, h   the widths (pixels), from the first to the last point within 0.2
         of the height of the centre where the yellowness is above 0.5 of
         the peak (FWHM), above 0.1 and above 0.03 of the peak (the extent
         of the halo)
  w/H, h/H  these widths divided by the screen height
  w/h    the shape: the same at every screen shape when the glow keeps
         the shape it has at 4:3
Measure the shots of ScreenGlowShowTexture 1 (ScreenGlowShowTextureScale 1):
the glow alone. In a full frame the halo is over a bright cyan background,
where the yellowness is low, so the numbers there are those of the marker
itself: look at full frames, do not measure them.
"""
import subprocess
import sys


def load(path):
    """The shot as (width, height, yellowness rows): min(R, G) - B, >= 0."""
    size = subprocess.run(['identify', '-format', '%w %h', path], check=True,
                          capture_output=True, text=True).stdout.split()
    W, H = int(size[0]), int(size[1])
    raw = subprocess.run(['convert', path, '-depth', '8', 'rgb:-'], check=True,
                         capture_output=True).stdout
    rows = []
    for y in range(H):
        line = raw[y * W * 3:(y + 1) * W * 3]
        r, g, b = line[0::3], line[1::3], line[2::3]
        rows.append([max(0, min(r[i], g[i]) - b[i]) for i in range(W)])
    return W, H, rows


def smooth(values, k):
    """A box filter of k values (k odd)."""
    h = k // 2
    n = len(values)
    acc = [0] * (n + 1)
    for i, v in enumerate(values):
        acc[i + 1] = acc[i] + v
    return [(acc[min(n, i + h + 1)] - acc[max(0, i - h)]) / (min(n, i + h + 1) - max(0, i - h))
            for i in range(n)]


def width(profile, centre, level, reach):
    """The outer width of the values >= level within reach of centre: from the
    first to the last such value (ends interpolated), so that dark gaps inside
    the object (a ring) do not cut it short."""
    n = len(profile)
    lo, hi = max(1, centre - reach), min(n - 2, centre + reach)
    over = [i for i in range(lo, hi + 1) if profile[i] >= level]
    if not over:
        return 0.0
    a, b = over[0], over[-1]
    left = a - (profile[a] - level) / max(profile[a] - profile[a - 1], 1e-6)
    right = b + (profile[b] - level) / max(profile[b] - profile[b + 1], 1e-6)
    return right - left


def measure(path):
    W, H, y = load(path)
    # blocks of 1% of the height, averaged, over the upper half: the centre
    blk = max(2, H // 100)
    bw, bh = W // blk, (H // 2) // blk
    grid = [[0.0] * bw for _ in range(bh)]
    for j in range(bh * blk):
        row = y[j]
        g = grid[j // blk]
        for i in range(bw):
            g[i] += sum(row[i * blk:(i + 1) * blk])
    # away from the HUD: 0.15 to 0.5 of the height down, within half the
    # height of the centre across
    area = [(j, i) for j in range(bh) for i in range(bw)
            if j * blk >= 0.15 * H and abs((i + 0.5) * blk - W / 2) <= H / 2]
    by, bx = max(area, key=lambda t: grid[t[0]][t[1]])
    top = grid[by][bx]
    near = [(j, i) for j in range(bh) for i in range(bw)
            if grid[j][i] >= 0.5 * top and abs(j - by) * blk < H // 6 and abs(i - bx) * blk < H // 6]
    cy = int((sum(j for j, _ in near) / len(near) + 0.5) * blk)
    cx = int((sum(i for _, i in near) / len(near) + 0.5) * blk)
    k = max(3, (H // 200) | 1)
    row = smooth(y[cy], k)
    col = smooth([y[j][cx] for j in range(H)], k)
    r = H // 20
    peak = max(max(row[max(0, cx - r):cx + r]), max(col[max(0, cy - r):cy + r]))
    res = {}
    for name, frac in (('fwhm', 0.5), ('ext10', 0.1), ('ext03', 0.03)):
        res[name] = (width(row, cx, frac * peak, H // 5), width(col, cy, frac * peak, H // 5))
    return W, H, cx, cy, res


def main():
    print('%-40s %9s %5s %5s' % ('shot', 'size', 'cx', 'cy') + ''.join(
        ' | %6s %6s %6s %6s %5s' % (n + ' w', 'h', 'w/H', 'h/H', 'w/h')
        for n in ('0.5', '0.1', '0.03')))
    for p in sys.argv[1:]:
        W, H, cx, cy, r = measure(p)
        out = '%-40s %9s %5d %5d' % (p[-40:], '%dx%d' % (W, H), cx, cy)
        for name in ('fwhm', 'ext10', 'ext03'):
            w, h = r[name]
            out += ' | %6.1f %6.1f %6.3f %6.3f %5.2f' % (w, h, w / H, h / H, w / h)
        print(out)


if __name__ == '__main__':
    main()
