#include "power.h"


void init_pmu(XPowersPMU* pmu) {
    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    if (!pmu->begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        pmu->setChargingLedMode(XPOWERS_CHG_LED_ON);
        delay(250);
        pmu->setChargingLedMode(XPOWERS_CHG_LED_OFF);
        delay(250);
    }

    //Set the working voltage of the modem, please do not modify the parameters
    pmu->setDC3Voltage(3000);    //SIM7080 Modem main power channel 2700~ 3400V
    pmu->enableDC3();

    //Modem GPS Power channel
    pmu->setBLDO2Voltage(3300);
    pmu->enableBLDO2();      //The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disabled, otherwise it cannot be charged
    pmu->disableTSPinMeasure();

    pmu->setChargingLedMode(XPOWERS_CHG_LED_ON);
}