#pragma once

#include <pebble.h>

// Emery (Pebble Time 2): 200x228, colour, rounded corners.
#define SCREEN_W 200
#define SCREEN_H 228

// The LCARS frame — shapes, captions and the three fixed icons — is a single
// background bitmap (resources/images/background.png). Only the values below
// are drawn in code, so every coordinate here is measured against that image.
// Re-measure with tools/prep_background.py if the artwork changes.

#define C_TEXT GColorBlack

// ---------------------------------------------------------------------------
// Left rail. Blocks sit at y 130..171, 173..201 and 203..227; the battery
// takes the bottom one, the other two are decorative in this artwork.
// ---------------------------------------------------------------------------
#define BATT_X 0
#define BATT_Y 203
#define BATT_W 50
#define BATT_H 25

// ---------------------------------------------------------------------------
// Content column. Time and date sit in the two large gaps the artwork leaves.
// ---------------------------------------------------------------------------
#define CONT_X 52
#define CONT_W 146

#define TIME_Y 16
#define TIME_H 56

#define DATE_Y 109
#define DATE_H 32

// ---------------------------------------------------------------------------
// 2x2 readout grid. Row 1 is the band at y 159..176, row 2 at y 194..213.
// The heart (x126..145), thermometer (x57..66) and footprints (x127..144) are
// painted into the background; only the weather icon changes, so only that one
// is drawn here.
// ---------------------------------------------------------------------------
#define ROW1_Y 159
#define ROW2_Y 194
#define ROW_H  20

// The weather icon sits at the column's left edge rather than lining up with
// the thermometer at x57: "CLEAR" and "CLOUD" need 47px, and starting at 57
// only leaves 43. Text runs into the white gap before the VITALS column.
#define COND_ICON_X 52
#define COND_ICON_Y 158
#define ICON_SZ     20

#define COND_TEXT_X 74
#define COND_TEXT_W (126 - COND_TEXT_X)

#define TEMP_TEXT_X 70          // right of the narrow thermometer glyph
#define TEMP_TEXT_W (122 - TEMP_TEXT_X)

#define HR_TEXT_X 148           // right of the heart
#define HR_TEXT_W (198 - HR_TEXT_X)

#define STEPS_TEXT_X 148        // right of the footprints
#define STEPS_TEXT_W (198 - STEPS_TEXT_X)

// ---------------------------------------------------------------------------
// Weather condition codes exchanged with the phone (our own enum, mapped from
// WMO codes in src/pkjs/index.js).
// ---------------------------------------------------------------------------
typedef enum {
  COND_UNKNOWN = 0,
  COND_CLEAR   = 1,
  COND_CLOUD   = 2,
  COND_RAIN    = 3,
  COND_SNOW    = 4,
} WeatherCondition;
