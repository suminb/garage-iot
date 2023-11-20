#include <SPI.h>
#include <SD.h>
#include <SDConfig.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ELMduino.h>

#include "LGFX.h"

// SPI
#define SPI_MOSI 2 // 1
#define SPI_MISO 41
#define SPI_SCK 42
#define SD_CS 1 // 2

#define LCD_CS 37
#define LCD_BLK 45

// ELM327 default WiFi router: 192.168.0.10
// Default SSID: V-LINK?

// https://python-obd.readthedocs.io/en/latest/Command%20Tables/

const char *config_file = "/config.cfg";

LGFX tft;
SDConfig cfg;
File root;
WiFiClient client;
ELM327 elm327;
bool opened = false;

bool config_loaded = false;
char *wifi_ssid = "V-LINK";
char *wifi_password = "";
char *elm327_host = "192.168.0.10";
ushort elm327_port = 35000;

char message[128];

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    Serial.println("Dir created");
  }
  else
  {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path)
{
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path))
  {
    Serial.println("Dir removed");
  }
  else
  {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("File renamed");
  }
  else
  {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println("File deleted");
  }
  else
  {
    Serial.println("Delete failed");
  }
}

void setup()
{

  pinMode(SD_CS, OUTPUT);
  pinMode(SPI_SCK, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);

  digitalWrite(SD_CS, LOW);

  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_BLK, OUTPUT);

  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_BLK, HIGH);

  Serial.begin(115200);
  delay(100);
  Serial.println("Starting...");

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);

  tft.println("Initializing system...");

  // SPI init
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  if (!SD.begin(SD_CS))
  {
    tft.println("Filesystem init failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    tft.println("No SD card attached");
    return;
  }

  tft.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    tft.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    tft.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    tft.println("SDHC");
  }
  else
  {
    tft.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  tft.printf("SD card size: %lluMB\n", cardSize);

  if (!SD.exists(config_file))
  {
    tft.println("Config file not found, fallback to default");
  }
  else
  {
    const int max_line_length = 127;
    if (cfg.begin(config_file, max_line_length))
    {
      tft.println("Loading config...");
      while (cfg.readNextSetting())
      {
        if (cfg.nameIs("wifi_ssid"))
        {
          // NOTE: Not sure how this memory allocation gets released later
          wifi_ssid = cfg.copyValue();
        }
        else if (cfg.nameIs("wifi_password"))
        {
          wifi_password = cfg.copyValue();
        }
        else if (cfg.nameIs("elm327_host"))
        {
          elm327_host = cfg.copyValue();
        }
        else if (cfg.nameIs("elm327_port"))
        {
          elm327_port = (ushort)cfg.getIntValue();
        }
        else
        {
          Serial.print("Unsupported config key: ");
          Serial.println(cfg.getName());
        }
      }
      tft.printf("SSID: %s\n", wifi_ssid);
      tft.printf("ELM: %s:%d\n", elm327_host, elm327_port);
      cfg.end();
      // config_loaded = true;
    }
  }

  Serial.print("SSID: ");
  Serial.println(wifi_ssid);
  Serial.print("Password: ");
  Serial.println(wifi_password);
  delay(100);

  WiFi.begin(wifi_ssid, wifi_password);
  Serial.println("Trying to connect to WiFi...");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    tft.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifi_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  tft.print("\r");
  tft.println(WiFi.localIP());

  if (!client.connect(elm327_host, elm327_port))
  {
    Serial.println("Connection to host failed");
    delay(1000);
    return;
  }
  elm327.begin(client, true, 2000, ISO_15765_11_BIT_500_KBAUD);

  delay(1000);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
}

// It seems like BMW M2 uses ISO 15765-4 (CAN 11/500) protocol

class Model
{
private:
  int rpm;
  int kph;
  float oil_temp;
  float throttle;

public:
  int get_rpm()
  {
    return rpm;
  }

  void set_rpm(int rpm)
  {
    this->rpm = rpm;
  }

  int get_kph()
  {
    return kph;
  }

  void set_kph(int kph)
  {
    this->kph = kph;
  }

  float get_oil_temp()
  {
    return oil_temp;
  }

  void set_oil_temp(float temp)
  {
    this->oil_temp = temp;
  }

  float get_throttle()
  {
    return throttle;
  }

  void set_throttle(float throttle)
  {
    this->throttle = throttle;
  }
};

typedef enum
{
  ENG_RPM,
  SPEED,
  OIL_TEMP,
  THROTTLE
} obd_pid_states;
obd_pid_states obd_state = ENG_RPM;
Model model;
uint32_t last_time, now = 0;            // RTC
const uint32_t refresh_threshold = 100; // in milliseconds

void refresh_view()
{
  char buf[128];

  tft.fillRect(10, 10, 240, 210, TFT_BLACK);

  sprintf(buf, "%d km/h", model.get_kph());
  tft.setCursor(10, 10);
  tft.print(buf);

  sprintf(buf, "%d rpm", model.get_rpm());
  tft.setCursor(10, 60);
  tft.print(buf);

  sprintf(buf, "%.1f C", model.get_oil_temp());
  tft.setCursor(10, 110);
  tft.print(buf);

  sprintf(buf, "%.0f%%", model.get_throttle());
  tft.setCursor(10, 160);
  tft.print(buf);
}

void update_model()
{
  switch (obd_state)
  {
  case ENG_RPM:
  {
    float rpm = elm327.rpm();

    if (elm327.nb_rx_state == ELM_SUCCESS)
    {
      model.set_rpm((int)rpm);
    }
    else if (elm327.nb_rx_state != ELM_GETTING_MSG)
    {
      elm327.printError();
    }
    obd_state = SPEED;
    break;
  }
  case SPEED:
  {
    int kph = elm327.kph();

    if (elm327.nb_rx_state == ELM_SUCCESS)
    {
      model.set_kph(kph);
    }
    else if (elm327.nb_rx_state != ELM_GETTING_MSG)
    {
      elm327.printError();
    }
    obd_state = OIL_TEMP;
    break;
  }
  case OIL_TEMP:
  {
    float oil_temp = elm327.oilTemp();

    if (elm327.nb_rx_state == ELM_SUCCESS)
    {
      model.set_oil_temp(oil_temp);
    }
    else if (elm327.nb_rx_state != ELM_GETTING_MSG)
    {
      elm327.printError();
    }
    obd_state = THROTTLE;
  }
  case THROTTLE:
  {
    float throttle = elm327.throttle();

    if (elm327.nb_rx_state == ELM_SUCCESS)
    {
      model.set_throttle(throttle);
    }
    else if (elm327.nb_rx_state != ELM_GETTING_MSG)
    {
      elm327.printError();
    }
    obd_state = ENG_RPM;
  }
  default:
    Serial.println("Unknown OBD state");
  }
}

void loop()
{
  update_model();

  now = millis();
  if (now - last_time >= refresh_threshold)
  {
    refresh_view();
    last_time = now;
  }
}