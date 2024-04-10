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

#define numCB 2

txFct usbCallback = NULL;
dccCbFct dccCallback[numCB] = {NULL, NULL};
//dccCbFct dccBypass = NULL;

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
	baudRate = 115200;
	m_invert = false;
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
//	Serial.printf("r %i w %i \n", que_rdPos, que_wrPos);
//	char* outStr = (char*)&txData.lnData[0];
//	Serial.println(outStr);
    uint8_t hlpQuePtr = (que_wrPos + 1) % queInjBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
//		Serial.printf("put to transmitQueue TxMsg %i\n", hlpQuePtr);
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].requestID = txData.requestID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, txData.lnMsgSize+1);
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{	
		Serial.printf("%i %i SerInj Port Write Error. Too many messages in queue\n", que_rdPos, que_wrPos);
		return 0;
	}
}

uint16_t IoTT_SerInjector::lnWriteMsg(lnReceiveBuffer txData)
{
//	Serial.println("SerInj put to transmitQueue RxMsg");
	//here we receive a LocoNet message from LocoNet and place it in the buffer so it gets sent to the PC
    uint8_t hlpQuePtr = (que_wrPos + 1) % queInjBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].msgType = txData.msgType;
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].requestID = txData.requestID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, lnMaxMsgSize); //txData.lnMsgSize);
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{	
		Serial.println("SerInj USB Port Write Error. Too many messages in queue");
		return 0;
	}
}

void IoTT_SerInjector::setTxCallback(txFct newCB)
{
	lnInBuffer.reqRecTime = 0;
	usbCallback = newCB;
}

void IoTT_SerInjector::setDCCCallback(dccCbFct newDCCCB, uint8_t ptrNr)
{
//	Serial.printf("Init Callback %i\n", ptrNr);
	lnInBuffer.reqRecTime = 0xFF;
	if (ppList.size() == 0)
	{
//		Serial.println("Init Param");
		ppElement newParam;
		newParam.dataType = 0xFF;
		newParam.paramNr = 0;
		ppList.push_back(newParam);
	}
	dccCallback[ptrNr] = newDCCCB; 
//	Serial.printf("Init Callback %i done\n", ptrNr);
}


void IoTT_SerInjector::processLNMsg(lnTransmitMsg* recData)
{
//	Serial.println("send to PC/USB");
	if (usbCallback)
		usbCallback(*recData);
}

void IoTT_SerInjector::processDCCEXMsg(std::vector<ppElement> * recData)
{
	for (uint8_t i = 0; i < numCB; i++)
	{
		if (dccCallback[i])
		{
//			Serial.printf("Send Callback %i \n", i);
			dccCallback[i](recData);
		}
	}
}

void IoTT_SerInjector::handleLNIn(uint8_t inData, uint8_t inFlags) //called for stuff that comes in through the HW uart
{
//	Serial.println("handleLNIn");
}

void IoTT_SerInjector::processLoop()
{
//	Serial.println(usedProtocol);
	switch (usedProtocol)
	{
		case LocoNet: processLNReceive(); processLNTransmit(); break;
		case OpenLCB: processLCBReceive(); processLCBTransmit(); break;
		case DCCEx: processDCCExReceive(); processDCCExTransmit(); break;
		case DCCBoost: processBoosterReceive(); processBoosterTransmit(); break;
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
			if (lnInBuffer.reqRecTime == 1) //awaiting data bytes but received OpCode
			{
				//incomplete message
				lnInBuffer.lnMsgSize = lnBufferPtr;
				lnBufferPtr = 0;
//				Serial.println("Ignored incomplete message");
//				processLNMsg(&lnInBuffer); //get rid of previous message
			}
			lnInBuffer.reqRecTime = 1; //await data bytes
			lnInBuffer.requestID = 0;
//			lnInBuffer.reqRecTime = micros(); 
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
			if (lnInBuffer.reqRecTime == 1) //collecting data
			{
//			Serial.println("Collecting Data");
				lnInBuffer.lnData[lnBufferPtr] = inData;
				if ((lnBufferPtr == 1) && (lnExpLen == 0xFF))
					lnExpLen  = (inData & 0x007F); //updating expected length for long message
				lnBufferPtr++; 
				lnXOR ^= inData;
				if (lnBufferPtr == lnExpLen) //message length received
				{
//					Serial.printf("Command complete, sending %02X", lnXOR);
					lnInBuffer.lnMsgSize = lnBufferPtr;  
					lnInBuffer.requestID = 0;
					lnInBuffer.reqRecTime = 0;
					if (lnXOR == 0xFF)
						processLNMsg(&lnInBuffer);
					else
						Serial.printf("Ignore invalid message (XOR check %02X)", lnXOR);
					lnBufferPtr = 0;
					lnInBuffer.reqRecTime = 0; //awaiting OpCode
				}  
//				else
//				{}
			}
			else
			{
			//unexpected data byte while waiting for OpCode
				lnInBuffer.lnMsgSize = 1;
				lnInBuffer.requestID = 0;
				lnInBuffer.reqRecTime = 0;
				lnBufferPtr = 0;
				Serial.println("Ignored unexpected data byte while waiting for OpCode");
//				processLNMsg(&lnInBuffer); 
			}
		}    
	}
}

