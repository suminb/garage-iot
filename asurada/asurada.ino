#include <stdarg.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "storage.h"
#include "display.h"

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

sd_t sd;
file_t file;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
    Serial.begin(9600);

    // Wait for serial for a finite amount of time
    for (int i = 0; !Serial && i < 10; i++)
    {
        delay(10);
    }
    Serial.println("Booting up...");

    lcd.init();
    lcd.backlight();

    Serial.println("Initializing temperature sensor...");
    if (!mlx.begin())
    {
        Serial.println("Failed to start temperature sensor");
    }
    initSD(Serial, lcd, sd, file);

    lcd.clear();
    if (!file)
    {
        lcd.setCursor(15, 0);
        lcd.print("S");
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
