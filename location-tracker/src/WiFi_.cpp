#include "WiFi_.h"
#include "utilities.h"
#include "SDConfig.h"

void init_wifi(SDConfig& config)
{
    if (config.wifi_ssid == NULL || config.wifi_password == NULL)
    {
        Serial.println("WiFi SSID or password not set");
        return;
    }

    WiFi.begin(config.wifi_ssid, config.wifi_password);

    uint8_t wifi_retry = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_retry++ < config.wifi_max_retries)
    {
        delay(250);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("Connected to %s\n", config.wifi_ssid);
    }
    else
    {
        Serial.printf("Failed to connect to %s\n", config.wifi_ssid);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
    }
}

void scan_wifi(Location& location)
{
    int16_t n = WiFi.scanComplete();
    Serial.printf("WiFi scan result, n = %d\n", n);
    if (n == WIFI_SCAN_FAILED)
    {
        WiFi.scanNetworks(true);
    }
    else if (n > 0)
    {
        for (uint16_t i = 0; i < n; i++)
        {
            const String ssid = WiFi.SSID(i);
            const String mac_addr = WiFi.BSSIDstr(i);
            const wifi_auth_mode_t auth_mode = WiFi.encryptionType(i);
            const int32_t channel = WiFi.channel(i);
            const int32_t rssi = WiFi.RSSI(i);

            record_wifi_scan_result("/wifiscan2.csv", ssid, mac_addr, auth_mode, channel, rssi, location);
            // Serial.printf("Scanned: %s\n", ssid.c_str());
        }
        WiFi.scanNetworks(true);
    }
}

String wifi_encryption_type_as_str(wifi_auth_mode_t mode)
{
    String mode_str;
    switch (mode)
    {
        case WIFI_AUTH_OPEN:
            mode_str = "OPEN";
            break;
        case WIFI_AUTH_WEP:
            mode_str = "WEP";
            break;
        case WIFI_AUTH_WPA_PSK:
            mode_str = "WPA_PSK";
            break;
        case WIFI_AUTH_WPA2_PSK:
            mode_str = "WPA2_PSK";
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            mode_str = "WPA_WPA2_PSK";
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            mode_str = "WPA2_ENTERPRISE";
            break;
        case WIFI_AUTH_WPA3_PSK:
            mode_str = "WPA3_PSK";
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            mode_str = "WPA2_WPA3_PSK";
            break;
        case WIFI_AUTH_WAPI_PSK:
            mode_str = "WAPI_PSK";
            break;
        case WIFI_AUTH_MAX:
            mode_str = "MAX";
            break;
        default:
            mode_str = "UNKNOWN";
    }
    return mode_str;
}

/**
 * @brief Append a single WiFi scan record to a file
 * 
 * @param filename 
 * @param ssid 
 * @param mac_addr 
 * @param auth_mode 
 * @param channel 
 * @param rssi 
 */
void record_wifi_scan_result(const char* filename, const String ssid, const String mac_addr, wifi_auth_mode_t auth_mode, int32_t channel, int32_t rssi, Location& location)
{
    char buf[512];

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d, \"%s\", %s, %s, %d, %d, %f, %f, %f, %f\n",
        location.year, location.month, location.day, location.hour, location.min, location.sec,
        ssid.c_str(), mac_addr.c_str(), wifi_encryption_type_as_str(auth_mode).c_str(), channel, rssi,
        location.lat, location.lon, location.alt, location.accuracy);
    Serial.print(buf);
    appendFile(SD_MMC, filename, buf);
}