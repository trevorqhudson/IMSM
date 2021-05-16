void CV_sweep(boolean CV_direction) {

  //default + safe switching mode is: flipflop=unflipped, bubble mux=bubble circuit(to isolate signal lines)

  pinMode(analog_out, OUTPUT);
  analogWrite(analog_out, 0);

  //CV variables
  const int CV_high = 1200; //[mV]
  const int CV_low = 1200; //[mV]
  float CV_scanrate = 100; //[mV/s]
  const int CV_loopcount = 15; //number of sweeps
  const int CV_timeresolution = 50; //time between current measurements [ms]
  CV_scanrate = CV_scanrate / 1000; //[mV/ms]

  float CV_rampup_time = CV_high / CV_scanrate;
  float CV_rampdown_time = CV_low / CV_scanrate;

  const float analog_resolution = 4095; //12 bit analog output

  //  digitalWrite(bubblemux_mod, HIGH);  //select bubble generation circuit
  digitalWrite(bubblemux_mod, LOW);  //select ZMUX circuit

  //  digitalWrite(ADG804_A0, LOW);  //set shunt mux to bubble resistor pin
  //  digitalWrite(ADG804_A1, HIGH);

  digitalWrite(ADG804_A0, HIGH);  //set shunt mux to ZMUX pin
  digitalWrite(ADG804_A1, LOW);


  /////////////////////////////////////////////////////////////////////////////////////////////////


  for (int i = 0; i < CV_loopcount; i++) {

    digitalWrite(flipflop_mod, !CV_direction);

    unsigned long start_time = millis();
    unsigned long time_now = start_time;

    while ((millis() - start_time) <= (2 * CV_rampup_time + 2 * CV_rampdown_time))  {

      if ((millis() - time_now) >= CV_timeresolution) {
        time_now = millis(); //put here so that sampling frequency is transparent and consistent
        writeSD(analogRead(heat_sense));
      }

      if ((millis() - start_time) <= CV_rampup_time) {
        analogWrite(analog_out, ((float)(millis() - start_time) / CV_rampup_time) * analog_resolution * (CV_high / V_power));
        //      Serial.println(((float)(millis() - start_time) / CV_rampup_time) * analog_resolution * (CV_high / V_power));
        //      writeSD(String(millis() - start_time) + " " + String(((float)(millis() - start_time) / CV_rampup_time) * analog_resolution * (CV_high / V_power));
        Serial.println("Phase 1");
      }

      if (((millis() - start_time) <= 2 * CV_rampup_time) && ((millis() - start_time) > CV_rampup_time)) {
        analogWrite(analog_out, (-(float)(millis() - start_time) + (2 * CV_rampup_time)) / CV_rampup_time * analog_resolution * (CV_high / V_power));
        Serial.println("Phase 2");
      }

      if ((millis() - start_time) <= (2 * CV_rampup_time + CV_rampdown_time) && ((millis() - start_time) > (2 * CV_rampup_time))) {
        digitalWrite(flipflop_mod, CV_direction);
        analogWrite(analog_out, ((float)(millis() - start_time) - (2 * CV_rampup_time)) / CV_rampdown_time * analog_resolution * (CV_low / V_power));
        Serial.println("Phase 3");
      }

      if ((millis() - start_time) > (2 * CV_rampup_time + CV_rampdown_time)) {
        analogWrite(analog_out, (-(float)(millis() - start_time) + (2 * CV_rampup_time + 2 * CV_rampdown_time)) / CV_rampdown_time * analog_resolution * (CV_low / V_power));
        Serial.println("Phase 4");
      }

      //      Serial.println(analog_out);

    }
  }

  //default + safe switching mode is: flipflop=unflipped, bubble mux=bubble circuit(to isolate signal lines), R_shunt=low R
  digitalWrite(bubblemux_mod, HIGH);
  digitalWrite(flipflop_mod, LOW);
}
