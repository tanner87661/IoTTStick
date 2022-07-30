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

#include <IoTT_SerInjector.h>
#include <HardwareSerial.h>

txFct usbCallback = NULL;

class IoTT_DigitraxBuffers;

IoTT_SerInjector::IoTT_SerInjector(int receivePin, int transmitPin, bool inverse_logic, int uartNr, unsigned int buffSize) : HardwareSerial(uartNr) 
{
	m_invert = inverse_logic;
	m_rxPin = receivePin;
	m_txPin = transmitPin;
	que_rdPos = que_wrPos = 0;
	receiveMode = true;
	transmitStatus = 0;
	m_uart = uartNr;
	m_buffsize = buffSize;
 }

IoTT_SerInjector::~IoTT_SerInjector() {
	
//	HardwareSerial::~HardwareSerial();
}

void IoTT_SerInjector::begin() {

	HardwareSerial::begin(baudRate, SERIAL_8N1, m_rxPin, m_txPin, m_invert);
	m_highSpeed = true;
}

void IoTT_SerInjector::setProtType(messageType thisType)
{
//	Serial.printf("Set Msg Type %i\n", thisType);
	usedProtocol = thisType;
}

messageType IoTT_SerInjector::getMsgType()
{
	return usedProtocol;
}

void IoTT_SerInjector::loadLNCfgJSON(DynamicJsonDocument doc)
{
//	Serial.println("Call JSON Serial");
	if (doc.containsKey("BaudRate"))
		baudRate = doc["BaudRate"];
	if (doc.containsKey("Invert"))
		m_invert = doc["Invert"];
	begin();
}

uint16_t IoTT_SerInjector::lnWriteMsg(lnTransmitMsg txData)
{
	//here we receive a LocoNet message from LocoNet and place it in the buffer so it gets sent to the PC
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
//		Serial.printf("put to transmitQueue TxMsg %i\n", hlpQuePtr);
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].reqID = txData.reqID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, txData.lnMsgSize+1);
//		Serial.println();
//		Serial.printf("LN Tx %2X", txData.lnData[0]);
//		for (int i = 1; i < txData.lnMsgSize; i++)
//			Serial.printf(", %2X", txData.lnData[i]);
//		Serial.println();
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{	
		Serial.println("USB Port Write Error. Too many messages in queue");
		return -1;
	}
}

uint16_t IoTT_SerInjector::lnWriteMsg(lnReceiveBuffer txData)
{
//	Serial.println("SerInj put to transmitQueue RxMsg");
	//here we receive a LocoNet message from LocoNet and place it in the buffer so it gets sent to the PC
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].msgType = txData.msgType;
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].reqID = txData.reqID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, lnMaxMsgSize); //txData.lnMsgSize);
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{	
		Serial.println("USB Port Write Error. Too many messages in queue");
		return -1;
	}
}


void IoTT_SerInjector::setTxCallback(txFct newCB)
{
	usbCallback = newCB;
}


void IoTT_SerInjector::processLNMsg(lnTransmitMsg* recData)
{
	//send to PC/USB
	if (usbCallback)
		usbCallback(*recData);
}

void IoTT_SerInjector::handleLNIn(uint8_t inData, uint8_t inFlags) //called for stuff that comes in through the HW uart
{
//	Serial.println("handleLNIn");
}

void IoTT_SerInjector::processLoop()
{
//	Serial.println(msgProtocol);
	switch (usedProtocol)
	{
		case LocoNet: processLNReceive(); processLNTransmit(); break;
		case OpenLCB: processLCBReceive(); processLCBTransmit(); break;
		case DCCEx: processDCCExReceive(); processDCCExTransmit(); break;
		default: Serial.println("unknown protocol"); break;
	}
	yield();
}

