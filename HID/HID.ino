/*
 * Joshua Mehlman and James Swartz
 *
 * ENGR 845 Spring 2024
 * Term Project:
 *
 */

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, phk@FreeBSD.ORG):
 * <iamtheeel> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Joshua Mehlman and James Swartz
 * ----------------------------------------------------------------------------
 */
// From Arduino
#include <Mouse.h>

// From AdaFruit
// Board is a  QT Py M0 (SAMD21)
#include <Adafruit_NeoPixel.h>

// From Us
#include "serialComms.h"
#include "gestureMap.h"

/***           Pinouts     *******/
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL); // The 3 Color LED
//const uint8_t xx = 0; //Pin 0, OR A0
//const uint8_t xx = 1; //Pin 1, OR A1
//const uint8_t xx = 2; //Pin 2, OR A2
//const uint8_t xx = 3; //Pin 3, OR A3
//const uint8_t xx = 4; //Pin 4
const uint8_t safeMode_pin = 5; //Pin 5
//Using serial 1 //Pin 6, OR A6, or Serial 1 TX
//Using serial 1 //Pin 7, OR A7, or Serial 1 RX
//const uint8_t xx = 8; //Pin 8, OR A8
//const uint8_t xx = 9; //Pin 9, OR A9
//const uint8_t xx = 10; //Pin 10, OR A10

/***    Serial    ***/
#define TERMBAUDRATE (9600)
#define HIDBAUDRATE (921600)
serialComms HIDSerial;
//usb_serial_class debugSerial;


/***      Timing       ***/
// Heartbeat
int runFreq = 250; // Hz
unsigned long delayTime_us = 1e6/runFreq;
unsigned long miliSecTaskClock = 0;
unsigned long uSSystemTaskClock = 0;
byte taskClockCycles25Hz = 0, taskClockCycles10Hz = 0, taskClockCycles5Hz, taskClockCycles1Hz = 0;


/***      HeartBeat/Status       ***/
#define HB_SAFEMODE 0
#define HB_NODATA 1
#define HB_RXDATA 2
#define HB_SETGESTMODE 3
uint8_t hbMode; 

/***      HID        ***/



void setup() {
  pinMode(safeMode_pin, INPUT_PULLUP);     // Safe mode
  hbMode = digitalRead(safeMode_pin);

  pixels.begin();  // initialize the pixel RGBW LED

  // Console Serial Port (USB)
  Serial.begin(TERMBAUDRATE);
  while (!Serial) {;}  // wait for serial port to connect. Needed for native USB port only 

  if(hbMode == HB_SAFEMODE)
  {
    // Safe moode bootup
    Serial.println((String) "Start Up: Safe Mode");
  }
  else
  {
    // Main Setups
    Serial.println((String) "Start Up: Normal Mode");

    // Serial port
    HIDSerial.serialPort = &Serial1;
    HIDSerial.debugSerial =&Serial;
    HIDSerial.initializeHardDuplex(HIDBAUDRATE, "HID");
    // The HID Interface to MatLab
    //Serial1.begin(HIDBAUDRATE);
    Serial.println((String) "Start Up: Mouse");
    Mouse.begin();
  }
} // End Setup

