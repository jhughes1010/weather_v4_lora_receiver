//rename to config.h

#define SerialMonitor

#define SYNC 0x54
//===========================================
//LoRa band
//===========================================
#define BAND 915E6  //you can set band here directly,e.g. 868E6,915E6,433E6

#define WDT_TIMEOUT 30   //watchdog timer

#define DEVID 0x11223344

//===========================================
//WiFi connection
//===========================================
char ssid[] = "ssid";      // WiFi Router ssid
char pass[] = "password";  // WiFi Router password

//===========================================
//MQTT broker connection
//===========================================
const char* mqttServer = "91.121.93.94";  //test.mosquitto.org
//const char* mqttServer = "192.168.5.66";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
const char mainTopic[20] = "RoyalGorge/";
#define RETAIN false

//===========================================
//Timezone information
//===========================================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -7 * 3600;
const int daylightOffset_sec = 3600;

//===========================================
//General defines
//===========================================
#define RSSI_INVALID -9999

//===========================================
//ADC calibration
//===========================================
#define ADCBattery 371 
#define ADCSolar 195

//===========================================
//Altitude offsets
//===========================================
#define OFFSET_IN 5.58 
#define OFFSET_MM 141.7

//===========================================
//James Hughes is using a Heltec_LoRa_v2 for receiver
//===========================================
//#define DEV_HELTEC_RECEIVER
//#define E_PAPER