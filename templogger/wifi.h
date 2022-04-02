#ifndef __WIFI_H__
#define __WIFI_H__

#include <ESP8266WiFi.h>

const char *ssid = "";
const char *password = "";

void init_wifi()
{
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
}

#endif // __WIFI_H__