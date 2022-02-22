#include <LiquidCrystal_I2C.h>

/**
 * @brief 
 * 
 * @param sout Serial output stream
 * @param lcd 
 * @param short_message 
 * @param long_message 
 */
void log(Stream &sout, LiquidCrystal_I2C &lcd, const char *short_message, const char *long_message);

void format_string(char *dest, const char *format);