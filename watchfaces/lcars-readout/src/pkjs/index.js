// Phone-side weather fetch. Open-Meteo needs no API key, which keeps the
// watchface installable without any per-user setup.

var COND_UNKNOWN = 0;
var COND_CLEAR = 1;
var COND_CLOUD = 2;
var COND_RAIN = 3;
var COND_SNOW = 4;
// Failure states, so a blank readout can be told apart from a broken one.
var COND_NO_LOCATION = 5;
var COND_NO_NET = 6;

var REFRESH_MS = 30 * 60 * 1000;
var RETRY_MS = 60 * 1000;
var MAX_RETRIES = 3;

// WMO weather interpretation codes -> the four buckets the watch can draw.
function conditionFromWmo(code) {
  if (code === 0 || code === 1) return COND_CLEAR;
  if (code === 2 || code === 3 || code === 45 || code === 48) return COND_CLOUD;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82) ||
      (code >= 95 && code <= 99)) return COND_RAIN;
  if ((code >= 71 && code <= 77) || code === 85 || code === 86) return COND_SNOW;
  return COND_UNKNOWN;
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
            '&longitude=' + lon + '&current=temperature_2m,weather_code';

  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.timeout = 20000;

  function failed(why) {
    console.log('weather fetch failed: ' + why);
    if (attempt < MAX_RETRIES) {
      setTimeout(function () { fetchWeather(lat, lon, attempt + 1); }, RETRY_MS);
    } else {
      send({ CONDITION: COND_NO_NET });
    }
  }

  req.onload = function () {
    if (req.status !== 200) return failed('http ' + req.status);
    try {
      var current = JSON.parse(req.responseText).current;
      send({
        CONDITION: conditionFromWmo(current.weather_code),
        TEMPERATURE: Math.round(current.temperature_2m)
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
        send({ CONDITION: COND_NO_LOCATION });
      }
    },
    // Generous: a cold GPS fix is slow, and a half-hour-old position is plenty
    // accurate for weather.
    { timeout: 30000, maximumAge: 30 * 60 * 1000 }
  );
}

Pebble.addEventListener('ready', function () {
  updateWeather();
  // Open-Meteo updates hourly, so polling faster only costs battery.
  setInterval(function () { updateWeather(); }, REFRESH_MS);
});
