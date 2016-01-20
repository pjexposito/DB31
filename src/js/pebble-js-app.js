Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
  updateWeather();



});

Pebble.addEventListener("appmessage", function(e) {
    //console.log("Got a message - Starting weather request...");
    updateWeather();
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://dl.dropboxusercontent.com/u/119376/config-time/index.html';

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  Pebble.sendAppMessage({"KEY_PIDE":1, "KEY_IDIOMA": configData.idioma,"KEY_VIBE": configData.vibe, "KEY_DATEFORMAT": configData.dateformat,"KEY_SEGUNDOS": configData.segundos,"KEY_HOURLYVIBE": configData.hourlyvibe,"KEY_BACK": configData.back }); 
});

var updateInProgress = false;

function updateWeather() {
    if (!updateInProgress) {
        updateInProgress = true;
        var locationOptions = { maximumAge:60000, timeout:15000 };
        navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    }
    else {
        console.log("Not starting a new request. Another one is in progress...");
    }
}

function locationSuccess(pos) {
    var coordinates = pos.coords;
    //console.log("Got coordinates: " + JSON.stringify(coordinates));
    fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
    //console.warn('Location error (' + err.code + '): ' + err.message);
  console.log("Error de localizacion");  
  Pebble.sendAppMessage({ "error": "Loc unavailable" });
    updateInProgress = false;
}

function fetchWeather(latitude, longitude) {
    console.log(latitude + " " + longitude);
    var response;
    var req = new XMLHttpRequest();
    req.open('GET', "http://api.openweathermap.org/data/2.5/weather?" +
        "lat=" + latitude + "&lon=" + longitude + "&cnt=1&appid=e1bb0121e6613061eccb789a4283fb0a", true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if(req.status == 200) {
                //console.log(req.responseText);
                response = JSON.parse(req.responseText);
                var temperature, condition;
                if (response) {
                    var tempResult = response.main.temp;
                    if (response.sys.country === "US") {
                        // Convert temperature to Fahrenheit if user is within the US
                        temperature = Math.round(((tempResult - 273.15) * 1.8) + 32);
                    }
                    else {
                        // Otherwise, convert temperature to Celsius
                        temperature = Math.round(tempResult - 273.15);
                    }		 
                    condition = response.weather[0].id;


                    console.log("Temperature: " + temperature + " Condition: " + condition);
                  Pebble.sendAppMessage({"KEY_PIDE":0, "KEY_CONDICION": condition,"KEY_TEMPERATURA": temperature});
                    updateInProgress = false;
                }
            } else {
                //console.log("Error");
                updateInProgress = false;
              console.log("Error de HTTP");
                Pebble.sendAppMessage({ "error": "HTTP Error" });
            }
        }
    };
    req.send(null);
}