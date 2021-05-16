void pressure() {
  // a little inelegant to block out into three wloops, but makes entering times and debugging easier

  //  float a_voltage_max = 3.1 * (4095 / 3.3);
  const unsigned long bubble_time = 700000;
  const unsigned long ramp_time = 1000;
  const unsigned long baseline_time = 5000;
  const unsigned long postbubble_time = 60 * 60 * 1000; //enter first number in minutes

  pinMode(analog_out, OUTPUT);
  unsigned long start_time = millis();
  unsigned long time_now = start_time - sense_resolution;

  while ((millis() - start_time) <= baseline_time)  {

    if ((millis() - time_now) >= sense_resolution) {
      time_now = millis(); //put here so that sampling frequency is transparent and consistent
      freqfunction(millis() - start_time);
    }
  }


  //  digitalWrite(INH, HIGH);  //disconnect Z measurement terminals
  start_time = millis();
  time_now = start_time - sense_resolution;

  while ((millis() - start_time) <= bubble_time)  {

    if ((millis() - time_now) >= sense_resolution) {
      time_now = millis(); //put here so that sampling frequency is transparent and consistent
      freqfunction(millis() - start_time);
    }

    if ((millis() - start_time) <= ramp_time) {
      analogWrite(analog_out, ((float)(millis() - start_time) / ramp_time) * a_voltage_max);
      //      Serial.println(((float)(millis() - start_time) / ramp_time) * a_voltage_max);
      //      writeSD(String(millis() - start_time) + " " + String(((float)(millis() - start_time) / ramp_time) * a_voltage_max));
    }

    if ((millis() - start_time) > ramp_time && (millis() - start_time) <= (bubble_time - ramp_time)) {
      analogWrite(analog_out, a_voltage_max);
      //      Serial.println(a_voltage_max);
      //      writeSD(String(millis() - start_time) + " " + String(a_voltage_max));
    }

    if ((millis() - start_time) > (bubble_time - ramp_time)) {
      analogWrite(analog_out, ((-(float)(millis() - start_time) + bubble_time) / ramp_time) * a_voltage_max);
      //      Serial.println(((-(float)(millis() - start_time) + bubble_time) / ramp_time) * a_voltage_max);
      //      writeSD(String(millis() - start_time) + " " + String(((-(float)(millis() - start_time) + bubble_time) / ramp_time) * a_voltage_max));
    }
  }

  //  digitalWrite(D0, LOW);  //reconnect Z measurement terminals
  //  digitalWrite(D1, HIGH);

  pinMode(analog_out, INPUT);
  start_time = millis();
  time_now = start_time - sense_resolution;

  while ((millis() - start_time) <= postbubble_time)  {
    if ((millis() - time_now) >= sense_resolution) {
      time_now = millis(); //put here so that sampling frequency is transparent and consistent
      freqfunction(millis() - start_time);
    }
  }
}
