#pragma once

#include <pebble.h>

// Emery (Pebble Time 2): 200x228, colour, rounded corners.
#define SCREEN_W 200
#define SCREEN_H 228

// ---------------------------------------------------------------------------
// Palette
//
// Light face: white ground, black type, pastel LCARS shapes. This is the
// inverse of classic LCARS and the whole reason the watchface exists — the
// numbers have to stay readable in daylight on a reflective display.
// ---------------------------------------------------------------------------
#define C_BG        GColorWhite
#define C_TEXT      GColorBlack
#define C_BAR       GColorBabyBlueEyes   // pale filler bars and the top block
#define C_CAP       GColorLavenderIndigo // pills and end caps
#define C_ELBOW_A   GColorPictonBlue     // upper (TIME) elbow
#define C_ELBOW_B   GColorRoseVale       // lower (STARDATE) elbow, LINK block
#define C_ACCENT    GColorSunsetOrange   // decorative red block, rail ticks
#define C_BATTERY   GColorRajah          // battery block

// ---------------------------------------------------------------------------
// Columns
// ---------------------------------------------------------------------------
#define RAIL_X 2
#define RAIL_W 46
#define RAIL_R (RAIL_X + RAIL_W)   // 48

#define CONT_X 52
#define CONT_W (SCREEN_W - CONT_X - 2)  // 146
#define CONT_R (CONT_X + CONT_W)

// ---------------------------------------------------------------------------
// Left rail blocks
// ---------------------------------------------------------------------------
#define TOPBLOCK_Y 2
#define TOPBLOCK_H 14

#define ELBOW_A_Y    18
#define ELBOW_A_H    72   // vertical arm, y 18..90
#define ELBOW_A_ARMY 78   // horizontal arm sits at the bottom of the L
#define ELBOW_A_ARMH 12
#define ELBOW_A_ARMW 94

#define ELBOW_B_Y    94
#define ELBOW_B_H    44   // vertical arm, y 94..138
#define ELBOW_B_ARMH 12   // horizontal arm sits at the top of the gamma
#define ELBOW_B_ARMW 64

#define LINK_Y 140
#define LINK_H 32
#define RED_Y  174
#define RED_H  22
#define BATT_Y 198
#define BATT_H 28

#define ELBOW_R 12   // outer corner radius
#define INNER_R 10   // concave inner corner radius
#define BLOCK_R 4

// ---------------------------------------------------------------------------
// Content column
// ---------------------------------------------------------------------------
#define HDR_H      12
#define HDR_PILL_W 7
#define HDR_CAP_W  9
#define HDR_GAP    3

#define TIME_HDR_Y 3
#define TIME_Y     13
#define TIME_H     64

#define RAIL_BAR_Y 80
#define RAIL_BAR_H 10
#define RAIL_BAR_X 100   // clears the elbow arm, which ends at x=96

// The STARDATE caption starts clear of the gamma elbow's arm, so the arm reads
// as running into the caption rather than being painted over.
#define DATE_HDR_X (RAIL_X + ELBOW_B_ARMW + 2)   // 68
#define DATE_HDR_W (SCREEN_W - 2 - DATE_HDR_X)
#define DATE_HDR_Y 94
#define DATE_Y     104
#define DATE_H     38

// 2x2 readout grid. The left column's second row (temperature) deliberately
// has no caption of its own — it reads as a continuation of SENSORS.
#define RO_L_X 52
#define RO_L_W 70
#define RO_R_X 126
#define RO_R_W 72

#define RO_HDR1_Y 143
#define RO_ROW1_Y 155
#define RO_HDR2_Y 179
#define RO_ROW2_Y 191
#define RO_ROW_H  24

#define ICON_SZ 20

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
