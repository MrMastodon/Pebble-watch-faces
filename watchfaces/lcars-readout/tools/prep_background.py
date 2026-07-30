#!/usr/bin/env python3
"""Turn the hand-designed background into a build-ready Pebble bitmap.

Two things have to happen before the artwork can be shipped:

1. **Flatten the alpha onto white.** The design is drawn on transparency, and
   the watchface has an opaque white ground.
2. **Snap every pixel to the Pebble-64 palette** (each channel to one of
   00/55/AA/FF). The watch can only show those 64 colours, so this is what it
   will render regardless — doing it here means the checked-in PNG is exactly
   what appears on the wrist, and any colour that shifts is visible in review
   rather than a surprise on the watch.

    python3 tools/prep_background.py
"""

import os

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
IMAGES = os.path.join(HERE, "..", "resources", "images")
SRC = os.path.join(IMAGES, "LCARS-readout_background.png")
DEST = os.path.join(IMAGES, "background.png")

LEVELS = np.array([0, 85, 170, 255])


def main():
    src = Image.open(SRC)
    if src.size != (200, 228):
        raise SystemExit(f"expected 200x228, got {src.size}")

    flat = Image.alpha_composite(
        Image.new("RGBA", src.size, (255, 255, 255, 255)), src.convert("RGBA")
    ).convert("RGB")

    a = np.asarray(flat).astype(int)
    snapped = LEVELS[np.abs(a[..., None] - LEVELS).argmin(-1)].astype(np.uint8)

    # Snapping antialiased edges scatters a long tail of near-unique colours —
    # a few dozen stray pixels each. Folding anything under MIN_PIXELS into its
    # nearest surviving colour drops the count under 16, which lets the bitmap
    # ship as 4BitPalette and halves its RAM footprint (22 KB instead of 45 KB).
    MIN_PIXELS = 32
    flatpx = snapped.reshape(-1, 3)
    colours, counts = np.unique(flatpx, axis=0, return_counts=True)
    keep = colours[counts >= MIN_PIXELS]
    drop = colours[counts < MIN_PIXELS]

    for col in drop:
        nearest = keep[np.abs(keep.astype(int) - col.astype(int)).sum(1).argmin()]
        flatpx[(flatpx == col).all(1)] = nearest
    snapped = flatpx.reshape(snapped.shape)

    out = Image.fromarray(snapped, "RGB")
    out.save(DEST, optimize=True)

    before = len(set(map(tuple, a.reshape(-1, 3))))
    after = len(set(map(tuple, snapped.reshape(-1, 3))))
    moved = int((snapped != a).any(-1).sum())
    print(f"wrote {os.path.relpath(DEST)}  ({os.path.getsize(DEST)} B)")
    print(f"  colours {before} -> {after}  "
          f"({len(drop)} rare colours folded into nearest)")
    print(f"  pixels shifted: {moved} ({100 * moved / (200 * 228):.1f}%)")
    if after > 16:
        print(f"  WARNING: {after} colours needs 8Bit; only <=16 fits 4BitPalette")


if __name__ == "__main__":
    main()
