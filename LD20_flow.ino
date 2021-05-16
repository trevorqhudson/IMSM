//partly copied from Sensirino docs

void LD20_flow() {
  // To perform a measurement, first send 0x3608 to switch to continuous
  // measurement mode, then read 3x (2 bytes + 1 CRC byte) from the sensor.
  Wire.beginTransmission(LD20_address);
  Wire.write(0x36);
  Wire.write(0x08);
  Wire.endTransmission();
  // read 1 byte, from address 8
  Wire.requestFrom(LD20_address, 9);
  if (Wire.available()) {
    sensor_flow_value = Wire.read() << 8; // read the MSB from the sensor
    sensor_flow_value |= Wire.read();      // read the LSB from the sensor
    sensor_flow_crc = Wire.read();
    sensor_temp_value = Wire.read() << 8; // read the MSB from the sensor
    sensor_temp_value |= Wire.read();      // read the LSB from the sensor
    sensor_temp_crc = Wire.read();
    aux_value = Wire.read() << 8; // read the MSB from the sensor
    aux_value |= Wire.read();      // read the LSB from the sensor
    aux_crc = Wire.read();
  }
  signed_flow_value = (int16_t) sensor_flow_value;
  //      Serial.print(", signed value: ");
  //    Serial.print(signed_flow_value);

  scaled_flow_value = ((float) signed_flow_value) / calibration_factor; //this is the final flow rate output value
  //      Serial.print(", scaled value: ");
  //      Serial.print(scaled_flow_value);
  //  Serial.println(scaled_flow_value, DEC);

  // To stop the continuous measurement, first send 0x3FF9.
  //    Wire.beginTransmission(LD20_ADDRESS);
  //    Wire.write(0x3F);
  //    Wire.write(0xF9);
  //    ret = Wire.endTransmission();
  //    if (ret != 0) {
  //      Serial.println("Error during write measurement mode command");
  //    }

  //  delay(1000); // milliseconds delay between reads (for demo purposes)
}
