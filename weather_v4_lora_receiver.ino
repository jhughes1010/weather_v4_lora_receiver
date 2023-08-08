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
                  Added WDT of 30 sec in case of hang up in loop() 
                  Set sender and receiver sync word to filter correct transmissions
                  Add 10 sec heartbeat LED for Heltec PCB   

   1.1.3 07-08-23 Removed Blynk header file reference and replace esp_wifi with WiFi

   1.2.0 07-30-23 Support for e-paper (4.2in Waveshare)
                  Basic, raw data output, nothing fancy
                  Output hardware and environment packet sizes as they need to match TX
                  #define for optional e-paper display

   1.2.1 08-06-23 Addition of Battery and Solar voltages in addition to ADC values
*/

//Hardware build target: ESP32
#define VERSION "1.2.1"


//#include "heltec.h"
#include <LoRa.h>
#include <spi.h>

#include <GxEPD.h>
#include <GxGDEW042T2/GxGDEW042T2.h>  // 4.2" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>



// e-paper pins mapping
#define CS 5
#define DC 27
#define DISP_RST 26
#define BUSY 25

GxIO_Class io(SPI, CS, DC, DISP_RST);
GxEPD_Class display(io, DISP_RST, BUSY);

#include "config.h"
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <PubSubClient.h>
#ifdef DEV_HELTEC_RECEIVER
#include <Wire.h>
#include <U8g2lib.h>
#endif

String rssi = "RSSI --";
String packSize = "--";
String packet;
byte packetBinary[512];

float rssi_wifi;
float rssi_lora;

#ifdef DEV_HELTEC_RECEIVER
U8G2_SSD1306_128X64_NONAME_F_SW_I2C led(U8G2_R0, /* clock=*/15, /* data=*/4, /* reset=*/16);
#endif

