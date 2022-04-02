#include <Wire.h>
#include <DHT.h>
#include "wifi.h"
#include "server.h"
#include "dust.h"
#include "display.h"

#define LED 2

#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
float d = 0.0;

char text[64];

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }
    Serial.println("Getting started...");

    init_display();
    display.clearDisplay();
    display_text("Booting up...");

    dht.begin();
    dustSensor.begin();
    pinMode(LED, OUTPUT);

    display_text("Initializing WiFi...");
    init_wifi();

    display_text("Initializing web server...");
    init_web_server();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
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

    Serial.print(F(" Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("C "));

    Serial.print(F("Dust density: "));
    Serial.println(d);

    display.clearDisplay();
    display.setCursor(1, 1);

    sprintf(text, "%.1fC %.0f%%", t, h);
    display.println(text);
    sprintf(text, "%.1f", d);
    display.println(text);
    display.display();

    delay(1000);
}