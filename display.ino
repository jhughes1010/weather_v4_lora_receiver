#ifdef DEV_HELTEC_RECEIVER_LED
void LEDTitle(void) {
  led.enableUTF8Print();
  led.clearBuffer();  // clear the internal memory
  led.setFont(u8g2_font_logisoso16_tf);
  led.setCursor(0, 16);
  led.print("LoRa Rx: ");
  //led.setCursor(74, 16);
  led.print(VERSION);
  led.sendBuffer();  // transfer internal memory to the display
}

void OLEDConnectWiFi(void) {
  led.clearBuffer();  // clear the internal memory
  led.setFont(u8g2_font_logisoso16_tf);
  led.setCursor(0, 16);
  led.print("WiFi on");
  led.sendBuffer();
}

void LEDStatus(int count, int Scount, int Hcount, int Xcount) {
  led.clearBuffer();  // clear the internal memory
  led.setCursor(0, 15);
  led.print("Tot #: ");
  led.print(count);

  led.setCursor(0, 31);
  led.print("Sen #: ");
  led.print(Scount);

  led.setCursor(0, 47);
  led.print("Hdw #: ");
  led.print(Hcount);

  led.setCursor(0, 63);
  led.print("XXX #: ");
  led.print(Xcount);

  led.sendBuffer();  // transfer internal memory to the display

#ifdef SerialMonitor
  Serial.printf("Total Count: %i\n", count);
  Serial.printf("Total Sensor Count: %i\n", Scount);
  Serial.printf("Total Hardware Count: %i\n", Hcount);
  Serial.printf("Total XXX Count: %i\n", Xcount);
#endif
}

void blink(int count) {
  int x;
  for (x = 0; x < count; x++) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(150);
  }
}
#endif
