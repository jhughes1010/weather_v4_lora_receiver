//=======================================================================
//  wifi_connect: connect to WiFi. Replys on WDT to reset unit if no connection exists.
//=======================================================================
long wifi_connect(void) {
  bool WiFiConnectHalt = false;
  long wifi_signal = 0;

  MonPrintf("Connecting to WiFi\n");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED && !WiFiConnectHalt) {
    MonPrintf("WiFi - attempting to connect\n");
    delay(500);
  }
  MonPrintf("WiFi connected\n");
  wifi_signal = WiFi.RSSI();
  return wifi_signal;
}