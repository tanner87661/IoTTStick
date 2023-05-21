/*

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
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

#include <IoTT_LocoNetHBESP32.h>
#include <HardwareSerial.h>

#define cdBackOffDelay 20  //20 bit-time Backoff Delay per LocoNet Standard

//cbFct lnCallback = NULL;

//uint8_t opcBusy[] = {0x81, 0x7E};

LocoNetESPSerial::LocoNetESPSerial(int uartNr) : HardwareSerial(uartNr) 
{
	m_uart = uartNr;
}

LocoNetESPSerial::~LocoNetESPSerial() 
{
//	HardwareSerial::~HardwareSerial();
}

/*
bool LocoNetESPSerial::hasMsgSpace()
{
	uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    return (hlpQuePtr != que_rdPos);
}
*/

void LocoNetESPSerial::sendLineBreak(uint16_t breakBits)
{
	hybrid_LineBreak(breakBits);
}

void LocoNetESPSerial::begin(int receivePin, int transmitPin, bool inverse_logicRx, bool inverse_logicTx) 
{
	m_invertRx = inverse_logicRx;
	m_invertTx = inverse_logicTx;
	m_rxPin = receivePin;
	m_txPin = transmitPin;
	m_StartCD = micros();
	que_rdPos = que_wrPos = 0;
	receiveMode = true;
	loopbackMode = false;
	transmitStatus = 0;
	HardwareSerial::begin(16667, SERIAL_8N1, m_rxPin, -1, m_invertRx);
	m_highSpeed = true;
	m_bitTime = 60; //round(1000000 / 16667); //60 uSecs
	pinMode(m_rxPin, INPUT_PULLUP); //needed to set this when using Software Serial. Seems to work here
	hybrid_begin(m_rxPin, m_txPin, m_invertRx, m_invertTx);
	HardwareSerial::flush();
//	setUartPort(this);
}

void LocoNetESPSerial::begin()
{
	receiveMode = true;
	loopbackMode = true;
}

void LocoNetESPSerial::setNetworkType(nodeType newNwType)
{
	hybrid_setNetworkType(newNwType); //activate insertion of OPC_BUSY
}

void LocoNetESPSerial::setUpstreamMode(bool newMode)
{
	upStreamMode = newMode; //select callback message
}

void LocoNetESPSerial::loadLNCfgJSON(DynamicJsonDocument doc)
{
	if (doc.containsKey("pinRx"))
		m_rxPin = doc["pinRx"];
	if (doc.containsKey("pinTx"))
		m_txPin = doc["pinTx"];
	if (doc.containsKey("invLogic"))
		m_invertRx = doc["invLogicRx"];
	if (doc.containsKey("invLogic"))
		m_invertTx = doc["invLogicTx"];
	begin(m_rxPin, m_txPin,m_invertRx, m_invertTx);
}

uint16_t LocoNetESPSerial::lnWriteMsg(lnTransmitMsg* txData)
{
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
//	Serial.printf("Serial lnWriteMsg tx %i Rd %i Wr %i Hlp %i\n", txData->lnMsgSize, que_rdPos, que_wrPos, hlpQuePtr);
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].lnMsgSize = txData->lnMsgSize;
		transmitQueue[hlpQuePtr].requestID = txData->requestID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData->lnData, txData->lnMsgSize);
		if ((!hybrid_getBusyMode()) || (txData->lnData[0] != 0x81)) //do not insert busy commands from outside if in busy mode
			que_wrPos = hlpQuePtr;
		
//		Serial.printf("LN TX %2X", txData->lnData[0]);
//		for (int i = 1; i < txData->lnMsgSize; i++)
//			Serial.printf(", %2X", txData->lnData[i]);
//		Serial.println();
//		hybrid_setBusyMode(false);
		return txData->lnMsgSize;
	}
	else
	{	
		Serial.println("LocoNet HB Write Error. Too many messages in queue");
		return 0;
	}
}

