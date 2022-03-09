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
	Serial.printf("Set Msg Type %i\n", thisType);
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
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, txData.lnMsgSize);
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
			Serial.println("LN unexpected data byte while waiting for OpCode");
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

int IoTT_SerInjector::parseDCCExNumVal(char** startAt, uint8_t* cntVal)
{
	while (!isDigit(**startAt) && (**startAt != '-')) 
	{
		*startAt = *startAt + 1;
		*cntVal += 1;  // = *cntVal + 1;
	}
	char* upTo = *startAt;
	while ((isDigit(*upTo)) || (*upTo == '-')) upTo++;
	char tempBuf[10];
	strncpy(&tempBuf[0], *startAt, upTo - *startAt);
	return atoi(tempBuf);
}

bool IoTT_SerInjector::parseDCCEx(lnTransmitMsg* thisEntry, lnTransmitMsg* txBuffer)
{
//	Serial.print("Parse DCC++ Message ");
	char* readPtr = (char*) &lnInBuffer.lnData[0];
	while ((*readPtr == '<') || (*readPtr == ' '))
		readPtr++;
	Serial.println(*readPtr);
	uint8_t posCtr = 0;
	if (readPtr != NULL)
	{
		char opCode = *readPtr;
		if ((opCode == 'Q') || (opCode == 'q')) //sensor input
		{
			txBuffer->lnData[0] = 11; //sensor input
//			Serial.println("sensor input");
			uint8_t retVal = parseDCCExNumVal(&readPtr, &posCtr);
			txBuffer->lnData[1] = retVal; //sensor nr
			txBuffer->lnData[2] = opCode == 'Q' ? 0 : 1;
			txBuffer->lnMsgSize = 3;
			txBuffer->reqID = 0x00; 
			return true;
		}
		if (opCode == 'T')
		{
			Serial.println("Cab Ctrl Return");
		}
		if (opCode == '-')
		{
			Serial.println("Release Locos");
		}
		if (opCode == '0')
		{
			Serial.println("Command Successfully executed");
		}
		if (opCode == 'r')
		{
			txBuffer->lnData[0] = 5; //progMode
//			Serial.println("programmer reply");
			uint16_t retVal = parseDCCExNumVal(&readPtr, &posCtr);
			txBuffer->lnData[1] = retVal >> 8; //priv code
			txBuffer->lnData[2] = retVal & 0x00FF;
			readPtr += posCtr;
			posCtr = 0;
			retVal = parseDCCExNumVal(&readPtr, &posCtr);
			txBuffer->lnData[3] = retVal >> 8; //priv code
			txBuffer->lnData[4] = retVal & 0x00FF;
			readPtr += posCtr;
			posCtr = 0;
			retVal = parseDCCExNumVal(&readPtr, &posCtr);
			txBuffer->lnData[5] = retVal >> 8; //CV nr
			txBuffer->lnData[6] = retVal & 0x00FF;
			readPtr += posCtr;
			posCtr = 0;
			int16_t cvVal = parseDCCExNumVal(&readPtr, &posCtr);
			txBuffer->lnData[7] = cvVal >> 8; //Value
			txBuffer->lnData[8] = cvVal & 0x00FF;
			txBuffer->lnMsgSize = 9;
			txBuffer->reqID = 0x00; 
			return true;
		}
		if (opCode == 'X')
		{
			Serial.println("Command execution failed");
		}
	}
	return false;
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
//					Serial.println("Parse");
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
	//take new message from transmit queue and send to USB/PC
    if (que_wrPos != que_rdPos) //override protection
    {
		uint8_t hlpQuePtr = (que_rdPos + 1) % queBufferSize;

		//send to USB port
//		Serial.printf("DCC++Ex Transmit %i %i %i\n", hlpQuePtr, transmitQueue[hlpQuePtr].lnData[0], transmitQueue[hlpQuePtr].lnData[1]);
		char txMsg[50];
		switch (transmitQueue[hlpQuePtr].lnData[0])
		{
			case 0: //Power management
				switch (transmitQueue[hlpQuePtr].lnData[1])
				{
					case 0: //off
//						Serial.print("<0>");
						write("<0>");
						break;
					case 1: //on
//						Serial.print("<1>");
						write("<1>");
						break;
					case 2: //idle
//						Serial.print("<!>");
						write("<!>");
						break;
				}
				break;
			case 1: //cab control
			{
				uint16_t cabAddr = (transmitQueue[hlpQuePtr].lnData[2] << 7) + (transmitQueue[hlpQuePtr].lnData[3] &0x7F);
				switch (transmitQueue[hlpQuePtr].lnData[1])
				{
					case 0: //remove from refresh buffer
					{
						sprintf(txMsg, "<- %i>", cabAddr, transmitQueue[hlpQuePtr].lnData[3], transmitQueue[hlpQuePtr].lnData[4]);
						write(txMsg);
//						Serial.println(txMsg);
					}
					break;
					case 1: //add to refresh buffer
					{
						sprintf(txMsg, "<t 1 %i %i %i>", cabAddr, transmitQueue[hlpQuePtr].lnData[4], ((transmitQueue[hlpQuePtr].lnData[5] & 0x20)>>5) ^ 0x01); //[4]: SPD, [5]:DIRF Dir bit change from LocoNet to DCC++
						write(txMsg);
//						Serial.println(txMsg);
					}
					break;
				}
				break;
			}
			case 2: //function control
			{
				uint16_t cabAddr = (transmitQueue[hlpQuePtr].lnData[1] << 7) + (transmitQueue[hlpQuePtr].lnData[2] &0x7F);
				sprintf(txMsg, "<F %i %i %i>", cabAddr, transmitQueue[hlpQuePtr].lnData[3], transmitQueue[hlpQuePtr].lnData[4]);
				write(txMsg);
//				Serial.println(txMsg);
				break;
			}
			case 3: //switch control
			{
				uint16_t swiAddr = (transmitQueue[hlpQuePtr].lnData[1] << 7) + (transmitQueue[hlpQuePtr].lnData[2] &0x7F) +1; //DCC Offset
				sprintf(txMsg, "<a %i %i>", swiAddr, transmitQueue[hlpQuePtr].lnData[3]);
				write(txMsg);
//				Serial.println(txMsg);
				break;
			}
			case 4: //immediate command
			{
				char subStr[5];
				sprintf(txMsg, "<M 0");
//				sprintf(txMsg, "<M 0", transmitQueue[hlpQuePtr].lnData[3]);
				for (uint8_t i = 1; i <   transmitQueue[hlpQuePtr].lnMsgSize; i++)
				{
					sprintf(subStr, " %2X", transmitQueue[hlpQuePtr].lnData[i]);
					strcat(txMsg, subStr);
				}
				strcat(txMsg, ">");
				write(txMsg);
//				Serial.println(txMsg);
				break;
			}
			case 5: //service mode programming
			{
				uint16_t cabAddr = (transmitQueue[hlpQuePtr].lnData[2] << 7) + (transmitQueue[hlpQuePtr].lnData[3] &0x7F);
				uint8_t progMode = transmitQueue[hlpQuePtr].lnData[1];
				uint16_t cvNr = (transmitQueue[hlpQuePtr].lnData[4] << 7) + (transmitQueue[hlpQuePtr].lnData[5] &0x7F);
				uint8_t cvVal = transmitQueue[hlpQuePtr].lnData[6];
				switch ((progMode & 0x04) >> 2)
				{
					case 0: // Service Mode
						progTrackActive = true;
						if ((progMode & 0x40) > 0) //write Op
						{
							if ((progMode & 0x20) > 0) //byte write Op
							{
								sprintf(txMsg, "<W %i %i %i %i>", cvNr, cvVal, 0, 0);
							}
							else //bit write Op
							{
								uint8_t bitNr;
								uint8_t bitVal;
								sprintf(txMsg, "<B %i %i %i %i %i>", cvNr, bitNr, bitVal, 0, 0);
							}
						}
						else //read op
						{
							sprintf(txMsg, "<R %i %i %i>", cvNr, 0, 0);
						}
						break;
					case 1: //Ops Mode
						sprintf(txMsg, "<w%i %i %i>", cabAddr, cvNr, cvVal);
						break;
				}
				write(txMsg);
//				Serial.print("Out: ");
//				Serial.println(txMsg);
				break;
			}
			case 99: //System configuration
			{
				uint16_t cfgId = (transmitQueue[hlpQuePtr].lnData[1] << 7) + (transmitQueue[hlpQuePtr].lnData[2] &0x7F);
				uint16_t cfgVal = (transmitQueue[hlpQuePtr].lnData[3] << 7) + (transmitQueue[hlpQuePtr].lnData[4] &0x7F);
				sprintf(txMsg, "<Z %i %i>", cfgId, cfgVal);
				break;
			}
		}
		Serial.print("Out: ");
		Serial.println(txMsg);
		que_rdPos = hlpQuePtr;
	}
}
	
