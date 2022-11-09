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
      delay(1000);
    }
  }
  temperatureF = environment.temperatureC * 9 / 5 + 32;

  MQTTPublish("imperial/temperature/", (int)temperatureF, true);
  MQTTPublish("imperial/windSpeed/", environment.windSpeed, true);
  MQTTPublish("imperial/rainfall/rainfall24/", (float)(environment.rainTicks24h * 0.011), true);
  MQTTPublish("imperial/rainfall/rainfall60m/", (float)(environment.rainTicks60m * 0.011), true);

  MQTTPublish("metric/windSpeed/", environment.windSpeed, true);
  MQTTPublish("metric/temperature/", (int)environment.temperatureC, true);
  MQTTPublish("metric/rainfall/rainfall24/", (float)(environment.rainTicks24h * 0.011 * 25.4), true);
  MQTTPublish("metric/rainfall/rainfall60m/", (float)(environment.rainTicks60m * 0.011 * 25.4), true);


  // TODO:  MQTTPublish("windDirection/", (int)environment.windDirection, true);
  //MQTTPublish("windCardinalDirection/", environment.windCardinalDirection, true);


  MQTTPublish("lux/", environment.lux, true);
  MQTTPublish("UVIndex/", environment.UVIndex, true);
  MQTTPublish("relHum/", environment.humidity, true);
  MQTTPublish("metric/pressure/", environment.barometricPressure, true);
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
      delay(1000);
    }
  }
  MQTTPublish("boot/", (int)hardware.bootCount, true);
  MQTTPublish("rssi/", rssi_lora, true);
  MQTTPublish("batteryVoltage/", hardware.batteryVoltage, true);

  MQTTPublish("caseTemperature/", hardware.BMEtemperature, true);
  MQTTPublish("batteryADC/", (int)hardware.batteryADC, true);
  //MQTTPublish("ESPcoreF/", (int)hardware->coreF, true);
  MQTTPublish("ESPcoreC/", (int)hardware.coreC, true);
  //MQTTPublish("timeEnabled/", (int)elapsedTime, true);
  //MQTTPublish("lowBattery/", lowBattery, true);
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