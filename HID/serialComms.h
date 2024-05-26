/*
 * Joshua Mehlman and James Swartz
 *
 * ENGR 845 Spring 2024
 * Term Project:
 *
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
    uint32_t portSpeed = 115200; //Default

    bool printDebug = false;
    Serial_ *debugSerial;


    bool transmitting = false;
    bool packetComplete = false;
    String host = "xx";
    byte packetType; 

    int8_t incommingGesture = 0x00;
    uint8_t incommingGestureMaping = 0x00;

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
    bool parsePacket();

    
    /*********** Send  *****************/
    void sendMY(unsigned long timeStampMS, float ch1);
    void sendMY(unsigned long timeStampMS, float ch1, float ch2);
    void sendMY(unsigned long timeStampMS, float ch1, float ch2, float ch3);
    void sendMY(unsigned long timeStampMS, float ch1, float ch2, float ch3, float ch4);
    void sendMY(unsigned long timeStampMS, float ch1, float ch2, float ch3, float ch4, float ch5);
    void sendMY(unsigned long timeStampMS, int16_t ch1, int16_t ch2, int16_t ch3, int16_t ch4, int16_t ch5, int16_t ch6);
    void sendMY(unsigned long timeStampMS, float ch1, float ch2, float ch3, float ch4, float ch5, float ch6, float ch7);
    void sendMY(unsigned long timeStampMS, float ch1, float ch2, float ch3, float ch4, float ch5, float ch6, float ch7, float ch8);



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
    void setRead();
    void resetAndSetRead();
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
    bool commsHit = false;

    bool m_packetDone = false;
    bool messageOK = false;
    byte packetLength = 0;
    byte packetIndex = 0;
    byte packetProgress;

    uint16_t delayTimeUS = 520;

    // The data
    bool m_dataAvailable = false;
    int8_t workingGesture = 0;
    uint8_t workingMap = 0;

};
#endif
