void mqtt_init(){
    // Turn on DC boost to power on the modem
  pinMode(BOARD_POWERON_PIN, OUTPUT);
  digitalWrite(BOARD_POWERON_PIN, HIGH);

  // Set modem reset pin ,reset modem
  pinMode(MODEM_RESET_PIN, OUTPUT);
  digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
  digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
  digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

  // Turn on modem
  pinMode(BOARD_PWRKEY_PIN, OUTPUT);
  digitalWrite(BOARD_PWRKEY_PIN, LOW);
  delay(100);
  digitalWrite(BOARD_PWRKEY_PIN, HIGH);
  delay(1000);
  digitalWrite(BOARD_PWRKEY_PIN, LOW);

  // Set modem baud
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

  Serial.println("Start modem...");
  delay(3000);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.init()) {
      DBG("Failed to restart modem, delaying 10s and retrying");
      return;
  }

  String name = modem.getModemName();
  DBG("Modem Name:", name);

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) {
      modem.simUnlock(GSM_PIN);
  }

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
      SerialMon.println(" fail");
      digitalWrite(LED_ERR, HIGH);
      delay(10000);
      return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
      SerialMon.println("Network connected");
  }

  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(" fail");
      digitalWrite(LED_ERR, HIGH);
      delay(10000);
      return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) {
      SerialMon.println("GPRS connected");
  }

  // MQTT Broker setup
  mqtt.setServer(broker, mqtt_port);
}

void mqtt_update(){
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
      SerialMon.println("Network disconnected");
      digitalWrite(LED_ERR, HIGH);
      if (!modem.waitForNetwork(180000L, true)) {
          SerialMon.println(" fail");
          delay(10000);
          return;
      }
      if (modem.isNetworkConnected()) {
          SerialMon.println("Network re-connected");
          signaliseException(GSM_RECONN);
          digitalWrite(LED_ERR, LOW);
      }

      // and make sure GPRS/EPS is still connected
      if (!modem.isGprsConnected()) {
          SerialMon.println("GPRS disconnected!");
          SerialMon.print(F("Connecting to "));
          SerialMon.print(apn);
          if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
              SerialMon.println(" fail");
              digitalWrite(LED_ERR, HIGH);
              delay(10000);
              return;
          }
          if (modem.isGprsConnected()) {
              SerialMon.println("GPRS reconnected");
              signaliseException(GSM_RECONN);
              digitalWrite(LED_ERR, LOW);
          }
      }
  }

  if (!mqtt.connected()) {
      SerialMon.println("=== MQTT NOT CONNECTED ===");
      // Reconnect every 10 seconds
      uint32_t t = millis();
      if (t - lastReconnectAttempt > 10000L) {
          lastReconnectAttempt = t;
          if (mqttConnect()) {
              lastReconnectAttempt = 0;
          }
      }
      delay(100);
      return;
  }

  mqtt.loop();

  rssi = -113 + (modem.getSignalQuality() * 2);
  Serial.print("Signal quality (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

bool mqttConnect(){
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  bool status = mqtt.connect(broker, mqtt_user, mqtt_pass);

  if (status == false) {
      SerialMon.println(" fail");
      digitalWrite(LED_ERR, HIGH);
      return false;
  }
  SerialMon.println(" success");
  mqtt.subscribe(topicInPSU);
  return mqtt.connected();
}

void send_crash_log() {
    esp_reset_reason_t reset_reason = esp_reset_reason();
    if (reset_reason == 1 || reset_reason == 2 || reset_reason == 3) {
      Serial.println("Power-On Reset");
      setState(false);
      mqtt.publish(topicSystem, "RST: POR");
    } else {
      char message[50];
      snprintf(message, sizeof(message), "RST: %d", reset_reason);
      mqtt.publish(topicSystem, message);
    }
}

void signaliseException(uint8_t type){
  switch (type){
    case OK_INFO:
      Serial.println("MQTT: OK");
      mqtt.publish(topicSystem, "MQTT: OK");
      break;
    case CRC_ERROR:
      Serial.println("UART: CRC mismatch");
      mqtt.publish(topicSystem, "UART: CRC mismatch");
      break;
    case OL_ERROR:
      Serial.println("UART: Over limit");
      mqtt.publish(topicSystem, "UART: Over limit");
      break;
    case TIME_ERROR:
      Serial.println("UART: Timeout");
      mqtt.publish(topicSystem, "UART: Timeout");
      break;
    case PSU_IN_ERROR:
      Serial.println("PSU: Invalid input");
      mqtt.publish(topicSystem, "PSU: Invalid input");
      break;
    case CAN_ERROR:
      Serial.println("CAN: Init failed");
      mqtt.publish(topicSystem, "CAN: Init failed");
      break;
    case GSM_RECONN:
      Serial.println("GSM: Network reconnected");
      mqtt.publish(topicSystem, "GSM: Network reconnected");
    case GPS_NODATA:
      Serial.println("GPS: No data");
      mqtt.publish(topicSystem, "GPS: No data");
      break;
  }
}