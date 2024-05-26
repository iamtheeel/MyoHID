/*
 * Joshua Mehlman and James Swartz
 *
 * ENGR 845 Spring 2024
 * Term Project: DAQ
 *
 */

/* Hardware Used
 * Boot Bypass pin12
 * Board Built in LED pin 13
 * Usb = Serial
 */
#include <Filters.h>
#include <Filters/Notch.hpp>

#include "serialComms.h"


/*   Pins  <Arduino UNO> */
// 0 Serial TX
// 1 Serial TX
//const uint8_t = 3; //PWM
//const uint8_t = 4; 
//const uint8_t DB_TX_PIN = 5; //PWM
//const uint8_t DB_RX_PIN = 6; //PWM
//const uint8_t commsBad_pin = 7; 
//const uint8_t commsBood_pin = 8; 
//const uint8_t = 9; //PWM
//const uint8_t = 10; //PWM
//const uint8_t = 11; //PWM
const uint8_t safeMode_pin = 12; // Hold btn on boot to bypass setup
const uint8_t heartBeat_pin = 13; // Built in LED
// 14 Used for A0
// 15 Used for A1
// 16 Used for A2
// 17 Used for A3
// 18 Used for A4
// 19 Used for A5

/* UNO Analog Pins */
const uint8_t CH0 = A0;
const uint8_t CH1 = A1;
const uint8_t CH2 = A2;
const uint8_t CH3 = A3;
const uint8_t CH4 = A4;
const uint8_t CH5 = A5;


/*     Serial comms  */
//SoftwareSerial debugSerial(DB_RX_PIN, DB_TX_PIN); // RX, TX
//debugSerial.begin(115200);     //RS232 for debuging

serialComms daqSerial;
//uint32_t commsSpeed =  115200;
//uint32_t commsSpeed =  2000000;
uint32_t commsSpeed =  2000000; // Max Matlab Rate

/*      Timing       */
// Heartbeat
int runFreq = 300; // Hz
unsigned long delayTime_us = 1e6/runFreq;
bool safeBoot = false;

unsigned long miliSecTaskClock = 0;
unsigned long uSSystemTaskClock = 0;
byte taskClockCycles25Hz = 0, taskClockCycles10Hz = 0, taskClockCycles5Hz, taskClockCycles1Hz = 0;

/*   Filter   https://github.com/tttapa/Arduino-Filters   */
const double f_c = 60; // Hz
const double f_n = 2 * f_c / runFreq; // Normalized notch frequency
auto filter1 = simpleNotchFIR(f_n);     // fundamental
auto filter2 = simpleNotchFIR(2 * f_n); // second harmonic

void setup() {
    // Pin setups
  pinMode(safeMode_pin, INPUT_PULLUP);     // Safe mode
  pinMode(heartBeat_pin, OUTPUT); //LED_BUILTIN

  safeBoot = !digitalRead(safeMode_pin);
  if(safeBoot)
  {
    // Debug when booted in safe mode
    Serial.begin(9600); while (!Serial);
    Serial.println((String)"Safe Mode = " + safeBoot);
  }
  else
  {
    daqSerial.serialPort = &Serial;
    //daqSerial.debugSerial = &debugSerial;
    daqSerial.initializeHardDuplex(commsSpeed, "DAQ");


    pinMode(CH0, INPUT);
    pinMode(CH1, INPUT);
    pinMode(CH2, INPUT);
    pinMode(CH3, INPUT);
    pinMode(CH4, INPUT);
    pinMode(CH5, INPUT);

    analogReference(EXTERNAL);

  }
}

void loop() {
/****************************** Clock Tasks *************************************/
  unsigned long miliSec = millis();
  unsigned long microSec = micros();

  int16_t Ch0_Data, Ch1_Data, Ch2_Data, /* Ch3_Data, Ch4_Data, */Ch5_Data;
  int16_t Ch0_Filtered, Ch1_Filtered, Ch2_Filtered, /* Ch3_Filtered, Ch4_Filtered,*/ Ch5_Filtered;
  float f, g; // for debuging

  /*    Main Dac     */
  if (microSec - uSSystemTaskClock >= delayTime_us) //Main Clock Run at runFreq Hz
  {
    uSSystemTaskClock = microSec;
    /*
     *  Main Daq loop
     *  Set runFreq = 200; // Hz
     *  to the desired run frequency
     */ 
    
    // Settable Freq Taskes 
    Ch0_Data = analogRead(CH0);
    Ch1_Data = analogRead(CH1);
    Ch2_Data = analogRead(CH2);
    //Ch3_Data = analogRead(CH3);
    //Ch4_Data = analogRead(CH4);
    Ch5_Data = analogRead(CH5);
    Ch0_Filtered = filter2( filter1(Ch0_Data));
    Ch1_Filtered = filter2( filter1(Ch1_Data));
    Ch2_Filtered = filter2( filter1(Ch2_Data));
    //Ch3_Filtered = filter2( filter1(Ch3_Data));
    //Ch4_Filtered = filter2( filter1(Ch4_Data));
    Ch5_Filtered = filter2( filter1(Ch5_Data));
    // Ch1_Data = 1000*PI;
    // Ch2_Data = 20*sin(2*PI*20*miliSec/1000);
    // Ch3_Data = 10*sin(2*PI*30*miliSec/1000);
    // Ch4_Data = 10*cos(2*PI*50*miliSec/1000);
    // Ch5_Data= analogRead(A5);
    
    f = sin(2*PI*200*miliSec/1000);
    g = PI;

    //if(!safeBoot){daqSerial.sendMY(miliSec, Ch0_Data, Ch1_Data, Ch2_Data, Ch3_Data, Ch4_Data, Ch5_Data, f, g);}
    //if(!safeBoot){daqSerial.sendMY(miliSec, Ch0_Filtered, Ch1_Filtered);}
    if(!safeBoot){daqSerial.sendMY(miliSec, Ch0_Data, Ch1_Data, Ch2_Data, /* Ch3_Data, Ch4_Data, */Ch5_Data);}

    
  }

  /*     Heartbeat and other such tasks     */
  if(miliSec - miliSecTaskClock >=10)
  {
    miliSecTaskClock = millis();

    // *** 100Hz tasks go here

    if (taskClockCycles1Hz == 100) // 1Hz 
    {
      taskClockCycles1Hz = 0;

      // Log in safe mode
      if(safeBoot) {Serial.println((String)"Safe Mode = " + safeBoot);}
      
    }

    if (taskClockCycles5Hz == 20) // 5Hz 
    {
      taskClockCycles5Hz = 0;

      if(!safeBoot) {heartBeat(heartBeat_pin);}
    }

    if (taskClockCycles10Hz == 10) // 10Hz 
    {
      taskClockCycles10Hz = 0;
    }

    if(taskClockCycles25Hz == 4) // 25Hz
    {
      taskClockCycles25Hz = 0;

      if(safeBoot) {heartBeat(heartBeat_pin);}
    }

    taskClockCycles1Hz++;
    taskClockCycles5Hz++;
    taskClockCycles10Hz++;
    taskClockCycles25Hz++;
  }
}

void heartBeat(int hbPin)
{
  static bool pinState;

  digitalWrite(hbPin, pinState);
  pinState = !pinState;
}
