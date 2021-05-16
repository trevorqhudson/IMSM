void freqfunction(unsigned long freq_timer) {

  //local variables
  int cycle_status = 0;
  int cycle_bit = 0;
  int16_t real_read = 0;
  int16_t imag_read = 0;
  int freq_status = 0;
  int freq_bit = 0;

  unsigned long start_code = 0;
  unsigned long start_82 = 0;
  unsigned long start_83 = 0;
  unsigned long start_84 = 0;

  unsigned long compare_82 = 16711680; //  first_compare = B111111110000000000000000;
  unsigned long compare_83 = 65280; //  second_compare = B000000001111111100000000;
  unsigned long compare_84 = 255; //  third_compare = B000000000000000011111111;

  float imp_mag;
  float imp_phase;

  float gain_factor;
  float gain_factor_intercept;

  //  float POA[18] = { -118, -105, -97, -101, -81, -73, -65, -54, -46, -37, -29, -20, -11, -4, -4, -11, 20, 27}; //[deg]: phase offset array, experimentally calibrated w/ 5.1 kohm resistor
  //  float POA[18] = {13.98, -11.67, -20.17, -11.39, -1.73, 7.46, 13.82, 25.48, 28.71, 34.29, 40.66, 46.34, 51.37, 56.46, 61.39, 67.32, 71.06, 76.2};
  //  float POA[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  //  float POA[18] = { -38.36, -33.82, -29.47, -25.19, -20.91, -16.55, -12.32, -7.95, -3.78, 0.47, 4.67, 8.9, 13.13, 17.33, 21.5, 25.7, 29.89, 34.09};

  start_code = 32 * start_freq; //(2^27)/(16MHz/4) = 32
  //  start_code = (start_freq / AD5933_freq) * 536870912; //2^27 * 4                        ***this part needs attention with flipping from float to int***
  start_82 = start_code & compare_82;  //imports start code and divides into 3 sections in hex
  start_82 = start_82 >> 16;
  start_83 = start_code & compare_83;
  start_83 = start_83 >> 8;
  start_84 = start_code & compare_84;

  if (bitRead(operand, 0)) {                             //change gain factor depending on set PGA gain (see '2019 1\EVD Iterations v2\EVD v7 Validation' for calibration data)
    gain_factor = gain_factor_1x;
    gain_factor_intercept = gain_factor_intercept_1x;
  } else {
    gain_factor = gain_factor_5x;
    gain_factor_intercept = gain_factor_intercept_5x;
  }


  //activation protocol for AD5933
  if (sweepflag == 0) {  //only do this first time
    //reset
    wirewriteloop(0x81, B00010000 | operand);        ////CHANGE HERE

    //assign start frequency registry ----- internal clock = 16.776 MHz
    wirewriteloop(0x82, start_82); //start freq: 50kHz = 1600116 (dec) = 0x186A74 (hex)
    wirewriteloop(0x83, start_83);
    wirewriteloop(0x84, start_84);

    //delta f
    wirewriteloop(0x85, 0x04);  //freq increments: 10 kHz = 320023 (dec) = 0x04E217 (hex)
    wirewriteloop(0x86, 0xE2);
    wirewriteloop(0x87, 0x17);

    //number of increments register
    wirewriteloop(0x88, 0x00);
    wirewriteloop(0x89, 0);

    //assign settling time cycles (not sure that this should go here)
    //    wirewriteloop(0x8A, 0x07);  //00000111(bin), x4 cycles
    //    wirewriteloop(0x8B, 0xFF);  //min total delay between measurements here is 69-70 msec
    wirewriteloop(0x8A, 0x00);  //00000111(bin), x4 cycles
    wirewriteloop(0x8B, 0x01);  //min total delay between measurements here is 26-27 msec

  }
  //control: enter standby mode (spec sheet says use a reset instead, test this if nec)
  wirewriteloop(0x80, B10110000 | operand);  //standby mode: 1011 0000

  //control: initialize with start frequency
  wirewriteloop(0x80, B00010000 | operand);  //init w start freq: 0001 0000


  //    if (sweepflag == 0) {
  //      delay(4000);  //settling time. check this first if getting noisy measurements w/ unexpected spikes!
  //    } else {
  //      delay(settling_time);
  //    }

  //frequency scan
  //control:
  wirewriteloop(0x80, B00100000 | operand);  //start freq sweep: 0010 0000


  while (!freq_bit) {
    //poll status register at cycle complete bit
    //place pointer on status register
    Wire.beginTransmission(AD5933ADDR);
    Wire.write(byte(0x8F));  //place I2C pointer on status register
    Wire.endTransmission();

    //request status
    Wire.requestFrom(AD5933ADDR, 1);  //request one byte (there should only be one byte at this register)

    //receive status
    if (Wire.available()) { //if one byte was received
      cycle_status = Wire.read();  //receive byte
      cycle_bit = bitRead(cycle_status, 1);  //read bit D1
      //      Serial.println("test point C");

      //      Serial.println("cycle bit is"); Serial.println(cycle_bit);

      if (cycle_bit) {  //if cycle is complete and data available
        //        Serial.println("test point D");

        //real data acquisition
        Wire.beginTransmission(AD5933ADDR);
        Wire.write(byte(0x94));  //point to real data MSB
        Wire.endTransmission();

        Wire.requestFrom(AD5933ADDR, 1);

        if (Wire.available()) {
          real_read = Wire.read();
          real_read = real_read << 8;
        }

        Wire.beginTransmission(AD5933ADDR);
        Wire.write(byte(0x95));  //point to real data LSB
        Wire.endTransmission();

        Wire.requestFrom(AD5933ADDR, 1);

        if (Wire.available()) {
          real_read |= Wire.read();
        }

        //        Serial.println("real_read is"); Serial.println(real_read);

        //imaginary data acquisition
        Wire.beginTransmission(AD5933ADDR);
        Wire.write(byte(0x96));  //point to imag data MSB
        Wire.endTransmission();

        Wire.requestFrom(AD5933ADDR, 1);

        if (Wire.available()) {
          imag_read = Wire.read();
          imag_read = imag_read << 8;
        }

        Wire.beginTransmission(AD5933ADDR);
        Wire.write(byte(0x97));  //point to imag data LSB
        Wire.endTransmission();

        Wire.requestFrom(AD5933ADDR, 1);

        if (Wire.available()) {
          imag_read |= Wire.read();
        }

        //Serial.println("imag_read is"); Serial.println(imag_read);

        //imp magnitude and phase calculation and storage
        imp_mag = sqrt(real_read * real_read + imag_read * imag_read);
        imp_mag = gain_factor / (float) imp_mag + gain_factor_intercept;
        //      imp_mag = 81150 / (pow(imp_mag, (float)1.097));  //use experimental fit
        global_imp_mag = imp_mag;

        //        imp_phase = atan2f((float)imag_read, (float)real_read) * (180 / pi);
        //        imp_phase -= -47.96;


        if (imag_read >= 0) {
          if (real_read >= 0)  { //first quadrant
            imp_phase = atan2f((float)imag_read, (float)real_read) * (180 / pi);
          } else {
            imp_phase = 180 + atan2f((float)imag_read, (float)real_read) * (180 / pi);
          }
        } else {
          if (real_read <= 0) {
            imp_phase = 180 + atan2f((float)imag_read, (float)real_read) * (180 / pi);
          } else {
            imp_phase = 360 + atan2f((float)imag_read, (float)real_read) * (180 / pi);
          }
        }

        String sweep_output[num_increm + 1];

        if (!sweep_on) {
          String output = String(measurement_index) + " " + String(start_freq) + " " + String(freq_timer) + " " + String(imp_mag, 4) + " " + String(imp_phase)
                          + " " + String(real_read) + " " + String(imag_read) + " " + String(millis()) + " " + String(heat_voltage) + " " + String(scaled_flow_value);
          writeSD(output); Serial.println(output);
        } else {
          //          imp_phase -= POA[sweep_index];
          sweep_output[sweep_index] = String(measurement_index) + " " + String(start_freq) + " " + String(sweep_index) + " " + String(imp_mag) + " " + String(imp_phase)
                                      + " " + String(real_read) + " " + String(imag_read) + " " + String(millis());
          //          Serial.println(sweep_output[sweep_index]);
          writeSD(sweep_output[sweep_index]);
          //save this in internal array
        }
        sweepflag = 1;
        //        wirewriteloop(0x80, B01000000 | operand);  //repeat
        freq_bit = 1;  //manually stop freq sweep - AD5933 was adding an extra sweep
      }
    }
  }
  //  AD5933_freq = 16000000; //reset back to AD5933 internal oscillator
  measurement_index++;

}
