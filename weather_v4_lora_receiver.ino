//LoRa baseed weather station - receiver node
//Hardware design by Debasish Dutta - opengreenenergy@gmail.com
//Software design by James Hughes - jhughes1010@gmail.com

/* History
   0.9.0 10-02-22 Initial development for Heltec ESP32 LoRa v2 devkit

   1.0.2 11-17-22 Remapping all mqtt topics

   1.1.0 11-24-22 sensor structure expanded to receive max wind speed also
                  LED display online for Heltec board

   1.1.1 12-13-22 Error in code where non-Heltec build would not compile properly
                  Missing #ifdef statements were added to correct this 

   1.1.2 01-27-23 Packet type details added to display.
                  Explicitly checking both packet sizes before memcpy
                  Added WDT of 10sec in case of hang up in loop()                 
*/

//Hardware build target: ESP32
#define VERSION "1.1.2"


//#include "heltec.h"
#include <LoRa.h>
#include <spi.h>

#include "config.h"
#include <esp_wifi.h>
#include <esp_task_wdt.h>
//#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <PubSubClient.h>
#ifdef DEV_HELTEC_RECEIVER
#include <Wire.h>
#include <U8g2lib.h>
#endif

String rssi = "RSSI --";
String packSize = "--";
String packet;
byte packetBinary[128];

float rssi_wifi;
float rssi_lora;

#ifdef DEV_HELTEC_RECEIVER
U8G2_SSD1306_128X64_NONAME_F_SW_I2C led(U8G2_R0, /* clock=*/15, /* data=*/4, /* reset=*/16);
#endif

//===========================================
// Weather-environment structure
//===========================================
struct sensorData {
  int windDirectionADC;
  int rainTicks24h;
  int rainTicks60m;
  float temperatureC;
  float windSpeed;
  float windSpeedMax;
  float barometricPressure;
  float humidity;
  float UVIndex;
  float lux;
};

struct diagnostics {
  float BMEtemperature;
  int batteryADC;
  int solarADC;
  int coreC;
  int bootCount;
  bool chargeStatusB;
};

struct derived {
  char cardinalDirection[5];
  float degrees;
};

struct sensorData environment;
struct diagnostics hardware;
struct derived wind;

//===========================================
// LoRaData: acknowledge LoRa packet received on OLED
//===========================================
void LoRaData() {
  static int count = 1;
  char buffer[20];
  sprintf(buffer, "Count: %i", count);
  //  Heltec.display->clear();
  //Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  //Heltec.display->setFont(ArialMT_Plain_10);
  MonPrintf("%s\n", "Received " + packSize + " bytes");
  MonPrintf("%s\n", buffer);
  MonPrintf("%f\n", rssi);
  //Heltec.display->display();
  count++;
}

//===========================================
// cbk: retreive contents of the received packet
//===========================================
void cbk(int packetSize) {
  //struct sensorData environment;
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packetBinary[i] = (char)LoRa.read();
  }
  //LoRa.receive
  rssi_lora = LoRa.packetRssi();
  rssi = "RSSI " + String(rssi_lora, DEC);
  if (packetSize == 40) {
    memcpy(&environment, &packetBinary, packetSize);
  } else if (packetSize == 24) {
    memcpy(&hardware, &packetBinary, packetSize);
  }
  LoRaData();
}

//===========================================
// setup:
//===========================================
void setup() {
  Serial.begin(115200);

  //Enable WDT for any lock-up events
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

#ifdef DEV_HELTEC_RECEIVER
  led.begin();
  LEDTitle();
#endif

  Serial.println("LoRa Receiver");
  Serial.println(VERSION);
#ifdef DEV_HELTEC_RECEIVER
  LoRa.setPins(18, 14, 26);
#else
  LoRa.setPins(15, 17, 13);
#endif
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  LoRa.receive();

  wifi_connect();
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  MonPrintf("LoRa receiver is online");
}

//===========================================
// loop:
//===========================================
void loop() {
  esp_task_wdt_reset();
  static int count = 0, Scount = 0, Hcount = 0, Xcount = 0;
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    Serial.printf("\n\n\nPacket size: %i\n", packetSize);
    cbk(packetSize);
    count++;
    if (packetSize == 24) {
      Hcount++;
    } else if (packetSize == 40) {
      Scount++;
    } else {
      Xcount++;
    }
#ifdef DEV_HELTEC_RECEIVER
    LEDStatus(count, Scount, Hcount, Xcount);
#endif
    //check for weather data packet
    if (packetSize == 40) {
      PrintEnvironment(environment);
      SendDataMQTT(environment);
    }
    //check for hardware data packet
    else if (packetSize == 24) {
      PrintHardware(hardware);
      SendDataMQTT(hardware);
    }
    //HexDump(packetSize);
  }
  delay(10);
}

//===========================================
// HexDump: output hex data of the environment structure - going away
//===========================================
void HexDump(int size) {
  //int size = 28;
  int x;
  char ch;
  char* p = (char*)&environment;

  for (x = 0; x < size; x++) {
    //ch = *(p+x);
    Serial.printf("%02X ", p[x]);
  }
  Serial.println();
}

//===========================================
// PrintEnvironment: Dump environment structure to console
//===========================================
void PrintEnvironment(struct sensorData environment) {
  Serial.printf("Rain Ticks 24h: %i\n", environment.rainTicks24h);
  Serial.printf("Rain Ticks 60m: %i\n", environment.rainTicks60m);
  Serial.printf("Temperature: %f\n", environment.temperatureC);
  Serial.printf("Wind speed: %f\n", environment.windSpeed);
  //TODO:  Serial.printf("Wind direction: %f\n", environment.windDirection);
  Serial.printf("barometer: %f\n", environment.barometricPressure);
  Serial.printf("Humidity: %f\n", environment.humidity);
  Serial.printf("UV Index: %f\n", environment.UVIndex);
  Serial.printf("Lux: %f\n", environment.lux);
}

//===========================================
// PrintEnvironment: Dump hardware structure to console
//===========================================
void PrintHardware(struct diagnostics hardware) {
  Serial.printf("Boot count: %i\n", hardware.bootCount);
  Serial.printf("Case Temperature: %f\n", hardware.BMEtemperature);
  //Serial.printf("Battery voltage: %f\n", hardware.batteryVoltage);
  Serial.printf("Battery ADC: %i\n", hardware.batteryADC);
  Serial.printf("Solar ADC: %i\n", hardware.solarADC);
  Serial.printf("ESP32 core temp C: %i\n", hardware.coreC);
}

//===========================================
// MonPrintf: diagnostic printf to terminal
//===========================================
void MonPrintf(const char* format, ...) {
  char buffer[200];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
#ifdef SerialMonitor
  Serial.printf("%s", buffer);
#endif
}
