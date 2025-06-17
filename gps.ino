float lat2      = 0;
float lon2      = 0;
float speed2    = 0;
float alt2      = 0;
int   vsat2     = 0;
int   usat2     = 0;
float accuracy2 = 0;
int   year2     = 0;
int   month2    = 0;
int   day2      = 0;
int   hour2     = 0;
int   min2      = 0;
int   sec2      = 0;
uint8_t    fixMode   = 0;

void gps_init(){
  delay(5000);
  modem.sendAT("+SIMCOMATI");
  modem.waitResponse();

  Serial.println("Enabling GPS/GNSS/GLONASS");
  while (!modem.enableGPS(MODEM_GPS_ENABLE_GPIO, MODEM_GPS_ENABLE_LEVEL)) {
      Serial.print(".");
  }
  Serial.println();
  Serial.println("GPS Enabled");

  modem.setGPSBaud(115200);
}

bool gps_update(){
  if (modem.getGPS(&fixMode, &lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2, &year2, &month2, &day2, &hour2, &min2, &sec2)) {
      Serial.print(lat2);
      Serial.print(" ");
      Serial.print(lon2);
      Serial.print(" ");
      Serial.println(vsat2);
      return true;
  } else {
      Serial.println("Couldn't get GPS/GNSS/GLONASS location.");
      signaliseException(GPS_NODATA);
      return false;
  }
}