/*
 * Binary Serial Communications
 * Joshua Mehlman
 */

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, phk@FreeBSD.ORG):
 * <iamtheeel> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   -- Theeel
 * ----------------------------------------------------------------------------
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

void serialComms::varReset()
{
  lastVariablesReset = millis();
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

/*************************** get RX from HID ****************************/



/*************************** Send Tools ****************************/

uint8_t serialComms::writeByte(byte datum)
{
    if(hardSerial) { serialPort->write(datum); }
#if !defined(TEENSYDUINO)
 //   else           { softSerialPort->write(datum); }
#endif
    return datum; // for the checksum
}
float serialComms::writeData(float datum)
{
  union
  {
    float a;
    unsigned char bytes[4];
  } byteUnion;
  byteUnion.a = datum;

  if(hardSerial) { serialPort->write(byteUnion.bytes, 4); }

  // loop throught the float to get the checksum
  return datum; // need to make the checksum work
}

float serialComms::writeData(unsigned long datum)
{
  union
  {
    unsigned long a;
    unsigned char bytes[4];
  } byteUnion;
  byteUnion.a = datum;

  if(hardSerial) { serialPort->write(byteUnion.bytes, 4); }

  // loop throught the float to get the checksum
  return (float)datum; // need to make the checksum work
}

uint8_t serialComms::writeBytes(uint16_t datum)
{
    uint8_t checksum = 0;
    checksum += writeByte(lowByte(datum)); 
    checksum += writeByte(highByte(datum));
    return checksum;
}

uint8_t serialComms::writeBytes(int16_t datum)
{
    uint8_t checksum = 0;
    checksum += writeByte(lowByte(datum)); 
    checksum += writeByte(highByte(datum));
    return checksum;
}


void serialComms::flushPort()
{
    if(hardSerial) { serialPort->flush();}
#if !defined(TEENSYDUINO)
 //   else           { softSerialPort->flush();}
#endif
}

uint8_t serialComms::startPacket(byte pkt_size, byte pkt_name)
{
    // note on delay. If the delay is 0 we present packets that don't exist.
    if(!duplex)
    {
        transmitting = true;
        digitalWrite(txEnablePin,HIGH);
        delayMicroseconds(delayTimeUS);
    }

    writeByte(SYNCBYTE);

    return pkt_size + pkt_name;
}

void serialComms::endPacket(float chk_sum)
{
    //writeData(chk_sum);  // matlab has 'CRLF' to terminate the string
    #ifdef MATLAB_TERMINATOR_A
    writeByte(MATLAB_TERMINATOR_A);
    #endif
    #ifdef MATLAB_TERMINATOR_B
    writeByte(MATLAB_TERMINATOR_B);
    #endif

    flushPort();

    if(!duplex)
    {
        delayMicroseconds(delayTimeUS);
        digitalWrite(txEnablePin,LOW);
        transmitting = false;
    }
}


/******************** Sends *******************************/
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1)
{
    const byte packetSize = 1; // How many Float16 ch of data, not including the timestamp
    const byte pktType = MYO_DATA_MSG;

    float checksum = packetSize + pktType;
    startPacket(packetSize, pktType);
     writeData(timeStampMS); 
    checksum += writeBytes(ch1);
    endPacket(checksum);
}

void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2){
  const byte packetSize = 2; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  endPacket(checksum);
}
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3) {
  const byte packetSize = 3; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  checksum += writeBytes(ch3);
  endPacket(checksum);
}
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4){
  const byte packetSize = 4; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  checksum += writeBytes(ch3);
  checksum += writeBytes(ch4);
  endPacket(checksum);
}
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5){
  const byte packetSize = 5; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  checksum += writeBytes(ch3);
  checksum += writeBytes(ch4);
  checksum += writeBytes(ch5);
  endPacket(checksum);
}
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6){
  const byte packetSize = 6; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  checksum += writeBytes(ch3);
  checksum += writeBytes(ch4);
  checksum += writeBytes(ch5);
  checksum += writeBytes(ch6);
  endPacket(checksum);
}
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6, int16_t ch7){
  const byte packetSize = 7; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  checksum += writeBytes(ch3);
  checksum += writeBytes(ch4);
  checksum += writeBytes(ch5);
  checksum += writeBytes(ch6);
  checksum += writeBytes(ch7);
  endPacket(checksum);
}
void serialComms::sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6, int16_t ch7, int16_t ch8){
  const byte packetSize = 8; // How many Float16 ch of data, not including the timestamp
  const byte pktType = MYO_DATA_MSG;

  float checksum = packetSize + pktType;
  startPacket(packetSize, pktType);
  writeData(timeStampMS); 
  checksum += writeBytes(ch1);
  checksum += writeBytes(ch2);
  checksum += writeBytes(ch3);
  checksum += writeBytes(ch4);
  checksum += writeBytes(ch5);
  checksum += writeBytes(ch6);
  checksum += writeBytes(ch7);
  checksum += writeBytes(ch8);
  endPacket(checksum);
}
