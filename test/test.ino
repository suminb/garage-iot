#include <SoftwareSerial.h>

SoftwareSerial wifi(2, 3); // RX, TX

void send_command(char *command) {
    wifi.write(command);
    wifi.write("\r\n");

    while (!wifi.available())
        delay(100);
    Serial.println("Response from WiFi module:");
    Serial.println(wifi.readString());
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Getting started!");
    
//    wifi.begin(74880);
//    delay(1000);
//    send_command("AT+UART_DEF=57600,8,1,0,0");
//    delay(1000);

    wifi.begin(57600);
    delay(1000);
    
//    send_command("AT");
//    send_command("AT+CWMODE=1");
//    send_command("AT+CWJAP=<ssid>,<pwd>");

    Serial.println("IP, MAC:");
    send_command("AT+CIFSR");
    
    Serial.println("Making an HTTP request...");
    send_command("AT+CIPSTART=\"TCP\",\"192.168.1.1\",80");
    send_command("AT+CIPSEND=16");
    send_command("GET / HTTP/1.1\r\n");

    Serial.println("Waiting for resp...");
    while (!wifi.available())
        delay(100);
    Serial.println("Response from server:");
    Serial.println(wifi.readString());

    send_command("AT+CIPCLOSE");
}

void loop()
{
    if (wifi.available())
    {
        Serial.write(wifi.read());
    }
    if (Serial.available())
    {
        wifi.write(Serial.read());
    }
}
