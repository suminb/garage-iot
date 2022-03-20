#include <Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
// NOTE: Refer https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/ for more details
#include <ESPAsyncWebServer.h>
#include "dust.h"

#define LED 2

#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
float d = 0.0;

const char *ssid = "FBI Undercover Van";
const char *password = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
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
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("dust").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/dust", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String &var)
{
    // Serial.println(var);
    if (var == "TEMPERATURE")
    {
        return String(t);
    }
    else if (var == "HUMIDITY")
    {
        return String(h);
    }
    else if (var == "DUST")
    {
        return String(d);
    }
    return String();
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }
    Serial.println("Getting started...");
    dht.begin();
    dustSensor.begin();
    pinMode(LED, OUTPUT);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    // Print ESP8266 Local IP Address
    Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html, processor); });
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", String(t).c_str()); });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", String(h).c_str()); });
    server.on("/dust", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", String(d).c_str()); });

    // Start server
    server.begin();
}

void loop()
{
    digitalWrite(LED, LOW);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();

    // float d = dustSensor.getDustDensity();
    d = read_dust_density();

    digitalWrite(LED, HIGH);

    // // Check if any reads failed and exit early (to try again).
    // if (isnan(h) || isnan(t))
    // {
    //     Serial.println(F("Failed to read from DHT sensor!"));
    //     delay(1950);
    //     return;
    // }

    Serial.print(F(" Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("C "));

    Serial.print(F("Dust density: "));
    Serial.println(d);

    delay(1000);
}