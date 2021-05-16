void main1() {
  writeSD("");
  writeSD("");
  writeSD("-------------------------------------------------");
  writeSD("BioFLEX_Firmware_v9.0");
  writeSD("AD5933_Interface Software_for_BioFLEX");
  writeSD("TH-Biomedical_Microsystems_Lab (Meng_Lab)");
  writeSD(String("Code_Version") + ": " + String("v13") + " " + String("heater_on") + ": " + String(heater_on) + " " + String("sleep_mins") + ": " + String(60 * sleep_hours + sleep_mins) + " " + String("sleep_secs") + " " + String(sleep_secs));
  writeSD(String("operand_code") + ": " + String(operand));
  //    writeSD(String("act_code1") + ": " + String(act_code1) + " " + String("act_code2") + ": " + String(act_code2) + " " + String("act_code3") + ": " + String(act_code3));
  writeSD(String("Measurement_Number") + ": " + String(measure_number) + " " + String("settling_time") + ": " + String(settling_time));
  writeSD(String("warmup_time") + ": " + String(warmup_time) + " " + String("heat_time") + ": " + String(heat_time) + " " + String("cooldown_time") + ": " + String(cooldown_time));
  writeSD(String(hour()) + ":" + String(minute()) + ":" + String(second()));
  writeSD(String(month()) + "/" + String(day()) + "/" + String(year()));
  writeSD("");

  measurement_index = 0;  //tracks number of ad5933 calls

  //flow1 (1)
  digitalWrite(ZMUX_A0, LOW);
  digitalWrite(ZMUX_A1, LOW);
  digitalWrite(ZMUX_EN, HIGH);
  writeSD("flow_1"); Serial.println("flow_1");
  sweep_on = false;
  start_freq = 50000;
  operand = B0001;

  for (int iii = 0; iii < 3; iii++) {
    flow();
    delay(1000);
  }


  //pressure (2)
  digitalWrite(ZMUX_A0, HIGH);
  digitalWrite(ZMUX_A1, LOW);
  writeSD("Pressure Sense"); Serial.println("Pressure Sense");


  operand = B0000;  //PGA gain = 5x
  sweep_on = true;
  start_freq = 5000;
  for (sweep_index = 0; sweep_index < (num_increm + 1); sweep_index++) {
    sweepflag = 0;
    freqfunction(millis());
    Serial.println(start_freq); Serial.println(global_imp_mag);
    wirewriteloop(0x80, 0xA0 | operand);
    start_freq += delta_freq;
  }


  //covered IDE (3)
  digitalWrite(ZMUX_A0, LOW);
  digitalWrite(ZMUX_A1, HIGH);
  writeSD("Covered IDE"); Serial.println("Covered IDE");


  operand = B0000;  //PGA gain = 5x
  sweep_on = true;
  start_freq = 5000;
  for (sweep_index = 0; sweep_index < (num_increm + 1); sweep_index++) {
    sweepflag = 0;
    freqfunction(millis());
    Serial.println(start_freq); Serial.println(global_imp_mag);
    wirewriteloop(0x80, 0xA0 | operand);
    start_freq += delta_freq;
  }

  //exposed IDE (4)
  digitalWrite(ZMUX_A0, HIGH);
  digitalWrite(ZMUX_A1, HIGH);
  writeSD("Exposed IDE"); Serial.println("Exposed IDE");

  operand = B0001; //set PGA gain to 1x
  sweep_on = true;
  start_freq = 5000;
  for (sweep_index = 0; sweep_index < (num_increm + 1); sweep_index++) {
    sweepflag = 0;
    freqfunction(millis());
    Serial.println(start_freq); Serial.println(global_imp_mag);
    wirewriteloop(0x80, 0xA0 | operand);
    start_freq += delta_freq;
  }


  //  digitalWrite(LED, LOW);
  writeSD("Meas_complete");
  //Serial.println("Meas_complete");
  digitalWrite(ZMUX_EN, LOW);
}
