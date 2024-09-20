#ifndef __LOCATION_H__
#define __LOCATION_H__

#include "utilities.h"
#include "power.h"

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>


// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

class Location {
  public:
    float lat      = 0;
    float lon      = 0;
    float speed    = 0;
    float alt      = 0;
    int   vsat     = 0;
    int   usat     = 0;
    float accuracy = 0;
    int   year     = 0;
    int   month    = 0;
    int   day      = 0;
    int   hour     = 0;
    int   min      = 0;
    int   sec      = 0;
    bool  level     = false;
};

void init_modem();
void enable_gps(XPowersPMU* pmu);
void track_location(XPowersPMU* pmu, Location* location);

#endif // __LOCATION_H__