void IoTT_SerInjector::processLNTransmit()
{
	//take new message from transmit queue and send to USB/PC
	uint8_t hlpQuePtr = (que_rdPos + 1) % queInjBufferSize;
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
				if (lnInBuffer.reqRecTime != 0) //receiving in progress, something is wrong
				{
//					Serial.println("Old Command not complete");
					lnInBuffer.lnMsgSize = lnExpLen;
					lnInBuffer.requestID = 0xFF; //invalid message
					lnBufferPtr = 0;
					processLNMsg(&lnInBuffer); //get rid of previous (invalid) message
				}
				lnInBuffer.reqRecTime = 1; //await data bytes
				lnInBuffer.requestID = 0;
//				lnInBuffer.reqRecTime = micros(); 
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
				lnInBuffer.reqRecTime = 0;
//				Serial.println(inData);
				break;
			case 'X' : //if on Pos 1 -> extended frame
			case 'S' : //if on Pos 1 -> standard frame	
				lnInBuffer.reqRecTime = 2;
				lnInBuffer.lnData[lnBufferPtr] = inData;
				lnBufferPtr++; 
				break;
			case 'N' : //if on pos 6 or pos 10 based on X/S -> normal frame
			case 'R' : //if on pos 6 or pos 10 based on X/S -> remote frame
				lnExpLen = 0;
				lnInBuffer.reqRecTime = 3;
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
	uint8_t hlpQuePtr = (que_rdPos + 1) % queInjBufferSize;
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

void IoTT_SerInjector::processDCCExReceive()
{
	while (available()) //read GridConnect protocol and package by message
	{
		char inData = read();
//		Serial.print(inData);
		if (parseDCCExNew(&inData, &lnInBuffer, &ppList))
		{
			processDCCEXMsg(&ppList);
//			Serial.println();
		}
	}
}

void IoTT_SerInjector::processDCCExTransmit()
{
	char txMsg[50] = {'\0'};
	//take new message from transmit queue and send to USB/PC
    if (que_wrPos != que_rdPos) //override protection
    {
		uint8_t hlpQuePtr = (que_rdPos + 1) % queInjBufferSize;
		char * outStr = (char*)&transmitQueue[hlpQuePtr].lnData;
		strcpy(txMsg, "<");
		strcat(txMsg, outStr);
		strcat(txMsg, ">\n\r");
		write(txMsg);
		que_rdPos = hlpQuePtr;
		SerialDebug("test");
		SerialDebugf("\"sent %i %i\n\", que_rdPos, que_wrPos");
//		Serial.printf("sent %i %i\n", que_rdPos, que_wrPos);
	}
}

void IoTT_SerInjector::processBoosterReceive()
{
//	Serial.println("processBoosterReceive");
	while (available()) //read GridConnect protocol and package by message
	{
		char inData = read();
//		Serial.print(inData);
		if ((inData != '\n') && (inData !='\r'))
		{
			inBuffer[lnBufferPtr] = inData;
			if (lnBufferPtr < genInBufLen-1)
			{
				lnBufferPtr++;
				inBuffer[lnBufferPtr] = '\0'; //terminating 0
			}
			else
				Serial.println("Buffer overflow");
		}
		if (((inData == '\n') || (inData =='\r')) && (lnBufferPtr > 0))
		{
			char* outStr = (char*)&inBuffer[0];
//			Serial.println(outStr);
			lnBufferPtr = 0;

			int8_t currClient = getWSClientByPage(0, "pgSilverHatCfg");
			DynamicJsonDocument doc(200);
			DeserializationError error = deserializeJson(doc, outStr);//, msgLen);
			if (!error)
			{
				if (doc.containsKey("Cmd"))
					if ((doc["Cmd"] == "LN") || (currClient < 0))
					{
						lnTransmitMsg lnInBuffer;
						if (doc["Cmd"] == "LN") //Loconet reporting
						{
							uint8_t opCode = doc["OPC"];
							uint16_t lnAddr = doc["Addr"];
							uint8_t lnData = doc["Stat"];
//							Serial.printf("LN Msg: %02X %i %i\n", opCode, lnAddr, lnData);
							switch (opCode)
							{
								case 0xB0: //sw_req
									sendSwitchCommand(opCode, lnAddr, lnData, 0);
									return;
								case 0xB2: //inp_report
									sendBlockDetectorCommand(lnAddr, lnData);
									return;
								case 0xED: //signal IMM
									sendSignalCommand(lnAddr, lnData);
									return;
								case 0xE5: //btn PEER_XFER
									sendButtonCommand(lnAddr, lnData);
									return;
								default: return;
							}
						}
						if (doc["Cmd"] == "SVR") //SV response if no web client
						{
//							Serial.println("Prepare and send SV Msg");
							JsonArray svData = doc["Prm"]["Vals"];
							uint8_t svrData[4];
							for (uint8_t i = 0; i < 4; i++)
								if (i < svData.size())
									svrData[i] = svData[i];
								else
									svrData[i] = 0;
							//{"Cmd":"SVR","Prm":{"Src":1234,"Opc":70,"Addr":284,"Vals":[129,0,31,0]}}
							sendSVCommand2(0x50, doc["Prm"]["Opc"], doc["Prm"]["Src"], doc["Prm"]["Addr"], svrData);

						}
					}
					else //web client found and not LN message
					{
						char myMqttMsg[150];
						serializeJson(doc, myMqttMsg);
//						Serial.println(myMqttMsg);
						while (currClient >= 0)
						{
							globalClients[currClient].wsClient->text(myMqttMsg);
							currClient = getWSClientByPage(currClient + 1, "pgSilverHatCfg");
						}
					}
			}
		}

	}
}

void IoTT_SerInjector::toggleBoosterOutput()
{
	char txMsg[100] = {'\0'};
	sprintf(txMsg, "{\"Cmd\":\"BTN\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", 0xF0, 0x01, 0xFFFF, 0xFE);
	write(txMsg);
}

void IoTT_SerInjector::dccToBooster(uint8_t opCode, uint16_t devAddr, uint8_t devData)
{
	char txMsg[100] = {'\0'};
	sprintf(txMsg, "{\"Cmd\":\"LN\",\"Prm\":{\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", opCode, devAddr, devData);
//	Serial.println(txMsg);
	write(txMsg);
}

void IoTT_SerInjector::processBoosterTransmit()
{
//	Serial.println("processBoosterTransmit");
	char txMsg[150] = {'\0'};
	//take new message from transmit queue and send to USB/PC
    if (que_wrPos != que_rdPos) //override protection
    {
		uint8_t hlpQuePtr = (que_rdPos + 1) % queInjBufferSize;
		switch (transmitQueue[hlpQuePtr].lnData[0])
		{
			case 0xBD : ; //OPC_SW_ACK
			case 0xB0 : //OPC_SW_REQ
				sprintf(txMsg, "{\"Cmd\":\"LN\",\"Prm\":{\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", transmitQueue[hlpQuePtr].lnData[0], ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F) << 7) + transmitQueue[hlpQuePtr].lnData[1], ((transmitQueue[hlpQuePtr].lnData[2] & 0x30) >> 4));
//				Serial.println(txMsg);
				write(txMsg);
			break;
			case 0xB2: //OPC_INPUT_REP
				sprintf(txMsg, "{\"Cmd\":\"LN\",\"Prm\":{\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", transmitQueue[hlpQuePtr].lnData[0], ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F) << 8) + (transmitQueue[hlpQuePtr].lnData[1] << 1) + ((transmitQueue[hlpQuePtr].lnData[2] & 0x20) >> 5), ((transmitQueue[hlpQuePtr].lnData[2] & 0x10) >> 4));
