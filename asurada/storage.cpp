#include "storage.h"

void initSD(Stream &sout, LiquidCrystal_I2C &lcd, sd_t &sd, file_t &file)
{
    Serial.println("Initializing SD...");
    if (!sd.begin(SD_CONFIG))
    {
        log(sout, lcd, "SD error (1)", "sd.begin() failed");
        // sd.initErrorHalt(&Serial);
    }
    else if (!file.open("data.csv", FILE_WRITE))
    {
        Serial.println("Could not open data.csv");
    }
}
