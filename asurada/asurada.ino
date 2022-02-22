#include <stdarg.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include "storage.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

sd_t sd;
file_t file;

void log(const char *shortMessage, const char *longMessage)
{
    Serial.println(longMessage);

    lcd.setCursor(0, 0);
    lcd.print(shortMessage);
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        yield();
    }
    Serial.println("Booting up...");

    lcd.init();
    lcd.backlight();

    Serial.println("Initializing temp. sensor...");
    if (!mlx.begin())
    {
        Serial.println("Failed to start temperature sensor");
    }
    initSD();

    lcd.clear();
    if (!file)
    {
        lcd.setCursor(15, 0);
        lcd.print("S");
    }
}

void initSD()
{
    Serial.println("Initializing SD...");
    if (!sd.begin(SD_CONFIG))
    {
        log("SD error (1)", "sd.begin() failed");
        // sd.initErrorHalt(&Serial);
    }
    if (!file.open("data.csv", FILE_WRITE))
    {
        Serial.println("Could not open data.csv");
    }
}

void loop()
{
    char temp[8];
    char line1[17];
    char line2[17];

    double ambTemp = mlx.readAmbientTempC();
    double objTemp = mlx.readObjectTempC();

    if (isnan(ambTemp) || isnan(objTemp))
    {
        Serial.println("Temperature is read as NaN");
        return;
    }

    dtostrf(ambTemp, 5, 1, temp);
    sprintf(line1, "AMB: %s", temp);
    dtostrf(objTemp, 5, 1, temp);
    sprintf(line2, "OBJ: %s", temp);
    //   sprintf(line1, "AMB: %.01f", ambTemp);
    //   sprintf(line2, "OBJ: %.01f", objTemp);

    Serial.print(line1);
    Serial.print(", ");
    Serial.println(line2);

    if (file)
    {
        writeRecord(&file, ambTemp, objTemp);
    }

    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);

    delay(100);
}

void writeRecord(file_t *fout, double ambTemp, double objTemp)
{
    fout->print(millis());
    fout->print(",");
    fout->print(ambTemp);
    fout->print(",");
    fout->print(objTemp);
    fout->println();
    fout->sync();
}
