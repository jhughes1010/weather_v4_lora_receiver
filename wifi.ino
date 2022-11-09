//=======================================================================
//  wifi_connect: connect to WiFi or explicitly connect to Blynk, if used
//=======================================================================
long wifi_connect(void) {
  bool WiFiConnectHalt = false;
  int retry = 0;
  long wifi_signal = 0;

  MonPrintf("Connecting to WiFi\n");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED && !WiFiConnectHalt) {
    delay(500);
    retry++;
    if (retry > 15) {
      MonPrintf("Max trys to connect to WiFi reached and failed");
      WiFiConnectHalt = true;
      wifi_signal = RSSI_INVALID;
      return wifi_signal;
    }

    MonPrintf("WiFi connected\n");
    wifi_signal = WiFi.RSSI();
  }
  return wifi_signal;
}