//				Serial.println(txMsg);
				write(txMsg);
			break;
			case 0xE5 : //OPC_PEER_XFER
				if ((transmitQueue[hlpQuePtr].lnData[1] == 0x10) && ((transmitQueue[hlpQuePtr].lnData[5] & 0xF0) == 0x10) && ((transmitQueue[hlpQuePtr].lnData[10] & 0xF0) == 0x10))
				{
					uint16_t svDest = transmitQueue[hlpQuePtr].lnData[6] + (transmitQueue[hlpQuePtr].lnData[7] << 8) + ((transmitQueue[hlpQuePtr].lnData[5] & 0x01) << 7) + ((transmitQueue[hlpQuePtr].lnData[5] & 0x02) << 14);
					uint16_t svAddr = transmitQueue[hlpQuePtr].lnData[8] + (transmitQueue[hlpQuePtr].lnData[9] << 8) + ((transmitQueue[hlpQuePtr].lnData[5] & 0x04) << 5) + ((transmitQueue[hlpQuePtr].lnData[5] & 0x08) << 12);
					uint8_t svCmd = transmitQueue[hlpQuePtr].lnData[3];
					uint8_t svData[4];
					svData[0] = transmitQueue[hlpQuePtr].lnData[11] + ((transmitQueue[hlpQuePtr].lnData[10] & 0x01) << 7);
					svData[1] = transmitQueue[hlpQuePtr].lnData[12] + ((transmitQueue[hlpQuePtr].lnData[10] & 0x02) << 6);
					svData[2] = transmitQueue[hlpQuePtr].lnData[13] + ((transmitQueue[hlpQuePtr].lnData[10] & 0x04) << 5);
					svData[3] = transmitQueue[hlpQuePtr].lnData[14] + ((transmitQueue[hlpQuePtr].lnData[10] & 0x08) << 4);
//					Serial.printf("SV Command %i %i %i %i %i %i %i\n", svDest, svAddr, svCmd, svData[0], svData[1], svData[2], svData[3]);
					switch (svCmd)
					{
						case 0x01: //write 1 byte
							{
								sprintf(txMsg, "{\"Cmd\":\"SV\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", svDest, svCmd, svAddr, svData[0]);
//								Serial.println(txMsg);
								write(txMsg);
							}
							break;
						case 0x02: //read 1 byte
							{
								sprintf(txMsg, "{\"Cmd\":\"SV\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Addr\":%i}}\n", svDest, svCmd, svAddr);
//								Serial.println(txMsg);
								write(txMsg);
							}
							break;
						case 0x05: //write 4 bytes
							{
								sprintf(txMsg, "{\"Cmd\":\"SV\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", svDest, svCmd, svAddr, svData[0], svData[1], svData[2], svData[3]);
//								Serial.println(txMsg);
								write(txMsg);
							}
							break;
						case 0x06: //read 4 bytes
							{
								sprintf(txMsg, "{\"Cmd\":\"SV\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Addr\":%i}}\n", svDest, svCmd, svAddr);
//								Serial.println(txMsg);
								write(txMsg);
							}
							break;
						case 0x07: //discover
						case 0x08: //identify
							{
								sprintf(txMsg, "{\"Cmd\":\"SV\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Vals\":[%i]}}\n", svDest, svCmd, 0);
//								Serial.println(txMsg);
								write(txMsg);
							}
							break;
						case 0x09: //change LN address
							{
								sprintf(txMsg, "{\"Cmd\":\"SV\",\"Prm\":{\"Dst\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n", svAddr, svCmd, svDest, svData[0], svData[1], svData[2], svData[3]);
//								Serial.println(txMsg);
								write(txMsg);
							}
							break;
						case 0x71: //universal input device
							{
								if (svData[0] == 1)
								{
									sprintf(txMsg, "{\"Cmd\":\"LN\",\"Prm\":{\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i]}}\n", svCmd, svAddr, svData[0], svData[1]);
//									Serial.println(txMsg);
									write(txMsg);
								}
								else
									return;
							}
					}
				}
			break;
			case 0xED: //OPC_IMM_PACKET
