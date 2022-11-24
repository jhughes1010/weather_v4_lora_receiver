//LoRa baseed weather station - receiver node
//Hardware design by Debasish Dutta - opengreenenergy@gmail.com
//Software design by James Hughes - jhughes1010@gmail.com

/* History
   0.9.0 10-02-22 Initial development for Heltec ESP32 LoRa v2 devkit

   1.0.2 11-17-22 Remapping all mqtt topics

   1.1.0 11-24-22 sensor structure expanded to receive max wind speed also
*/

//Hardware build target: ESP32
#define VERSION "1.1.0"


//#include "heltec.h"
#include <LoRa.h>
#include <spi.h>

#include "config.h"
#include <esp_wifi.h>
//#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <PubSubClient.h>


String rssi = "RSSI --";
String packSize = "--";
String packet;
byte packetBinary[128];

float rssi_wifi;
float rssi_lora;



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
  } else {
    memcpy(&hardware, &packetBinary, packetSize);
  }
  LoRaData();
}

//===========================================
// setup:
//===========================================
void setup() {
  //WIFI Kit series V1 not support Vext control
  //Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Serial.begin(115200);

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
  //Heltec.display->init();
  //Heltec.display->flipScreenVertically();
  //Heltec.display->setFont(ArialMT_Plain_10);

  //Heltec.display->clear();

  //Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  //Heltec.display->drawString(0, 10, "Wait for incoming data...");
  //Heltec.display->display();

  LoRa.receive();

  wifi_connect();
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  MonPrintf("LoRa receiver is online");
}

//===========================================
// loop:
//===========================================
void loop() {

  int packetSize = LoRa.parsePacket();
  /* if (millis() % 300000 <= 10)
    {
     MonPrintf("\nCalibrating RTC\n");
     printLocalTime();
     //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
     printLocalTime();
     delay(100);
    }*/

  if (packetSize) {
    Serial.printf("\n\n\nPacket size: %i\n", packetSize);
    cbk(packetSize);
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
