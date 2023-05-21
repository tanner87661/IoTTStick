/*
LocoNetESPSerial.h

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

#ifndef LocoNetESP32_h
#define LocoNetESP32_h

#include <inttypes.h>
#include <Wire.h>
#include <IoTT_CommDef.h>
#include <IoTT_LocoNetHybrid.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>


// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.

#define lnBusy 0
#define lnAwaitBackoff 1
#define lnNetAvailable 2

#define rxBufferSize 64
#define txBufferSize 64
#define verBufferSize 48

#define queBufferSize 50 //messages that can be written in one burst before buffer overflow
//#define queReplyBufferSize 5 //messages to that queue get sent high priority

extern void callbackLocoNetMessage(lnReceiveBuffer * newData);
extern void callbackLocoNetMessageUpstream(lnReceiveBuffer * newData); //this is the landing point for incoming LocoNet messages from LN Subnet

class LocoNetESPSerial : public HardwareSerial
{
public:
	LocoNetESPSerial(int uartNr = 2);
	~LocoNetESPSerial();
	void begin(int receivePin, int transmitPin, bool inverse_logicRx, bool inverse_logicTx);
	void begin();
	void setNetworkType(nodeType newNwType);
	void processLoop();
	void setBusyLED(int8_t ledNr, bool logLevel = true);
	uint16_t lnWriteMsg(lnTransmitMsg* txData);
	uint16_t lnWriteMsg(lnReceiveBuffer* txData);
	uint16_t lnWriteReply(lnTransmitMsg* txData);
	void setUpstreamMode(bool newMode);
//	void setLNCallback(cbFct newCB);
//	int cdBackoff();
	bool carrierOK();
//	bool hasMsgSpace();
	void loadLNCfgJSON(DynamicJsonDocument doc);
	void sendLineBreak(uint16_t breakBits);
   
private:
   
   // Member functions
   void handleLNIn(uint8_t inData, uint8_t inFlags = 0);
   void processLNMsg(lnReceiveBuffer * recData);
   void processLNReceive();
   void processLNTransmit();
   void processLoopBack();
	void execLNCallback(lnReceiveBuffer * recData);

//   void sendBreakSequence();
   uint8_t getXORCheck(uint8_t * msgData, uint8_t * msgLen);

   
   // Member variables
   lnTransmitMsg transmitQueue[queBufferSize];
//   lnTransmitMsg replyQueue[queReplyBufferSize];
   uint8_t que_rdPos = 0, que_wrPos = 0;
   uint8_t que_lastPos = 0; //used to prevent sending multiple time while echo not received
//   uint8_t que_replyRdPos = 0, que_replyWrPos = 0;
   lnReceiveBuffer lnInBuffer, lnEchoBuffer;
   int m_rxPin, m_txPin;
   bool m_invertRx = true;
   bool m_invertTx = true;
   uint8_t m_uart = 2;
   uint8_t m_buffsize = 64;
   bool receiveMode = true;
   bool loopbackMode = false;
   uint8_t busyLED;
   uint8_t m_bitTime = 60;
   bool m_highSpeed = true;
   uint32_t m_StartCD;
//   uint16_t rx_buffer[rxBufferSize];
//   uint8_t tx_buffer[txBufferSize];

	bool upStreamMode = false; 
	
   uint8_t    bitRecStatus = 0;    //0: waiting for OpCode; 1: waiting for package data
   uint8_t    lnBufferPtr = 0; //index of next msg buffer location to read
   uint8_t    lnExpLen = 0;

   uint8_t transmitStatus;
   uint32_t transmitTime = micros() + 1000;
   uint8_t numWrite = 0, numRead = 0;
   
   uint32_t respTime; //store here the micros for each message that goes out
   uint8_t  respOpCode; //store OpCode of every message that is sent so we remember if it has a reply
   uint16_t respID; //store here the reqID of each message that goes out

};

//this is the callback function. Provide a function of this name and parameter in your application and it will be called when a new message is received
//extern void onLocoNetMessage(lnReceiveBuffer * recData) __attribute__ ((weak));

#endif