//				Serial.println("OPC_IMM_PACKET");
				uint8_t dccData[4] = {0,0,0,0};
				uint8_t byteCtr = 0;
				uint8_t bitMask = 0x01;
				uint8_t msgLen = (transmitQueue[hlpQuePtr].lnData[3] & 0x70)>>4;
				for (uint8_t i = 0; i < msgLen; i++)
				{
					dccData[byteCtr] = transmitQueue[hlpQuePtr].lnData[5+i] | ((transmitQueue[hlpQuePtr].lnData[4] & bitMask) << (7-i));
					bitMask = bitMask << 1;
					byteCtr++;
				}
				if (byteCtr >= 2)
				{
					if ((dccData[0] > 0x7F) && (dccData[0] <= 0xBF)) //Basic Accessory Decoders with 9 bit and Extended Accessory 
                                                            //Decoders with 11-bit addresses
					{
						if (byteCtr == 2)
							return; // "Basic Accessory Decoder";
						else
						{
							uint8_t baseAddr = (((~dccData[1]) & 0x70) << 2) | ( dccData[0] & 0x3F);
							uint8_t groupAddr = (dccData[1] & 0x06) >> 1;
							uint8_t pairAddr = dccData[1] & 0x01;
							uint8_t accStatus = (dccData[1] & 0x08)>>3;
							uint16_t mastAddr = (4 * baseAddr) + groupAddr - 2; //sigOffset;
							sprintf(txMsg, "{\"Cmd\":\"LN\",\"Prm\":{\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n", transmitQueue[hlpQuePtr].lnData[0], mastAddr, (dccData[2] & 0x1F));
//							Serial.println(txMsg);
							write(txMsg);
						}
					}
				}
 			break;
		}
		que_rdPos = hlpQuePtr;
//		Serial.printf("sent %i %i\n", que_rdPos, que_wrPos);
	}
}
