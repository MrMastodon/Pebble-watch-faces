#pragma once

#include <pebble.h>

// Emery (Pebble Time 2) display: 200x228, color, rounded corners.
#define SCREEN_WIDTH  200
#define SCREEN_HEIGHT 228

// Left "elbow" column.
#define ELBOW_WIDTH        40
#define ELBOW_CORNER_RADIUS 12
#define ELBOW_UPPER_RATIO  0.4f // fraction of height that is the upper (orange) block

// Top date bar, spans from the elbow to the right edge.
#define TOP_BAR_HEIGHT 20

// Rounded corner radius used on the light readout panels.
#define PANEL_CORNER_RADIUS 8

// Colors. All are part of the Pebble 64-color palette and render correctly
// on Emery / Basalt / Chalk.
#define LCARS_COLOR_ELBOW_UPPER   GColorOrange
#define LCARS_COLOR_ELBOW_LOWER   GColorVividViolet
#define LCARS_COLOR_TOP_BAR       GColorChromeYellow
#define LCARS_COLOR_LABEL_BAR     GColorRoseVale

#define LCARS_COLOR_PANEL_BG      GColorWhite
#define LCARS_COLOR_PANEL_BG_ALT  GColorLightGray
#define LCARS_COLOR_PANEL_TEXT    GColorBlack
#define LCARS_COLOR_LABEL_TEXT    GColorBlack

#define LCARS_COLOR_BACKGROUND    GColorBlack
