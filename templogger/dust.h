#include <GP2YDustSensor.h>

// Sharp dust sensor
// https://github.com/luciansabo/GP2YDustSensor
// Adafruit Unified Sensor library is required
const uint8_t SHARP_LED_PIN = 14; // Sharp Dust/particle sensor Led Pin
const uint8_t SHARP_VO_PIN = A0;   // Sharp Dust/particle analog out pin used for reading
GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1014AU0F, SHARP_LED_PIN, SHARP_VO_PIN);

float read_dust_density()
{
    float vo_measured = analogRead(SHARP_VO_PIN);

    // 0 - 5V mapped to 0 - 1023 integer values
    // recover voltage
    float calc_voltage = vo_measured * (5.0 / 1024.0);

    Serial.print("calc_voltage: "); Serial.print(calc_voltage);

    // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
    // Chris Nafis (c) 2012
    float dust_density = 170 * calc_voltage - 0.1;

    return dust_density;
}

void loop_()
{
    Serial.print(F("Dust density: "));
    Serial.println(dustSensor.getDustDensity());
    // Serial.println(read_dust_density());
}