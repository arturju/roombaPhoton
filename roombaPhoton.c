/*-----------------------------------------------------------------------------
 **                        � Arturo Jumpa
 ** File: roombaPhoton.c
 **
 ** Description:
 ** Connect's particle photon with Roomba Open Interface commands

Summary of Roomba Open Interface Spec:

3 operating modes: Passive, Safe, and Full
  -Passive(128): Sleeps after 5 min inactive. Other modes drain battery
  -Safe(131):    Allows user control but safety condition reverts it to Passive mode
  -Full(132):    Complete complete, ignores cliff, wheel-drop, charger, etc
-------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 ** Includes
 **---------------------------------------------------------------------------*/
#include "roomba.h"

/*-----------------------------------------------------------------------------
 ** Global Defines/Typedefs/Enums/Macros.
 ** 
 **  pin3Rxd -> White wire -> Photon TX pin
 **  pin4Txd -> Green wire -> Photon RX pin
 **  pin5Brc -> Black Wire -> Photon pin D5
 **---------------------------------------------------------------------------*/
din7Connector roombaDin  =  {.pin1Vpwr =0, .pin2Vpwr=0, .pin3Rxd= 6, .pin4Txd= 7, .pin5Brc = D5, .pin6Gnd=0, .pin7Gnd=0 };
roombaObj     artuRoomba =  {.roombaState = ASLEEP, .roombaUpdateFunc = roombaCmdFromSleep};
sensorPacketList sensors = { .chargingState={21,1,false}, .voltage={22,2,false}, .current={23,2,true}, .temperature={24,1,true}, .batteryCharge={25,2,false}, .batteryCapacity={26,2,false}   };

unsigned long int roombaIOtimeout = 240000;   // 4 min (actually less? double check)
unsigned long     roombaTimeMark  = 0;
int 			  ledPin 		  = D7;
int               ledState        = 0;
int               roombaCmd       = 0;
String            inSerial        = "";
const size_t 	  READ_BUF_SIZE   = 64;
size_t 			  readBufOffset	  = 0;
char readBuf[READ_BUF_SIZE];


//---------------------Setup()---------------------------------
void setup() {
	Serial1.begin(19200);		                // for Roomba communication via RX/TX pins
	Serial.begin(19200); 		                // for debugging through terminal
// 	pinMode(roombaDin.pin5Brc, OUTPUT);         // Roomba's BRC pin. Used to wake it up
	pinMode(D5, OUTPUT);
	pinMode(ledPin,OUTPUT);                     // use built in led for feedback
	roombaTimeMark = millis();                  // start timer
	
	// set up API connection
	Spark.function("webCmd",receiveWebCmd);
	Serial.println("Setup complete");
}

//---------------------Spark web cmd()---------------------------------
int receiveWebCmd(String command){
    Serial.print("Command # "); Serial.print(command); Serial.println(" received from web!");
	roombaCmd = command.toInt();	
	return 1;
}


/*****************************************************************************************
 *             todo:  Rreading roomba sensor packets
 * - Bug with Serial1.read(). Sometimes reads more than 8 bits...  
 * 
 * ********************************************************************************************/
//  #define    NO_BYTE_READ 0x100
//  #define    ROOMBA_READ_TIMEOUT    1000     /* in ms */
//  int8_t      signedByteArray[2]     = {0,0};
//  uint8_t     customSensorArray[10]  ={};      // will hold list of values
//  int16_t  current = 0;
// uint16_t voltage = 0;
// uint16_t battCharge = 0;
// uint16_t battCapacity = 0;

// int readByte(int8_t &readByte, int timeout){
//   int count = 0;
//   const uint8_t DELAY = 10;
//   const int MAX_INDEX = 1 + ((timeout -1)/DELAY);

//   for(count = 0; count <= MAX_INDEX; count++)  {
//       if(Serial1.available())      {
//           readByte = Serial1.read();
//           Serial.print("readByte() read: "); Serial.println(readByte, BIN);
//           return  0;
//       }
//       delay(DELAY);
//   }
//   Serial.println("--no byte read --");
//   return NO_BYTE_READ;
// }

// uint16_t getRoombaSensor(sensorPacket oiSensorPacket){              // Needs different routine to read signed integers???
//     uint16_t    finalByte=0;
    
//     Serial1.write(142);
//     Serial1.write(oiSensorPacket.packetID);
//     Serial.print("Requested roomba sensor ID: ");    Serial.println(oiSensorPacket.packetID);
    
        
//     Serial.print("Pre-writing to array: "); Serial.print(signedByteArray[0],BIN); Serial.print(" | "); Serial.println(signedByteArray[1],BIN);
    
//     for(int byteCounter= 0; byteCounter<oiSensorPacket.dataBytes ; byteCounter++){
//     readByte(signedByteArray[byteCounter], 100);
//     Serial.print("Byte #");             Serial.print(byteCounter); 
//     Serial.print(" | BIN = ");          Serial.print( (signedByteArray[byteCounter] ),BIN);
//     Serial.print(" | (int8_t) = ");     Serial.println(signedByteArray[byteCounter]);
//     // todo: clever way to parse two 8bit ints into a 16bit int
//     }
//     if(oiSensorPacket.dataBytes == 1)       { finalByte =           signedByteArray[0];}
//     if(oiSensorPacket.dataBytes == 2)       { finalByte = (uint16_t)signedByteArray[0]<<8 | signedByteArray[1];}
    
