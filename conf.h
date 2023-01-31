//rename to config.h

#define SerialMonitor

#define SYNC 0xc7
//===========================================
//LoRa band
//===========================================
#define BAND 915E6  //you can set band here directly,e.g. 868E6, 915E6, 433E6

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
//const char* mqttServer = "test.mosquitto.org";
const char* mqttServer = "192.168.5.66";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "mqtt_password";
const char mainTopic[20] = "RoyalGorgee/";

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
//James Hughes is using a Heltec_LoRa_v2 for receiver
//===========================================
//#define DEV_HELTEC_RECEIVER