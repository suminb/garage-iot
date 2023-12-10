/**
 * @file      MinimalModemGPSExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "Storage.h"
#include "utilities.h"

XPowersPMU  PMU;

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>
#include "utilities.h"


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

float lat2      = 0;
float lon2      = 0;
float speed2    = 0;
float alt2      = 0;
int   vsat2     = 0;
int   usat2     = 0;
float accuracy2 = 0;
int   year2     = 0;
int   month2    = 0;
int   day2      = 0;
int   hour2     = 0;
int   min2      = 0;
int   sec2      = 0;
bool  level     = false;

void setup()
{
    Serial.begin(115200);

    //Start while waiting for Serial monitoring
    // while (!Serial)
    //     delay(50);

    delay(1000);

    Serial.println();

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

    /*********************************
     * step 2 : start sd card
    ***********************************/

    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);   //set sdcard pin use 1-bit mode

    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("Card Mount Failed");
        while (1) {
            delay(1000);
        }

    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
        while (1) {
            delay(1000);
        }
    }

    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

    /*********************************
     * step 2 : start modem
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);

    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (retry++ > 15) {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            retry = 0;
            Serial.println("Retry start modem .");
        }
    }
    Serial.println();
    Serial.print("Modem started!");

    /*********************************
     * step 3 : start modem gps function
    ***********************************/

    //  When configuring GNSS, you need to stop GPS first
    modem.disableGPS();
    delay(500);

#if 0
    /*
    ! GNSS Work Mode Set
     <gps mode> GPS work mode.
        1 Start GPS NMEA out.
    <glo mode> GLONASS work mode.
        0 Stop GLONASS NMEA out.
        1 Start GLONASS NMEA out.
    <bd mode> BEIDOU work mode.
        0 Stop BEIDOU NMEA out.
        1 Start BEIDOU NMEA out.
    <gal mode> GALILEAN work mode.
        0 Stop GALILEAN NMEA out.
        1 Start GALILEAN NMEA out.
    <qzss mode> QZSS work mode.
        0 Stop QZSS NMEA out.
        1 Start QZSS NMEA out.*/

    //GNSS Work Mode Set GPS+BEIDOU
    modem.sendAT("+CGNSMOD=1,1,0,0,0");
    modem.waitResponse();


    /*
    GNSS Command,For more parameters, see <SIM7070_SIM7080_SIM7090 Series_AT Command Manual> 212 page.
    <minInterval> range: 1000-60000 ms
     minInterval is the minimum time interval in milliseconds that must elapse between position reports. default value is 1000.
    <minDistance> range: 0-1000
     Minimum distance in meters that must be traversed between position reports. Setting this interval to 0 will be a pure time-based tracking/batching.
    <accuracy>:
        0  Accuracy is not specified, use default.
        1  Low Accuracy for location is acceptable.
        2 Medium Accuracy for location is acceptable.
        3 Only High Accuracy for location is acceptable.
    */
    // minInterval = 1000,minDistance = 0,accuracy = 0
    modem.sendAT("+SGNSCMD=2,1000,0,0");
    modem.waitResponse();

    // Turn off GNSS.
    modem.sendAT("+SGNSCMD=0");
    modem.waitResponse();
#endif
    delay(500);

    // GPS function needs to be enabled for the first use
    if (!modem.enableGPS()) {
        Serial.print("Modem enable gps function failed!!");

        while (1)
        {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
            delay(250);
            PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
            delay(1000);
        }
    }
}

void loop()
{
    char buf[1024];

    if (modem.getGPS(&lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                     &year2, &month2, &day2, &hour2, &min2, &sec2))
    {
        Serial.println();
        Serial.print("lat:"); Serial.print(String(lat2, 8)); Serial.print("\t");
        Serial.print("lon:"); Serial.print(String(lon2, 8)); Serial.println();
        Serial.print("speed:"); Serial.print(speed2); Serial.print("\t");
        Serial.print("altitude:"); Serial.print(alt2); Serial.println();
        Serial.print("year:"); Serial.print(year2);
        Serial.print(" month:"); Serial.print(month2);
        Serial.print(" day:"); Serial.print(day2);
        Serial.print(" hour:"); Serial.print(hour2);
        Serial.print(" minutes:"); Serial.print(min2);
        Serial.print(" second:"); Serial.print(sec2); Serial.println();
        Serial.println();

        // After successful positioning, the PMU charging indicator flashes quickly
        PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_4HZ);
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d, %f, %f, %f, %f, %f, %d, %d\n",
            year2, month2, day2, hour2, min2, sec2, lat2, lon2, alt2, accuracy2, speed2, vsat2, usat2);
        appendFile(SD_MMC, "/records.csv", buf);

        delay(100);
    }
    else
    {
        // Blinking PMU charging indicator
        PMU.setChargingLedMode(level ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
        level ^= 1;
        delay(1000);
    }
}
