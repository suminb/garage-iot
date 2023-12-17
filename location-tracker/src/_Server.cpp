#include "_Server.h"
#include <SD_MMC.h>
#include <ESPAsyncWebServer.h>

const char PARAM_MESSAGE[] = "file";
AsyncWebServer web_server(80);

void handle_index()
{
    // web_server.send(200, "text/html", String("<h1>Hello from ESP32!</h1>"));
}

void handle_web_client()
{
    // web_server.handleClient();
}

void init_web_server()
{
    web_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String message;
        String ftype;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
            message.replace('~', '/');
        }
        if (request->hasParam("ftype")) {
            ftype = request->getParam("ftype")->value();
            ftype.replace('~', '/');
        }
        request->send(SD_MMC, "/"+message, ftype);
        Serial.printf("User requested file '%s' using file type '%s'\n", message, ftype);
    });

    web_server.serveStatic("/", SD_MMC, "/");

    web_server.begin();
    Serial.printf("Web server started at %s\n", WiFi.localIP().toString().c_str());
}