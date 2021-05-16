void EIS_sweep() {

  digitalWrite(CLOCK_CTRL1, LOW); //activate DS1077L

  byte access_DIV = 0x01;
  byte access_MUX = 0x02;
  byte access_BUS = 0x0D;

  //set N div word to 10
  Wire.beginTransmission(DS1077LADDR);
  delayMicroseconds(100);

  Wire.write(access_DIV);
  delayMicroseconds(100);

  Wire.write(B00000010);
  delayMicroseconds(100);

  Wire.write(B0);
  delayMicroseconds(100);


  //set MUX with appropriate parameters: CTRL0 grounded disables OUT0, CTRL1 powers down device, P1 prescaler set to 8 (resulting in 500 kHz clock)
  Wire.beginTransmission(DS1077LADDR);
  delayMicroseconds(100);

  Wire.write(access_MUX);
  delayMicroseconds(100);

  Wire.write(B1000001);
  delayMicroseconds(100);

  Wire.write(B10 << 6);
  delayMicroseconds(100);



  operand = B0001;  //use this for lower impedance measurement (exposed IDE)
  //      operand = B0000;  //use this for higher impedance measurement (covered IDE)

  sweep_on = true;
  start_freq = 1000;
  for (sweep_index = 0; sweep_index < (num_increm + 1); sweep_index++) {  //figure out how to do this with log scale numbers (10^(1/5))^10,15,20, etc
    for (int i = 0; i < 5; i++) {
      if (start_freq < 1000) {
        AD5933_freq = 500000; //hard-coded based on DS1077L parameters set above to 500 kHz
      }
      sweepflag = 0;
      freqfunction(millis());
      Serial.println(start_freq); Serial.println(global_imp_mag);
    }
    wirewriteloop(0x80, 0xA0 | operand);
    start_freq += delta_freq;
  }

}
