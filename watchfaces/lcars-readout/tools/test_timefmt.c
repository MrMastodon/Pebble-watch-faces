// Walks all 1440 minutes of the day through the same clock_format_12h() the
// watchface uses, so the midnight and noon edges are checked by something other
// than a glance at the emulator.
//
//     gcc -o /tmp/test_timefmt tools/test_timefmt.c && /tmp/test_timefmt

#include <stdio.h>
#include <string.h>

#include "../src/c/clock_format.h"

static int failures = 0;

static void fail(const char *why, int h, int m,
                 const char *digits, const char *suffix) {
  printf("  FAIL %02d:%02d -> \"%s %s\": %s\n", h, m, digits, suffix, why);
  failures++;
}

static void check_case(int h, int m, const char *want) {
  char digits[8], suffix[4], got[16];
  clock_format_12h(h, m, digits, sizeof(digits), suffix, sizeof(suffix));
  snprintf(got, sizeof(got), "%s %s", digits, suffix);
  if (strcmp(got, want) != 0) {
    printf("  FAIL %02d:%02d -> \"%s\", wanted \"%s\"\n", h, m, got, want);
    failures++;
  }
}

int main(void) {
  // The edges, spelled out. 00:xx and 12:xx are the two that % 12 gets wrong.
  check_case(0, 0, "12:00 AM");
  check_case(0, 59, "12:59 AM");
  check_case(1, 0, "1:00 AM");
  check_case(11, 59, "11:59 AM");
  check_case(12, 0, "12:00 PM");
  check_case(12, 59, "12:59 PM");
  check_case(13, 0, "1:00 PM");
  check_case(23, 59, "11:59 PM");

  // And the invariants, over the whole day.
  for (int h = 0; h < 24; h++) {
    for (int m = 0; m < 60; m++) {
      char digits[8], suffix[4];
      clock_format_12h(h, m, digits, sizeof(digits), suffix, sizeof(suffix));

      int gh = 0, gm = 0;
      if (sscanf(digits, "%d:%d", &gh, &gm) != 2) {
        fail("unparseable", h, m, digits, suffix);
        continue;
      }
      if (gh < 1 || gh > 12) fail("hour outside 1..12", h, m, digits, suffix);
      if (gm != m) fail("minute changed", h, m, digits, suffix);
      if (digits[0] == '0') fail("leading zero on the hour", h, m, digits, suffix);

      const char *colon = strchr(digits, ':');
      if (!colon || strlen(colon + 1) != 2) {
        fail("minute is not two digits", h, m, digits, suffix);
      }

      const char *want = (h < 12) ? "AM" : "PM";
      if (strcmp(suffix, want) != 0) fail("wrong suffix", h, m, digits, suffix);

      // The watchface buffer is char[8]; anything longer would be truncated
      // there rather than here.
      if (strlen(digits) > 5) fail("digits longer than \"12:59\"", h, m, digits, suffix);
    }
  }

  if (failures == 0) {
    printf("ok: 1440 minutes + 8 edge cases\n");
    return 0;
  }
  printf("%d failure(s)\n", failures);
  return 1;
}
