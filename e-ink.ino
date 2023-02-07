
//U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;


void title() {
  const GFXfont* f = &FreeMonoBold9pt7b;
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  display.println("Weather Station v4");
  display.println(VERSION);
  display.println("Debasish Dutta/ James Hughes");
  display.drawCircle(200, 150, 30, GxEPD_BLACK);
  display.drawCircle(200, 150, 32, GxEPD_BLACK);
  display.update();
}

void consoleUpdate(void) {
  const GFXfont* f = &FreeMonoBold9pt7b;
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  box();
  lastUpdate();
  DisplayStatusSection(100, 100,-95);
  display.update();
}

void lastUpdate(void) {
  //struct tm timeinfo;
  getLocalTime(&timeinfo);
  display.setCursor(0, 290);
  display.printf("Date:%02i %02i %i Time: %02i:%02i:%02i\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void box(void) {
  display.drawRect(0, 0, 399, 278, GxEPD_BLACK);
}
//#########################################################################################
void DisplayStatusSection(int x, int y, int rssi) {
  display.drawRect(x - 28, y - 26, 115, 51, GxEPD_BLACK);
  display.drawLine(x - 28, y - 14, x - 28 + 114, y - 14, GxEPD_BLACK);
  display.drawLine(x - 28 + 115 / 2, y - 15, x - 28 + 115 / 2, y - 26, GxEPD_BLACK);
  //u8g2Fonts.setFont(u8g2_font_helvR08_tf);
  //drawString(x, y - 24, TXT_WIFI, CENTER);
  //drawString(x + 55, y - 24, TXT_POWER, CENTER);
  //DrawRSSI(x - 8, y + 5, rssi);
  DrawBattery(x + 47, y + 5);;
}

void DrawBattery(int x, int y) {
  uint8_t percentage = 85;
  //float voltage = analogRead(35) / 4096.0 * 7.46;
  //float voltage = atof(batteryVoltage);
  float voltage = 4.05;
  if (voltage > 1) {  // Only display if there is a valid reading
    Serial.println("Voltage = " + String(voltage));
    percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
    if (voltage >= 4.20) percentage = 100;
    if (voltage <= 3.50) percentage = 0;
    display.drawRect(x + 15, y - 12, 19, 10, GxEPD_BLACK);
    display.fillRect(x + 34, y - 10, 2, 5, GxEPD_BLACK);
    display.fillRect(x + 17, y - 10, 15 * percentage / 100.0, 6, GxEPD_BLACK);
    //drawString(x + 10, y - 11, String(percentage) + "%", RIGHT);
    drawString(x + 13, y + 5,  String(voltage, 2) + "v", CENTER);
  }
}

//#########################################################################################
void drawString(int x, int y, String text, alignment align) {
  int16_t  x1, y1; //the bounds of x,y and w and h of the variable 'text' in pixels.
  uint16_t w, h;
  display.setTextWrap(false);
  display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  if (align == RIGHT)  x = x - w;
  if (align == CENTER) x = x - w / 2;
  u8g2Fonts.setCursor(x, y + h);
  u8g2Fonts.print(text);
}