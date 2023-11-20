#include <Wire.h>
#include "display.h"

void log(Stream &sout, LiquidCrystal_I2C &lcd, const char *short_message, const char *long_message)
{
    sout.println(long_message);

    lcd.setCursor(0, 0);
    lcd.print(short_message);
}

void format_string(char *dest, const char *format) {

}