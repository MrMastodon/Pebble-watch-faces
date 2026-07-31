#!/usr/bin/env python3
"""Measure how wide a string renders in Antonio, against the field it goes in.

Letter count is a bad proxy: W is roughly twice the width of I, so SHWRS
(5 chars) overflows the readout column while CLEAR (5 chars) does not. This
renders with the actual font so a candidate can be checked before it ships.

    python3 tools/measure_text.py SHWR SNSH STRM
"""

import os
import sys

from PIL import ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
FONT = os.path.join(HERE, "..", "resources", "fonts", "Antonio-Bold.ttf")

# Calibrated so CLEAR matches the 47px it actually renders on the watch at
# Antonio 22; PIL's sizing does not map 1:1 to Pebble's font generator.
CAL_PX = 21
BUDGET = 48      # readout column, x74..121
SAFE = 45        # leave room for the ~1px calibration error

# Treat this as a filter, not a verdict. It under-estimates strings containing
# the degree sign: PIL called "999°C" 47px, but on the watch it overflows 49
# and ellipsises. Anything close to the budget should still be checked in the
# emulator.


def main():
    words = sys.argv[1:] or ["CLEAR", "PTCLD", "CLDY", "FOG", "DRIZL", "RAIN",
                             "FZRN", "SNOW", "SHWR", "SNSH", "STRM"]
    font = ImageFont.truetype(FONT, CAL_PX)
    worst = 0
    for w in words:
        box = font.getbbox(w)
        width = box[2] - box[0]
        worst = max(worst, width)
        verdict = "ok" if width <= SAFE else (
            "marginal" if width <= BUDGET else "TOO WIDE")
        print(f"  {w:8} {width:3d} px  {verdict}")
    print(f"\nwidest {worst} px of {BUDGET} px budget")
    return 1 if worst > BUDGET else 0


if __name__ == "__main__":
    sys.exit(main())
