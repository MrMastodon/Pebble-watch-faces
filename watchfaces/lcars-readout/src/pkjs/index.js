// Phone-side weather fetch. Open-Meteo needs no API key, which keeps the
// watchface installable without any per-user setup.

// Must match WeatherCondition in src/c/lcars_theme.h.
var COND = {
  UNKNOWN: 0, CLEAR: 1, PARTLY: 2, CLOUD: 3, FOG: 4, DRIZZLE: 5,
  RAIN: 6, FZRAIN: 7, SNOW: 8, SHOWERS: 9, SNOWSH: 10, STORM: 11,
  NO_LOCATION: 90, NO_NET: 91
};

var REFRESH_MS = 30 * 60 * 1000;
var RETRY_MS = 60 * 1000;
var MAX_RETRIES = 3;

// WMO weather interpretation codes -> the conditions the watch can show.
// Exported for tools/test_conditions.js, which walks every documented code.
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

function send(msg) {
  Pebble.sendAppMessage(msg,
    function () { console.log('sent ' + JSON.stringify(msg)); },
    function (e) { console.log('send failed: ' + JSON.stringify(e)); });
}

// Remember the last fix. Weather does not need precision, so a stale position
// beats no weather at all when the GPS will not answer.
function rememberPosition(lat, lon) {
  try {
    localStorage.setItem('lat', String(lat));
    localStorage.setItem('lon', String(lon));
  } catch (e) { /* storage unavailable; not worth failing over */ }
}

function lastPosition() {
  try {
    var lat = parseFloat(localStorage.getItem('lat'));
    var lon = parseFloat(localStorage.getItem('lon'));
    if (!isNaN(lat) && !isNaN(lon)) return { lat: lat, lon: lon };
  } catch (e) { /* ditto */ }
  return null;
}

function fetchWeather(lat, lon, attempt) {
  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + lat +
            '&longitude=' + lon + '&current=temperature_2m,weather_code,is_day';

  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.timeout = 20000;

  function failed(why) {
    console.log('weather fetch failed: ' + why);
    if (attempt < MAX_RETRIES) {
      setTimeout(function () { fetchWeather(lat, lon, attempt + 1); }, RETRY_MS);
    } else {
      send({ CONDITION: COND.NO_NET });
    }
  }

  req.onload = function () {
    if (req.status !== 200) return failed('http ' + req.status);
    try {
      var current = JSON.parse(req.responseText).current;
      send({
        CONDITION: conditionFromWmo(current.weather_code),
        TEMPERATURE: Math.round(current.temperature_2m),
        // Drives the moon variants for clear and partly-cloudy.
        IS_DAY: current.is_day ? 1 : 0
      });
    } catch (e) {
      failed('parse error: ' + e);
    }
  };
  req.onerror = function () { failed('network error'); };
  req.ontimeout = function () { failed('timeout'); };
  req.send();
}

function updateWeather(attempt) {
  attempt = attempt || 0;

  navigator.geolocation.getCurrentPosition(
    function (pos) {
      rememberPosition(pos.coords.latitude, pos.coords.longitude);
      fetchWeather(pos.coords.latitude, pos.coords.longitude, 0);
    },
    function (err) {
      console.log('location unavailable: ' + err.message);
      var last = lastPosition();
      if (last) {
        fetchWeather(last.lat, last.lon, 0);
      } else if (attempt < MAX_RETRIES) {
        // A cold fix indoors can easily miss the first window; retrying beats
        // going quiet until the next half-hourly tick.
        setTimeout(function () { updateWeather(attempt + 1); }, RETRY_MS);
      } else {
        send({ CONDITION: COND.NO_LOCATION });
      }
    },
    // Generous: a cold GPS fix is slow, and a half-hour-old position is plenty
    // accurate for weather.
    { timeout: 30000, maximumAge: 30 * 60 * 1000 }
  );
}

// Guarded so the module can be required by the mapping test, where no Pebble
// runtime exists.
if (typeof Pebble !== 'undefined') {
  Pebble.addEventListener('ready', function () {
    updateWeather();
    // Open-Meteo updates hourly, so polling faster only costs battery.
    setInterval(function () { updateWeather(); }, REFRESH_MS);
  });
}

// Exposed for the offline mapping test; harmless in the phone runtime.
if (typeof module !== 'undefined') {
  module.exports = { COND: COND, conditionFromWmo: conditionFromWmo };
}
