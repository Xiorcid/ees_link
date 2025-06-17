#include "login_data.h"
#include "utilities.h"

#define TINY_GSM_MUX_BUFFER 1024
#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_TX_BUFFER 1024
#define MQTT_MAX_PACKET_SIZE 512

#define SerialMon Serial
#define SerialAT Serial1

#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#define PSU_DATA_TIME 500
#define GPS_DATA_TIME 500

// set GSM PIN, if any
#define GSM_PIN "1528"

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <buildTime.h>
#include "CRC16.h"
#include "CRC.h"
#include <Ticker.h>
#include <GyverPortal.h>
#include <GSON.h>
#include <esp_system.h>
#include <CAN.h>
#include <FileData.h>
#include <Wire.h>                 // Must include Wire library for I2C
#include "SparkFun_MMA8452Q.h"    // Modification for other 845x family chips is necessary, also i2c address
#include <SPIFFS.h> // 3 MB APP, 1 MB SPIFFS

GyverPortal ui;

CRC16 crc;

MMA8452Q accel(ACCEL_ADDR); 

TinyGsm        modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient  mqtt(client);

struct PSU_REGs{
  uint16_t actOutVoltage;
  uint16_t actCurrent;
  uint16_t actPower;
  uint16_t actInVoltage;
  uint16_t actOutEnergy;
  bool areDataValid;
};
PSU_REGs psu;

struct LOCAL_DATA{
  float w_diam;
};
LOCAL_DATA fs_data;

float VBat = 0;

uint32_t lastReconnectAttempt = 0;

uint32_t zeroEnergy;
uint32_t correctionEnergy;

uint8_t resetReason = 0;

bool isAccelReady = false;
bool isCANReady = false;

int16_t rssi;

FileData fs_data_FD(&SPIFFS, "/w_diam.dat", 'B', &fs_data, sizeof(fs_data));

void mqttCallback(char *topic, byte *payload, unsigned int len){
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();
  
  String pl;
  for (int i = 0; i < len; i++){
    pl += (char)(payload[i]);
  }

  signaliseException(OK_INFO);
  char echo_buf[256];
  pl.toCharArray(echo_buf, len);
  mqtt.publish(topicSystem, echo_buf);
  
  parsePSUInputData(pl);
}

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 18, 19);
  Serial2.setTimeout(10);

  Serial.printf("BUILD %02d%02d%d%d-%02d%02d\n", BUILD_DAY, BUILD_MONTH, BUILD_YEAR_CH2, BUILD_YEAR_CH3, BUILD_HOUR, BUILD_MIN);

  pinMode(LED_ERR, OUTPUT);
  pinMode(LED_ACT, OUTPUT);
  digitalWrite(LED_ERR, LOW);
  digitalWrite(LED_ACT, LOW);

  SPIFFS.begin();
  fs_data_FD.read();

  mqtt_init();
  mqtt.setCallback(mqttCallback);

  gps_init();
  can_init();
  accel_init();

  portal_init();
  
  mqtt_update();
  send_crash_log();
  hall_init();

  digitalWrite(LED_ACT, HIGH);
}

void loop(){
  static uint32_t tmr;
  static uint32_t gps_tmr;

  mqtt_update();
  ui.tick();
  
  if (millis() - gps_tmr > GPS_DATA_TIME){
    if (gps_update()){
      char message[256];
      buildGPSTelemetryPackage().toCharArray(message, 100);
      mqtt.publish(topicOutGPS, message);
    }
    gps_tmr = millis();
  }

  if (millis() - tmr > PSU_DATA_TIME){
    char message[256];
    send_speed();

    psu = readPSURegisters();
    Serial.print(psu.actOutVoltage);
    Serial.print(" V ");
    Serial.print(psu.actCurrent);
    Serial.print(" A ");
    Serial.print(psu.actPower);
    Serial.print(" W ");
    Serial.print(psu.actInVoltage);
    Serial.print(" V ");
    Serial.print(psu.actOutEnergy);
    Serial.println(" Wh");

    if(psu.areDataValid){
      buildPSUTelemetryPackage().toCharArray(message, 100);
      mqtt.publish(topicOutPSU, message);
    }

    if(getAccelData()){
      buildAccelTelemetryPackage().toCharArray(message, 100);
      mqtt.publish(topicOutAccel, message);
      can_send_accel_data();
    }

    snprintf(message, sizeof(message), "RSSI: %d", rssi);
    mqtt.publish(topicSystem, message);
    
    tmr = millis();
  }
}




