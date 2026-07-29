#!/usr/bin/env python3
"""Generate the watchface's icon set as 1-bit black-on-transparent PNGs.

Each icon is drawn 4x oversized and downsampled with a hard threshold, which
keeps the edges crisp instead of muddy at 18x18 — Pebble renders these with
GCompOpSet, so any grey fringe would show up as speckle.

Run from the project root:  python3 tools/make_icons.py
"""

import os
from PIL import Image, ImageDraw

S = 18          # final icon size
F = 8           # oversampling factor
BIG = S * F
OUT = os.path.join(os.path.dirname(__file__), "..", "resources", "images")

BLACK = (0, 0, 0, 255)


def canvas():
    im = Image.new("RGBA", (BIG, BIG), (0, 0, 0, 0))
    return im, ImageDraw.Draw(im)


def px(v):
    """Convert an 18px-space coordinate to oversampled space."""
    return v * F


def circle(d, cx, cy, r, fill=BLACK, width=0, outline=None):
    d.ellipse([px(cx - r), px(cy - r), px(cx + r), px(cy + r)],
              fill=fill, outline=outline, width=width)


def sun():
    im, d = canvas()
    circle(d, 9, 9, 3.6)
    # Eight rays around the disc.
    import math
    for i in range(8):
        a = math.radians(i * 45)
        x0, y0 = 9 + 5.2 * math.cos(a), 9 + 5.2 * math.sin(a)
        x1, y1 = 9 + 8.0 * math.cos(a), 9 + 8.0 * math.sin(a)
        d.line([px(x0), px(y0), px(x1), px(y1)], fill=BLACK, width=int(1.5 * F))
    return im


def cloud():
    im, d = canvas()
    circle(d, 6.2, 9.5, 3.4)
    circle(d, 10.4, 8.4, 4.3)
    circle(d, 13.4, 10.4, 3.0)
    d.rectangle([px(6.2), px(10.0), px(13.4), px(13.4)], fill=BLACK)
    return im


def rain():
    im, d = canvas()
    circle(d, 6.2, 7.6, 3.0)
    circle(d, 10.2, 6.6, 3.8)
    circle(d, 12.9, 8.3, 2.7)
    d.rectangle([px(6.2), px(7.9), px(12.9), px(10.6)], fill=BLACK)
    for x in (6.0, 9.2, 12.4):
        d.line([px(x), px(12.4), px(x - 1.4), px(15.8)], fill=BLACK, width=int(1.4 * F))
    return im


def snow():
    im, d = canvas()
    circle(d, 6.2, 7.6, 3.0)
    circle(d, 10.2, 6.6, 3.8)
    circle(d, 12.9, 8.3, 2.7)
    d.rectangle([px(6.2), px(7.9), px(12.9), px(10.6)], fill=BLACK)
    for x in (6.2, 9.4, 12.6):
        circle(d, x, 14.2, 1.2)
    return im


def thermometer():
    im, d = canvas()
    # Solid silhouette — an outlined bulb loses its shape once downsampled.
    d.rounded_rectangle([px(6.9), px(1.8), px(11.1), px(12.6)],
                        radius=px(2.1), fill=BLACK)
    circle(d, 9, 13.4, 4.0)
    return im


def heart():
    im, d = canvas()
    circle(d, 5.9, 6.6, 3.5)
    circle(d, 12.1, 6.6, 3.5)
    d.polygon([(px(2.5), px(7.6)), (px(15.5), px(7.6)), (px(9), px(16.2))], fill=BLACK)
    return im


def steps():
    im, d = canvas()

    # Two staggered prints, each a tilted sole with its toe pad tucked close
    # against it. Separate toe circles read as mushrooms at this size.
    def print_at(cx, cy, tilt):
        foot = Image.new("RGBA", (BIG, BIG), (0, 0, 0, 0))
        fd = ImageDraw.Draw(foot)
        # One fat oval per print — anything narrower thins out to a comma once
        # rotated and thresholded down to 18px.
        fd.ellipse([px(cx - 3.0), px(cy - 5.2), px(cx + 3.0), px(cy + 5.2)], fill=BLACK)
        return foot.rotate(tilt, resample=Image.BICUBIC, center=(px(cx), px(cy)))

    im.alpha_composite(print_at(4.8, 11.6, 14))
    im.alpha_composite(print_at(13.2, 6.4, -14))
    return im


ICONS = {
    "icon_clear": sun,
    "icon_cloud": cloud,
    "icon_rain": rain,
    "icon_snow": snow,
    "icon_temp": thermometer,
    "icon_heart": heart,
    "icon_steps": steps,
}


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, fn in ICONS.items():
        im = fn().resize((S, S), Image.LANCZOS)
        # Hard threshold: anything at least half-opaque becomes solid black.
        out = Image.new("RGBA", (S, S), (0, 0, 0, 0))
        out.putdata([BLACK if p[3] >= 128 else (0, 0, 0, 0) for p in im.getdata()])
        path = os.path.join(OUT, name + ".png")
        out.save(path)
        print("wrote", os.path.relpath(path))


if __name__ == "__main__":
    main()
