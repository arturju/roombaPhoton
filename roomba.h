#ifndef ROOMBA_H
#define ROOMBA_H

/* Code for using roomba in passive mode with a Particle Photon
 uses Serial1 as the serial communication method\
 */

#define ASLEEP    1
#define CLEANING  2
#define DOCKING   3
/*-----------------------------------------------------------------------------
 ** Global Defines/Typedefs/Enums/Macros.
 **---------------------------------------------------------------------------*/
#define RESET_OI  7
#define STOP_OI   173


#define START_OI  128
#define BAUD      129
#define SAFE_MODE 131
#define FULL_MODE 132

#define CLEAN     135
#define SEEK_DOCK 143

typedef struct oiPacket{
    int    packetID;
    int    dataBytes;
    bool   signedInt;
}sensorPacket;

typedef struct sensorPackets {
    sensorPacket    chargingState;      // 21, 1 , unsigned
    sensorPacket    voltage;            // 22, 2, unsigned
    sensorPacket    current;            // 23, 2, signed
    sensorPacket    temperature;        // 24, 1, signed
    sensorPacket    batteryCharge;      // 25, 2, unsigned
    sensorPacket    batteryCapacity;    // 26, 2, unsigned
    
}sensorPacketList;






//  typedef struct opCodeList{
//      int startOI    = 128;
//      int baud       = 129;
//      int safeMode   = 131;
//      int fullMode   = 132;
//      int spotBtn    = 134;
//      int cleanBtn   = 135;
//      int dockBtn    = 143;
//      int stream     = 148;
//      int quearyList = 150;
//      int song       = 140;
//      int doStream   = 142;
//      int shcedule   = 167;
//      int stopOI     = 173;
//      int resetOI    = 7;
//  }opCode;


typedef struct miniDin7 {
    int pin1Vpwr;
    int pin2Vpwr;
    int pin3Rxd;
    int pin4Txd;
    int pin5Brc;
    int pin6Gnd;
    int pin7Gnd;
}din7Connector;

typedef struct roombaInfo {
    int roombaState;
    void (*roombaUpdateFunc)(int* , din7Connector, int);
}roombaObj;

/*-----------------------------------------------------------------------------
 ** Function: IsTime()
 **
 ** Arguments:
 ** *timeMark   : pointer to value of timeMark variable. Gets compared with millis()
 ** timeInterval: time that must elapse before function returns true
 **---------------------------------------------------------------------------*/
boolean IsTime(unsigned long *timeMark, unsigned long timeInterval) {
    if (millis() - *timeMark >= timeInterval) {
        *timeMark = millis();
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------------
 ** Functions:
 ** wake up and general cmd functions including wait-time
 **---------------------------------------------------------------------------*/
void wakeUp (din7Connector miniDin)
{
    digitalWrite(miniDin.pin5Brc, HIGH);    // line must start high
    delay(100);
    digitalWrite(miniDin.pin5Brc, LOW);    // 500ms LOW to wake up
    delay(500);
    digitalWrite(miniDin.pin5Brc, HIGH);
    delay(1000);
}


void cmdRoomba(int command){
    Serial1.write(command);
    delay(1000);
}

/*-----------------------------------------------------------------------------
 ** Function:
 ** Updates roomba states if currently in sleep mode
 **---------------------------------------------------------------------------*/
void roombaCmdFromSleep(int* roombaState, din7Connector roombaDin, int cmdOpCode){
    Serial.println("Waking up roomba...Starting IO");
    wakeUp(roombaDin);
    cmdRoomba(START_OI);
    if(cmdOpCode == CLEAN)        *roombaState = CLEANING;
    if(cmdOpCode == SEEK_DOCK)    *roombaState = DOCKING;
    cmdRoomba(cmdOpCode);
}

/*-----------------------------------------------------------------------------
 ** Function:
 ** Updates roomba states if currently awake and Cleaning
 **---------------------------------------------------------------------------*/
void roombaCmdFromClean(int* roombaState, din7Connector roombaDin, int cmdOpCode){
    Serial.println("Received cmd while cleaning...");
    if(cmdOpCode == CLEAN)        Serial.println("Already cleaning...");
    if(cmdOpCode == SEEK_DOCK)    {
        Serial.println("Stopping cleaning to dock...");
        cmdRoomba(CLEAN);            // stops cleaning
        cmdRoomba(cmdOpCode);
        *roombaState = DOCKING;
    };
}

/*-----------------------------------------------------------------------------
 ** Function:
 ** Updates roomba states if currently Seeking Dock
 **---------------------------------------------------------------------------*/
void roombaCmdFromSeek(int* roombaState, din7Connector roombaDin, int cmdOpCode){
    Serial.println("Received cmd while docking...");
    if(cmdOpCode == SEEK_DOCK)        Serial.println("Already seeking dock...");
    if(cmdOpCode == CLEAN)            {
        Serial.println("Stopping docking to clean...");
        cmdRoomba(CLEAN);            // stops seeking dock
        cmdRoomba(cmdOpCode);
        *roombaState = CLEANING;
    };
}


/*-----------------------------------------------------------------------------
 ** Serial sequence: [148] [Number of packets] [Packet ID 1] [Packet ID 2] [Packet ID 3] etc
 ** The format of the data returned is:
 ** [19][N-bytes][Packet ID 1][Packet 1 data…][Packet ID 2][Packet 2 data…][Checksum]
 ** [148] [2] [29] [13]
 ** Packet ID 21: Charging
 **---------------------------------------------------------------------------*/
void askSensorData(int packetID){
    
    cmdRoomba(148);
    cmdRoomba(1);
    cmdRoomba(packetID);
}



#endif ROOMBA_H