//===========================================
// Weather-environment structure
//===========================================
struct sensorData {
  int deviceID;
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
  int deviceID;
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
  MonPrintf("%s\n", buffer);
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
  if (packetSize == sizeof(environment)) {
    memcpy(&environment, &packetBinary, packetSize);
  } else if (packetSize == sizeof(hardware)) {
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
  pinMode(LED, OUTPUT);
#else
  LoRa.setPins(15, 17, 13);
#endif
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  LoRa.receive();
  wifi_connect();
#ifdef DEV_HELTEC_RECEIVER
  OLEDConnectWiFi();
#endif
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  LoRa.enableCrc();
  LoRa.setSyncWord(SYNC);
  Serial.printf("LoRa receiver is online\n");

#ifdef E_PAPER
  // Initialize the display
  display.init();
  display.setRotation(0);             // Set the rotation if needed (0, 1, 2, or 3)
  display.setTextColor(GxEPD_BLACK);  // Set the text color to black
  eTitle();
#endif

  //data structure stats
  MonPrintf("Hardware size: %i\n", sizeof(hardware));
  MonPrintf("Sensor size: %i\n", sizeof(environment));
}

//===========================================
// loop:
//===========================================
void loop() {
  static bool firstUpdate = true;
  esp_task_wdt_reset();
  static int count = 0, Scount = 0, Hcount = 0, Xcount = 0;
  int upTimeSeconds = 0;
  int packetSize = LoRa.parsePacket();

  environment.deviceID = 0;
  hardware.deviceID = 0;

  if (packetSize) {
    count++;
    cbk(packetSize);
    Serial.printf("Packet size: %i\n", packetSize);

    MonPrintf("Environment deviceID %x\n", environment.deviceID);
    MonPrintf("Hardware deviceID %x\n", hardware.deviceID);
    //check for weather data packet
    if (packetSize == sizeof(environment) && environment.deviceID == DEVID) {
      PrintEnvironment(environment);
      SendDataMQTT(environment);
      Scount++;
    }
    //check for hardware data packet
    else if (packetSize == sizeof(hardware) && hardware.deviceID == DEVID) {
      PrintHardware(hardware);
      SendDataMQTT(hardware);
      Hcount++;
      hardware.bootCount = hardware.bootCount % 1440;
    } else {
      Xcount++;
    }
#ifdef DEV_HELTEC_RECEIVER
    LEDStatus(count, Scount, Hcount, Xcount);
#endif
  }
  delay(10);
  if (firstUpdate | millis() % 60000 <= 200) {
    firstUpdate = false;
    MonPrintf(".");
    upTimeSeconds = millis() / 60000;
#ifdef E_PAPER
    display.fillScreen(GxEPD_WHITE);
    eUpdate(count, Hcount, Scount, Xcount, upTimeSeconds);
    eSensors();
    eHardware();
    display.update();
#endif
  }
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
  MonPrintf("Rain Ticks 24h: %i\n", environment.rainTicks24h);
  MonPrintf("Rain Ticks 60m: %i\n", environment.rainTicks60m);
  MonPrintf("Temperature: %f\n", environment.temperatureC);
  MonPrintf("Wind speed: %f\n", environment.windSpeed);
  //TODO:  Serial.printf("Wind direction: %f\n", environment.windDirection);
  MonPrintf("barometer: %f\n", environment.barometricPressure);
  MonPrintf("Humidity: %f\n", environment.humidity);
  MonPrintf("UV Index: %f\n", environment.UVIndex);
  MonPrintf("Lux: %f\n", environment.lux);
}

//===========================================
// PrintEnvironment: Dump hardware structure to console
//===========================================
void PrintHardware(struct diagnostics hardware) {
  MonPrintf("Boot count: %i\n", hardware.bootCount);
  MonPrintf("Case Temperature: %f\n", hardware.BMEtemperature);
  //Serial.printf("Battery voltage: %f\n", hardware.batteryVoltage);
  MonPrintf("Battery ADC: %i\n", hardware.batteryADC);
  MonPrintf("Solar ADC: %i\n", hardware.solarADC);
  MonPrintf("ESP32 core temp C: %i\n", hardware.coreC);
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

void eTitle(void) {

  // Set the text size and cursor position
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.print(" Weather Station V4.0 ");

  display.setCursor(60, 180);
  display.print("Debasish Dutta");
  display.setCursor(60, 210);
  display.println("James Hughes 2023");
  display.setCursor(60, 230);
  display.print("Ver: ");
  display.println(VERSION);

  // Update the display
  display.update();
  delay(2000);
}

void eUpdate(int count, int hardware, int sensor, int ignore, int upTimeSeconds) {
  int xStart, yStart;
  //int x, xOffset;


  display.setCursor(5, 281);
  display.setTextSize(2);
  display.print(count);
  display.print(" ");
  display.print(sensor);
  display.print(" ");
  display.print(hardware);
  display.print(" ");
  display.print(ignore);
  display.print(" ");
  display.print(upTimeSeconds);
}

void eSensors(void) {
  int xS, yS, y, yOffset;

  xS = 205;
  yS = 5;
  yOffset = 22;
  y = yS;

  display.setTextSize(2);
  display.drawRect(200, 0, 200, 280, GxEPD_BLACK);
  display.setCursor(xS, yS);
  display.print("Sensors:");
  //display.update();
  y += yOffset;
  display.setCursor(xS, y);
  display.print("TempC:");
  display.print(environment.temperatureC);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Rel Hum:");
  display.print(environment.humidity);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("mmHg:");
  display.print(environment.barometricPressure);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Dir:");
  display.print(environment.windDirectionADC);
  display.print(" sp:");
  display.print(environment.windSpeedMax);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Rn 1h:");
  display.print(environment.rainTicks60m);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Rn 24h:");
  display.print(environment.rainTicks24h);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Lux:");
  display.print(environment.lux);
}

void eHardware(void) {
  int xS, yS, y, yOffset;

  xS = 5;
  yS = 5;
  yOffset = 22;
  y = yS;

  display.setTextSize(2);
  display.drawRect(0, 0, 200, 280, GxEPD_BLACK);
  display.setCursor(xS, yS);
  display.print("Hardware:");
  y += yOffset;
  display.setCursor(xS, y);
  display.print("Solar:");
  display.print(hardware.solarADC);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Solar V:");
  float vSolar = (float)hardware.solarADC/ADCBattery;
  display.print(vSolar);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Battery:");
  display.print(hardware.batteryADC);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("Battery V:");
  float vBat = (float)hardware.batteryADC/ADCBattery;
  display.print(vBat);

  y += yOffset;
  display.setCursor(xS, y);
  display.print("BME Temp:");
  display.print(hardware.BMEtemperature);
}
