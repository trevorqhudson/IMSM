// changelog
// 5_22 - 6_6: gather user inputs as global variables, add time_comments option, eliminate phase flowchart for simple atan2 output, add sweep option, change PGA gain to x5,
//             comment out SD pins reset at start of setup()
// 6_7: reset 'time_now' in flow() before calling freqfunction() such that sampling rate is more predictable and consistent
// 6_9: slightly changed string definition in freqfunction(), added 'flow_timer' input to freqfunction(), moved pinMode declarations inside of flow(), changed all outputs pins to
//      digital - research how grounding a microcontroller works, is it standard practice to add pulldown resistors
// 6_22:snooze library, heater_on to user-controllable vars, set unused I/O pins to output
// 6_23:cleaning up sweep operations
// 6_26:snooze lib readded, manually code in sweep
// 6_27:finished manually coding in sweep, set real_ and imag_read to record on SD
// 6_28:start removing features until we find problem
// 7_26:save program parameters in SD write header
// 8_3: snocontinuing with saving parameters in SD write header, added cut-off check for fluidic short
// 8_4: add pressure sensing code
// 8_9: change LED to constantly on, add phase constants for patency sweep
// 8_10:change flow programming to make AD5933 calls smoother
// 9_6: add 'operand' variable to ease user changes to signal type, add sig figs to impedance displaygain
// 11_21: this version contains voltage measurement apparatus on analog read pins, in flow function
// v11.99: commented out all "Serial" references to try to save power. this should be the only change from v11.98
// v12: add sweepflag=o before all naked freqfunction calls, and follow by turning off AD5933 - otherwise, startup register sets won't happen
// v12.02: sent to CHLA on 3.27 - rename this version to change parameters
// v12.04: added bubble generation voltage
// v12.05: combining Priya's code for higher frequency measurement. removed delay
// v12.06: protocol for Alex
// v13:    rewriting all for PCB v2: temp sensing, LD20, portable CV, pressure ramp [switching architecture]
// v14:    new board update. ADG709 mux replaces CD4052 mux. new pin layouts. added DS1077L external crystal. changed low-side current sensing circuit.
// v14.2:  add PGA gain variation, 1x on exposed IDE and 5x on flow and covered IDE. covered IDE is still out of range here
// v14.3:  add EIS distinct function

#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <math.h>
#include <Time.h>
#include <TimeLib.h>
#include <Snooze.h>
#include <SnoozeBlock.h>

//snooze
SnoozeAlarm alarm;
SnoozeBlock config1(alarm);


//user-controllable vars, in no particular order
const uint32_t heat_time = 10000;  //time heater is on in msecs (flow)
const uint32_t warmup_time = 10000;
const uint32_t cooldown_time = 60000;
const uint32_t total_time = heat_time + cooldown_time + warmup_time;
const int NC_time = 10000;  //time nuc core electrodes are active in msecs (pressure)l
//gain factors calibrated for a 610 gain resistor for Z_mux
const float gain_factor_1x = 5.9788E6;  //this could be calculated w/ auto calibration step (freqfunction)
const float gain_factor_intercept_1x = -42.82;
const float gain_factor_5x = 3.0360E6;  //this could be calculated w/ auto calibration step (freqfunction)
const float gain_factor_intercept_5x = -397.23;
const boolean time_comments = false;
const int sense_resolution = 200; // msecs between ad5933 calls
boolean heater_on = true;
const int sleep_hours = 0;
const int sleep_mins = 55; //measurement cycle as is takes 04:05 [m:s] to complete, so 55 mins, 55 secs = 1 start per hour
const int sleep_secs = 55;
boolean CV_direction = true; //true = positive voltage defined as transmit electrodes, false = positive voltage through return electrodes

//deeper user vars
//  uint32_t act_code1 = 0xB0;
//  uint32_t act_code2 = 0x10;
//  uint32_t act_code3 = 0x20;
uint8_t operand = B0001;  //control interrogation voltage and PGA gain; see AD5933 datasheet [0=5x, 1=1x]
const int settling_time = 300;

//freq sweep vars
const int num_increm = 17;  //number of freq increments. if you change this, change phase offset array in freqfunction()
unsigned long start_freq;
const unsigned long delta_freq = 5000;
int sweep_index = 0;  //0 is start frequency, 1 is start + delta frequency, etc
bool sweepflag = 0; //0 is first sweep, 1 is mid-sweep,  controls activating/powering down AD5933
unsigned long AD5933_freq = 16000000; //16 MHz internal oscillator

