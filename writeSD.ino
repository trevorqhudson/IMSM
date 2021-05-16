void writeSD(String writeString) {
  SD.begin(CS);
  File datafile = SD.open("log1.txt", FILE_WRITE);

  if (datafile) {
    datafile.println(writeString);
    datafile.close();
  }
}