void loop() {
  unsigned long miliSec = millis();

  // Must be static, cuz Arduino does weird things with "loop()"
  static int8_t gesture = 0;
  static uint8_t map = 0;

  if(hbMode != HB_SAFEMODE) {
  while(HIDSerial.parsePacket())
    {
      gesture = HIDSerial.incommingGesture;
      map = HIDSerial.incommingGestureMaping;
      hbMode = HB_RXDATA;

      Serial.println((String) "Incomming Packet (Gesture, Map): " + gesture + ", " + map);
      HIDSerial.setRead();
    }
  }

  if(miliSec - miliSecTaskClock >=10) // 100Hz loop
  {
    miliSecTaskClock = millis();
    // *** 100Hz tasks go here

    if(hbMode != HB_SAFEMODE) {
      if(gesture)
      {
        gesture--; // Gestrues start at 1, array at 0
        //Serial.println((String) "Send to mouse (Gesture, Map): " + gesture + ", " + map);
        if(gestMap[gesture] >= 0)
        { // Command Mode
          if(gestMap[gesture] <= 8)
          {
            int8_t xDir = 0; int8_t yDir = 0;
            switch(gestMap[gesture])
            {
              case mouse_xNon_yNon: xDir =  0; yDir =  0;
                Serial.println((String) "Mouse Stop");
                break;

              case mouse_xPos_yNon: xDir =  1; yDir =  0;
                Serial.println((String) "Mouse Right");
                break;
              case mouse_xNeg_yNon: xDir = -1; yDir =  0;
                Serial.println((String) "Mouse Left");
                break;

                case mouse_xNon_yPos: xDir =  0; yDir =  1; break; // Mouse down
                case mouse_xNon_yNeg: xDir =  0; yDir = -1; break; // Mouse up
                case mouse_xPos_yPos: xDir =  1; yDir =  1; break; // Down/Right
                case mouse_xNeg_yNeg: xDir = -1; yDir = -1; break; // Up/Left
                case mouse_xNeg_yPos: xDir = -1; yDir =  1; break; // down/left
                case mouse_xPos_yNeg: xDir =  1; yDir = -1; break; // up /right
                default: xDir = 0; yDir = 0;  // when in doubt, stop
            }
            moveMouse(xDir, yDir, 0); // Moves mouse X, Y, scroll. relitive to the current position
          } // End mouse
          
          // 9, 10, 13 are TAB, LF, CR
          else if(gestMap[gesture] == mouse_clickL) {Mouse.click(MOUSE_LEFT);}
          else if(gestMap[gesture] == mouse_clickR) {Mouse.click(MOUSE_RIGHT);}
          else if(gestMap[gesture] == mouse_clickM) {Mouse.click(MOUSE_MIDDLE);}
          // add TAB, LF, CR
          else if(gestMap[gesture] >= 32) // Send the ASCII
          {
            // Keyboard press
          }
        }
        else
        {
          // Train Mode
          /*
           * Items to train:
           *    Mouse Command
           *    Mouse speed
           *    Key command
           */
        }
        gesture = 0;
      }

      if (millis() - HIDSerial.lastVariablesReset > 100) {//Reset communication variables if unresponsive
        hbMode = HB_NODATA;
          //debugSerial.println((String) "No comms on HLC, reseting");
        }
    }

    if(taskClockCycles25Hz == 4) // 25Hz
    {
      taskClockCycles25Hz = 0;

      if(hbMode == HB_SAFEMODE) {heartBeat();}
      //else{}
    }
    if (taskClockCycles10Hz == 10) // 10Hz 
    {
      if(hbMode != HB_SAFEMODE) {
        //
      }
      
      taskClockCycles10Hz = 0;
    }
    if (taskClockCycles5Hz == 20) // 5Hz 
    {
      taskClockCycles5Hz = 0;

      if(hbMode != HB_SAFEMODE) {
         heartBeat();
      }
      //else {; }
    }

    if (taskClockCycles1Hz == 100) // 1Hz 
    {
      taskClockCycles1Hz = 0;

      if(hbMode != HB_SAFEMODE) {
      //Serial.println((String) "1 Hz Loop, hbMode: " + hbMode);
      }
    }

    taskClockCycles1Hz++;
    taskClockCycles5Hz++;
    taskClockCycles10Hz++;
    taskClockCycles25Hz++;
  } // safe boot


} // Loop

void moveMouse(int8_t xDir, int8_t yDir, int8_t scroll)
{
  static int nXMoves = 0;
  static int nYMoves = 0;

  //Serial.println((String) "moving mouse (X, Y): " + CMD_MouseSpeedX*xDir + ", " + CMD_MouseSpeedX*yDir);
  Mouse.move(CMD_MouseSpeedX*xDir, CMD_MouseSpeedY*yDir, scroll); // move X pixels, move Y pixels, scroll wheel
}

void heartBeat()
{
  static bool pinState = 0;

  static uint8_t led_r = 0;
  static uint8_t led_g = 0;
  static uint8_t led_b = 0;

  //Serial.println((String) "Heart Beat, pinState: " + pinState);
  switch(hbMode)
  {
    case HB_SAFEMODE: 
      led_r = 100; 
      led_g = 0;
      led_b = 0;
      break;
    case HB_NODATA: 
      led_r = 0;
      led_g = 0;
      led_b = 50; 
      break;
    case HB_RXDATA: 
      led_r = 0;
      led_g = 50; 
      led_b = 0;
      break;
    case HB_SETGESTMODE:
      led_r = 0;
      led_b += 10;
      led_g += 10;
      break;
  }
  if(pinState)
  {
    //Pixel index, starting from 0, 32bit containing(r,g,b)
    pixels.setPixelColor(0, pixels.Color(led_r, led_g, led_b));
  }
  else
  {
    pixels.clear();
  }
  pixels.show();
  pinState = !pinState;
}