uint16_t LocoNetESPSerial::lnWriteMsg(lnReceiveBuffer* txData)
{
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
//	Serial.printf("Serial lnWriteMsg rx %i Rd %i Wr %i Hlp %i\n", txData->lnMsgSize, que_rdPos, que_wrPos, hlpQuePtr);
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].lnMsgSize = txData->lnMsgSize;
		transmitQueue[hlpQuePtr].requestID = txData->requestID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData->lnData, txData->lnMsgSize);
		if ((!hybrid_getBusyMode()) || (txData->lnData[0] != 0x81)) //do not insert busy commands from outside if in busy mode
			que_wrPos = hlpQuePtr;
//		Serial.printf("LN RX %2X", txData->lnData[0]);
//		for (int i = 1; i < txData.lnMsgSize; i++)
//			Serial.printf(", %2X", txData.lnData[i]);
//		Serial.println();
//		hybrid_setBusyMode(false);
		return txData->lnMsgSize;
	}
	else
	{	
		Serial.println("LocoNet HB Lib Write Error. Too many messages in queue");
		return 0;
	}
}

uint16_t LocoNetESPSerial::lnWriteReply(lnTransmitMsg* txData)
{
	return lnWriteMsg(txData);
	
//if needed, this would be used to add a high priority queue that would be sent out first
/*	
//	Serial.println("Serial lnWriteMsg");
    uint8_t hlpQuePtr = (que_replyWrPos + 1) % queReplyBufferSize;
    if (hlpQuePtr != que_replyRdPos) //override protection
    {
		replyQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		replyQueue[hlpQuePtr].reqID = txData.reqID;
		replyQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(replyQueue[hlpQuePtr].lnData, txData.lnData, txData.lnMsgSize);
		que_replyWrPos = hlpQuePtr;
//		Serial.println();
//		Serial.printf("LN Tx %2X", txData.lnData[0]);
//		for (int i = 1; i < txData.lnMsgSize; i++)
//			Serial.printf(", %2X", txData.lnData[i]);
//		Serial.println();
		return txData.lnMsgSize;
	}
	else
	{	
		Serial.println("LocoNet Write Error. Too many messages in reply queue");
		return -1;
	}
*/
}

void LocoNetESPSerial::execLNCallback(lnReceiveBuffer * recData)
{
	if (upStreamMode)
		callbackLocoNetMessageUpstream(recData);
	else
		callbackLocoNetMessage(recData);
}

void LocoNetESPSerial::processLNMsg(lnReceiveBuffer * recData)
{
//	Serial.println();
//	Serial.printf("LN Rx %2X %i\n", recData->lnData[0], recData->errorFlags);
//	if (lnCallback != NULL)
//		lnCallback(recData);
	execLNCallback(recData);
}

