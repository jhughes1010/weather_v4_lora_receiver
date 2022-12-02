
//=======================================================
//  readWindDirection: Read ADC to find wind direction
//=======================================================
void setWindDirection(int DirectionADC) {
  int windPosition;
  //Initial direction
  //Prove it is not this direction
  String windDirection = "0";
  String windCardinalDirection = "N";
  int analogCompare[15] = { 150, 300, 450, 600, 830, 1100, 1500, 1700, 2250, 2350, 2700, 3000, 3200, 3400, 3800 };
  String windDirText[15] = { "157.5", "180", "247.5", "202.5", "225", "270", "292.5", "112.5", "135", "337.5", "315", "67.5", "90", "22.5", "45" };
  String windDirCardinalText[15] = { "SSE", "S", "WSW", "SSW", "SW", "W", "WNW", "ESE", "SE", "NNW", "NW", "ENE", "E", "NNE", "NE" };
  char buffer[10];
  //int vin = analogRead(WIND_DIR_PIN);

  for (windPosition = 0; windPosition < 15; windPosition++) {
    if (DirectionADC < analogCompare[windPosition]) {
      windDirection = windDirText[windPosition];
      windCardinalDirection = windDirCardinalText[windPosition];
      break;
    }
  }
  MonPrintf("Analog value: %i Wind direction: %s  \n", DirectionADC, windDirection);
  windDirection.toCharArray(buffer, 5);
  wind.degrees = atof(buffer);
  strcpy(wind.cardinalDirection, windCardinalDirection.c_str());
}