void IoTT_SerInjector::processLNReceive()
{
	//collect bytes from USB/PC and if message is complete, send it to the Gateway or LocoNet Port
	while (available())
	{
		byte inData = read();
//		Serial.println(inData,16);
		if (inData >= 0x80) //OpCode, start of new message
		{
//			Serial.println("New Command");
			if (bitRecStatus == 1) //awaiting data bytes but received OpCode
			{
				//incomplete message
//			Serial.println("Old Command not complete");
				lnInBuffer.lnMsgSize = lnBufferPtr;
				lnBufferPtr = 0;
				processLNMsg(&lnInBuffer); //get rid of previous message
			}
			bitRecStatus = 1; //await data bytes
			lnInBuffer.reqID = 0;
			lnInBuffer.reqRecTime = micros(); 
			lnBufferPtr = 0;
			uint8_t swiByte = (inData & 0x60) >> 5;
			switch (swiByte)
			{
				case 0: lnExpLen  = 2; break;
				case 1: lnExpLen  = 4; break;
				case 2: lnExpLen  = 6; break;
				case 3: lnExpLen  = 0xFF; break;
				default: lnExpLen = 0;
			}
			lnXOR  = inData;
			lnInBuffer.lnData[lnBufferPtr] = inData;
			lnBufferPtr++; 
		}
		else //received regular data byte
		{
			if (bitRecStatus == 1) //collecting data
			{
//			Serial.println("Collecting Data");
				lnInBuffer.lnData[lnBufferPtr] = inData;
				if ((lnBufferPtr == 1) && (lnExpLen == 0xFF))
					lnExpLen  = (inData & 0x007F); //updating expected length for long message
				lnBufferPtr++; 
				if (lnBufferPtr == lnExpLen) //message length received
				{
//			Serial.println("Command complete, sending");
					lnInBuffer.lnMsgSize = lnBufferPtr;  
					lnInBuffer.reqID = 0;
					lnInBuffer.reqRecTime = 0;
					processLNMsg(&lnInBuffer);
					lnBufferPtr = 0;
					bitRecStatus = 0; //awaiting OpCode
				}  
				else
					lnXOR ^= inData;
			}
			else
			{
//			Serial.println("LN unexpected data byte while waiting for OpCode");
			//unexpected data byte while waiting for OpCode
				lnInBuffer.lnMsgSize = 1;
				lnInBuffer.reqID = 0;
				lnInBuffer.reqRecTime = 0;
				lnBufferPtr = 0;
				processLNMsg(&lnInBuffer); 
			}
		}    
	}
}

void IoTT_SerInjector::processLNTransmit()
{
	//take new message from transmit queue and send to USB/PC
	uint8_t hlpQuePtr = (que_rdPos + 1) % queBufferSize;
    if (que_wrPos != que_rdPos) //override protection
    {
		//send to USB port
		
		for (int i = 0; i < transmitQueue[hlpQuePtr].lnMsgSize; i++)
		{
//			Serial.print(transmitQueue[hlpQuePtr].lnData[i],16);
			write(transmitQueue[hlpQuePtr].lnData[i]);
	    }
		que_rdPos = hlpQuePtr;
	}
}

/*
Receive Statuses
0: await start char
1: await frame type
2: await ID
3: await end char
*/ 
void IoTT_SerInjector::processLCBReceive()
{
	while (available()) //read GridConnect protocol and package by message
	{
		char inData = read();
//		Serial.print(inData);
		switch (inData)
		{
			case ':' : //start new message
				if (bitRecStatus != 0) //receiving in progress, something is wrong
				{
//					Serial.println("Old Command not complete");
					lnInBuffer.lnMsgSize = lnExpLen;
					lnInBuffer.reqID = 0xFF; //invalid message
					lnBufferPtr = 0;
					processLNMsg(&lnInBuffer); //get rid of previous (invalid) message
				}
				bitRecStatus = 1; //await data bytes
				lnInBuffer.reqID = 0;
				lnInBuffer.reqRecTime = micros(); 
				lnInBuffer.msgType = OpenLCB;
				lnBufferPtr = 0;
				lnInBuffer.lnData[lnBufferPtr] = inData;
				lnBufferPtr++; 
				break;
			case ';' : //terminate message
				lnInBuffer.lnData[lnBufferPtr] = inData; //leave terminator intact
				lnInBuffer.lnMsgSize = lnExpLen; //this is # of nibbles, so 2x byte length
				lnBufferPtr = 0;
				processLNMsg(&lnInBuffer); //get rid of previous (invalid) message
				bitRecStatus = 0;
//				Serial.println(inData);
				break;
			case 'X' : //if on Pos 1 -> extended frame
			case 'S' : //if on Pos 1 -> standard frame	
				bitRecStatus = 2;
				lnInBuffer.lnData[lnBufferPtr] = inData;
				lnBufferPtr++; 
				break;
			case 'N' : //if on pos 6 or pos 10 based on X/S -> normal frame
			case 'R' : //if on pos 6 or pos 10 based on X/S -> remote frame
				lnExpLen = 0;
				bitRecStatus = 3;
				lnInBuffer.lnData[lnBufferPtr] = inData;
				lnBufferPtr++; 
				break;
			default  : //must be data (max 8 bytes)
				lnInBuffer.lnData[lnBufferPtr] = inData;
				lnExpLen++;
				lnBufferPtr++; 
				break;
		}
	}
}

