/*
 * Binary Serial Communications
 * Joshua Mehlman
 */

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, phk@FreeBSD.ORG):
 * <iamtheeel> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.      -- Theeel
 * ----------------------------------------------------------------------------
 */

#ifndef SERIAL_COMMS
#define SERIAL_COMMS

//#include <SoftwareSerial.h> // do not use for RX
#include <Arduino.h>

#define SYNCBYTE  0xAA


/*** packet types ***/
#define MYO_DATA_MSG 0x01


// 0 -> Length, 1-> packetType, 2 to end -> data (largest expected)
#define packetLen 82

#define MATLAB_TERMINATOR_A 0x0D// CR
#define MATLAB_TERMINATOR_B 0x0A// LF

class serialComms
{
    public:
    HardwareSerial *serialPort;
    uint32_t portSpeed = 115200;

    bool printDebug = false;

    HardwareSerial *debugSerial;


    bool transmitting = false;
    bool packetComplete = false;
    String host = "xx";
    byte packetType; 

    byte packet[packetLen]; 
    
    void initializeMasterSlave(byte enablePin, byte goodLedPin, byte badLedPin);
    void initializeMasterSlave(byte enablePin);
#if !defined(TEENSYDUINO)
    void initializeSoftDuplex(byte goodLedPin, byte badLedPin);
#endif
    void initializeHardDuplex(uint32_t portSpeed, String host);
    void initializeHardDuplex(byte goodLedPin, byte badLedPin);
    void initLeds(byte goodLedPin, byte badLedPin);

    bool parse();//int &progress, byte &packetLength, int &type, int &index, byte hlcPacket[]);

    
    /*********** Send  *****************/
    void sendMY(unsigned long timeStampMS, int16_t ch1);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6, int16_t ch7);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6, int16_t ch7, int16_t ch8);



    /*********** Data Parsing *****************/
    uint8_t  getUint8(uint8_t  packetPos){return(packet[packetPos]);};
    int8_t   getInt8(uint8_t   packetPos){return(packet[packetPos]);};
    uint16_t getUint16(uint8_t packetPos){return(packet[packetPos+1] << 8 | packet[packetPos]);};
    int16_t  getInt16(uint8_t  packetPos){return(packet[packetPos+1] << 8 | packet[packetPos]);};

    void zeroData(uint8_t errorBit);
// For e-thrusters and e-jets
//                               int16_t battATemp_degCx100, int16_t battBTemp_degCx100, int16_t battCTemp_degCx100,
//                               int16_t portMCTemp_degCx100, int16_t stbdMCTemp_degCx100);

    unsigned long lastVariablesReset, lastCommsSuccess;
    bool commsLost;
    void varReset();
    void setDelayUS(uint16_t newDelayUS){delayTimeUS = newDelayUS;};

    private:
    uint8_t writeByte(byte datum);
    uint8_t writeBytes(uint16_t datum); 
    uint8_t writeBytes(int16_t datum);
    float writeData(float datum);
    float writeData(unsigned long datum);

    void flushPort();
    uint8_t startPacket(byte pkt_size, byte pkt_name);
    void endPacket(float chk_sum);

    bool duplex = false;
    bool hardSerial = true;
    bool useLED = true;
    byte txEnablePin, rxStatus[2];
    byte packetLength = 0;
    byte packetIndex = 0;
    byte packetProgress;

    uint16_t delayTimeUS = 520;
};
#endif
