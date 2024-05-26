/*
 * Joshua Mehlman and James Swartz
 *
 * ENGR 845 Spring 2024
 * Term Project:
 *
 */

/*
 * Comms to and fro motor pods and other assembiliges
 */

#include "serialComms.h"


void serialComms::initializeMasterSlave(byte enablePin, byte goodLedPin, byte badLedPin)
{
    initLeds(goodLedPin, badLedPin);
    initializeMasterSlave(enablePin);
}

void serialComms::initializeMasterSlave(byte enablePin)
{
    txEnablePin = enablePin;
    pinMode(txEnablePin,OUTPUT);
    digitalWrite(txEnablePin,LOW);

    serialPort->begin(portSpeed);
    varReset();
}
#if !defined(TEENSYDUINO)
void serialComms::initializeSoftDuplex(byte goodLedPin, byte badLedPin)
{
    duplex = true;
    hardSerial = false;
    initLeds(goodLedPin, badLedPin);

    //softSerialPort->begin(portSpeed);
    varReset();
}
#endif

void serialComms::initializeHardDuplex(uint32_t portSpeed, String host)
{
  useLED = false;
  duplex = true;
  debugSerial->println((String) "Init Serial: " + portSpeed + ", " + host);
  serialPort->begin(portSpeed);
  varReset();
}

void serialComms::initializeHardDuplex(byte goodLedPin, byte badLedPin)
{
    duplex = true;
    initLeds(goodLedPin, badLedPin);

    serialPort->begin(portSpeed);
    varReset();
}

void serialComms::initLeds(byte goodLedPin, byte badLedPin)
{
    rxStatus[0] = goodLedPin;
    rxStatus[1] = badLedPin;
    pinMode(rxStatus[0],OUTPUT);
    pinMode(rxStatus[1],OUTPUT);
    digitalWrite(rxStatus[0],LOW);
    digitalWrite(rxStatus[1],HIGH);
}

void serialComms::setRead()
{
  packetType = 0x00;
  m_dataAvailable = false;
  lastVariablesReset = millis();
}

void serialComms::varReset()
{
  packetProgress = 0;
  packetLength = 0;
  packetType = 0;
  packetIndex = 0;
  packetComplete = false;
}

void serialComms::zeroData(uint8_t errorBit)
{
    for(uint8_t i = 0; i < packetLen; i++)
    {
        if(i != errorBit+1){packet[i] = 0;}
    }
}

bool serialComms::parsePacket()
{

    if(serialPort->available())
    {
      //if(printDebug)debugSerial->println((String) "Packet Available " );
      m_packetDone = parse();
    }
    else
    {
        //if(printDebug)debugSerial->println((String) "no port available " );
        //digitalWrite(rxStatus[0], LOW);
        m_packetDone = false;
    }
    if(m_packetDone)
    {
        //if(printDebug)debugSerial->println((String) "packet done " );
        // blink the comms light every other packet
        if(messageOK)
        {
            //digitalWrite(rxStatus[0], commsHit);
        }
        //else // packet is never done on a bad message
        //{
            //digitalWrite(rxStatus[1], commsHit);
        //}
        commsHit = !commsHit;
    }

    return m_packetDone;
}

bool serialComms::parse()
{

  byte incomingByte = serialPort->read();

  if (packetProgress == 0) {
    messageOK = true; // don't error here, lets loop through to the next message
    if (incomingByte == SYNCBYTE) {
      packetProgress = 1;  //Sync byte 1 found
      messageOK = true;
      if(printDebug) debugSerial->println("*** SYNCBYTE ***");
    }
    else
    {
      //if(printDebug) debugSerial->println((String)"Non SYNCBYTE " + incomingByte );
    }
  }
  else if (packetProgress == 1) {
    workingGesture = incomingByte;
    if(printDebug)debugSerial->println((String) "serialComms:parse Gesture: " + workingGesture );

    messageOK = true;
    if (workingGesture >= 0){
      // this is our command
      packetProgress = 3;
    }
    else{
      // We are in set mode
      // Got one more packet to grab
      packetProgress = 2; // We are expecting one more byte
    }
  }
  else if (packetProgress == 2) {
    workingMap = incomingByte;
    if(printDebug)debugSerial->println((String) "serialComms:parse Map: " + workingMap );
    packetProgress = 3;
  }
  else if (packetProgress == 3) {
    // incomingByte is what to map our gesture to
    packetComplete = true;
    messageOK = true;
    commsLost = false;
    lastCommsSuccess = millis();

    packetProgress = 0; // Good, bad, or ugly, we still gotta start over

  }

  if(packetComplete || !messageOK)
  {
    packetComplete = false; // Set up for the next run

    if(!messageOK)
    { 
        //if(printDebug)debugSerial->println((String) "Bad message, packetProgress = " + packetProgress);
        //if(rxStatus[1] != 255){ digitalWrite(rxStatus[1], true);}
        // light up the bad light
    }
    else
    {
        //if(rxStatus[1] != 255){ digitalWrite(rxStatus[1], false);}
        //packetType = working_packetType;
        m_dataAvailable = true;
        incommingGesture = workingGesture;
        incommingGestureMaping = workingMap;
        if(printDebug)debugSerial->println((String) "serialComms:parse (Gesture, Map): " + incommingGesture + ", " + incommingGestureMaping);
    }
  } // End packetComplete

  return m_dataAvailable;
} // end serialComms::parse()
    




/******************** Sends *******************************/

// Copy from DAQ