#include <Arduino.h>

#include "utilities.h"
#include "power.h"

#include "SDConfig.h"

// Model struct
struct VehicleModel {
    float speed;
    int rpm;
    SemaphoreHandle_t mutex;
};

// Global model instance
VehicleModel gModel;

// Function to update model values
void updateModel(float speed, int rpm) {
    xSemaphoreTake(gModel.mutex, portMAX_DELAY);
    gModel.speed = speed;
    gModel.rpm = rpm;
    xSemaphoreGive(gModel.mutex);
}

// Function to get speed
float getSpeed() {
    xSemaphoreTake(gModel.mutex, portMAX_DELAY);
    float speed = gModel.speed;
    xSemaphoreGive(gModel.mutex);
    return speed;
}

// Function to get RPM
int getRPM() {
    xSemaphoreTake(gModel.mutex, portMAX_DELAY);
    int rpm = gModel.rpm;
    xSemaphoreGive(gModel.mutex);
    return rpm;
}

// Task functions
void task1(void *pvParameters) {
    float speed = 0.0;
    for(;;) {
        speed += 0.5;
        if (speed > 100.0) speed = 0.0;
        updateModel(speed, (int)(speed * 100));
        Serial.printf("Task 1: Updated speed to %.1f km/h\n", speed);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

void task2(void *pvParameters) {
    for(;;) {
        float currentSpeed = getSpeed();
        int currentRPM = getRPM();
        Serial.printf("Task 2: Current speed: %.1f km/h, RPM: %d\n", currentSpeed, currentRPM);
        Serial.printf("Task 2: Free stack: %d\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 2 seconds
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);  // Wait for Serial to be ready
    }
    Serial.println("ESP32 Arduino RTOS Example");

    // Initialize model
    gModel.speed = 0.0;
    gModel.rpm = 0;
    gModel.mutex = xSemaphoreCreateMutex();

    init_pmu();
    init_sd();

    if (!read_config())
    {
        Serial.println("Failed to read config");
        while (true) {
            delay(1000);
        }
    }

    // Create tasks with increased stack size
    xTaskCreate(task1, "task1", 4096, NULL, 1, NULL);
    xTaskCreate(task2, "task2", 4096, NULL, 1, NULL);

}

void loop() {
    // Empty. Things are done in Tasks.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}