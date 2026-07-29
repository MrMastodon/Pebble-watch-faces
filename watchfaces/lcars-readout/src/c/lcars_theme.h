#pragma once

#include <pebble.h>

// Emery (Pebble Time 2): 200x228, color, rounded corners.
#define SCREEN_WIDTH  200
#define SCREEN_HEIGHT 228

// ---------------------------------------------------------------------------
// Left LCARS elbow column
// ---------------------------------------------------------------------------
#define ELBOW_W        40
#define ELBOW_STUB_W   90   // horizontal arm of the top-left elbow
#define ELBOW_RADIUS   14
#define ELBOW_UPPER_H  118
#define ELBOW_LOWER_Y  124
#define ELBOW_LOWER_H  82
#define ELBOW_FOOT_Y   210
#define ELBOW_FOOT_H   (SCREEN_HEIGHT - ELBOW_FOOT_Y)

// ---------------------------------------------------------------------------
// Content column (right of the elbow)
// ---------------------------------------------------------------------------
#define CONTENT_X 44
#define CONTENT_W (SCREEN_WIDTH - CONTENT_X)

#define TOP_BAR_H   22
#define DATE_BAR_X  94
#define DATE_BAR_W  (SCREEN_WIDTH - DATE_BAR_X)

#define TIME_PANEL_Y 26
#define TIME_PANEL_H 58

// Three stacked readout slots. Slot 0 and 1 are reserved for phase-2 data
// (weather, health); slot 2 carries the live battery/Bluetooth readout.
#define READOUT_COUNT   3
#define READOUT_FIRST_Y 88
#define READOUT_H       42
#define READOUT_LABEL_H 13
#define READOUT_GAP     4
#define READOUT_Y(i)    (READOUT_FIRST_Y + (i) * (READOUT_H + READOUT_GAP))

#define PANEL_RADIUS 6

// ---------------------------------------------------------------------------
// Palette — all part of the Pebble 64-color set.
// The frame stays dark and colorful like classic LCARS; every readout sits on
// a light panel with black text, which is the whole point of this watchface.
// ---------------------------------------------------------------------------
#define LCARS_COLOR_BACKGROUND   GColorBlack
#define LCARS_COLOR_ELBOW        GColorOrange
#define LCARS_COLOR_ELBOW_LOWER  GColorVividViolet
#define LCARS_COLOR_ELBOW_FOOT   GColorRoseVale
#define LCARS_COLOR_DATE_BAR     GColorChromeYellow

#define LCARS_COLOR_LABEL_BAR    GColorRoseVale
#define LCARS_COLOR_LABEL_BAR_2  GColorLavenderIndigo
#define LCARS_COLOR_LABEL_BAR_3  GColorChromeYellow

#define LCARS_COLOR_PANEL_BG     GColorWhite
#define LCARS_COLOR_PANEL_TEXT   GColorBlack
#define LCARS_COLOR_LABEL_TEXT   GColorBlack
