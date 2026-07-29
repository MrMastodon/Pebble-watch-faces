// Phone-side weather fetch. Open-Meteo needs no API key, which keeps the
// watchface installable without any per-user setup.

var COND_UNKNOWN = 0;
var COND_CLEAR = 1;
var COND_CLOUD = 2;
var COND_RAIN = 3;
var COND_SNOW = 4;

// WMO weather interpretation codes -> the four buckets the watch can draw.
function conditionFromWmo(code) {
  if (code === 0 || code === 1) return COND_CLEAR;
  if (code === 2 || code === 3 || code === 45 || code === 48) return COND_CLOUD;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82) ||
      (code >= 95 && code <= 99)) return COND_RAIN;
  if ((code >= 71 && code <= 77) || code === 85 || code === 86) return COND_SNOW;
  return COND_UNKNOWN;
}

function sendWeather(cond, tempC) {
  Pebble.sendAppMessage(
    { CONDITION: cond, TEMPERATURE: Math.round(tempC) },
    function () { console.log('weather sent: ' + cond + ' ' + tempC); },
    function (e) { console.log('weather send failed: ' + JSON.stringify(e)); }
  );
}

function fetchWeather(lat, lon) {
  var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + lat +
            '&longitude=' + lon + '&current=temperature_2m,weather_code';

  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function () {
    if (req.status !== 200) {
      console.log('open-meteo http ' + req.status);
      return;
    }
    try {
      var current = JSON.parse(req.responseText).current;
      sendWeather(conditionFromWmo(current.weather_code), current.temperature_2m);
    } catch (e) {
      console.log('open-meteo parse error: ' + e);
    }
  };
  req.onerror = function () { console.log('open-meteo request failed'); };
  req.send();
}

function updateWeather() {
  navigator.geolocation.getCurrentPosition(
    function (pos) { fetchWeather(pos.coords.latitude, pos.coords.longitude); },
    function (err) { console.log('location unavailable: ' + err.message); },
    { timeout: 15000, maximumAge: 60000 }
  );
}

Pebble.addEventListener('ready', function () {
  updateWeather();
  // Open-Meteo updates hourly, so polling faster only costs battery.
  setInterval(updateWeather, 30 * 60 * 1000);
});
