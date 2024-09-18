#include <Arduino.h>

#include "utilities.h"

#include "power.h"
XPowersPMU pmu;

#include "SDConfig.h"
SDConfig config;

#include "WiFi_.h"

#include "Location.h"
Location location;


// Task functions
void manage_location(void *params) {
    init_modem();
    enable_gps(&pmu);
    while(1) {
        Serial.println("Acquiring location...");
        track_location(&pmu, &location);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 10ms
    }
}

void manage_wifi(void *params) {
    // init_wifi(config);
    while (1) {
        // Check if location has changed or enough time has elapsed
        scan_wifi(location);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000ms
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);  // Wait for Serial to be ready
    }
    Serial.println("Initializing system...");

    // gModel.mutex = xSemaphoreCreateMutex();

    init_pmu(&pmu);
    init_sd();

    if (!config.read())
    {
        Serial.println("Failed to read config");
        while (true) {
            delay(1000);
        }
    }
    Serial.printf("WiFi SSID: %s\n", config.wifi_ssid);

    // appendFile(SD_MMC, "/test.txt", "This is a test");

    // Create tasks with increased stack size
    xTaskCreate(manage_location, "manage_location", 4096, NULL, 1, NULL);
    xTaskCreate(manage_wifi, "manage_wifi", 1024 * 10, NULL, 1, NULL);

}

void loop() {
}