//global vars
const int dtime = 10;  //delay between I2C operations, in usecs
const int CS = 4;  //pin select for CS
const float pi = 3.14159265;
boolean sweep_on = 0; //is  your measurement a frequency sweep or a single frequency?
const int AD5933ADDR = 0x0D;
uint16_t measure_number = 0;
int heat_voltage; //heater voltage sense
const float V_power = 3300; //[mB], PCB positive supply voltage
const float a_voltage_max = 4095; //12 bit resolution
const int DS1077LADDR = B1011000;

//sensirion LD20 vars
const int LD20_address = 8;
int ret;
float calibration_factor = 0.9; //20(mL/hr)^-1 => 1.2(uL/min)^-1; should be around 1.2, but is diff for every sensor
uint16_t sensor_flow_value;
int16_t signed_flow_value;
float scaled_flow_value;
byte sensor_flow_crc;
uint16_t sensor_temp_value;
byte sensor_temp_crc;
uint16_t aux_value;
byte aux_crc;

//pin assignments
const int ADG804_A0 = 0;
const int ADG804_A1 = 1;
const int ADG804_EN = 2;

const int LED = 3;

const int bubblemux_mod = 7; //mux between 4 signal electrode pairs (LOW) OR bubble input (HIGH)
const int flipflop_mod = 8; //flipflop mux between analog out as ch.1 and heater grond as ch.2 (LOW), or heater ground as ch.1 and analog out as ch.2 (HIGH)

const int CLOCK_CTRL1 = 14;
const int ZMUX_A0 = 15;
const int ZMUX_A1 = 16;
const int ZMUX_EN = 17;

const int heat_sense = 21;
const int heatercntrl = 22;
const int ground = 23;
const int analog_out = A14;

int myregister = 0;
int myregisterval = 0;

int measurement_index;  //tracks number of ad5933 calls

int global_imp_mag;


time_t getTeensy3Time() {
  return Teensy3Clock.get();
}


void setup() {
  Wire.begin();
  Serial.begin(9600);

  //  Serial.println("take3");

  //snooze stuff
  alarm.setRtcTimer(sleep_hours, sleep_mins, sleep_secs);

  //prepare analog output pin
  analogWriteResolution(12);
  analogWrite(analog_out, 0);

  //initialize SD card and make a file
  if (!SD.begin(CS)) {
    Serial.println("Card failed to read");
    return;
  }
  Serial.println("Card initialized");

  setSyncProvider(getTeensy3Time);

  //ground ground pin
  pinMode(ground, OUTPUT);
  digitalWrite(ground, LOW);

  //set unused I/O pins to OUTPUT to save power
  pinMode(0, OUTPUT); pinMode(1, OUTPUT); pinMode(2, OUTPUT); pinMode(5, OUTPUT); pinMode(6, OUTPUT);
  pinMode(10, OUTPUT); pinMode(11, OUTPUT); pinMode(12, OUTPUT); pinMode(13, OUTPUT); //pinMode(17, OUTPUT);

  //  Serial.end(); // shut off USB

  pinMode(LED, OUTPUT);      //turn on LED all the time
  digitalWrite(LED, HIGH);

  // Set all pinModes here for speed - none should change during program

  //talk to mux
  pinMode(ZMUX_A0, OUTPUT);  //corresponds to mux logic A
  pinMode(ZMUX_A1, OUTPUT);  //logic B
  pinMode(ZMUX_EN, OUTPUT);

  pinMode(heatercntrl, OUTPUT);

  pinMode(bubblemux_mod, OUTPUT);
  pinMode(flipflop_mod, OUTPUT);

  pinMode(heat_sense, INPUT);

  pinMode(CLOCK_CTRL1, OUTPUT);

  //default + safe switching mode is: flipflop=unflipped, bubble mux=bubble circuit(to isolate signal lines), R_shunt=low R
  digitalWrite(bubblemux_mod, HIGH);
  digitalWrite(flipflop_mod, LOW);

  digitalWrite(ADG804_EN, HIGH); //default open shunt resistor mux

  digitalWrite(CLOCK_CTRL1, HIGH); //default power down DS1077L

  delay(3000);  //time to write new program
}

void loop() {
  main1();
  measure_number++;

  delay(2000);  //don't remove this or snooze will crash

  Snooze.deepSleep(config1);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
