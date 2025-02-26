void can_init(){
  pinMode(CAN_STBY, OUTPUT);
  digitalWrite(CAN_STBY, LOW);
  CAN.setPins(CAN_RX, CAN_TX);
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
  }else{
    Serial.println("Starting CAN OK!");
  }
}

// void can_send_hello(){
//   Serial.print("Sending packet ... ");
//   CAN.beginPacket(0x12);
//   CAN.write('h');
//   CAN.write('e');
//   CAN.write('l');
//   CAN.write('l');
//   CAN.write('o');
//   CAN.endPacket();
// }

void can_send_psu_data(uint8_t *data){
  Serial.print("Sending PSU CAN packet ... ");
  CAN.beginPacket(0x12);
  // U
  CAN.write(data[7]);
  CAN.write(data[8]);
  // I
  CAN.write(data[9]);
  CAN.write(data[10]);
  // E
  CAN.write(data[19]);
  CAN.write(data[20]);
  CAN.endPacket();
}

void can_send_speed_data(float speed){
  Serial.print("Sending SPD HAL packet ... ");
  CAN.beginPacket(0xED);
  uint16_t spd = speed*100;
  CAN.write((spd >> 8) & 0xFF);
  CAN.write(spd & 0xFF);

  CAN.endPacket();
}

void can_send_accel_data(){
  Serial.print("Sending ACCEL packet ... ");
  CAN.beginPacket(0xC3);

  uint16_t x = accel_x*100;
  CAN.write((x >> 8) & 0xFF);
  CAN.write(x & 0xFF);

  uint16_t y = accel_y*100;
  CAN.write((y >> 8) & 0xFF);
  CAN.write(y & 0xFF);

  uint16_t z = accel_z*100;
  CAN.write((z >> 8) & 0xFF);
  CAN.write(z & 0xFF);

  CAN.endPacket();
}