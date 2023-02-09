//struct tm timeinfo;

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

void getCurrentTime(void) {
  char buffer[40];
  if (!getLocalTime(&timeinfo)) {
    MonPrintf("Failed to obtain time\n");
    return;
  }
  sprintf(buffer, "%02i:%02i:%02i", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  timeCurr = String(buffer);
  strftime(buffer, 40, "%a %d %b %G", &timeinfo);
  dateCurr = String(buffer);
}