#include "server.h"

extern float t;
extern float h;
extern float d;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

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

char json_content[128];

void init_web_server()
{
    // Print ESP8266 Local IP Address
    Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        sprintf(json_content, "{\"ip\":\"%s\", \"temperature\":%f, \"humidity\":%f, \"dust_density\":%f}", WiFi.localIP().toString().c_str(), t, h, d);
        request->send_P(200, "application/json", json_content);
    });

    // Start server
    server.begin();
}