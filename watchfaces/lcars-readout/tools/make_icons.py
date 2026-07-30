#!/usr/bin/env python3
"""Build the watchface's icon set from Material Symbols (Apache-2.0).

Material Symbols ships SVG only, so each glyph is rasterised well above target
size, cropped to its ink, then downsampled with a hard alpha threshold. The
threshold matters: Pebble draws these with GCompOpSet, so any antialiased grey
fringe would land on the watch as speckle.

The SVGs are checked in under resources/images/src/, so re-running this needs
no network. Pass --download to refresh them from upstream.

    python3 tools/make_icons.py [--download]
"""

import argparse
import io
import os
import urllib.request

import cairosvg
from PIL import Image

SIZE = 20        # final icon size, matches ICON_SZ in src/c/lcars_theme.h
RENDER = 512     # rasterise this big before downsampling
BLACK = (0, 0, 0, 255)

HERE = os.path.dirname(os.path.abspath(__file__))
IMAGES = os.path.join(HERE, "..", "resources", "images")
SRC = os.path.join(IMAGES, "src")

BASE = ("https://raw.githubusercontent.com/google/material-design-icons/"
        "master/symbols/web/{name}/materialsymbolsrounded/{name}_fill1_24px.svg")

# icon_*.png  ->  Material Symbols glyph name
#
# Only the weather icons live here. The thermometer, heart and footprints are
# painted into the background artwork, since they never change.
ICONS = {
    "icon_clear": "sunny",
    "icon_cloud": "cloud",
    "icon_rain": "rainy",
    "icon_snow": "weather_snowy",
}


def download():
    os.makedirs(SRC, exist_ok=True)
    for glyph in sorted(set(ICONS.values())):
        url = BASE.format(name=glyph)
        dest = os.path.join(SRC, glyph + ".svg")
        with urllib.request.urlopen(url) as r:
            data = r.read()
        with open(dest, "wb") as f:
            f.write(data)
        print("fetched", glyph)


def render(glyph):
    svg = os.path.join(SRC, glyph + ".svg")
    png = cairosvg.svg2png(url=svg, output_width=RENDER, output_height=RENDER)
    return Image.open(io.BytesIO(png)).convert("RGBA")


def fit(im):
    """Crop to the glyph's ink and scale it to fill SIZE, keeping aspect.

    Material Symbols carry ~2px of padding inside their 24px viewBox. Cropping
    it away buys roughly 15% more ink at this size, which is the difference
    between the footprints reading as shoes and reading as blobs.
    """
    box = im.getchannel("A").getbbox()
    im = im.crop(box)

    scale = SIZE / max(im.width, im.height)
    w, h = max(1, round(im.width * scale)), max(1, round(im.height * scale))
    im = im.resize((w, h), Image.LANCZOS)

    out = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    out.paste(im, ((SIZE - w) // 2, (SIZE - h) // 2))
    return out


def threshold(im):
    out = Image.new("RGBA", im.size, (0, 0, 0, 0))
    out.putdata([BLACK if a >= 128 else (0, 0, 0, 0)
                 for a in im.getchannel("A").getdata()])
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--download", action="store_true",
                    help="refetch the SVGs from upstream before rendering")
    args = ap.parse_args()

    if args.download or not os.path.isdir(SRC):
        download()

    for name, glyph in ICONS.items():
        threshold(fit(render(glyph))).save(os.path.join(IMAGES, name + ".png"))
        print("wrote", name + ".png", "<-", glyph)


if __name__ == "__main__":
    main()
