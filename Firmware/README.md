# Readme
Arduino code to interface AD5933 impedance analyzer and Teensy 3.2 microcontroller.
Used for flow and pressure sensing in platinum/Parylene C thin-film device.

Features:
* IMSM thermal flow sensor, bubble pressure sensor, and substrate/electrode integrity sensors using AD5933 LCR IC
* Low-side current sensing thru flow sensor heater 
* Sensirion LD20 flow sensor (I2C)
* EIS/CV sweeps addressable to any of 4 measurement electrode pairs + electrolysis electrodes
* Extends frequency range of AD5933 both low and high w/ external clock (code incomplete, but tapeout done)
* Completely autonomous data collection - designed for clinician use, not for engineers
* Low-power + sleep mode extends battery life



### Libraries ###
* Wire.h
* SD.h
* SPI.h
* math.h
* Time.h
* TimeLib.h
* Snooze.h
* SnoozeBlock.h
