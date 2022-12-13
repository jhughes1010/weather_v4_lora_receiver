#ifdef DEV_HELTEC_RECEIVER
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

void LEDStatus(int count) {
  led.clearBuffer();  // clear the internal memory
  led.setCursor(0, 16);
  led.print("Rx #: ");
  led.print(count);
  led.sendBuffer();  // transfer internal memory to the display
}
#endif
