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

// set GSM PIN, if any
#define GSM_PIN "2618"

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "CRC16.h"
#include "CRC.h"
#include <Ticker.h>
#include <GyverPortal.h>
#include <GSON.h>
#include <esp_system.h>
#include <CAN.h>
#include <FileData.h>
#include <Wire.h>                 // Must include Wire library for I2C
#include "SparkFun_MMA8452Q.h"    // Click here to get the library: http://librarymanager/All#SparkFun_MMA8452Q
#include <SPIFFS.h> // 3 MB APP, 1 MB SPIFFS

const char *topicOutGPS      = "GPS_OUT/1";
const char *topicOutPSU      = "PSU_OUT/1";
const char *topicInPSU       = "PSU_IN/1";
const char *topicSystem      = "SUS_OUT/1";

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
};
PSU_REGs psu;

struct LOCAL_DATA{
  float w_diam;
};
LOCAL_DATA fs_data;

float VBat = 0;

uint32_t lastReconnectAttempt = 0;

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
  parsePSUInputData(pl);
}

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 18, 19);
  Serial2.setTimeout(10);

  pinMode(LED_ERR, OUTPUT);
  pinMode(LED_ACT, OUTPUT);
  digitalWrite(LED_ERR, LOW);
  digitalWrite(LED_ACT, HIGH);

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
}

void loop(){
  static uint32_t tmr;
  static uint32_t gps_tmr;
  static bool flg;
  mqtt_update();
  if (millis() - gps_tmr > 5000){
    if (gps_update()){
      char message[256];
      buildGPSTelemetryPackage().toCharArray(message, 100);
      mqtt.publish(topicOutGPS, message);
    }
    gps_tmr = millis();
  }

  if (Serial2.available()){
    Serial.print(Serial2.read(), HEX);
    Serial.print(" ");
  }

  if(Serial.available()){
    setVoltage(Serial.parseInt());
  }

  ui.tick();

  if (millis() - tmr > 1000){
    // can_send_hello();
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

    buildPSUTelemetryPackage().toCharArray(message, 100);
    mqtt.publish(topicOutPSU, message);
    tmr = millis();
    
    getAccelData();
    can_send_accel_data();
  }
}




