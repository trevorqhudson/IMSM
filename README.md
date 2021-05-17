# IMSM
integrated multi-sensor module (IMSM) for transducing flow dynamics in hydrocephalic shunts

This repo contains firmware + schematics for a portable datalogger designed for an prototype polymer sensor device.

Features:
* IMSM thermal flow sensor, bubble pressure sensor, and substrate/electrode integrity sensors using AD5933 LCR IC
* Low-side current sensing thru flow sensor heater 
* Sensirion LD20 flow sensor (I2C)
* EIS/CV sweeps addressable to any of 4 measurement electrode pairs + electrolysis electrodes
* Extends frequency range of AD5933 both low and high w/ external clock (code incomplete, but tapeout done)
* Completely autonomous data collection - designed for clinician use, not for researchers only
* Low-power + sleep mode extends battery life

[Meng Lab research page](https://biomems.usc.edu/ "easter egg text")
