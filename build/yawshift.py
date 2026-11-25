#!/usr/bin/env python3
"""yawshift.py -- how far the view turned between two screenshots.

    python3 build/yawshift.py <before.png> <after.png> [...]

For each pair (a0 a1, a1 a2, ...) it prints the horizontal shift, in pixels
of a 320-pixel-wide copy, that best lines up the middle band of the second
shot with the first (the smallest mean absolute difference), and that
shift as degrees of a 90-degree horizontal field of view (the 4:3 view;
near the centre, where the band is compared, a pixel is about 90/320
degrees). A turn to the right moves the scene to the left: the shift is
positive then. Used by build/playtest-mouselook.txt (MODERN.md EN-24) to
compare the mouse look of two builds. Needs ImageMagick's convert (the
build container has it); no other module.
"""
import subprocess
import sys

W, H = 320, 240
MAXSHIFT = 150


def gray(path):
    data = subprocess.run(["convert", path, "-resize", f"{W}x{H}!", "-colorspace", "Gray",
                           "-depth", "8", "gray:-"], check=True, capture_output=True).stdout
    return [data[y * W:(y + 1) * W] for y in range(H)]


def shift(a, b):
    best = None
    rows = range(H // 4, 3 * H // 4, 2)
    for s in range(-MAXSHIFT, MAXSHIFT + 1):
        x0, x1 = max(0, s), min(W, W + s)      # b[x - s] against a[x]
        if x1 - x0 < W // 4:
            continue
        tot = 0
        n = 0
        for y in rows:
            ra, rb = a[y], b[y]
            for x in range(x0, x1, 2):
                tot += abs(ra[x] - rb[x - s])
                n += 1
        m = tot / n
        if best is None or m < best[0]:
            best = (m, s)
    return best


def main():
    paths = sys.argv[1:]
    if len(paths) < 2:
        print(__doc__)
        return 2
    imgs = [gray(p) for p in paths]
    for i in range(len(imgs) - 1):
        m, s = shift(imgs[i], imgs[i + 1])
        print(f"{paths[i].split('/')[-1]} -> {paths[i + 1].split('/')[-1]}: "
              f"shift {s:+d} px ({s * 90.0 / W:+.1f} deg), residual {m:.1f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
