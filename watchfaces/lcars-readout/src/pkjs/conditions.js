// WMO weather interpretation codes -> the conditions the watch can show.
//
// Kept in its own module so tools/test_conditions.js can exercise it without
// pulling in index.js, which requires Clay — and Clay requires a config page
// that only exists after the SDK build step has run.
//
// Values must match WeatherCondition in src/c/lcars_theme.h.
var COND = {
  UNKNOWN: 0, CLEAR: 1, PARTLY: 2, CLOUD: 3, FOG: 4, DRIZZLE: 5,
  RAIN: 6, FZRAIN: 7, SNOW: 8, SHOWERS: 9, SNOWSH: 10, STORM: 11,
  NO_LOCATION: 90, NO_NET: 91
};

function conditionFromWmo(code) {
  if (code === 0 || code === 1) return COND.CLEAR;
  if (code === 2) return COND.PARTLY;
  if (code === 3) return COND.CLOUD;
  if (code === 45 || code === 48) return COND.FOG;
  if (code >= 51 && code <= 55) return COND.DRIZZLE;
  if (code === 56 || code === 57) return COND.FZRAIN;
  if (code >= 61 && code <= 65) return COND.RAIN;
  if (code === 66 || code === 67) return COND.FZRAIN;
  if ((code >= 71 && code <= 75) || code === 77) return COND.SNOW;
  if (code >= 80 && code <= 82) return COND.SHOWERS;
  if (code === 85 || code === 86) return COND.SNOWSH;
  if (code >= 95 && code <= 99) return COND.STORM;
  return COND.UNKNOWN;
}

module.exports = { COND: COND, conditionFromWmo: conditionFromWmo };
