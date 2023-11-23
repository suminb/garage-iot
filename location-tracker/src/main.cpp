#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16
static const unsigned char PROGMEM logo_bmp[] =
    {0b00000000, 0b11000000,
     0b00000001, 0b11000000,
     0b00000001, 0b11000000,
     0b00000011, 0b11100000,
     0b11110011, 0b11100000,
     0b11111110, 0b11111000,
     0b01111110, 0b11111111,
     0b00110011, 0b10011111,
     0b00011111, 0b11111100,
     0b00001101, 0b01110000,
     0b00011011, 0b10100000,
     0b00111111, 0b11100000,
     0b00111111, 0b11110000,
     0b01111100, 0b11110000,
     0b01110000, 0b01110000,
     0b00000000, 0b00110000};

/*
   This sample demonstrates TinyGPS++'s capacity for extracting custom
   fields from any NMEA sentence.  TinyGPS++ has built-in facilities for
   extracting latitude, longitude, altitude, etc., from the $GPGLL and
   $GPRMC sentences.  But with the TinyGPSCustom type, you can extract
   other NMEA fields, even from non-standard NMEA sentences.

   It requires the use of SoftwareSerial, and assumes that you have a
   9600-baud serial GPS device hooked up on pins 4(RX) and 3(TX).
*/
// static const int RXPin = 4; // D1
// static const int TXPin = 5; // D2
static const int RXPin = 14; // D5
static const int TXPin = 12; // D6
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

/*
   By declaring TinyGPSCustom objects like this, we announce that we
   are interested in the 15th, 16th, and 17th fields in the $GPGSA
   sentence, respectively the PDOP (F("positional dilution of precision")),
   HDOP (F("horizontal...")), and VDOP (F("vertical...")).

   (Counting starts with the field immediately following the sentence name,
   i.e. $GPGSA.  For more information on NMEA sentences, consult your
   GPS module's documentation and/or http://aprs.gids.nl/nmea/.)

   If your GPS module doesn't support the $GPGSA sentence, then you
   won't get any output from this program.
*/

TinyGPSCustom pdop(gps, "GNGLL", 1); // $GPGSA sentence, 15th element
TinyGPSCustom hdop(gps, "GNGLL", 3); // $GPGSA sentence, 16th element

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (1)
      delay(50);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  Serial.println(F("UsingCustomFields.ino"));
  Serial.println(F("Demonstrating how to extract any NMEA field using TinyGPSCustom"));
  Serial.print(F("Testing TinyGPSPlus library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

char line1[32], line2[32], line3[32], line4[32], line5[32], line6[32];
char buf[128];
int buf_read = 0;
int checkpoint = 0;

float latitude, longitude;
int year, month, date, hour, minute, second;
String date_str, time_str, lat_str, lng_str;
int pm;

void loop()
{
  unsigned long current = millis();

  display.clearDisplay();

  while (ss.available() > 0)
  {
    buf_read = ss.read(buf, ss.available());

    if (current >= checkpoint)
    {
      Serial.printf("Read %d bytes", buf_read);
      Serial.println(buf);
    }

    sprintf(line2, "Sattlites: %d", gps.satellites.value());

    for (int i = 0; i < buf_read; i++)
    {
      gps.encode(buf[i]);
    }
    if (gps.location.isValid())
    {
      latitude = gps.location.lat();
      // lat_str = String(latitude, 6); // latitude location is stored in a string
      longitude = gps.location.lng();
      // lng_str = String(longitude, 6); // longitude location is stored in a string

      sprintf(line3, "%.3f, %.3f", latitude, longitude);
      // Serial.println(text);
    }
    else
    {
      char *msg = "Unknown position";
      strncpy(line3, msg, strlen(msg));
    }

    if (gps.date.isValid())
    {
      sprintf(line4, "%04d-%02d-%02d %02d:%02d:%02d",
        gps.date.year(), gps.date.month(), gps.date.day(),
        gps.time.hour(), gps.time.minute(), gps.time.second());
    }
    else
    {
      char *msg = "Unknown datetime";
      strncpy(line4, msg, strlen(msg));
    }
  }

  if (current >= checkpoint)
  {
    sprintf(line5, "%d", current);

    display.setCursor(0, 0);
    display.print(line1);
    display.setCursor(0, 20);
    display.print(line2);
    display.setCursor(0, 30);
    display.print(line3);
    display.setCursor(0, 40);
    display.print(line4);
    display.setCursor(0, 50);
    display.print(line5);
    display.display();

    checkpoint += 100;
  }
  delay(99);
}
