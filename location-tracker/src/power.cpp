#include "power.h"

XPowersPMU PMU;

void init_pmu() {
    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
        delay(250);
        PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
        delay(250);
    }

    //Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    //Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      //The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disabled, otherwise it cannot be charged
    PMU.disableTSPinMeasure();

    PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
}