void IoTT_SerInjector::processLCBTransmit()
{
	//take new message from transmit queue and send to USB/PC
	uint8_t hlpQuePtr = (que_rdPos + 1) % queBufferSize;
    if (que_wrPos != que_rdPos) //override protection
    {
		//send to USB port
//		Serial.println("SerInj Send Message to OLCB Serial");
		
		int index = 0;
		while (index < lnMaxMsgSize) //emergency stop when ; is missing
		{
//			Serial.print(char(transmitQueue[hlpQuePtr].lnData[index]));
			write(transmitQueue[hlpQuePtr].lnData[index]);
			if (char(transmitQueue[hlpQuePtr].lnData[index]) == ';')
				break;
			index++;
		}
		que_rdPos = hlpQuePtr;
	}
}

/** from JMRI class def
009 * Class for GridConnect messages for a CAN hardware adapter.
010 * <p>
011 * The GridConnect protocol encodes messages as an ASCII string of up to 24
012 * characters of the form: :ShhhhNd0d1d2d3d4d5d6d7;
013 * <p>
014 * The S indicates a standard
015 * CAN frame :XhhhhhhhhNd0d1d2d3d4d5d6d7; The X indicates an extended CAN frame
016 * hhhh is the two byte header N or R indicates a normal or remote frame, in
017 * position 6 or 10 d0 - d7 are the (up to) 8 data bytes
018 * <p>
019 *
020 * @author Andrew Crosland Copyright (C) 2008
021 */

int IoTT_SerInjector::parseDCCExParam(char** startAt, uint8_t ppNum, ppElement * outBuffer) //ret is also ppElement.dataType; values 0: invalid; 1-4 # of chars in str; 10: int; 20: float; everything else invalid
{
	uint8_t posCtr = 0;
	uint8_t decPtCtr = 0;
	uint8_t charCtr = 0;
	uint8_t digitsCtr = false;
	bool negLead = false;
	bool hexLead = false;
	outBuffer->dataType = 0; //invalid data
	outBuffer->paramNr = ppNum;
	while ((**startAt == ' ') || (**startAt == '|'))
		*startAt += 1; //skip leading blanks and |
	char * origPtr = *startAt;
	while ((**startAt != ' ') && (**startAt != '|') && (**startAt != '>'))
	{
//		Serial.printf("Testing: %c\n", **startAt);
		if (posCtr == 0)
		{
			hexLead = (**startAt == '0');
			negLead = (**startAt == '-');
		}
		if (posCtr == 1)
			hexLead = hexLead && ((**startAt == 'x') || (**startAt == 'X'));
//		if ((isAlpha(**startAt)) || (**startAt == '#'))
		if ((!isDigit(**startAt)) && (**startAt != '*')) //filter out comments and numbers
			charCtr++;
		else
			if (isDigit(**startAt))
				digitsCtr++;
			else
				if (**startAt == '.')
					decPtCtr++;
				else
				{
					Serial.print("COMMENT "); //invalid char
					Serial.println(**startAt);
					return 0;
				}
		*startAt += 1;
		posCtr++;
		if (ppNum == 0) //first param is OpCode and only 1 char long
			break;
	}
	if (decPtCtr > 1) return 0; //can't be a float with more than 1 dec pt
	uint8_t msgLen = *startAt - origPtr;

// ret = strtol(str, &ptr, 16); //hex conversion
	if ((negLead) & (charCtr > 0)) charCtr--;
	
//	Serial.printf("neg: %i chr: %i dig: %i \n", negLead, charCtr, digitsCtr);
	
	if (charCtr > 0) //this is a string param, we return max 4 chars. It can have some digits as well
	{
		msgLen = min((uint8_t)4, msgLen);
		strncpy(&outBuffer->payload.strVal[0], origPtr, msgLen);
		outBuffer->dataType = msgLen;
	}
	else
		if (digitsCtr > 0) //this is numeric but no chars
		{
			char tempBuf[10] = "";
			strncpy(&tempBuf[0], origPtr, msgLen);
			tempBuf[msgLen] = 0;
			if (decPtCtr == 0) //integer
			{
				int thisRes = atoi(tempBuf);
				outBuffer->payload.longVal = thisRes;
				outBuffer->dataType = 10;
			}
			else //float
			{
				outBuffer->payload.floatVal = atof(tempBuf);
				outBuffer->dataType = 20;
			}
		}
		return outBuffer->dataType;
}