//     Serial.print("Final value, BIN: "); Serial.print(finalByte, BIN);  Serial.print("(uint16_t) "); Serial.println(finalByte);
        

//     Serial.print("Post-writing to array: "); Serial.print(signedByteArray[0],BIN); Serial.print(" | "); Serial.println(signedByteArray[1],BIN);  
    
//     return finalByte;
// }

// void customSensorsUpdate(void){
//     Serial1.write(149);
//     Serial1.write(4);                                           // only interested in 4 values.. for now
//     Serial1.write(sensors.voltage.packetID);
//     Serial1.write(sensors.current.packetID);
//     Serial1.write(sensors.batteryCharge.packetID);
//     Serial1.write(sensors.batteryCapacity.packetID);
//     getXsensorPackets(customSensorArray, 9);
//     Serial.print(customSensorArray[0]); Serial.print(" | ");
//     Serial.print(customSensorArray[1]); Serial.print(" | ");
//     Serial.print(customSensorArray[2]); Serial.print(" | ");
//     Serial.print(customSensorArray[3]); Serial.print(" | ");
//     Serial.print(customSensorArray[4]); Serial.print(" | ");
//     Serial.print(customSensorArray[5]); Serial.print(" | ");
//     Serial.print(customSensorArray[6]); Serial.print(" | ");
//     Serial.print(customSensorArray[7]); Serial.print(" | ");
//     Serial.print(customSensorArray[8]); Serial.print(" | ");
//     Serial.print(customSensorArray[9]); Serial.print(" | ");
//     Serial.print(customSensorArray[10]); Serial.println("");
// }

// bool getXsensorPackets(uint8_t* dest, uint8_t len)
// {
//   while (len--  >0)  {
//     unsigned long startTime = millis();
//     while (Serial1.available()){
//       // Look for a timeout
//       if (millis() > startTime + ROOMBA_READ_TIMEOUT)        return false; // Timed out
//     }
//     *dest++ = Serial1.read();
//   }
//   return true;
// }

/*--------------------------------------------------------------------
End of Sensor reading buggy code
----------------------------------------------------------------*/






/*---------------------Main Loop()---------------------------------
- Receive cmds from Serial or API
- Perform cmd and update state
- check timers and read Roomba response
----------------------------------------------------------------*/
void loop() {
	/*---------------------Receive cmds from Serial or API---------------------------------*/
	
	// Commanding from serial monitor. Ascii to decimal
	// examples:  "*" -> 128;     "1" -> 135;       "9" -> 143,      "?" -> 149
	if (Serial.available() > 0 ){
		roombaCmd = Serial.read()+86;                                           
		Serial.print("--Cmd from serial(usb): "); Serial.println(roombaCmd);
		cmdRoomba(roombaCmd);
	}	
	// receiveWebCmd happens asynchronously
	
	/*---------------------Perform cmd and update state---------------------------------*/
	if(roombaCmd != 0){
		Serial.print("--Cmd: "); Serial.println(roombaCmd);
		digitalWrite(ledPin, ledState);
		switch(artuRoomba.roombaState){    
		  case ASLEEP:
			  artuRoomba.roombaUpdateFunc= &roombaCmdFromSleep;
			  break;
		   case CLEANING:
			  artuRoomba.roombaUpdateFunc = &roombaCmdFromClean;
			  break;
		   case DOCKING:
			  artuRoomba.roombaUpdateFunc = &roombaCmdFromSeek;
			  break;
		}
		artuRoomba.roombaUpdateFunc(&artuRoomba.roombaState, roombaDin, roombaCmd);
		roombaTimeMark= millis();           // reset sleep timers
		ledState = !ledState;    			// flip led to signal cmd was received
		roombaCmd = 0; 						// reset received cmd
	}// End if(roombaCmd)  	
	
	/*--------------------Check timers and read Roomba response---------------------------------*/
	if(IsTime(&roombaTimeMark, roombaIOtimeout) && artuRoomba.roombaState != ASLEEP  ) {
		Serial.println("Roomba went to sleep!");  
		artuRoomba.roombaState=ASLEEP;
		ledState = 0;
		digitalWrite(ledPin, ledState);
	}
	
	/*----- todo: implement reading sensor packets to update roomba's state  ---*/
	
	
	// sample serial received msgs that are not sensor packets. 
	while(Serial1.available()) {
		if (readBufOffset < READ_BUF_SIZE) {
			char c = Serial1.read();
			if (c != '\n') readBuf[readBufOffset++] = c;
			else {
				readBuf[readBufOffset] = 0;
				Serial.printlnf("Received from serial: %s", readBuf);
				readBufOffset = 0;
			}
		}
		else {			readBufOffset = 0;		}		// empty buffer; overflow
	}
	
}