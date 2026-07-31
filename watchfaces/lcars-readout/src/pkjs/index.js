// Phone-side weather fetch plus the Clay settings bridge. Open-Meteo needs no
// API key, which keeps the watchface installable without any per-user setup.

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var conditions = require('./conditions');
var COND = conditions.COND;
var conditionFromWmo = conditions.conditionFromWmo;

var REFRESH_MS = 30 * 60 * 1000;
var RETRY_MS = 60 * 1000;
var MAX_RETRIES = 3;

// Temperature is converted by Open-Meteo rather than on the watch — it takes a
// unit parameter, so it is one field in the URL instead of arithmetic and a
// rounding rule in C.
function tempUnit() {
  try {
    var v = localStorage.getItem('clay-settings');
    if (v) {
      var s = JSON.parse(v);
      if (s.TEMP_UNIT === '1' || s.TEMP_UNIT === 1) return 'fahrenheit';
    }
  } catch (e) { /* fall through to the default */ }
  return 'celsius';
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
            '&longitude=' + lon + '&current=temperature_2m,weather_code,is_day' +
            '&temperature_unit=' + tempUnit();

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
        IS_DAY: current.is_day ? 1 : 0,
        // So the watch knows which suffix to print.
        TEMP_UNIT: tempUnit() === 'fahrenheit' ? 1 : 0
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

  // Clay hands the saved settings straight to the watch; refetch afterwards
  // because the temperature unit changes what we have to ask Open-Meteo for.
  Pebble.addEventListener('webviewclosed', function (e) {
    if (!e || !e.response) return;
    updateWeather();
  });
}
