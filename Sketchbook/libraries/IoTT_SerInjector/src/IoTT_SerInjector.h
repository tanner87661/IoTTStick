/*
IoTT_SerInjector.h

Based on SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

Adaptation to LocoNet (half-duplex network with DCMA) by Hans Tanner. 
See Digitrax LocoNet PE documentation for more information

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef IoTT_SerInjector_h
#define IoTT_SerInjector_h

#include <inttypes.h>
#include <IoTT_CommDef.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <IoTT_DigitraxBuffers.h>


// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.

//#define lnBusy 0
//#define lnAwaitBackoff 1
//#define lnNetAvailable 2

#define rxBufferSize 64
#define txBufferSize 64
#define verBufferSize 48

#define genInBufLen 200

#define queInjBufferSize 50 //messages that can be written in one burst before buffer overflow

class IoTT_DigitraxBuffers;

class IoTT_SerInjector : public HardwareSerial
{
public:
	IoTT_SerInjector(int receivePin, int transmitPin, bool inverse_logic = false, int uartNr = 1, unsigned int buffSize = 64);
	~IoTT_SerInjector();
	void begin();
	void processLoop();
	void setProtType(messageType thisType);
	messageType getMsgType();
	uint16_t lnWriteMsg(lnTransmitMsg txData);
	uint16_t lnWriteMsg(lnReceiveBuffer txData);

	void setTxCallback(txFct newCB);
	void setDCCCallback(dccCbFct newDCCCB, uint8_t ptrNr);
	void loadLNCfgJSON(DynamicJsonDocument doc);
	void toggleBoosterOutput();
	void dccToBooster(uint8_t opCode, uint16_t devAddr, uint8_t devData);
private:
   
   // Member functions
	void handleLNIn(uint8_t inData, uint8_t inFlags = 0);
	void processLNMsg(lnTransmitMsg* recData);
	void processDCCEXMsg(std::vector<ppElement> * recData);
	void processLNReceive();
	void processLNTransmit();
	void processLCBReceive();
	void processLCBTransmit();
	void processDCCExReceive();
	void processDCCExTransmit();
	void processBoosterReceive();
	void processBoosterTransmit();
	
   // Member variables
   lnTransmitMsg transmitQueue[queInjBufferSize];
   uint8_t que_rdPos, que_wrPos = 0;
   lnTransmitMsg lnInBuffer;
   std::vector<ppElement> ppList;
   int m_rxPin, m_txPin;
   bool m_invert = true;
   uint8_t m_uart = 1;
   uint8_t m_buffsize = 64;
   uint32_t baudRate;
   bool receiveMode;
   int8_t busyLED = -1;
   bool m_highSpeed = true;
   messageType usedProtocol = LocoNet;
   uint8_t transmitStatus;
   uint32_t transmitTime;
   uint8_t numWrite, numRead;

   bool progTrackActive = false;
   
//   uint8_t    bitRecStatus = 0x00;    	//LocoNet 0: waiting for OpCode; 1: waiting for package data
										//OLCB:	0: await start char 1: await frame type 2: await ID 3: await end char
										//DCC EX Awaiting param nr, 0xFF after completion
	//now use lnInBuffer.reqRecTime instead

	char inBuffer[genInBufLen];
   uint8_t    lnBufferPtr = 0; //index of next msg buffer location to read
   uint8_t    lnXOR = 0;
   uint8_t    lnExpLen = 0;
   
};

extern void sendBlockDetectorCommand(uint16_t bdNr, uint8_t bdStatus);
extern void sendSignalCommand(uint16_t signalNr, uint8_t signalAspect);
extern void sendButtonCommand(uint16_t btnNr, uint8_t  btnEvent);
extern void sendSwitchCommand(uint8_t opCode, uint16_t swiNr, uint8_t swiTargetPos, uint8_t coilStatus);
extern void sendSVCommand2(uint8_t SRC, uint8_t SV_CMD,uint16_t DST, uint16_t ADDR, uint8_t DTA[4]);

#endif
