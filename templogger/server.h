#ifndef __SERVER_H__
#define __SERVER_H__

// NOTE: Refer https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/ for more details
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Helvetica, sans-serif;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Air Quality Monitoring</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fa-solid fa-face-weary"></i>
    <span class="dht-labels">Dust Density</span>
    <span id="dust">%DUST%</span>
    <sup class="units"></sup>
  </p>
</body>
<script>
setInterval(function () {
  var req = new XMLHttpRequest();
  req.responseType = "json";
  req.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var resp = req.response;
      document.getElementById("temperature").innerHTML = resp.temperature.toFixed(1);
      document.getElementById("humidity").innerHTML = resp.humidity.toFixed(0);
      document.getElementById("dust_density").innerHTML = resp.dust_density.toFixed(1);
    }
  };
  req.open("GET", "/data", true);
  req.send();
}, 10000);

</script>
</html>)rawliteral";

void init_web_server();
String processor(const String &var);

#endif // __SERVER_H__