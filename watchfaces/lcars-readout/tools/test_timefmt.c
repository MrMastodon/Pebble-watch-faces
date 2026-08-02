// Exercises src/c/clock_format.h on the host: all 1440 minutes of the day
// through clock_format_12h(), and every shape a firmware might hand back
// through clock_split_time_string(). The midnight and noon edges, and the
// padded "07:30 PM" that the leading-zero-on-12h preference produces, are not
// things to check by squinting at an emulator — and this SDK's firmware cannot
// produce the padded form at all.
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

// The watchface's own buffers, so the size limits under test are the real ones.
static void check_split(const char *raw, bool want_ok,
                        const char *want_digits, const char *want_suffix) {
  char digits[8], suffix[4];
  bool ok = clock_split_time_string(raw, digits, sizeof(digits),
                                    suffix, sizeof(suffix));
  if (ok != want_ok) {
    printf("  FAIL split \"%s\": returned %d, wanted %d\n", raw, ok, want_ok);
    failures++;
    return;
  }
  if (!ok) return;
  if (strcmp(digits, want_digits) != 0 || strcmp(suffix, want_suffix) != 0) {
    printf("  FAIL split \"%s\": got \"%s\"+\"%s\", wanted \"%s\"+\"%s\"\n",
           raw, digits, suffix, want_digits, want_suffix);
    failures++;
  }
}

int main(void) {
  // What clock_copy_time_string() can hand back. The first is what Pebble OS
  // 4.17 actually returns, measured; the third is the padded form the
  // leading-zero-on-12h preference produces on a newer firmware.
  check_split("7:30 AM", true, "7:30", "AM");
  check_split("12:00 PM", true, "12:00", "PM");
  check_split("07:30 PM", true, "07:30", "PM");
  check_split("7:30PM", true, "7:30", "PM");    // no separator
  check_split("7:30 pm", true, "7:30", "PM");   // Antonio has no lowercase
  check_split("19:23", true, "19:23", "");      // 24h shape, no suffix
  check_split("", false, "", "");               // nothing at all
  check_split("PM", false, "", "");             // no digits
  check_split("123456789 AM", false, "", "");   // longer than the buffer

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
    printf("ok: 1440 minutes + 8 edge cases + 9 firmware string shapes\n");
    return 0;
  }
  printf("%d failure(s)\n", failures);
  return 1;
}
