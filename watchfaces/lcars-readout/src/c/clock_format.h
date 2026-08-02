#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Clock string handling, kept apart from the watchface so tools/test_timefmt.c
// can exercise the same code on the host. Midnight, noon and the shapes a
// firmware might hand back are not things to check by squinting at an emulator.

// 12-hour formatting, used when the firmware's own string is unusable.
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

// Splits what clock_copy_time_string() handed us into digits and suffix. The
// suffix starts at the first character that is not part of a clock reading,
// whatever it turns out to be, so this does not care whether the firmware writes
// "7:30 PM", "07:30 PM" or "7:30PM" — the padded form is what the
// leading-zero-on-12h preference produces, and this SDK's firmware predates it.
//
// The suffix is uppercased on the way in for the same reason %p is avoided
// above: a firmware that writes "pm" would otherwise render as nothing.
//
// Returns false if the string is not a clock reading at all, or if the digits
// would not fit, so the caller can fall back rather than draw nothing.
static inline bool clock_split_time_string(const char *raw,
                                           char *digits, size_t dn,
                                           char *suffix, size_t sn) {
  size_t i = 0;
  while (raw[i] && (isdigit((unsigned char)raw[i]) || raw[i] == ':')) i++;
  if (i == 0 || i >= dn) return false;

  memcpy(digits, raw, i);
  digits[i] = '\0';

  while (raw[i] == ' ') i++;
  size_t j = 0;
  while (raw[i] && j + 1 < sn) {
    suffix[j++] = (char)toupper((unsigned char)raw[i++]);
  }
  suffix[j] = '\0';
  return true;
}
