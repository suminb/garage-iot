#include "Location.h"
#include "Storage.h"

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

void init_modem() {
    /*********************************
     * step 2 : start modem
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);

    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (retry++ > 15) {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
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

    uint8_t resp;

    //  When configuring GNSS, you need to stop GPS first
    modem.disableGPS();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Serial.println("Starting GPS hot");
    // modem.sendAT("+CGNSHOT");
    // uint8_t resp = modem.waitResponse();
    // Serial.printf("Response from modem = %d\n", resp);

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
    Serial.println("Requesting GNSS...");
    modem.sendAT("+CGNSMOD=1,1,0,0,0");
    resp = modem.waitResponse();
    Serial.printf("Response from modem = %d\n", resp);

    /*
    GNSS Command,For more parameters, see <SIM7070_SIM7080_SIM7090 Series_AT Command Manual> 212 page.
    <minInterval> range: 1000-60000 ms
     minInterval is the minimum time interval in milliseconds that must elapse between position reports. default value is 1000.
    <minDistance> range: 0-1000
     Minimum distance in meters that must be traversed between position reports. Setting this interval to 0 will be a pure time-based tracking/batching.
    <accuracy>:
        0 Accuracy is not specified, use default.
        1 Low Accuracy for location is acceptable.
        2 Medium Accuracy for location is acceptable.
        3 Only High Accuracy for location is acceptable.
    */
    // minInterval = 1000,minDistance = 0,accuracy = 0
    modem.sendAT("+SGNSCMD=2,1000,0,1");
    resp = modem.waitResponse();
    Serial.printf("Response from modem = %d\n", resp);

    // Turn off GNSS.
    modem.sendAT("+SGNSCMD=0");
    resp = modem.waitResponse();
    Serial.printf("Response from modem = %d\n", resp);
#endif
}

void enable_gps(XPowersPMU* pmu) {
    // GPS function needs to be enabled for the first use
    Serial.println("Enabling GPS...");
    if (!modem.enableGPS()) {
        Serial.println("Failed to enable GPS");

        while (true)
        {
            pmu->setChargingLedMode(XPOWERS_CHG_LED_ON);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            pmu->setChargingLedMode(XPOWERS_CHG_LED_OFF);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            pmu->setChargingLedMode(XPOWERS_CHG_LED_ON);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            pmu->setChargingLedMode(XPOWERS_CHG_LED_OFF);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}


void track_location(XPowersPMU* pmu, Location* location)
{
    char buf[1024];

    if (modem.getGPS(
            &location->lat, &location->lon, &location->speed,
            &location->alt, &location->vsat, &location->usat,
            &location->accuracy, &location->year, &location->month,
            &location->day, &location->hour, &location->min, &location->sec))
    {
        Serial.println();
        Serial.print("lat:"); Serial.print(String(location->lat, 8)); Serial.print("\t");
        Serial.print("lon:"); Serial.print(String(location->lon, 8)); Serial.println();
        Serial.print("speed:"); Serial.print(location->speed); Serial.print("\t");
        Serial.print("altitude:"); Serial.print(location->alt); Serial.println();
        Serial.print("year:"); Serial.print(location->year);
        Serial.print(" month:"); Serial.print(location->month);
        Serial.print(" day:"); Serial.print(location->day);
        Serial.print(" hour:"); Serial.print(location->hour);
        Serial.print(" minutes:"); Serial.print(location->min);
        Serial.print(" second:"); Serial.print(location->sec);
        Serial.println();
        Serial.println();

        // After successful positioning, the PMU charging indicator flashes quickly
        // PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_4HZ);
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d, %f, %f, %f, %f, %f, %d, %d\n",
            location->year, location->month, location->day, location->hour, location->min, location->sec, location->lat, location->lon, location->alt, location->accuracy, location->speed, location->vsat, location->usat);
        appendFile(SD_MMC, "/locations.csv", buf);

        pmu->setChargingLedMode(XPOWERS_CHG_LED_ON);
        vTaskDelay(150 / portTICK_PERIOD_MS);
        pmu->setChargingLedMode(XPOWERS_CHG_LED_OFF);
    }
    else
    {
        // Blinking PMU charging indicator
        // PMU.setChargingLedMode(level ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
        // level ^= 1;
        for (int i = 0; i < 2; i++)
        {
            pmu->setChargingLedMode(XPOWERS_CHG_LED_ON);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            pmu->setChargingLedMode(XPOWERS_CHG_LED_OFF);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        pmu->setChargingLedMode(XPOWERS_CHG_LED_OFF);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}