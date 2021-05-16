void flow() {
  //  Serial.println("heat time"); Serial.println(heat_time);
  Serial.println("pre-measurement");
  writeSD("Pre-measurement");

  int i = 0;
  heat_voltage = 0;
  unsigned long flow_timer;

  int heat_flag = 0;
  sweepflag = 0;

  freqfunction(00);  //test open circuit

  //  if (global_imp_mag > 100) {      //killswitch for open circuit
  if (global_imp_mag > 5000000) {
    return;
  }

  digitalWrite(ADG804_A0, LOW);  //set shunt mux to heater resistor
  digitalWrite(ADG804_A1, LOW);

  unsigned long start_time = millis();
  unsigned long time_now = start_time - sense_resolution;

  while (millis() - start_time <= total_time) {
    heat_voltage = analogRead(heat_sense);

    if ((millis() - time_now) >= sense_resolution) {
      flow_timer = millis() - start_time;
      time_now = millis(); //put here so that sampling frequency is transparent and consistent
      //            Serial.println("Run flow at"); Serial.println(millis() - start_time);
      freqfunction(flow_timer);
      LD20_flow();
    }

    if ((millis() - start_time) >= warmup_time && (millis() - start_time) <= (warmup_time + heat_time)) {

      if (heater_on && heat_flag == 0) {
        digitalWrite(heatercntrl, HIGH);  //turn on heater
        Serial.println("heater on"); //writeSD("heater on");  //you only get to writeSD once between freqfunction - twice will cause an overflow (sometimes) and mess up timing
        heat_flag = 1;
      }
      //      pinMode(LED, OUTPUT);      //turn on LED during heating
      //      digitalWrite(LED, HIGH);

    } else {
      //delay(10);
    }

    if ((millis() - start_time) > (warmup_time + heat_time) && heat_flag == 1) {
      digitalWrite(heatercntrl, LOW);  //turn off heater
      Serial.println("heater off"); //writeSD("heater off");
      heat_flag = 2;
    }
  }
  wirewriteloop(0x80, 0xA0 | operand);  //turn off ad5933 after measurements
}
