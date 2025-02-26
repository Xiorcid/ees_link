float accel_x;
float accel_y;
float accel_z;

void accel_init(){
  Wire.begin();
  if(!accel.begin()){
    digitalWrite(LED_ERR, HIGH);
  }
  accel.setScale(SCALE_2G);
}

void getAccelData(){
  if (accel.available()) {
    accel_x = accel.getCalculatedX();
    accel_y = accel.getCalculatedY();
    accel_z = accel.getCalculatedZ();
  }
}