void LocoNetESPSerial::handleLNIn(uint8_t inData, uint8_t inFlags) //called for stuff that comes in through the HW uart
{
/*	this no longer exists, the HW uart does not give any error statuses
//	Serial.printf("Handle data %i Flags %i\n", inData, inFlags);
  if ((inFlags & (errorCollision | errorTimeout | errorFrame | errorCarrierLoss)) > 0)  //this will only come from transmit
  {
	lnInBuffer.errorFlags = inFlags | msgIncomplete | msgXORCheck; 
	if ((inFlags & msgEcho) > 0)
	  lnInBuffer.reqRespTime = micros() - lnInBuffer.reqRecTime;
    lnInBuffer.lnData[lnBufferPtr] = inData;
	lnInBuffer.reqRespTime = 0;
	lnInBuffer.lnMsgSize = lnBufferPtr+1;
	lnBufferPtr = 0;
	lnInBuffer.echoTime = 0;
	processLNMsg(&lnInBuffer);
    bitRecStatus = 0; //awaiting OpCode
	return;  
  }
*/ 
	
//  if (inData >= 0x80) Serial.println();
//  Serial.printf("%02X ", inData);

  if (inData >= 0x80) //OpCode, start of new message
  {
    lnInBuffer.echoTime = 0;
	lnInBuffer.reqRespTime = 0;
	lnInBuffer.reqRecTime = 0;
	lnInBuffer.requestID = 0;
    if (bitRecStatus == 1) //awaiting data bytes but received OpCode
    {
      //incomplete message
 	  lnInBuffer.lnMsgSize = lnBufferPtr;
      lnInBuffer.errorFlags |= msgIncomplete;
      if (getXORCheck(&lnInBuffer.lnData[0], &lnInBuffer.lnMsgSize) != 0xFF)
		lnInBuffer.errorFlags |= msgXORCheck;
      processLNMsg(&lnInBuffer); //get rid of previous message
      lnInBuffer.lnMsgSize = 0;
 	  lnBufferPtr = 0;
    }
    bitRecStatus = 1; //await data bytes
    lnBufferPtr = 0;
    lnInBuffer.errorFlags = 0;
    uint8_t swiByte = (inData & 0x60) >> 5;
    switch (swiByte)
    {
      case 0: lnExpLen  = 2; break;
      case 1: lnExpLen  = 4; break;
      case 2: lnExpLen  = 6; break;
      case 3: lnExpLen  = 0xFF; break;
      default: lnExpLen = 0;
    }
    lnInBuffer.lnData[lnBufferPtr] = inData;
    lnBufferPtr++; 
  }
  else //received regular data byte
  {
//    lnInBuffer.errorFlags |= inFlags;
    if (bitRecStatus == 1) //collecting data
    {
      lnInBuffer.lnData[lnBufferPtr] = inData;
      if ((lnBufferPtr == 1) && (lnExpLen == 0xFF))
        lnExpLen  = (inData & 0x007F); //updating expected length for long message
      lnBufferPtr++; 
      if (lnBufferPtr == lnExpLen) //message length received
      {
//		Serial.println("as exp");
		lnInBuffer.lnMsgSize = lnBufferPtr;  
        if (getXORCheck(&lnInBuffer.lnData[0], &lnInBuffer.lnMsgSize) != 0xFF)
	  	  lnInBuffer.errorFlags |= msgXORCheck;
		if ((lnEchoBuffer.lnData[0] == lnInBuffer.lnData[0]) && ((lnEchoBuffer.errorFlags & msgXORCheck) == 0) && ((lnInBuffer.errorFlags & msgXORCheck) == 0)) //valid echo message
		{
//			Serial.printf("echo ok r %i w %i\n", que_rdPos, que_wrPos);
			if (que_rdPos != que_wrPos)
			{
				que_rdPos = (que_rdPos + 1) % queBufferSize; //finalize transmit process
//				Serial.println("Update rd ptr");
			}
			lnInBuffer.errorFlags |= msgEcho;
			lnInBuffer.echoTime = micros() - lnInBuffer.reqRecTime;
//			Serial.printf("incr read: %02X %02X %02X \n", lnEchoBuffer.lnData[0], lnInBuffer.lnData[0], lnEchoBuffer.errorFlags);
		}
//		else
//			Serial.printf("No Echo: %02X %02X %02X \n", lnEchoBuffer.lnData[0], lnInBuffer.lnData[0], lnEchoBuffer.errorFlags);
		
		if (((lnInBuffer.errorFlags & msgEcho) > 0) || (((respOpCode & 0x08) > 0) && ((lnInBuffer.lnData[0]==0xB4) || (lnInBuffer.lnData[0]==0xE7) || (lnInBuffer.lnData[0]==0x81)) && ((lnInBuffer.errorFlags & msgEcho) == 0))) 
		{
			lnInBuffer.requestID = respID;
			lnInBuffer.reqRespTime = micros() - respTime;
		}
		else
		{
			lnInBuffer.requestID = 0;
			lnInBuffer.reqRespTime = 0;
		}
//		processLNMsg(&lnEchoBuffer);
//		lnEchoBuffer.lnData[0] = 0; //reset Echo Buffer
		//if limitedMaster, set flag to insert OPC_BUSY messages while waiting for response from application side (CS or MQTT)
		if ((hybrid_getNetworkType() != standardMode) && (lnInBuffer.errorFlags & msgEcho) == 0) //received from LocoNet
			if ((lnInBuffer.lnData[0] & 0x08) > 0)
				hybrid_setBusyMode(true); //(dataByte[0] & 0x08) > 0); Puts hybrid in autonmous OPC_BUSY transmit mode while active
		if ((lnInBuffer.lnData[0] != 0x81) || (hybrid_getNetworkType() == standardMode))
		{
			processLNMsg(&lnInBuffer); //don't send OPC_BUSY in  master mode
		}
        lnBufferPtr = 0;
        lnInBuffer.errorFlags = 0;
        bitRecStatus = 0; //awaiting OpCode
      }  
    }
    else
    {
      //unexpected data byte while waiting for OpCode
		lnInBuffer.errorFlags |= msgStrayData;
		lnInBuffer.lnMsgSize = 1;
		lnInBuffer.echoTime = 0;
		lnInBuffer.requestID = 0;
		lnInBuffer.reqRecTime = 0;
		lnInBuffer.reqRespTime = micros();
		lnBufferPtr = 0;
		hybrid_setBusyMode(false); //clear hybrid autonomous mode after ending current transmission of OPC_BUSY
		if (inData != 0)
			processLNMsg(&lnInBuffer); 
    }
  }    
}

