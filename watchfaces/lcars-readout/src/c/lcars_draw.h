#pragma once

#include <pebble.h>

// A filled LCARS block with all four corners rounded.
void lcars_block(GContext *ctx, GRect rect, int radius, GColor color);

// Pale filler bar with a hairline outline — without it the pale blue washes
// out against the white ground.
void lcars_bar(GContext *ctx, GRect rect, int radius, GColor color);

// Elbow with the horizontal arm at the *bottom* of the vertical (an "L").
// `vert` is the vertical arm; the horizontal arm shares its left edge.
void lcars_elbow_bottom(GContext *ctx, GRect vert, int arm_y, int arm_w, int arm_h,
                        int radius, int inner_radius, GColor color, GColor bg);

// Elbow with the horizontal arm at the *top* of the vertical (a "gamma").
void lcars_elbow_top(GContext *ctx, GRect vert, int arm_w, int arm_h,
                     int radius, int inner_radius, GColor color, GColor bg);

// Caption row: pill, label, filler bar, rounded end cap. With `leading_pill`
// the pill sits before the label (TIME, SENSORS, ...); without it the pill
// becomes a small block just after the label (STARDATE). The filler bar is
// dropped automatically when the label leaves no room for it.
void lcars_header(GContext *ctx, GRect box, const char *label, GFont font,
                  bool leading_pill, GColor pill, GColor bar, GColor cap);
