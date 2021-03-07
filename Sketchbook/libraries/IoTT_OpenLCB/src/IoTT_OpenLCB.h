/*
IoTT_OpenLCB.h

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

#ifndef IoTT_OpenLCB_h
#define IoTT_OpenLCB_h

#include <arduino.h>
#include <inttypes.h>
#include <ESP32CAN.h>
#include <gc_format.h>
#include <CAN_config.h>
#include <IoTTCommDef.h>


// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.


#define queBufferSize 50 //messages that can be written in one burst before buffer overflow


class IoTT_OpenLCB
{
public:
   IoTT_OpenLCB(gpio_num_t receivePin, gpio_num_t transmitPin);
   ~IoTT_OpenLCB();
   void begin();
   void processLoop();
   uint16_t lnWriteMsg(lnTransmitMsg txData);
   uint16_t lnWriteMsg(lnReceiveBuffer txData);
   void setOlcbCallback(cbFct newCB, bool useOnOut);
   bool canEnabled();
  
private:
   // Member functions
   void handleLNIn(uint8_t inData, uint8_t inFlags = 0);
   void processLNMsg(lnReceiveBuffer * recData);
   void processLNReceive();
   void processLNTransmit();
   void	updateNodes(lnReceiveBuffer * recData);
   void initOpenLCBAccess();

   // Member variables
   const int rx_queue_size = 10;       // Receive Queue size
   CAN_frame_t rx_frame;
   lnTransmitMsg transmitQueue[queBufferSize];
   uint8_t que_rdPos, que_wrPos;
   lnReceiveBuffer lnInBuffer, lnEchoBuffer;
   gpio_num_t m_rxPin, m_txPin;
   uint8_t initStatus = 0;
   bool isConnected = false;

   uint8_t    bitRecStatus = 0;    //0: waiting for OpCode; 1: waiting for package data
   uint8_t    lnBufferPtr = 0; //index of next msg buffer location to read
   uint8_t    lnXOR = 0;
   uint8_t    lnExpLen = 0;

   uint8_t numWrite, numRead;
   
   uint32_t respTime;
   uint8_t  respOpCode;
   uint16_t respID;
   
   bool useAlways = false;
   

};

//this is the callback function. Provide a function of this name and parameter in your application and it will be called when a new message is received
//extern void onOpenLCBMessage(lnReceiveBuffer * recData) __attribute__ ((weak));

#endif
