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
        request->send_P(200, "application/json", build_response_body().c_str());
    });

    // Start server
    server.begin();
}

String encode_float(float value) {
    if (isnan(value)) {
        return String("null");
    }
    else {
        return String(value);
    }
}

String build_response_body() {
    char json_content[128];
    sprintf(json_content,
        "{\"ip\":\"%s\", \"temperature\":%s, \"humidity\":%s, \"dust_density\":%s}",
        WiFi.localIP().toString().c_str(),
        encode_float(t).c_str(),
        encode_float(h).c_str(),
        encode_float(d).c_str());
    return String(json_content);
}