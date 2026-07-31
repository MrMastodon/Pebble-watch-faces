#pragma once

#include <pebble.h>

// Emery (Pebble Time 2): 200x228, colour, rounded corners.
#define SCREEN_W 200
#define SCREEN_H 228

// The LCARS frame — shapes, captions and the three fixed icons — is a single
// background bitmap (resources/images/background.png). Only the values below
// are drawn in code, so every coordinate here is measured against that image.
// Re-measure whenever the artwork changes.
//
// Artwork reference points (LCARS-readout_background_4.png):
//   left rail blocks   y 0..20, 22..87, 92..128, 130..171, 173..201, 203..227
//   heart icon         x 128..144, y 159..173
//   thermometer icon   x  63..70,  y 199..215
//   footprints icon    x 128..144, y 200..215
//
// The two row-2 icons are not on a shared baseline, so each value gets its own
// box rather than sharing a grid row.

#define C_TEXT GColorBlack

// ---------------------------------------------------------------------------
// Left rail. Battery sits in the red block (y 174..200 inside its outline),
// not the bottom one — it reads far better against that colour.
// ---------------------------------------------------------------------------
#define BATT_X 0
#define BATT_Y 182
#define BATT_W 50
#define BATT_H 15

// Bluetooth indicator in the top block (interior x 1..48, y 1..19). Shows LINK
// while connected and nothing at all when not, so a broken link reads as an
// absence rather than as another label to parse. The block is 19px tall
// against the bottom one's 23, so the glyphs sit tighter here.
#define LINK_X 0
#define LINK_Y 4
#define LINK_W 50
#define LINK_H 15

// ---------------------------------------------------------------------------
// Content column
// ---------------------------------------------------------------------------
#define CONT_X 52
#define CONT_W 146

#define TIME_Y 16
#define TIME_H 56

#define DATE_Y 109
#define DATE_H 32

// ---------------------------------------------------------------------------
// Readouts. Columns split at x126: SENSORS on the left, VITALS/STEPS right.
// ---------------------------------------------------------------------------
#define ICON_SZ 17

// Weather icon — the only one drawn in code, since it is the only one that
// changes. Sits on the thermometer's x so the two left-column rows line up.
#define COND_ICON_X 57
#define COND_ICON_Y 158

#define COND_TEXT_X 76
#define COND_TEXT_W (125 - COND_TEXT_X)
#define COND_TEXT_Y 159
#define COND_TEXT_H 15

#define HR_TEXT_X 147           // right of the heart
#define HR_TEXT_W (198 - HR_TEXT_X)
#define HR_TEXT_Y 159
#define HR_TEXT_H 15

// Shares COND_TEXT_X so the two left-column values line up, even though the
// thermometer glyph is narrower than the weather icon above it.
#define TEMP_TEXT_X COND_TEXT_X
#define TEMP_TEXT_W (125 - TEMP_TEXT_X)
#define TEMP_TEXT_Y 200
#define TEMP_TEXT_H 16

#define STEPS_TEXT_X 147        // right of the footprints
#define STEPS_TEXT_W (198 - STEPS_TEXT_X)
#define STEPS_TEXT_Y 200
#define STEPS_TEXT_H 16

// ---------------------------------------------------------------------------
// Weather condition codes exchanged with the phone (our own enum, mapped from
// WMO codes in src/pkjs/index.js).
// ---------------------------------------------------------------------------
// Settings from the Clay panel. Values must match src/pkjs/config.js.
typedef enum {
  DATE_DMY = 0,   // 31.12.2026
  DATE_MDY = 1,   // 12.31.2026
  DATE_ISO = 2,   // 2026-12-31
} DateFormat;

typedef enum {
  VIBE_OFF   = 0,
  VIBE_SHORT = 1,
  VIBE_LONG  = 2,
} VibeStrength;

typedef enum {
  COND_UNKNOWN = 0,
  COND_CLEAR   = 1,
  COND_PARTLY  = 2,
  COND_CLOUD   = 3,
  COND_FOG     = 4,
  COND_DRIZZLE = 5,
  COND_RAIN    = 6,
  COND_FZRAIN  = 7,
  COND_SNOW    = 8,
  COND_SHOWERS = 9,
  COND_SNOWSH  = 10,
  COND_STORM   = 11,
  COND_LAST_REAL = COND_STORM,
  // Failure states from the phone, so an empty readout can be told apart from
  // a broken one when debugging on the wrist. Numbered clear of the real
  // conditions so those can grow without colliding.
  COND_NO_LOCATION = 90,
  COND_NO_NET      = 91,
} WeatherCondition;
