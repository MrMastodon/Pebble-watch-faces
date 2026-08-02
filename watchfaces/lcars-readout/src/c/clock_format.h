#pragma once

#include <stdio.h>
#include <stddef.h>

// 12-hour formatting, kept apart from the watchface so tools/test_timefmt.c can
// exercise the same code on the host. Midnight and noon are where a hastily
// written % 12 goes wrong, and that is not something to check by squinting at
// an emulator.
//
// Deliberately not strftime("%I") and "%p":
//   %I pads to two digits, so 9am reads "09:05" instead of "9:05".
//   %p depends on the locale. If it ever returns lowercase "am", Antonio's
//   character set has no lowercase glyphs and the suffix renders as nothing at
//   all — a silent failure. Deriving both from the hour avoids the question.
static inline void clock_format_12h(int hour24, int minute,
                                    char *digits, size_t dn,
                                    char *suffix, size_t sn) {
  int h = hour24 % 12;
  if (h == 0) h = 12;          // 00:xx and 12:xx both display as 12
  snprintf(digits, dn, "%d:%02d", h, minute);
  snprintf(suffix, sn, "%s", hour24 < 12 ? "AM" : "PM");
}