void LocoNetESPSerial::processLoopBack()
{
	lnReceiveBuffer recData;
	if (que_wrPos != que_rdPos)
	{
		digitalWrite(busyLED, 0);
		que_rdPos = (que_rdPos + 1) % queBufferSize;
		recData.msgType = LocoNet;
		recData.lnMsgSize = transmitQueue[que_rdPos].lnMsgSize;
		recData.requestID = transmitQueue[que_rdPos].requestID;
		recData.reqRecTime = micros();
		recData.errorFlags = msgEcho;
		memcpy(recData.lnData, transmitQueue[que_rdPos].lnData, transmitQueue[que_rdPos].lnMsgSize);
		processLNMsg(&recData);
		digitalWrite(busyLED, 1);
	}
}

void LocoNetESPSerial::processLoop()
{
	if (loopbackMode)
		processLoopBack();
	else
		if (receiveMode)
			processLNReceive();
		else
			processLNTransmit();
}

void LocoNetESPSerial::processLNReceive()
{
//	Serial.printf("Proc Rec %i\n", receiveMode);
	hybrid_highSpeed(false); //sets the speed low, but only if uart is idling
//	while (uart_available() > 0) //empty that input buffer
	while (HardwareSerial::available() > 0) //empty that input buffer
	{
//		uint8_t newData = uart_read();
		uint8_t newData = HardwareSerial::read();
        handleLNIn((newData), 0); //and process incoming bytes
	}
	if ((que_wrPos != que_rdPos) && (hybrid_LocoNetAvailable() == lnNetAvailable) && ((que_lastPos != que_rdPos) || (micros() > transmitTime))) //if rxBuffer is empty, load next command, if available
	{
//		Serial.printf("TxBuf: r %i w %i\n", que_rdPos, que_wrPos);
//		Serial.printf("TxBuf: %i\n", que_wrPos > que_rdPos ? que_wrPos - que_rdPos :  que_rdPos - que_wrPos);
		que_lastPos = que_rdPos;
		receiveMode = false;
	}
}

