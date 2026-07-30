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
SRC = os.path.join(IMAGES, "LCARS-readout_background_2.png")
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

    # Snapping antialiased edges leaves a tail of fringe colours. Separating
    # them from real design colours by pixel count alone gets it wrong — a
    # small solid block can be rarer than a fringe smeared over the whole
    # canvas. Compactness is the reliable signal: a design colour fills its
    # bounding box, a fringe scatters thinly across the image.
    #
    # Folding the fringes away keeps the palette under 16 entries, which lets
    # the bitmap ship as 4BitPalette for 22 KB of heap instead of 45 KB.
    MIN_SHARE = 0.015   # this common is structural whatever shape it takes
    MIN_RUN = 4         # ...otherwise it must form a solid run somewhere

    total = snapped.shape[0] * snapped.shape[1]
    colours = np.unique(snapped.reshape(-1, 3), axis=0)

    def longest_run(mask):
        """Longest horizontal or vertical unbroken run of this colour.

        Bounding-box density fails here: pills and end caps are solid but
        scattered across the whole canvas, so collectively they look as sparse
        as noise. Run length asks the question that actually matters — does
        this colour ever fill a solid stretch, or is it always a thin fringe?
        """
        best = 0
        for grid in (mask, mask.T):
            for line in grid:
                run = 0
                for v in line:
                    run = run + 1 if v else 0
                    if run > best:
                        best = run
        return best

    keep, drop = [], []
    for col in colours:
        c = col.astype(int)
        # The only true neutrals in Pebble-64 are #000000/#555555/#AAAAAA/
        # #FFFFFF. The mid two only ever arise as antialiasing between ink and
        # paper, so they are resolved along that axis rather than kept.
        neutral_fringe = c.max() == c.min() and 0 < c[0] < 255
        mask = (snapped == col).all(-1)
        n = int(mask.sum())
        if not neutral_fringe and (n / total >= MIN_SHARE or longest_run(mask) >= MIN_RUN):
            keep.append(col)
        else:
            drop.append(col)

    keep_arr = np.array(keep, dtype=int)
    flatpx = snapped.reshape(-1, 3)
    for col in drop:
        c = col.astype(int)
        if c.max() == c.min():
            target = np.array([0, 0, 0]) if c.mean() < 128 else np.array([255, 255, 255])
        else:
            target = keep_arr[(((keep_arr - c) ** 2).sum(1)).argmin()]
        flatpx[(flatpx == col).all(1)] = target
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
