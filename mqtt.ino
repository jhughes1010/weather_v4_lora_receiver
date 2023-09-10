//mqtt data send



WiFiClient espClient;
PubSubClient client(espClient);


//=======================================================================
//  SendDataMQTT: send MQTT data to broker with 'retain' flag set to TRUE
//=======================================================================

void SendDataMQTT(struct sensorData environment) {
  char bufferTempF[5];
  char bufferTempC[5];
  char bufferRain[10];
  char bufferRain24[10];
  float temperatureF;
  float windSpeedMPH;
  float windSpeedMaxMPH;
  float inHg, mmHg;
  float vBatttery, vSolar;

  //int hourPtr = timeinfo.tm_hour;
  client.setServer(mqttServer, mqttPort);
  //client.setCallback(callback);

  while (!client.connected()) {
    MonPrintf("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      //delay(1000);
      while (1)
        ;
    }
  }
  temperatureF = environment.temperatureC * 9 / 5 + 32;
  windSpeedMPH = environment.windSpeed * 1 / 1.609;
  windSpeedMaxMPH = environment.windSpeedMax / 1.609;
  inHg = environment.barometricPressure * 0.0002953 + OFFSET_IN;
  mmHg = environment.barometricPressure * 0.0002953 * 25.4 + OFFSET_MM;
  setWindDirection(environment.windDirectionADC);

  MQTTPublish("sensors/imperial/temperature/", (int)temperatureF, PERSISTANCE);
  MQTTPublish("sensors/imperial/windSpeed/", (float)windSpeedMPH, PERSISTANCE);
  MQTTPublish("sensors/imperial/windSpeedMax/", (float)windSpeedMaxMPH, PERSISTANCE);
  MQTTPublish("sensors/imperial/rainfall/rainfall24/", (float)(environment.rainTicks24h * 0.011), PERSISTANCE);
  MQTTPublish("sensors/imperial/rainfall/rainfall60m/", (float)(environment.rainTicks60m * 0.011), PERSISTANCE);
  MQTTPublish("sensors/imperial/pressure/", inHg, PERSISTANCE);

  MQTTPublish("sensors/metric/windSpeed/", (float)environment.windSpeed, PERSISTANCE);
  MQTTPublish("sensors/metric/windSpeedMax/", (float)environment.windSpeedMax, PERSISTANCE);
  MQTTPublish("sensors/metric/temperature/", (int)environment.temperatureC, PERSISTANCE);
  MQTTPublish("sensors/metric/rainfall/rainfall24/", (float)(environment.rainTicks24h * 0.011 * 25.4), PERSISTANCE);
  MQTTPublish("sensors/metric/rainfall/rainfall60m/", (float)(environment.rainTicks60m * 0.011 * 25.4), PERSISTANCE);
  MQTTPublish("sensors/metric/pressure/", mmHg, PERSISTANCE);

  MQTTPublish("sensors/windDirection/", (float)wind.degrees, PERSISTANCE);
  MQTTPublish("sensors/windCardinalDirection/", wind.cardinalDirection, PERSISTANCE);


  MQTTPublish("sensors/lux/", environment.lux, PERSISTANCE);
  MQTTPublish("sensors/UVIndex/", environment.UVIndex, PERSISTANCE);
  MQTTPublish("sensors/relHum/", environment.humidity, PERSISTANCE);
  MonPrintf("Issuing mqtt disconnect\n");
  client.disconnect();
  MonPrintf("Disconnected\n");
}

//=======================================================================
//  SendDataMQTT: send MQTT data to broker with 'retain' flag set to TRUE
//=======================================================================
void SendDataMQTT(struct diagnostics hardware) {
  char bufferTempF[5];
  char bufferTempC[5];
  char bufferRain[10];
  char bufferRain24[10];
  float vBattery, vSolar;


  //int hourPtr = timeinfo.tm_hour;
  client.setServer(mqttServer, mqttPort);
  //client.setCallback(callback);

  while (!client.connected()) {
    MonPrintf("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      while (1)
        ;
      //delay(1000);
    }
  }

  vSolar = (float)hardware.solarADC / ADCSolar;
  vBattery = (float)hardware.batteryADC / ADCBattery;

  MQTTPublish("hardware/boot/", (int)hardware.bootCount, PERSISTANCE);
  MQTTPublish("hardware/rssi/", (int)rssi_lora, PERSISTANCE);
  MQTTPublish("hardware/vBattery/", (float)vBattery, PERSISTANCE);
  MQTTPublish("hardware/vSolar/", (float)vSolar, PERSISTANCE);
  MQTTPublish("hardware/chargeStatusB/", (bool)hardware.chargeStatusB, PERSISTANCE);
  MQTTPublish("hardware/caseTemperature/", hardware.BMEtemperature, PERSISTANCE);
  MQTTPublish("hardware/ADCbattery/", (int)hardware.batteryADC, PERSISTANCE);
  MQTTPublish("hardware/ADCsolar/", (int)hardware.solarADC, PERSISTANCE);
  //MQTTPublish("ESPcoreF/", (int)hardware->coreF, PERSISTANCE);
  MQTTPublish("hardware/ESPcoreC/", (int)hardware.coreC, PERSISTANCE);
  //MQTTPublish("timeEnabled/", (int)elapsedTime, PERSISTANCE);
  MQTTPublish("hardware/lowBattery/", false, PERSISTANCE);  //TODO
  MonPrintf("Issuing mqtt disconnect\n");
  client.disconnect();
  MonPrintf("Disconnected\n");
}

//=======================================================================
//  MQTTPublishString: routine to publish string
//=======================================================================
void MQTTPublish(const char topic[], char *value, bool retain) {
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%s", value);
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  MQTTPublishInt: routine to publish int values as strings
//=======================================================================
void MQTTPublish(const char topic[], int value, bool retain) {
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%i", value);
  MQTTSend(topicBuffer, payload, retain);
}


//=======================================================================
//  MQTTPublish Long: routine to publish int values as strings
//=======================================================================
void MQTTPublish(const char topic[], long value, bool retain) {
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%li", value);
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  MQTTPublishFloat: routine to publish float values as strings
//=======================================================================
void MQTTPublish(const char topic[], float value, bool retain) {
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  sprintf(payload, "%6.3f", value);
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  MQTTPublishBool: routine to publish bool values as strings
//=======================================================================
void MQTTPublish(const char topic[], bool value, bool retain) {
  char topicBuffer[256];
  char payload[256];

  strcpy(topicBuffer, mainTopic);
  strcat(topicBuffer, topic);
  if (!client.connected()) reconnect();
  client.loop();
  if (value) {
    sprintf(payload, "true");
  } else {
    sprintf(payload, "false");
  }
  MQTTSend(topicBuffer, payload, retain);
}

//=======================================================================
//  reconnect: MQTT reconnect
//=======================================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//=======================================================================
//  MQTTSend: MQTT send topic with value to broker
//=======================================================================
void MQTTSend(char *topicBuffer, char *payload, bool retain) {
  int status = 0;
  int retryCount = 0;
#ifdef ExtendedMQTT
  MonPrintf("%s: %s\n", topicBuffer, payload);
#endif
  while (!status && retryCount < 5) {
    status = client.publish(topicBuffer, payload, retain);
#ifdef ExtendedMQTT
    MonPrintf("MQTT status: %i\n", status);
#endif
    delay(50);
    retryCount++;
  }
}