void LocoNetESPSerial::processLNTransmit()
{
//	Serial.printf("Proc Tx %i\n", transmitStatus);
	switch (transmitStatus)
	{
		case 0: //not yet started, transfer data and start transmission
		{
			if (!hybrid_availableForWrite()) //uart only takes one message at a time
			{
				receiveMode = true;
				return;
			}
			//do not set que_rdPos as this would break mode switch from Receive to Transmit. que_rdPos is only updated after successful transmission
			transmitStatus = 1; //in case we have to stop transmission, process can be repeated
			//break; //don't break, just go on and start transmit
		}
		case 1: //
		{
			hybrid_highSpeed(true);
			numRead = 0;
			int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
			numWrite = transmitQueue[hlpQuePtr].lnMsgSize;
			hybrid_write(&transmitQueue[hlpQuePtr].lnData[0], transmitQueue[hlpQuePtr].lnMsgSize); //send bytes
//			if (hybrid_getBusyMode())
			hybrid_setBusyMode(false);
		    lnEchoBuffer.requestID = transmitQueue[hlpQuePtr].requestID;
			lnEchoBuffer.reqRecTime = transmitQueue[hlpQuePtr].reqRecTime;
			lnEchoBuffer.reqRespTime = 0;
			lnEchoBuffer.echoTime = 0;
			lnEchoBuffer.errorFlags = 0;
			lnEchoBuffer.lnMsgSize = 0;
			lnEchoBuffer.lnData[0] = 0; //reset Echo Buffer
			//store these to look up when a reply comes in
			respTime = micros();
			respID = lnEchoBuffer.requestID;
			respOpCode = transmitQueue[hlpQuePtr].lnData[0];
			transmitTime = micros() + (numWrite * 600) + 500000; //set timeout condition, LocoNet not echoing bytes sent. must be > 500ms to allow network access trys and low processing rate
			transmitStatus = 2; //set status for verification of echo bytes
			break;
		}
		case 2: //message sent to buffer. Check for incoming bytes and verify collision status. Return to receive mode when complete or timeout
		{
			//wait for echo bytes, verify against buffer
			while ((hybrid_available() > 0) && (numRead < numWrite))
			{
				uint16_t newByte = hybrid_read();
//				Serial.printf("Ec %i %02X\n", numRead, newByte);
				lnEchoBuffer.lnData[numRead] = newByte & 0x00FF;
				lnEchoBuffer.errorFlags |= (newByte >> 8);
				lnEchoBuffer.lnMsgSize++;
				numRead++;
			}
			//success, update read pointer, set status and mode
			if ((numRead == numWrite) || (micros() > transmitTime)) //success or timeout
			{
				if (micros() > transmitTime)
				{
//					Serial.println("timeout");
					lnEchoBuffer.errorFlags |= errorTimeout;
				}
				if (getXORCheck(&lnEchoBuffer.lnData[0], &lnEchoBuffer.lnMsgSize) != 0xFF)
					lnEchoBuffer.errorFlags |= msgXORCheck;
				lnEchoBuffer.echoTime = micros() - lnEchoBuffer.reqRecTime;
				transmitStatus = 0;
				receiveMode = true;
//				Serial.println("rec true");
				hybrid_flush();
			}
			break;
		}
	}
}

uint8_t LocoNetESPSerial::getXORCheck(uint8_t * msgData, uint8_t * msgLen)
{
	uint8_t xorChk = 0;
	for (uint8_t i = 0; i < *msgLen; i++)
		xorChk ^= msgData[i];
	return xorChk;
}


bool LocoNetESPSerial::carrierOK()
{
	return hybrid_carrierOK();
}

void LocoNetESPSerial::setBusyLED(int8_t ledNr, bool logLevel)
{
	if (ledNr >= 0)
	{
		hybrid_setBusyLED(ledNr, logLevel);
		busyLED = ledNr;
	}
}

/*
int LocoNetESPSerial::cdBackoff() {
	return hybrid_LocoNetAvailable();
}
*/
