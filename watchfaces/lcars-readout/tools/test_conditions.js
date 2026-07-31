// Checks the WMO -> condition mapping in src/pkjs/index.js.
//
// The mapping is a dozen numeric ranges, which is precisely the shape of code
// where an off-by-one hides quietly: a wrong bound does not crash, it just
// shows RAIN when it should show SNOW, on some days, on the phone, where it is
// nearly impossible to notice. Walking every documented code is cheap.
//
//   node tools/test_conditions.js

var w = require('../src/pkjs/conditions.js');
var C = w.COND;

// Every code Open-Meteo documents, with what it should map to.
var EXPECT = {
  0: C.CLEAR, 1: C.CLEAR, 2: C.PARTLY, 3: C.CLOUD,
  45: C.FOG, 48: C.FOG,
  51: C.DRIZZLE, 53: C.DRIZZLE, 55: C.DRIZZLE,
  56: C.FZRAIN, 57: C.FZRAIN,
  61: C.RAIN, 63: C.RAIN, 65: C.RAIN,
  66: C.FZRAIN, 67: C.FZRAIN,
  71: C.SNOW, 73: C.SNOW, 75: C.SNOW, 77: C.SNOW,
  80: C.SHOWERS, 81: C.SHOWERS, 82: C.SHOWERS,
  85: C.SNOWSH, 86: C.SNOWSH,
  95: C.STORM, 96: C.STORM, 99: C.STORM
};

var failures = 0;
var name = {};
Object.keys(C).forEach(function (k) { name[C[k]] = k; });

Object.keys(EXPECT).forEach(function (code) {
  var got = w.conditionFromWmo(Number(code));
  if (got !== EXPECT[code]) {
    console.error('WMO ' + code + ': expected ' + name[EXPECT[code]] +
                  ', got ' + name[got]);
    failures++;
  }
});

// Undocumented codes must fall through rather than land on a real condition.
[4, 20, 44, 50, 60, 70, 79, 84, 90, 100, -1].forEach(function (code) {
  if (w.conditionFromWmo(code) !== C.UNKNOWN) {
    console.error('WMO ' + code + ': undocumented code should be UNKNOWN, got ' +
                  name[w.conditionFromWmo(code)]);
    failures++;
  }
});

// The failure codes have to stay clear of the real ones, since the watch
// persists anything in the real range as a genuine reading.
if (C.NO_LOCATION <= C.STORM || C.NO_NET <= C.STORM) {
  console.error('failure codes overlap the real condition range');
  failures++;
}

if (failures) {
  console.error('\n' + failures + ' failure(s)');
  process.exit(1);
}
console.log('condition mapping OK (' + Object.keys(EXPECT).length +
            ' WMO codes + fallthrough + range check)');