bool IoTT_SerInjector::parseDCCEx(lnTransmitMsg* thisEntry, lnTransmitMsg* txBuffer)
{
//	Serial.println("Parse DCC++ Message ");

	for (uint8_t i = 0; i < lnMaxMsgSize; i++)
		txBuffer->lnData[i] = 0;
	char* readPtr = (char*) &thisEntry->lnData[0];
	txBuffer->lnMsgSize = lnMaxMsgSize;
	while ((*readPtr == '<') || (*readPtr == ' ') || (*readPtr == '|'))
		readPtr += 1;	//set read pointer to the first byte with data
	uint8_t paramCtr = 0;

	ppElement * thisData = (ppElement*) &(txBuffer->lnData[0]);

	while ((parseDCCExParam(&readPtr, paramCtr, &thisData[paramCtr])) && (paramCtr < 5)) //max 48 bytes
		paramCtr += 1;
	thisData[0].numParams = paramCtr;
	return paramCtr > 0 ? true : false;
}	
	
void IoTT_SerInjector::processDCCExReceive()
{
	while (available()) //read GridConnect protocol and package by message
	{
		char inData = read();
//		Serial.print(inData);
		switch (inData)
		{
			case '<' : //start new message
				if (bitRecStatus != 0) //receiving in progress, something is wrong
				{
					Serial.println("Old Command not complete");
					lnInBuffer.lnMsgSize = lnBufferPtr;
					lnInBuffer.reqID = 0xFF; //invalid message
//					processLNMsg(&lnInBuffer); //get rid of previous (invalid) message
				}
				bitRecStatus = 1; //await data bytes
				lnInBuffer.reqID = 0;
				lnInBuffer.reqRecTime = micros(); 
				lnInBuffer.msgType = DCCEx;
				lnBufferPtr = 0;
				lnInBuffer.lnData[lnBufferPtr] = inData;
				lnBufferPtr++; 
				break;
			case '>' : //terminate message
				{
					lnInBuffer.lnData[lnBufferPtr] = inData; //leave terminator intact
					lnInBuffer.lnData[lnBufferPtr+1] = 0; //terminating 0 for char* interpretation
					lnInBuffer.lnMsgSize = lnBufferPtr; //this is # of nibbles, so 2x byte length
					lnBufferPtr = 0;
					lnTransmitMsg txOutBuffer;
//					Serial.println();
					if (parseDCCEx(&lnInBuffer, &txOutBuffer))
						processLNMsg(&txOutBuffer);
					bitRecStatus = 0;
				}
				break;
			default  : //must be data (max 8 bytes)
				if (lnBufferPtr > 0) //ignore CRLF and any char before <
				{
					lnInBuffer.lnData[lnBufferPtr] = inData;
					if (lnBufferPtr < lnMaxMsgSize)
						lnBufferPtr++; 
				} 
				break;
		}
	}
}

void IoTT_SerInjector::processDCCExTransmit()
{
	char txMsg[50];
	//take new message from transmit queue and send to USB/PC
    if (que_wrPos != que_rdPos) //override protection
    {
		uint8_t hlpQuePtr = (que_rdPos + 1) % queBufferSize;
		char * outStr = (char*)&transmitQueue[hlpQuePtr].lnData;
		strcpy(txMsg, "<");
		strcat(txMsg, outStr);
		strcat(txMsg, ">\n\r");
		write(txMsg);
//		Serial.print("Out: ");
//		Serial.println(txMsg);
		que_rdPos = hlpQuePtr;
	}
}

