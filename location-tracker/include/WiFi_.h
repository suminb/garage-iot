#include <WiFi.h>
#include "SDConfig.h"
#include "Location.h"

void init_wifi(SDConfig& config);
void scan_wifi(Location& location);
String wifi_encryption_type_as_str(wifi_auth_mode_t mode);
void record_wifi_scan_result(const char* filename, const String ssid, const String mac_addr, wifi_auth_mode_t auth_mode, int32_t channel, int32_t rssi, Location& location);