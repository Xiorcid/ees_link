String buildPSUTelemetryPackage(){
  gson::string gs;
  gs.beginObj();
    gs.beginObj("PSU");
      gs["Uop"] = (uint16_t)psu.actOutVoltage;
      gs["Iop"] = (uint16_t)psu.actCurrent;
      gs["Pop"] = (uint16_t)psu.actPower;
      gs["Uip"] = (uint16_t)psu.actInVoltage;
      gs["Wh"] = (uint16_t)psu.actOutEnergy;
    gs.endObj();
  gs.endObj();
  gs.end();
  Serial.println(gs);
  return String(gs);
}

String buildGPSTelemetryPackage(){
  gson::string gs;
  // char date[30];
  // char time[30];
  // sprintf(date, "%d/%d/%d", day2, month2, year2);
  // sprintf(time, "%d:%d:%d", hour2, min2, sec2);
  gs.beginObj();
    gs.beginObj("GPS");
      gs["Lat"] = lat2*10000;
      gs["Lon"] = lon2*10000;
      gs["Spd"] = speed2;
      // gs["Date"] = date;
      // gs["Time"] = time;
    gs.endObj();
  gs.endObj();
  gs.end();
  Serial.println(gs);
  return String(gs);
}

String buildAccelTelemetryPackage(){
  gson::string gs;
  // char date[30];
  // char time[30];
  // sprintf(date, "%d/%d/%d", day2, month2, year2);
  // sprintf(time, "%d:%d:%d", hour2, min2, sec2);
  gs.beginObj();
    gs.beginObj("Accel");
      gs["X"] = accel_x*100;
      gs["Y"] = accel_y*100;
      gs["Z"] = accel_z*100;
      // gs["Date"] = date;
      // gs["Time"] = time;
    gs.endObj();
  gs.endObj();
  gs.end();
  Serial.println(gs);
  return String(gs);
}

void parsePSUInputData(String data){
  gson::Parser p;
  p.parse(data);
  if(p["PSU"].has("U")){
    setVoltage((uint16_t)p["PSU"]["U"]);
  }
  if(p["PSU"].has("I")){
    setCurrent((uint16_t)p["PSU"]["I"]);
  }
  if(p["PSU"].has("St")){
    setState((bool)int(p["PSU"]["St"]));
  }
}
