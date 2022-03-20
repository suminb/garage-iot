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


void init_web_server()
{
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