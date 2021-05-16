void wirewriteloop(int myregister, int myregisterval) {
  Wire.beginTransmission(AD5933ADDR);
  Wire.write(byte(myregister));
  Wire.write(byte(myregisterval));
  Wire.endTransmission();
  delayMicroseconds(dtime);
}
