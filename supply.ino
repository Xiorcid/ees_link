#define AUX_BAT_ADC_CALIBRATION 1.08


void setVoltage(uint16_t voltage){
  uint8_t command[] = {0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  command[5] = voltage & 0xFF;
  command[4] = (voltage >> 8) & 0xFF;
  uint16_t crc = calculateCRC(command, 6);
  command[6] = crc & 0xFF;
  command[7] = (crc >> 8) & 0xFF;
  Serial2.write(command, sizeof(command));
  waitForResponse(true);
}

void setCurrent(uint16_t current){
  uint8_t command[] = {0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
  command[5] = current & 0xFF;
  command[4] = (current >> 8) & 0xFF;
  uint16_t crc = calculateCRC(command, 6);
  command[6] = crc & 0xFF;
  command[7] = (crc >> 8) & 0xFF;
  Serial2.write(command, sizeof(command));
  waitForResponse(true);
}

// uint16_t getActVoltage(){
//   uint8_t command[] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA};
//   uint8_t response[7];
//   uint8_t i;
//   Serial2.write(command, sizeof(command));
//   waitForResponse(false);
//   while (Serial2.available()){
//     response[i] = Serial2.read();
//     i++;
//   }
//   return (response[3] << 8) | response[4];
// }

// uint16_t getActCurrent(){
//   uint8_t command[] = {0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A};
//   uint8_t response[7];
//   uint8_t i;
//   Serial2.write(command, sizeof(command));
//   waitForResponse(false);
//   while (Serial2.available()){
//     response[i] = Serial2.read();
//     i++;
//   }
//   return (response[3] << 8) | response[4];
// }

// uint32_t getActPower(){
//   uint8_t command[] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x02, 0x65, 0xCB};
//   uint8_t response[9];
//   uint8_t i;
//   Serial2.write(command, sizeof(command));
//   waitForResponse(false);
//   while (Serial2.available()){
//     response[i] = Serial2.read();
//     i++;
//   }
//   uint16_t U = (response[3] << 8) | response[4];
//   uint16_t I = (response[5] << 8) | response[6];
//   return U*I/100;
// }

PSU_REGs readPSURegisters(){
  uint8_t command[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x14, 0x45, 0xC5};
  uint8_t response[50];
  uint8_t i;
  PSU_REGs pwr;

  Serial2.write(command, sizeof(command));

  if(waitForResponse(false)){
    return pwr;
  }

  while (Serial2.available()){
    response[i] = Serial2.read();
    i++;
  }
  // Serial.println(i);
  if ((i == 0) || ((response[i-1] << 8) | response[i-2]) != calculateCRC(response, i-2)){
    signaliseException(1);
    return pwr;
  }

  pwr.actOutVoltage = (response[7] << 8) | response[8];
  pwr.actCurrent = (response[9] << 8) | response[10];
  pwr.actPower = (response[11] << 8) | response[12];
  pwr.actInVoltage = (response[13] << 8) | response[14];
  pwr.actOutEnergy = (response[19] << 8) | response[20];
  can_send_psu_data(response);

  return pwr;
}

void setState(bool state){
  if (state){
    uint8_t command[] = {0x01, 0x06, 0x00, 0x12, 0x00, 0x01, 0xE8, 0x0F};
    Serial2.write(command, sizeof(command));
  }else{
    uint8_t command[] = {0x01, 0x06, 0x00, 0x12, 0x00, 0x00, 0x29, 0xCF};
    Serial2.write(command, sizeof(command));
  }
  waitForResponse(true);
}

bool waitForResponse(bool clear){
  uint32_t tmr = millis();
  while(!Serial2.available() && millis() - tmr < 250){
    delayMicroseconds(100);
  }
  if (clear){
    delay(1);
    // uint8_t response[50];
    // uint8_t i;
    while (Serial2.available()){
      // response[i] = Serial2.read();
      // i++;
      Serial2.read();
    }

    // if (((response[i-1] << 8) | response[i-2]) == calculateCRC(response, i-2)){
    //   signaliseException(0);
    // }
  }
  return (millis() - tmr > 250);
}

uint16_t getAuxBatVoltage(){
  return analogRead(BOARD_BAT_ADC_PIN)*0.1611328125*AUX_BAT_ADC_CALIBRATION;
}

uint16_t calculateCRC(uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}