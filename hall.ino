uint32_t comm_tmr, last_tmr;

void IRAM_ATTR HISR(){
  comm_tmr = millis() - last_tmr;
  last_tmr = millis();
}

void hall_init(){
  pinMode(CNT_HALL, INPUT_PULLUP);
  attachInterrupt(CNT_HALL, HISR, FALLING);
}

float get_speed(){ 
  if(millis() - last_tmr > 11.31*fs_data.w_diam){return 0;} // V < 1km/h => V = 0
  if(comm_tmr == 0){return 0;}                      // T = 0 ms => V = 0
  return (11309.4*fs_data.w_diam)/comm_tmr;                 // V = ((60000 / comm_tmr)*3.1415*w_diam*60)/1000 (km/h)
}

void send_speed(){
  char message[20];
  float spd = get_speed();
  snprintf(message, sizeof(message), "SPD: %f", spd);
  can_send_speed_data(spd);
  // mqtt.publish(topicSystem, message);
  Serial.println(message);
}