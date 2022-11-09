struct tm timeinfo;

//=======================================================================
//  printLocalTime: prints local timezone based time
//=======================================================================
void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    MonPrintf("Failed to obtain time\n");
    return;
  }
  Serial.printf("Date:%02i %02i %i Time: %02i:%02i:%02i\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void printLocalTimeLCD(void) {
  if (!getLocalTime(&timeinfo)) {
    MonPrintf("Failed to obtain time\n");
    return;
  }
  //jh  display.printf("Date:%02i %02i %i\nTime: %02i:%02i:%02i\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}