float accel_x;
float accel_y;
float accel_z;

void accel_init(){
  Wire.begin();
  if(!mma.begin(ACCEL_ADDR)){
    digitalWrite(LED_ERR, HIGH);
    return;
  }
  isAccelReady = true;
  mma.setRange(MMA8451_RANGE_2_G);
}

bool getAccelData(){
  if(!isAccelReady){
    return false;
  }
  mma.read();
  sensors_event_t event; 
  mma.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  accel_x = event.acceleration.x;
  accel_y = event.acceleration.y;
  accel_z = event.acceleration.z;

  Serial.print("X: \t"); Serial.print(event.acceleration.x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(event.acceleration.y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(event.acceleration.z); Serial.print("\t");
  Serial.println("m/s^2 ");
  
  return true;
}