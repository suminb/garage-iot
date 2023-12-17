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
#include <WiFi.h>
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

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon


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


void init_wifi()
{
    const char wifi_ssid[] = "FBI Undercover Van";
    const char wifi_password[] = "";
    const uint8_t wifi_max_retries = 5;
    WiFi.begin(wifi_ssid, wifi_password);

    uint8_t wifi_retry = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_retry++ < wifi_max_retries)
    {
        delay(250);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("Connected to %s\n", wifi_ssid);
    }
    else
    {
        Serial.printf("Failed to connect to %s\n", wifi_ssid);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
    }
}

const char apn[] = "lte.ktfwing.com";
const char gprsUser[] = "";
const char gprsPass[] = "";

void init_sim()
{
    String res;

    Serial.println("========SIMCOMATI======");
    modem.sendAT("+SIMCOMATI");
    modem.waitResponse(1000L, res);
    res.replace(GSM_NL "OK" GSM_NL, "");
    Serial.println(res);
    res = "";
    Serial.println("=======================");

    Serial.println("=====Preferred mode selection=====");
    modem.sendAT("+CNMP?");
    if (modem.waitResponse(1000L, res) == 1)
    {
        res.replace(GSM_NL "OK" GSM_NL, "");
        Serial.println(res);
    }
    res = "";
    Serial.println("=======================");

    Serial.println("=====Preferred selection between CAT-M and NB-IoT=====");
    modem.sendAT("+CMNB?");
    if (modem.waitResponse(1000L, res) == 1)
    {
        res.replace(GSM_NL "OK" GSM_NL, "");
        Serial.println(res);
    }
    res = "";
    Serial.println("=======================");

    // automatic mode
    // modem.setNetworkMode(2);

    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);

    Serial.println("Waiting for network...");
    if (!modem.waitForNetwork(300000L, false))
    {
        Serial.println("Network not ready");
        return;
    }

    if (modem.isNetworkConnected())
    {
        Serial.println("Network connected");
    }

    Serial.printf("Connecting to %s\n", apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        Serial.printf("Failed to connect to %s\n", apn);
        return;
    }

    bool connected = modem.isGprsConnected();
    Serial.printf("GPRS status: %s\n", connected ? "connected" : "not connected");

    String ccid = modem.getSimCCID();
    Serial.printf("CCID: %s\n", ccid.c_str());

    String imei = modem.getIMEI();
    Serial.printf("IMEI: %s\n", imei.c_str());

    String imsi = modem.getIMSI();
    Serial.printf("IMSI: %s\n", imsi.c_str());

    String cop = modem.getOperator();
    Serial.printf("Operator: %s\n", cop.c_str());

    IPAddress local = modem.localIP();
    Serial.print("Local IP:");
    Serial.println(local);

    int csq = modem.getSignalQuality();
    Serial.printf("Signal quality: %d\n", csq);
}

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

    uint8_t resp;

    //  When configuring GNSS, you need to stop GPS first
    modem.disableGPS();
    delay(500);

    // Serial.println("Starting GPS hot");
    // modem.sendAT("+CGNSHOT");
    // uint8_t resp = modem.waitResponse();
    // Serial.printf("Response from modem = %d\n", resp);

#if 1
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
    init_wifi();
    // Initiate WiFi scan asynchronoulsy
    WiFi.scanNetworks(true);
    delay(500);

    // init_sim();

    // GPS function needs to be enabled for the first use
    Serial.println("Enabling GPS...");
    if (!modem.enableGPS()) {
        Serial.println("Failed to enable GPS");

        while (true)
        {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
            delay(300);
            PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
            delay(200);
            PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
            delay(100);
            PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
            delay(1000);
        }
    }

    // Serial.println("Requesting current GSM location");
    // if (modem.getGsmLocation(&lat2, &lon2, &accuracy2, &year2, &month2, &day2, &hour2,
    //                          &min2, &sec2))
    // {
    //     Serial.printf("Latitude: %f\tLongitude: %f\n", lat2, lon2);
    //     Serial.printf("Accuracy: %f\n", accuracy2);
    //     Serial.printf("Year: %d\tMonth: %d\tDay: %d\n", year2, month2, day2);
    //     Serial.printf("Hour: %d\tMinute: %d\tSecond: %d\n", hour2, min2, sec2);
    // }
    // else
    // {
    //     Serial.println("Couldn't get GSM location.");
    // }
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
void record_wifi_scan_result(const char* filename, const String ssid, const String mac_addr, wifi_auth_mode_t auth_mode, int32_t channel, int32_t rssi)
{
    char buf[1024];

    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d, \"%s\", %s, %s, %d, %d, %f, %f, %f\n",
        year2, month2, day2, hour2, min2, sec2,
        ssid.c_str(), mac_addr.c_str(), wifi_encryption_type_as_str(auth_mode).c_str(), channel, rssi,
        lat2, lon2, alt2);
    Serial.print(buf);
    appendFile(SD_MMC, filename, buf);
}

void track_location()
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
        // PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_4HZ);
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d, %f, %f, %f, %f, %f, %d, %d\n",
            year2, month2, day2, hour2, min2, sec2, lat2, lon2, alt2, accuracy2, speed2, vsat2, usat2);
        appendFile(SD_MMC, "/locations.csv", buf);

        PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
        delay(150);
        PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
    }
    else
    {
        // Blinking PMU charging indicator
        // PMU.setChargingLedMode(level ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
        // level ^= 1;
        for (int i = 0; i < 2; i++)
        {
            PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
            delay(100);
            PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
            delay(100);
        }
        PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
        delay(1000);
    }
}

void scan_wifi()
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

            record_wifi_scan_result("/wifiscan.csv", ssid, mac_addr, auth_mode, channel, rssi);
        }
        WiFi.scanNetworks(true);
    }
}

void loop()
{
    track_location();
    scan_wifi();
    Serial.printf("Battery = %d (%d)\n", PMU.getBatteryPercent(), PMU.getBattVoltage());
}
