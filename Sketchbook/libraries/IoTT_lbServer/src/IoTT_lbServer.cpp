/*

TCP Access library to send LocoNet commands to an lbServer or act as lbServer
* 
* The library makes use of two topics to deal with LocoNet commands:
* 
* Broadcast command (default is lnIn) is used for regular message flow. A Loconet device within the MQTT network can send a message using the broadcast topic to send it
* to the gateway. The gateway will send it to LocoNet. 
* 
* Echo topic (default is nlEcho) is used by teh gateway to send confirmation that a received message has been sent to the gateway. This way any application can have positive confirmation that a 
* sent message was indeed sent to Loconet.

Concept and implementation by Hans Tanner. See https://youtu.be/e6NgUr4GQrk for more information
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

#include <IoTT_lbServer.h>

#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())



//cbFct lbsCallback = NULL;

void handleTopNewClient(void* arg, AsyncClient* client)
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleNewClient(client);
}

void handleTopConnect(void *arg, AsyncClient *client)
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleConnect(client);
}

void handleTopDisconnect(void* arg, AsyncClient* client) 
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleDisconnect(client);
}

void handleTopError(void* arg, AsyncClient* client, int8_t error) 
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleError(client, error);
}

void handleTopDataFromClient(void* arg, AsyncClient* client, void *data, size_t len) 
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleDataFromClient(client, data, len);
}

void handleTopDataFromServer(void* arg, AsyncClient* client, void *data, size_t len) 
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleDataFromServer(client, data, len);
}

void handleTopTimeOut(void* arg, AsyncClient* client, uint32_t time) 
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleTimeOut(client, time);
}

void handleTopLNPoll(void *arg, AsyncClient *client)        //every 125ms when connected
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleLNPoll(client);
}

void handleTopWIPoll(void *arg, AsyncClient *client)        //every 125ms when connected
{
	IoTT_LBServer * thisServer = (IoTT_LBServer*)arg;
	thisServer->handleWIPoll(client);
}

void wiAddrCallback(std::vector<ppElement>* ppList)
{
	if (wiServer != NULL)
		wiServer->wiAddrCallback(ppList);
}

tcpDef::tcpDef()
{
//	Serial.println("Create tcpDef");
}

tcpDef::~tcpDef()
{
	if (thisClient)
		delete(thisClient);
	if(wiHWIdentifier) //potential memory leak
		free(wiHWIdentifier);
	if(wiDeviceName)
		free(wiDeviceName);
	slotList.clear();
}

void tcpDef::addLoco(uint16_t locoAddr, char thID)
{
//	Serial.printf("Add Loco %i\n", locoAddr);
	lnTransmitMsg txData;
	locoDef * thisLoco = getLocoByAddr(NULL, locoAddr, thID);
	if (!thisLoco)
	{
		locoDef newLoco;
//		if (useExCmd == 0)
			newLoco.throttleID = thID;
//		else
//			newLoco.throttleID = slotList.size();
		newLoco.locoAddr = locoAddr;
		newLoco.slotStatus = 0;
		newLoco.activeSlot = false;
		slotList.push_back(newLoco);
	}
	prepLocoAddrReqMsg(&txData, locoAddr);
	sendMsg(txData);
}

void tcpDef::removeLoco(uint16_t locoAddr, char thID)
{
//	Serial.printf("Remove Loco %i from throttle %c\n", locoAddr, thID);
	char replBuf[20] = {'\0'};
	lnTransmitMsg txData;
	if (locoAddr == 0xFFFF) //for all
	{
//		Serial.println(slotList.size());
		for (int8_t i = (slotList.size()-1); i >= 0;  i--)
		{
			if ((slotList[i].throttleID == thID) || (thID == '*'))
			{
//				Serial.printf("Remove Loco %i\n", i);
				slotList[i].activeSlot = false;
				prepSlotStat1Msg(&txData, slotList[i].slotNum, 0x13);
				sendMsg(txData);
				slotList.erase(slotList.begin() + i);
			}
		}
		sprintf(replBuf, "M%c-*<;>r\r\n", thID);
	}
	else
	{
		locoDef * thisLoco = getLocoByAddr(NULL, locoAddr, thID);
		if ((thisLoco) && ((thisLoco->throttleID == thID) || (thID == '*')))
		{
			prepSlotStat1Msg(&txData, thisLoco->slotNum, 0x13);
			sendMsg(txData);
			sprintf(replBuf, "M%c-%c%i<;>r\r\n", thID, thisLoco->locoAddr > 127 ? 'L' : 'S', thisLoco->locoAddr);
			for (uint8_t i = 0; i < slotList.size(); i++)	
				if (&slotList[i] == thisLoco)
				{	
					slotList.erase(slotList.begin() + i);
					break;
				}
		}
	}
	if (strlen(replBuf) > 0)
		if (thisClient->space() > strlen(replBuf)+2)
		{
			thisClient->add(replBuf, strlen(replBuf));
			thisClient->send();
		}
}

void tcpDef::confirmLoco(uint8_t slotAddr, uint16_t locoAddr, char thID, uint8_t slotStat, uint8_t slotSpeed, uint16_t dirFctFlags)
{
	char replBuf[20] = {'\0'};
	locoDef * thisLoco = getLocoByAddr(NULL, locoAddr, thID);
	if (thisLoco)
	{
		currSpeed = slotSpeed;
		if (thisLoco->activeSlot) 
			return;
		thisLoco->slotNum = slotAddr;
		thisLoco->dirFct = dirFctFlags;
		thisLoco->slotStatus = slotStat;
		bool consistCheck = (slotStat & 0x40) == 0; //not a submember in a consist
		if (!consistCheck)
			return;
		uint8_t refreshStat = (slotStat & 0x30) >> 4;
		//if loco is assigned, reply with steal request, otherwise, assign it
//		Serial.printf("Ref: %i\n", refreshStat);
		if (refreshStat == 0x03) //in Use
			sprintf(replBuf, "M%cS%c%i<;>\r\n", thisLoco->throttleID, thisLoco->locoAddr > 127 ? 'L' : 'S', thisLoco->locoAddr);
		else
		{
			thisLoco->activeSlot = true;
			sprintf(replBuf, "M%c+%c%i<;>\r\n", thisLoco->throttleID, thisLoco->locoAddr > 127 ? 'L' : 'S', thisLoco->locoAddr);
			lnTransmitMsg txData;
			prepSlotMoveMsg(&txData, slotAddr, slotAddr);
			sendMsg(txData);
		}
//		Serial.println(replBuf);
		if (thisClient->space() > strlen(replBuf)+2)
		{
			thisClient->add(replBuf, strlen(replBuf));
			thisClient->send();
		}
	}
//	else
//		Serial.println("not found");
}

void tcpDef::stealLoco(uint16_t locoAddr, char thID)
{
	char replBuf[20] = {'\0'};
	locoDef * thisLoco = getLocoByAddr(NULL, locoAddr, thID);
	if (thisLoco)
	{
		if (thisLoco->activeSlot) return;
		bool consistCheck = (thisLoco->slotStatus & 0x40) == 0; //not a submember in a consist
		if (!consistCheck) return;
		sprintf(replBuf, "M%c+%c%i<;>\r\n", thisLoco->throttleID, thisLoco->locoAddr > 127 ? 'L' : 'S', thisLoco->locoAddr);
		thisLoco->activeSlot = true;
		lnTransmitMsg txData;
		prepSlotMoveMsg(&txData, thisLoco->slotNum, thisLoco->slotNum);
		sendMsg(txData);
//		Serial.println(replBuf);
		if (thisClient->space() > strlen(replBuf)+2)
		{
			thisClient->add(replBuf, strlen(replBuf));
			thisClient->send();
		}
	}
}

void tcpDef::setTurnout(char pos, char* addr)
{
	lnTransmitMsg txData;
	uint16_t swiAddr = strtol(addr, NULL, 10);
	uint8_t swiPos = 2;//toggle
	if (pos == 'C') swiPos = 1;
	if (pos == 'T') swiPos = 0;
	prepTurnoutMsg(&txData, false, swiAddr, swiPos);
	sendMsg(txData);
}

locoDef* tcpDef::getLocoByAddr(locoDef* startAt, uint16_t locoAddr, char thID)
{
	bool trigOK = (startAt == NULL);
	for (uint8_t i = 0; i < slotList.size(); i++)
	{
		if (!trigOK)
			trigOK = &slotList[i] == startAt;
		else
			if ((slotList[i].locoAddr == locoAddr) && ((slotList[i].throttleID == thID) || (thID == '*')))
				return &slotList[i];
	}
	return NULL;
}

locoDef* tcpDef::getLocoBySlot(locoDef* startAt, uint8_t slotAddr, char thID)
{
	bool trigOK = (startAt == NULL);
	for (uint8_t i = 0; i < slotList.size(); i++)
	{
		if (!trigOK)
			trigOK = &slotList[i] == startAt;
		else
			if ((slotList[i].slotNum == slotAddr) && ((slotList[i].throttleID == thID) || (thID == '*')))
				return &slotList[i];
	}
	return NULL;
}

locoDef* tcpDef::getLocoFromList(locoDef* startAt, char thID)
{
	bool trigOK = (startAt == NULL);
	for (uint8_t i = 0; i < slotList.size(); i++)
	{
		if (!trigOK)
			trigOK = &slotList[i] == startAt;
		else
			if ((slotList[i].throttleID == thID) || (thID == '*'))
				return &slotList[i];
	}
	return NULL;
}

void tcpDef::setLocoAction(uint16_t locoAddr, char thID, const char* ActionCode, uint16_t extFctMask)
{
//	Serial.printf("Loco Action Throttle %c Addr %i %s\n", thID, locoAddr, ActionCode);
	char replBuf[800] = {'\0'};
	char* nextBuf = &replBuf[0]; 
	locoDef * thisLoco = NULL;
	if (locoAddr != 0xFFFF)
		thisLoco = getLocoByAddr(NULL, locoAddr, thID);
	const char* paramStr  = &ActionCode[1];
	uint8_t paramVal = 0;
	lnTransmitMsg txData;
	bool forceVal = false;
	switch (ActionCode[0])
	{
		case 'f': forceVal = true; //force Function F
		case 'F':
			{
				paramVal = ActionCode[1] - '0';
				uint8_t fctNum = strtol(&ActionCode[2], NULL, 10);
				uint16_t fctMask = 0;
				
				if (fctNum == 0) fctMask = 0x1000;
				else if (fctNum <= 4) fctMask = (0x0100 << (fctNum - 1));
				else if (fctNum <= 8) fctMask = (0x0001 << (fctNum - 5));
				else return;

				if ((thisLoco) && ((thisLoco->throttleID == thID) || (thID == '*')))
				{
					while (thisLoco)
					{
						bool isMomentary = (fctMask & thisLoco->noLatchFct) || forceVal;
						if ((paramVal == 0) && (!isMomentary))
						{
							thisLoco = getLocoByAddr(thisLoco, locoAddr, thID);
							continue; //no button release action for latching function)
						}
						uint16_t currFct = (digitraxBuffer->slotBuffer[thisLoco->slotNum][3] << 8) + digitraxBuffer->slotBuffer[thisLoco->slotNum][7];
						if (isMomentary)
							currFct = (paramVal != 0) ? (currFct | fctMask) : (currFct & (~fctMask));
						else
							currFct = (currFct ^ fctMask);
						currFct &= 0x7F7F; //clear leading bits
						if (fctNum <= 4)
							prepSlotDirFMsg(&txData, thisLoco->slotNum, currFct >> 8);
						else if (fctNum <= 8)
							prepSlotSndMsg(&txData, thisLoco->slotNum, currFct & 0x7F);
						else 
						{
							thisLoco = getLocoByAddr(thisLoco, locoAddr, thID);
							continue; //no button release action for latching function)
						}
//					Serial.printf("Num: %i Stat: %i Mask: 0x%2X Fct Val: 0x%2X\n", fctNum, paramVal, fctMask, thisLoco->dirFct);
						sendMsg(txData);
						thisLoco = getLocoByAddr(thisLoco, locoAddr, thID);
					}
				}
				else
					for (uint8_t i = 0; i < slotList.size(); i++)
					{
						if ((slotList[i].throttleID == thID) || (thID == '*'))
						{
							bool isMomentary = (fctMask & slotList[i].noLatchFct) || forceVal;
							if ((paramVal == 0) && (!isMomentary))
								continue; //no button release action for latching function)
							uint16_t currFct = (digitraxBuffer->slotBuffer[slotList[i].slotNum][3] << 8) + digitraxBuffer->slotBuffer[slotList[i].slotNum][7];
							if (isMomentary)
								currFct = (paramVal != 0) ? (currFct | fctMask) : (currFct & (~fctMask));
							else
								currFct = (currFct ^ fctMask);
							currFct &= 0x7F7F; //clear leading bits
							if (fctNum <= 4)
								prepSlotDirFMsg(&txData, slotList[i].slotNum, currFct >> 8);
							else if (fctNum <= 8)
								prepSlotSndMsg(&txData, slotList[i].slotNum, currFct & 0x7F);
							else continue;
//							Serial.printf("Addr: %i Num: %i Stat: %i Mask: 0x%2X Fct Val: 0x%2X\n", slotList[i].locoAddr, fctNum, paramVal, fctMask, slotList[i].dirFct);
							sendMsg(txData);
						}
					}
			}
			break;
		case 'q':
			{
				for (uint8_t i = 0; i < slotList.size(); i++)
				{
					if ((slotList[i].throttleID == thID) || (thID == '*'))
					{
						slotData * thisSlot = digitraxBuffer->getSlotData(slotList[i].slotNum);
						if (!thisSlot) return;
//						Serial.printf("Slots %i:  %2x %2X\n", slotList[i].slotNum, (*thisSlot)[2],(*thisSlot)[3]);
						switch (ActionCode[1])
						{
							case 'V': 
									sprintf(replBuf, "M%cA%c%i<;>V%i\r\n", slotList[i].throttleID, slotList[i].locoAddr > 127 ? 'L' : 'S', slotList[i].locoAddr, (*thisSlot)[2]); 
									break;
							case 'R': 
								{
									uint16_t currFct = (digitraxBuffer->slotBuffer[slotList[i].slotNum][3] << 8) + digitraxBuffer->slotBuffer[slotList[i].slotNum][7];
//									Serial.printf("Curr Dir %i\n", currFct & 0x2000);
									uint16_t fctChgMask = currFct ^ slotList[i].dirFct;
//									Serial.printf("Fct: %2X Curr: %2X Chg: %2X\n", currFct, slotList[i].dirFct, fctChgMask);
									if (fctChgMask & 0x2000)
										slotList[i].dirFct ^= 0x2000;
//									Serial.printf("Return Dir %i\n", slotList[i].dirFct & 0x2000);
//									sprintf(replBuf, "M%cA%c%i<;>R%i\r\n", slotList[i].throttleID, slotList[i].locoAddr > 127 ? 'L' : 'S', slotList[i].locoAddr, (((*thisSlot)[3] & 0x20) >> 5) ^ 0x01); 
									sprintf(replBuf, "M%cA%c%i<;>R%i\r\n", slotList[i].throttleID, slotList[i].locoAddr > 127 ? 'L' : 'S', slotList[i].locoAddr, ((slotList[i].dirFct & 0x2000) >> 13) ^ 0x01);
								}
								break;
							case 'F': 
								{
//									uint16_t currFct = slotList[i].dirFct;
									uint16_t currFct = (digitraxBuffer->slotBuffer[thisLoco->slotNum][3] << 8) + digitraxBuffer->slotBuffer[thisLoco->slotNum][7];
									uint16_t fctChgMask = (currFct ^ slotList[i].dirFct);
									if (extFctMask) 
										fctChgMask = (extFctMask & 0x1F0F);
									uint16_t bitMaskF = 0x0100;
									uint16_t bitMaskS = 0x0001;
//									Serial.printf("Fct: %2X Curr: %2X Chg: %2X\n", currFct, slotList[i].dirFct, fctChgMask);
									if (fctChgMask)
									{
										if (fctChgMask & 0x1000)
											nextBuf += sprintf(nextBuf, "M%cA%c%i<;>F%i%i\r\n", slotList[i].throttleID, slotList[i].locoAddr > 127 ? 'L' : 'S', slotList[i].locoAddr, (currFct & 0x1000)>>12, 0);
										for (uint8_t j = 0; j < 4; j++)
										{
											if (fctChgMask & bitMaskF)
												nextBuf += sprintf(nextBuf, "M%cA%c%i<;>F%i%i\r\n", slotList[i].throttleID, slotList[i].locoAddr > 127 ? 'L' : 'S', slotList[i].locoAddr, (currFct & bitMaskF)>>(j+8), j+1);
											if (fctChgMask & bitMaskS)
												nextBuf += sprintf(nextBuf, "M%cA%c%i<;>F%i%i\r\n", slotList[i].throttleID, slotList[i].locoAddr > 127 ? 'L' : 'S', slotList[i].locoAddr, (currFct & bitMaskS)>>j, j+5);
											bitMaskF <<= 1;
											bitMaskS <<= 1;
										}
											
									}
									slotList[i].dirFct = currFct;
								}
								break;
							default: Serial.println("No code"); return;
						}
					}
				}
				if (thisClient->space() > strlen(replBuf))
				{
//					Serial.print(thisClient->remoteIP());
//					Serial.print(" Out: ");
//					Serial.println(strlen(replBuf));
					thisClient->add(replBuf, strlen(replBuf));
					thisClient->send();
				}
			}
			break;
		case 'R':
			{
				paramVal = strtol(paramStr, NULL, 10);
				uint16_t targetFct = 0;
				if ((thisLoco) && ((thisLoco->throttleID == thID) || (thID == '*')))
				{
//					Serial.printf("Curr Dir %i to %i\n", thisLoco->dirFct & 0x2000, paramVal);
					targetFct = ((paramVal == 0) ? (thisLoco->dirFct | 0x2000) : (thisLoco->dirFct & ~0x2000));
					targetFct &= 0x7F7F; //clear leading bits
//					Serial.printf("Set Dir %i\n", targetFct & 0x2000);
					prepSlotDirFMsg(&txData, thisLoco->slotNum, targetFct >> 8);
					sendMsg(txData);
				}
				else
					for (uint8_t i = 0; i < slotList.size(); i++)
					{
						if ((slotList[i].throttleID == thID) || (thID == '*'))
						{
							targetFct = paramVal == 0 ? (slotList[i].dirFct | 0x2000) : (slotList[i].dirFct & ~0x2000);
							targetFct &= 0x7F7F; //clear leading bits
							prepSlotDirFMsg(&txData, slotList[i].slotNum, targetFct >> 8);
							sendMsg(txData);
						}
					}
			}
			break;
		case 'V':
			{
				paramVal = strtol(paramStr, NULL, 10);
				currSpeed = paramVal;
				if ((thisLoco) && ((thisLoco->throttleID == thID) || (thID == '*')))
				{
					prepSlotSpeedMsg(&txData, thisLoco->slotNum, paramVal);
					sendMsg(txData);
				}
				else
					for (uint8_t i = 0; i < slotList.size(); i++)
					{
						if ((slotList[i].throttleID == thID) || (thID == '*'))
						{
							prepSlotSpeedMsg(&txData, slotList[i].slotNum, paramVal);
							sendMsg(txData);
						}
					}
			}
			break;
	}

}

void tcpDef::setTrackPowerStatus(uint8_t newStatus)
{
	lnTransmitMsg txData;
	prepTrackPowerMsg(&txData, newStatus);
	sendMsg(txData);
}

/*
void tcpDef::initReadCV(uint8_t progMode, uint8_t progMethod, uint16_t dccAddr, uint16_t cvNr)
{
	lnTransmitMsg txData;
	readProg(dccAddr, progMode, progMethod, cvNr);
	sendMsg(txData);
}

void tcpDef::writeReadCV(uint8_t progMode, uint8_t progMethod, uint16_t dccAddr, uint16_t cvNr, uint8_t cvVal)
{
	lnTransmitMsg txData;
	writeProg(dccAddr, progMode, progMethod, cvNr, cvVal);
	sendMsg(txData);
}
*/

uint8_t tcpDef::getThrottleIDList(char * addHere)
{
	uint8_t numTh = 0;
	for (uint8_t i = 0; i < slotList.size(); i++)
	{
		if (strchr(addHere, slotList[i].throttleID) == NULL)
		{
			addHere[numTh] = slotList[i].throttleID;
			numTh++;
			addHere[numTh] = 0;
		}
	}
	return numTh;
}

/*
'C' consist
'c' consist lead from roster entry
'd' dispatch.
'E' sets an address from a roster entry. Format EID.
'F' function key. Format F{0|1}Function.
'f' force function. Format f{0|1}Function.
'I' idle, which sets the speed to 0
'L' sets a long DCC address.
'm' momentary. Format m{0|1}Function.
'q' ask for current settings, such as speed or direction
'Q' quit
'R' set direction. Format R{0|1}.
'r' release.
'S' sets a short DCC address.
's' set speed step mode
'V' sets the speed (velocity). Format Vspeed.
'X' emergency stop.
*/
 
IoTT_LBServer::IoTT_LBServer()
{
//	setCallback(psc_callback);
	nextPingPoint = millis() + pingInterval - random(500);
}

IoTT_LBServer::~IoTT_LBServer()
{
	clients.clear();
	if (lntcpServer)
		delete(lntcpServer);
	if (lntcpClient)
	{
		lntcpClient->close();
		delete(lntcpClient);
	}
}

IoTT_LBServer::IoTT_LBServer(Client& client)
{
//	setCallback(psc_callback);
	nextPingPoint = millis() + pingInterval - random(500);
}

void IoTT_LBServer::initLBServer(bool serverMode)
{
	isServer = serverMode;
	if (isServer)
	{
		Serial.println("Init lb Server");
		lntcpServer = new AsyncServer(WiFi.localIP(), lbs_ServerPort);
		lntcpServer->onClient(&handleTopNewClient, this);
	}
	else //client mode
	{
		Serial.println("Init lb Client");
		lntcpClient = new AsyncClient();
		lntcpClient->onData(handleTopDataFromClient, this);
		lntcpClient->onConnect(handleTopConnect, this);
		lntcpClient->onPoll(handleTopLNPoll, this);
	}
}

void IoTT_LBServer::initMDNS()
{
	if (MDNS.begin(WiFi.getHostname())) 
	{
		Serial.println("mDNS responder started");
		// Add service to MDNS-SD
        MDNS.addService("withrottle", "tcp", lbs_Port);
//        Serial.println("mDNS WiThrottle Service added");
	}
	else
		Serial.println("Error setting up MDNS responder");
}

void IoTT_LBServer::initWIServer(bool serverMode)
{
	isServer = serverMode;
	isWiThrottle = true;
	if (isServer) //add code here to support server mode
	{
		Serial.println("Init WiThrottle Server");
		lntcpServer = new AsyncServer(WiFi.localIP(), lbs_ServerPort);
		lntcpServer->onClient(&handleTopNewClient, this);
	}
	else //client mode
	{
		Serial.println("Init WiThrottle Client");
		lntcpClient = new AsyncClient();
		lntcpClient->onData(handleTopDataFromClient, this);
		lntcpClient->onConnect(handleTopConnect, this);
		lntcpClient->onPoll(handleTopWIPoll, this);
	}
}

void IoTT_LBServer::startServer()
{
	if (isServer && lntcpServer)
	{
		lntcpServer->begin();
		if (isWiThrottle)
			initMDNS();
	}
}

void IoTT_LBServer::handleNewClient(AsyncClient* client)
{
	Serial.printf("A new %s client has been connected to server, ip: %s with timeout %i %i\n", isWiThrottle ? "WiThrottle" : "LN TCP", client->remoteIP().toString().c_str(), client->getAckTimeout(), client->getRxTimeout());
//	Serial.println(String(ESP.getFreeHeap()));
	tcpDef * newClientData;
	newClientData = new tcpDef();
	// add to list
	newClientData->thisClient = client;
//	newClientData->sendInitSeq = numInitSeq; //currently 10 init messages
	clients.push_back(newClientData);
	Serial.printf("New total is %i client(s)\n", clients.size());
//	delay(100);
	for (int i = 0; i < clients.size(); i++)
	{
		Serial.println(clients[i]->thisClient->remoteIP());
	}
	// register events
	client->onData(&handleTopDataFromServer, this);
	client->onError(&handleTopError, this);
	client->onDisconnect(&handleTopDisconnect, this);
	client->onTimeout(&handleTopTimeOut, this);
//	Serial.println(String(ESP.getFreeHeap()));
	updateClientList();
	if (!isWiThrottle)
		sendLNClientText(client, "VERSION", LNTCPVersion);
}

 /* clients events */
void IoTT_LBServer::handleError(AsyncClient* client, int8_t error) 
{
	Serial.printf("\n%s\n", client->errorToString(error));
}

void IoTT_LBServer::handleDisconnect(AsyncClient* client) 
{
	Serial.println("Delete Client");
//	Serial.println(String(ESP.getFreeHeap()));
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i]->thisClient == client)
		{
//			if (clients[i]->wiHWIdentifier)
//				free(clients[i]->wiHWIdentifier);
//			if (clients[i]->wiDeviceName)
//				free(clients[i]->wiDeviceName);
			delete(clients[i]);
			clients.erase(clients.begin() + i);
			break;
		}
	}
	Serial.printf("Client disconnected. %i clients remaining \n", clients.size());
	updateClientList();
//	Serial.println(String(ESP.getFreeHeap()));
	yield();
}

void IoTT_LBServer::handleTimeOut(AsyncClient* client, uint32_t time) 
{
  Serial.printf("Client ACK timeout ip: %s for %ims\n", client->remoteIP().toString().c_str(), time);
  client->close(true);
}

/*
void IoTT_LBServer::setLNCallback(cbFct newCB)
{
	lbsCallback = newCB;
}
*/

void IoTT_LBServer::loadLBServerCfgJSON(DynamicJsonDocument doc)
{
	if (doc.containsKey("PortNr"))
		lbs_Port = doc["PortNr"];
	if (doc.containsKey("PortNr"))
		lbs_ServerPort = doc["ServerPortNr"];
	if (doc.containsKey("ServerIP"))
	{
		String thisIP = doc["ServerIP"];
		lbs_IP.fromString(thisIP);
	}
	if (doc.containsKey("PowerMode"))
		allowPwrChg = doc["PowerMode"];
	if (doc.containsKey("DefinitionSource"))
		defSource = doc["DefinitionSource"];
	if (defSource == 0) //load local lists
	{
		if (doc.containsKey("Turnouts"))
		{
			JsonArray turnoutList = doc["Turnouts"];
			for (uint16_t i = 0; i < turnoutList.size(); i++)
			{
				turnoutSupport.push_back((uint16_t) turnoutList[i]);
			}
		}
		if (doc.containsKey("Locos"))
		{
			JsonArray locoList = doc["Locos"];
			for (uint16_t i = 0; i < locoList.size(); i++)
			{
				locoSupport.push_back((uint16_t) locoList[i]);
			}
		}
	}


//	if (doc.containsKey("UpdateFC"))
//		fcUpdate = doc["UpdateFC"];
}

uint8_t IoTT_LBServer::getConnectionStatus()
{
	if (isServer)
		return clients.size();
	else
		if (lntcpClient->connected())
			return 1;
		else
			return 0;
}

uint16_t IoTT_LBServer::lnWriteMsg(lnTransmitMsg* txData)
{
//	Serial.printf("LN over TCP Tx %02X\n", txData.lnData[0]);
	uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].msgType = txData->msgType;
		transmitQueue[hlpQuePtr].lnMsgSize = txData->lnMsgSize;
		transmitQueue[hlpQuePtr].requestID = txData->requestID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData->lnData, lnMaxMsgSize); //txData.lnMsgSize);
		transmitQueue[hlpQuePtr].reqRespTime = 0;
		transmitQueue[hlpQuePtr].echoTime = 0;
		transmitQueue[hlpQuePtr].errorFlags = 0;
		que_wrPos = hlpQuePtr;
		return txData->lnMsgSize;
	}
	else
	{	
		Serial.println("LN over TCP Write Error. Too many messages in queue");
		return -1;
	}
}

uint16_t IoTT_LBServer::lnWriteMsg(lnReceiveBuffer* txData)
{
// 	Serial.printf("LN TCP Tx %02X", txData.lnData[0]);
// 	for (int i = 1; i < txData.lnMsgSize; i++)
//		Serial.printf(" %02X", txData.lnData[i]);
//	Serial.println();
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].msgType = txData->msgType;
		transmitQueue[hlpQuePtr].lnMsgSize = txData->lnMsgSize;
		transmitQueue[hlpQuePtr].requestID = txData->requestID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData->lnData, lnMaxMsgSize); //txData.lnMsgSize);
		transmitQueue[hlpQuePtr].reqRespTime = txData->reqRespTime;
		transmitQueue[hlpQuePtr].echoTime = txData->echoTime;
		transmitQueue[hlpQuePtr].errorFlags = txData->errorFlags;
		que_wrPos = hlpQuePtr;
		return txData->lnMsgSize;
	}
	else
	{	
		Serial.println("TCP Write Error. Too many messages in queue");
		return -1;
	}
}

void IoTT_LBServer::handleDataFromServer(AsyncClient* client, void *data, size_t len) 
{
	//identify client
	tcpDef * currClient = NULL;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i]->thisClient == client)
		{
//			Serial.print("Message from ");
//			Serial.println(clients[i].thisClient->remoteIP());
//			Serial.write((uint8_t *)data, len);
//			Serial.println();
			currClient = clients[i];
			break;
		}
	}
	if (currClient)
	{
		if ((((char*)data)[len-1] == '\n') || (((char*)data)[len-1] == '\r'))
		{
			//if command is complete, call handle data
			handleData(currClient->thisClient, (char*) data, len);
		}
	}
}

void IoTT_LBServer::handleDataFromClient(AsyncClient* client, void *data, size_t len) 
{
//	Serial.println("handleDataFromClient");
//	Serial.write((uint8_t *)data, len);
//	Serial.println(len);
	if (lntcpClient == client) 
	{
		
		if ((((char*)data)[len-1] == '\n') || (((char*)data)[len-1] == '\r'))
		{
			//if command is complete, call handle data
//			lntcpClient.rxBuffer[lntcpClient.rxPtr] = '\0';
			handleData(lntcpClient, (char*) data, len);
//			lntcpClient.rxPtr = 0;
		}
		else
			Serial.println("No CRLF in data");
	}
	else
		Serial.println("Not for us");
}


//this is called when data is received, either in server or client mode
void IoTT_LBServer::handleData(AsyncClient* client, char *data, size_t len)
{
//	if (isWiThrottle)
//		Serial.println(data);
	char *p = data;
    char *subStr;
    char *strEnd = data + len - 1;
    if (strchr(p, '\n') != NULL)
		while ((subStr = strtok_r(p, "\n", &p)) != NULL) // delimiter is the new line
		{			
			while(((*subStr=='\n') || (*subStr=='\r') || (*subStr=='\'')) && (subStr < strEnd))
				subStr++;
			if (subStr < strEnd)
				if (isWiThrottle)
					processWIMessage(client, subStr);
				else
					processLNServerMessage(client, subStr);
		}
	else
		if (strchr(p, '\r') != NULL)
			while ((subStr = strtok_r(p, "\r", &p)) != NULL) // delimiter is the carriage return
			{
				while(((*subStr=='\n') || (*subStr=='\r') || (*subStr=='\'')) && (subStr < strEnd))
					subStr++;
				if (subStr < strEnd)
				if (isWiThrottle)
					processWIMessage(client, subStr);
				else
					processLNServerMessage(client, subStr);
			}
		else
			Serial.println("No delimiter");

}

void IoTT_LBServer::handleLNPoll(AsyncClient *client)        //every 125ms when connected
{
//	nextPingPoint = millis() + pingInterval;
}

void IoTT_LBServer::handleWIPoll(AsyncClient *client)        //every 125ms when connected
{
//	nextPingPoint = millis() + pingInterval;
}

void IoTT_LBServer::strToWI(char * str, lnReceiveBuffer * recData)
{
}

void IoTT_LBServer::tcpToLN(char * str, lnReceiveBuffer * thisData)
{
	char * p = str;
	uint8_t xorCheckByte = 0;
	while ((str = strtok_r(p, " ", &p)) != NULL) // delimiter is the space
	{
		uint8_t thisByte = strtol(str, NULL, 16) & 0x000000FF;
		thisData->lnData[thisData->lnMsgSize] = thisByte;
		xorCheckByte = xorCheckByte ^ thisByte;
		thisData->lnMsgSize++;
	}
	if (xorCheckByte != 0xFF)
	{
//		Serial.println(p);
		thisData->errorFlags = msgXORCheck;
	}
	if (isSameMsg(thisData, &transmitQueue[lastOutMsg]))
	{
		thisData->errorFlags |= msgEcho;
		thisData->requestID = transmitQueue[lastOutMsg].requestID;
	}
	transmitQueue[lastOutMsg].lnData[0] = 0;
}

bool IoTT_LBServer::processWIMessage(AsyncClient* client, char * c)
{
	while (c[0] == '\'')
		c++;
//	Serial.println(c);
	uint16_t len = strlen(c);
	if (len == 0) return false;

	if (isServer) //Server Mode, handle incoming commands from client
	{
		return processWIServerMessage(client, c);
	}
	else //client mode, handle incoming commands from server
		return processWIClientMessage(client, c);
}

//this is called from Digitrax Buffers to send loco address after CV read, including long format
void IoTT_LBServer::wiAddrCallback(std::vector<ppElement>* ppList)
{
	for (uint16_t i = 0; i < clients.size(); i++)
	{
		if (clients[i]->useExCmd > 0) //DCC EX
		{
			char outBuf[10];
			sprintf(outBuf, "r %i", ppList->at(1).payload.longVal);
			sendWIClientMessage(clients[i]->thisClient, outBuf, clients[i]->useExCmd > 0);
		}
	}
}

//this is called when data is received in server mode
bool IoTT_LBServer::processWIServerMessage(AsyncClient* client, char * c)
{
	uint16_t len = strlen(c);
	tcpDef * currClient = NULL;
//	bool isDCCEX = (strcmp(c, "<s>") == 0); 
	bool isDCCEX = (c[0] == '<'); 
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients.at(i)->thisClient == client)
		{
			currClient = clients.at(i);
			if (currClient->useExCmd < 0) //data format not determined
			{
				currClient->useExCmd = isDCCEX ? 1 : 0;
				currClient->sendInitSeq = numInitSeq; //start init sequence
			}
			break;
		}
	}
	if (currClient)
	{
		if (currClient->useExCmd > 0)
		{
			locoDef * thisLoco = NULL;
			lnTransmitMsg exInBuffer;
			std::vector<ppElement> ppList;
			exInBuffer.lnData[0] = 0;
			exInBuffer.lnMsgSize = 0;
			exInBuffer.requestID = 0;
			exInBuffer.msgType = DCCEx;
			exInBuffer.reqRecTime = 0xFF;

			uint8_t i = 0;
			while (i < len) //read GridConnect protocol and package by message
			{
				if (parseDCCExNew(&c[i], &exInBuffer, &ppList))
				{
					uint8_t paramCount = ppList.at(0).numParams;
					char opCode = ppList.at(0).payload.strVal[0];

/*
					for (uint8_t i = 0; i < paramCount; i++)
					{
						switch (ppList.at(i).dataType)
						{
							case 0:  Serial.printf("Param %i %i String %s \n", i, ppList.at(i).paramNr, ppList.at(i).payload.strPtr); break;
							case 10: Serial.printf("Param %i %i Int = %i\n", i, ppList.at(i).paramNr, ppList.at(i).payload.longVal); break;
							case 20: Serial.printf("Param %i %i Float = %.2f\n", i, ppList.at(i).paramNr, ppList.at(i).payload.floatVal); break;
							default: Serial.printf("Param %i %i Short String %s\n", i, ppList.at(i).paramNr, ppList.at(i).payload.strVal); break;
						}
					}
*/
					switch (opCode)
					{
						case '0':
						{
							if (allowPwrChg > 0)
							{
								if (allowPwrChg == 2) //on-off
									currClient->setTrackPowerStatus(0x82); //OFF
								else
									currClient->setTrackPowerStatus(0x85); //Idle
							}
//							return sendWIServerMessageStringEX(currClient->thisClient, 2); //power status
						}
						break;
						case '1': 
						{
							if (allowPwrChg > 0)
								currClient->setTrackPowerStatus(0x83); //ON
//							return sendWIServerMessageStringEX(currClient->thisClient, 2); //power status
						}
						break;
						case '#': 
						{
							if (paramCount == 1) // <#>
								return sendWIServerMessageStringEX(currClient->thisClient, 1); 
							break;
						}
						case 'J': 
						{
							if (paramCount > 1) // <JT> <JR> <JA>
								switch (ppList.at(1).payload.strVal[0])
								{
									case 'R': //Roster
									{
										if (paramCount == 2) //<JR>
											return sendWIServerMessageStringEX(currClient->thisClient, 4); 

										if (paramCount == 3) //<JR addr> request
										{
											String outStr = digitraxBuffer->getRosterInfo(ppList.at(2).payload.longVal, true);
//											Serial.println(outStr);
											return sendWIClientMessage(currClient->thisClient, outStr, true);
										}
									}
									break;
									case 'T': 
									{
//										Serial.printf("Params: %i\n", myParams[0].numParams);
										if (paramCount == 2) //<JT>
											return sendWIServerMessageStringEX(currClient->thisClient, 3); 
										if (paramCount == 3) //<JT id>
										{
											String outStr = digitraxBuffer->getTurnoutInfo(ppList.at(2).payload.longVal, true);
//											Serial.println(outStr);
											return sendWIClientMessage(currClient->thisClient, outStr, true);
										}
									}
									break; //Turnouts
									case 'A': //Automations
									{
										if (paramCount == 2) //<JA>
											return sendWIServerMessageStringEX(currClient->thisClient, 5); 
										if (paramCount == 3) //<JA id>
										{
											String outStr = digitraxBuffer->getRouteInfo(ppList.at(2).payload.longVal, true);
//											Serial.println(outStr);
											return sendWIClientMessage(currClient->thisClient, outStr, true);
										}
									}
									break;
								}
						} 
						break;
						case 'R':
						{
//							Serial.printf("R Ct %i\n", paramCount);
							if (paramCount == 1) //<R> Read Address
								digitraxBuffer->readAddrOnly();
							if (paramCount == 2) //<R cv> Read Address
								digitraxBuffer->readProg(0, 0, 0, ppList.at(1).payload.longVal);
						
						}
						break;
						case 'S':
						{
//							Serial.println("Send Sensor List");
							int numSensors = (digitraxBuffer->getSensorInfo(0xFFFF, true)).toInt();
							if (numSensors > 0)
							{
								char outStr[20] = {'\0'};
								for (uint16_t i = 0; i < numSensors; i++)
								{
//									Serial.println(digitraxBuffer->getSensorInfo(i, true));
									if (!sendWIClientMessage(currClient->thisClient, digitraxBuffer->getSensorInfo(i, true), true))
										return false;
								}
							}
						} 
						break;
						case 's': 
						{
							currClient->sendInitSeq = numInitSeq; //return sendWIServerMessageStringEX(currClient->thisClient, 0); break;
							String dispStr = String(EDExDispStr);
							currClient->wiDeviceName = (char*) realloc(currClient->wiDeviceName, dispStr.length() + 1);
							strcpy(&currClient->wiDeviceName[0], dispStr.c_str());
							updateClientList();
						} 
						break; 
/*
						case 't':
						{
							if (myParams[0].numParams == 2) //<t cab>
							{
								thisLoco = currClient->getLocoByAddr(thisLoco, myParams[1].payload.longVal, '*');
								if (thisLoco)
								{
									Serial.println("request loco info");
								}
							}
							if (myParams[0].numParams == 5) //<t reg cab speed dir>
							{
								thisLoco = currClient->getLocoByAddr(thisLoco, myParams[2].payload.longVal, '*');
								if (thisLoco)
								{
									Serial.println("Send Speed / Dir / F");
//								<l	 cab reg speedByte functMap>
								}
							}
							
						}
						break;
*/
						case 'T':
						{
							if (paramCount == 3) //<T 55 T>
							{
								char dccAddr[5];
								sprintf(dccAddr, "%i", ppList.at(1).payload.longVal);
								currClient->setTurnout(ppList.at(2).payload.strVal[0], &dccAddr[0]);
							}
						}
						break;
						case 'U':
						{
							if (paramCount == 2) //<U DISCONNECT>
							{
								char thisParam[5] = {'\0'};
								memcpy(&thisParam[0], &ppList.at(1).payload.strVal[0], 4);
								thisParam[4] = '\0';
								if (strcmp(&thisParam[0], "DISC") == 0)
								{
									if (currClient->thisClient->connected())
										currClient->thisClient->stop();  //Quit, close connection
								}
							}
						}
						break;
						case 'w':
						{
							if (paramCount == 4) //<w cab cv value> write cv on the main
								digitraxBuffer->writeProg(ppList.at(1).payload.longVal, 1, 0, ppList.at(2).payload.longVal, ppList.at(3).payload.longVal);
						}
						break;
						case 'W':
						{
							if (paramCount == 2) //<R val> Read Address
								digitraxBuffer->writeProg(0, 0, 0, 1, ppList.at(1).payload.longVal);
							if (paramCount == 3) //<R cv val> Read Address
								digitraxBuffer->writeProg(0, 0, 0, ppList.at(1).payload.longVal, ppList.at(2).payload.longVal);
						}
						break;
					}
				}
//				Serial.printf("Unprocessed Native DCC EX message %i bytes: %s\n", len-2, c);
				i++;
			}
			return false;
		}
		else
		{
//			Serial.printf("WiClient message %i bytes: %s\n", len, c);
			currClient->nextPing = millis() + pingInterval + random(500);
			char* strList[5];
			untokstr(strList, 4, c, "<;>"); //IoTT_CommDef
			switch (c[0])
			{
				case 'T':
				case 'S': return false;
				case 'M': // multi throttle command// M0+S12<;>S12
					if (len > 1)
					{
						char throttleID = c[1];
						if (len > 2)
						{
							uint16_t locoAddr = 0xFFFF;
							if (c[3] != '*')
								locoAddr = strtol(&c[4], &strList[1]-3, 10);
//							Serial.println(locoAddr);
							switch (c[2])
							{
								case 'A': //Action
									{
										currClient->setLocoAction(locoAddr, throttleID, strList[1]);
									} 
									return true;
								case '+': //Add Locomotive
									{
										currClient->addLoco(locoAddr, throttleID);
									} 
									updateClientList();
									return true;
								case '-': //Remove Locomotive
									{
										currClient->removeLoco(locoAddr, throttleID);
//										strcat(c, "<;>");
//										sendWIClientMessage(currClient->thisClient, c);
									}
									updateClientList();
									return true;
								case 'S': //Steal Locomotive	
									currClient->stealLoco(locoAddr, throttleID);
									updateClientList();
									return true;
							}
						}
						return false;			
					}
					return false;
			
				case 'D':  return false;
				case '*':
					if (len > 1)
					{
//						c[len-1] = '\0';
						switch (c[1])
						{
							case '-':
								currClient->nextPing = 0; //do not request pings
								return true;
							case '+':
								currClient->nextPing = millis() + pingInterval - random(500); //set ping time
								return true;
							default:
								return true;
						}
					}
					return false;
				case 'C':  return false;
				case 'N': //set device name
					if (len > 1)
					{
//						c[len-1] = '\0';
						if ((!currClient->wiDeviceName) || (strcmp(currClient->wiDeviceName, &c[1]) != 0))
						{
							currClient->wiDeviceName = (char*) realloc(currClient->wiDeviceName, len);
							strcpy(&currClient->wiDeviceName[0], &c[1]);
							updateClientList();
						}
						sendWIServerMessageStringWI(currClient->thisClient, 9); //ping time
//						Serial.printf("Device Name: %s\n", currClient->wiDeviceName);
						//send wiReply *HeartBeatInterval
						return true;
					}
					return false;
				case 'H': 
					if (len > 1)
					{
//						c[len-1] = '\0';
						switch (c[1])
						{
							case 'U': //set device ID
								if ((!currClient->wiHWIdentifier) || (strcmp(currClient->wiHWIdentifier, &c[1]) != 0))
								{
									currClient->wiHWIdentifier = (char*) realloc(currClient->wiHWIdentifier, len);
									strcpy(&currClient->wiHWIdentifier[0], &c[2]);
									updateClientList();
								}
//								Serial.printf("Hardware Identifier: %s", currClient->wiHWIdentifier);
								return true;
						}
						return false;
					}
					return false;
				case 'P': //Panel Command
					if (len > 3)
					{
						switch (c[1])
						{
							case 'P': //Power Command
								switch (c[2])
								{
									case 'A': //Power On/Off
//										Serial.println(allowPwrChg);
										if (allowPwrChg > 0)
											if (c[3] == '1')
												currClient->setTrackPowerStatus(0x83); //ON
											else
												if (allowPwrChg == 2) //on-off
													currClient->setTrackPowerStatus(0x82); //ON
												else
													currClient->setTrackPowerStatus(0x85); //Idle
										sendWIServerMessageStringWI(currClient->thisClient, 2); //power status
//										Serial.println("Power State");
										return true;
								}
							case 'T': //Turnout Command
							{
								switch (c[2])
								{
									case 'A': //Turnout Command
									{
										currClient->setTurnout(c[3], &c[4]);
										return true;
									}
								}
							}
						}
					}
					return false;
				case 'R': break; //Roster command
				case 'Q': //Quit, close connection
				{
					if (currClient->thisClient->connected())
						currClient->thisClient->stop(); 
				}
				break; 
				default: 	Serial.printf("Unprocessed WiClient message %i bytes: %s\n", len, c); break;

			}
			return false;
		}
	}
	else
		return false;
}

//this is called when data is received in client mode
bool IoTT_LBServer::processWIClientMessage(AsyncClient* client, char * c)
{
//	Serial.print("In: ");
//	Serial.println(c);
	uint16_t len = strlen(c);
	if (lntcpClient == client)  //client mode, handle message from server
	{
		pingSent = false;
		nextPingPoint = millis() + pingInterval - random(500);

		if (len > 1 && c[0]=='*') //heartbeat interval
		{
			uint16_t hbInt = 0;
			uint8_t startPtr = 1;
			while ((c[startPtr] >= '0') && (c[startPtr] <= '9') && (startPtr < len))
			{
				hbInt = (10 * hbInt) + (c[startPtr] - '0');
				startPtr++;
			}
			if (hbInt > 5)
				pingInterval = 1000 * (hbInt - 5);
			else
				pingInterval = 5000;
			return true;
		}
		else if (len > 3 && c[0]=='P' && c[1]=='F' && c[2]=='T') 
		{
//			Serial.println("Process FastTime");
			return true;
//      	return processFastTime(c+3, len-3);
		}
		else if (len > 3 && c[0]=='P' && c[1]=='P' && c[2]=='A') 
		{
			uint8_t newStat = c[3] - c['0'];
			digitraxBuffer->setPowerStatus((newStat & 0x01) + 0x82);
			return true;
		}
		else if (len > 3 && c[0]=='P' && c[1]=='R' && c[2]=='T') 
		{
//		Serial.println("Process Route List");
//        processTrackPower(c+3, len-3);
			return true;
		}
		else if (len > 4 && c[0]=='P' && c[1]=='T' && c[2]=='A') 
		{
			uint8_t newStat = c[3] - c['0'];
			if ((newStat == 2) || (newStat == 4))
			{
				uint16_t swiAddr = 0;
				uint8_t startPtr = 4;
				while ((c[startPtr] >= '0') && (c[startPtr] <= '9') && (startPtr < len))
				{
					swiAddr = (10 * swiAddr) + (c[startPtr] - '0');
					startPtr++;
				}
				digitraxBuffer->setSwiStatus(swiAddr, newStat == 4, false); //true if thrown
			}
			return true;
		}
		else if (len > 3 && c[0]=='P' && c[1]=='T' && c[2]=='L') 
		{
//		Serial.println("Process defined Turnouts");
//        processTrackPower(c+3, len-3);
			return true;
		}
		else if (len > 3 && c[0]=='P' && c[1]=='T' && c[2]=='T') 
		{
//		Serial.println("Process Turnout List");
//        processTrackPower(c+3, len-3);
			return true;
		}
		else if (len > 2 && c[0]=='V' && c[1]=='N') 
		{
			uint8_t rdBuf = 0;
			uint8_t startPtr = 2;
			while ((c[startPtr] >= '0') && (c[startPtr] <= '9') && (startPtr < len))
			{
				rdBuf = (10 * rdBuf) + (c[startPtr] - '0');
				startPtr++;
			}
			wiVersion = (rdBuf << 8);
			rdBuf = 0;
			startPtr +=2;
			while ((c[startPtr] >= '0') && (c[startPtr] <= '9') && (startPtr < len))
			{
				rdBuf = (10 * rdBuf) + (c[startPtr] - '0');
				startPtr++;
			}
			wiVersion += rdBuf;
//			Serial.printf("WiThrottle Version %i.%i\n", (wiVersion & 0xFF00)>>8, wiVersion & 0x00FF);
			return true;
		}
		else if (len > 2 && c[0]=='H' && c[1]=='T') 
		{
			c[len-1] = '\0';
			wiServerType = (char*) realloc(wiServerType, len - 2);
			strcpy(&wiServerType[0], &c[2]);
//			Serial.println(wiServerType);
			return true;
		}
		else if (len > 2 && c[0]=='H' && c[1]=='t') 
		{
			c[len-1] = '\0';
			wiServerDescription = (char*) realloc(wiServerDescription, len - 2);
			strcpy(&wiServerDescription[0], &c[2]);
//			Serial.println(wiServerDescription);
			return true;
		}	
		else if (len > 2 && c[0]=='H' && ((c[1]=='M') || (c[1]=='m'))) 
		{
			c[len-1] = '\0';
			wiServerMessage = (char*) realloc(wiServerMessage, len - 2);
			strcpy(&wiServerMessage[0], &c[2]);
//			Serial.println(wiServerMessage);
			return true;
		}	
		else if (len > 2 && c[0]=='P' && c[1]=='W') 
		{
//		Serial.println("Process Web Port");
//        processWebPort(c+2, len-2);
			return true;
		}
		else if (len > 3 && c[0]=='R' && c[1]=='C' && c[2]=='C') 
		{
//		Serial.println("Process Consist List");
//        processConsitList(c+3, len-3);
			return true;
		}
		else if (len > 2 && c[0]=='R' && c[1]=='L') 
		{
//		Serial.println("Process Roster List");
//        processRosterList(c+2, len-2);
			return true;
		}	
		else if (len > 6 && c[0]=='M' && c[1]=='0' && c[2]=='S') 
		{
//		Serial.println("Process Steal");
			String outStr = String(c);
			sendWIClientMessage(lntcpClient, outStr, false);
//        processStealNeeded(c+3, len-3);
			return true;
		}
		else if (len > 6 && c[0]=='M' && c[1]=='0' && (c[2]=='+' || c[2]=='-')) 
		{
        // we want to make sure the + or - is passed in as part of the string to process
			uint16_t dccAddr = 0;
			uint8_t startPtr = 4;
			while (c[startPtr] != '<')
			{
				dccAddr = (10 * dccAddr) + c[startPtr] - '0';
				startPtr++;
			}
		
			uint8_t slotNr = digitraxBuffer->getSlotOfAddr(dccAddr & 0x7F, (dccAddr >> 7 & 0x7F));
//			Serial.println(slotNr);
			slotData * thisSlot = NULL;
			if (slotNr != 0xFF)
				thisSlot = digitraxBuffer->getSlotData(slotNr);
			lnReceiveBuffer recBuffer;
			lnTransmitMsg * txBuffer = (lnTransmitMsg*)&recBuffer;
			if (c[2] == '+')
			{
				currentWIDCC = dccAddr;
				if (thisSlot)
				{
					(*thisSlot)[1] = dccAddr & 0x7F;
					(*thisSlot)[6] = (dccAddr >> 7);
					(*thisSlot)[0] = 0x33; //set slot status to in use, refreshed
				}
//			Serial.printf("Process Add %i \n", dccAddr);
			}
			else
			{
				currentWIDCC = -1;
				if (thisSlot)
					(*thisSlot)[0] = 0x03; //set slot status to not in use, not refreshed
//			Serial.printf("Process Remove %i \n", dccAddr);
			return true;
			}
			if (thisSlot)
			{
				prepSlotReadMsg(txBuffer, slotNr);
				recBuffer.reqRecTime = micros();
				callbackLocoNetMessage(&recBuffer);
//				if (lbsCallback)
//					lbsCallback(&recBuffer);
			}
			return true;
		}
		else if (len > 8 && c[0]=='M' && c[1]=='0' && c[2]=='A') 
		{
//		Serial.println("Process Loco Action");
			uint16_t dccAddr = 0;
			uint8_t startPtr = 4;
			while (c[startPtr] != '<')
			{
				dccAddr = (10 * dccAddr) + c[startPtr] - '0';
				startPtr++;
			}
			uint8_t slotNr = digitraxBuffer->getSlotOfAddr(dccAddr & 0x7F, (dccAddr >> 7 & 0x7F));
//		Serial.println(slotNr);
			slotData * thisSlot = NULL;
			if (slotNr != 0xFF)
				thisSlot = digitraxBuffer->getSlotData(slotNr);
			if (thisSlot)
			{
				while (c[startPtr] != '>')
					startPtr++;
				startPtr++;
				char cmdCode = c[startPtr];
				startPtr++;
				uint cmdVal = 0;
				while ((c[startPtr] >= '0') && (c[startPtr] <= '9')) 
				{
					cmdVal = (10 * cmdVal) + c[startPtr] - '0';
					startPtr++;
				}
				switch (cmdCode)
				{
					case 'V':
						(*thisSlot)[2] = cmdVal;
						break;
					case 'R':
						(*thisSlot)[3] = cmdVal == 0 ? ((*thisSlot)[3] & 0xDF) : ((*thisSlot)[3] | 0x20);
						break;
					default: return true;
				}
				lnReceiveBuffer recBuffer;
				lnTransmitMsg * txBuffer = (lnTransmitMsg*)&recBuffer;
				if (thisSlot)
				{
					prepSlotReadMsg(txBuffer, slotNr);
					recBuffer.reqRecTime = micros();
					callbackLocoNetMessage(&recBuffer);
//					if (lbsCallback)
//						lbsCallback(&recBuffer);
				}	
			}
			return true;
		}
		else if (len > 8 && c[0]=='M' && c[1]=='0' && c[2]=='L') 
		{
//		Serial.println("Receive Loco Information");
			return true;
		}
		else if (len > 3 && c[0]=='A' && c[1]=='T' && c[2]=='+') 
		{
//		Serial.println("Process Ignore");
			return true;
        // this is an AT+.... command that the LnWi sometimes emits and we
        // ignore these commands altogether
		}
		else 
		{
			Serial.printf("unknown command %s\n", c);
			return true;
        // all other commands are explicitly ignored
		}
		return true;
	}
	else
		return false;
}

void IoTT_LBServer::processLNServerMessage(AsyncClient* client, char * data)
{
//	Serial.println("processServerMessage");
//	Serial.write(data);
//	Serial.println();
	lnReceiveBuffer recData;
	lnTransmitMsg* txData = (lnTransmitMsg*) &recData; 
	char *p = data;
    char *str;
	str = strtok_r(p, " ", &p);
	if (str != NULL) //has additional data
	{
		if (strcmp(str,"SEND") == 0) //if this happens, we are in server mode and a client requests sending data to LocoNet
		{
			tcpToLN(p, &recData);
//			Serial.println("Process SEND");
			if (recData.errorFlags == 0)
			{
//				Serial.write(data);
//				Serial.println();
//				Serial.printf("Sending %i bytes to LocoNet\n", recData.lnMsgSize);
				recData.requestID = random(0xFF);
				recData.requestID |= fromLBServer;
				sendMsg(*txData);
//				if (lbsCallback)
//					lbsCallback(&recData);
				lastTxClient = client;
				lastTxData = recData;
			}
			else
				Serial.printf("Cancel Sending %i bytes because of error flags 0x%2X\n", recData.lnMsgSize, recData.errorFlags);
			return;
		} //everything below means we are in client mode because only a server is sending these messages
		nextPingPoint = millis() + pingInterval + random(4500);
//		Serial.println("Ping reset");
		pingSent = false;
		if (strcmp(str,"VERSION") == 0)
		{
//			Serial.println(data);
			return;
		}
		if (strcmp(str,"RECEIVE") == 0)
		{
//			Serial.println("Process RECEIVE");
			tcpToLN(p, &recData);
			recData.reqRecTime = micros();
//			if (recData.errorFlags == 0)
				callbackLocoNetMessage(&recData);
//			else
//			{
//				Serial.printf("Receive Error 0x%02X\n", recData.errorFlags);
//			}
			return;
		}
		if (strcmp(str,"SENT") == 0)
		{
//			str = strtok_r(p, " ", &p);
//			Serial.println(str);
			return;
		}
		if (strcmp(str,"TIMESTAMP") == 0)
		{
//			Serial.println(data);
			return;
		}
		if (strcmp(str,"BREAK") == 0)
		{
//			Serial.println(data);
			return;
		}
		if (strcmp(str,"ERROR") == 0)
		{
//			Serial.println(data);
			return;
		}
	}
}

void IoTT_LBServer::handleConnect(AsyncClient *client)
{
	nextPingPoint = millis() + pingInterval + random(4500);
	reconnectInterval = lbs_reconnectStartVal;  //if not connected, try to reconnect every 10 Secs initially, then increase if failed
	pingSent = false;
	sendID = true;
	if (isWiThrottle)
	{
		Serial.printf("WiThrottle client is now connected to server %s on port %d \n", client->remoteIP().toString().c_str(), isServer ? lbs_ServerPort : lbs_Port);
		pingInterval = 10000;
	}
	else
	{
		Serial.printf("LocoNet over TCP client is now connected to server %s on port %d \n", client->remoteIP().toString().c_str(), isServer ? lbs_ServerPort : lbs_Port);
		pingInterval = 10000;
	}
}

void IoTT_LBServer::clearWIThrottle(AsyncClient * thisClient)
{
	if (currentWIDCC > 0)
	{
		String outStr = "M0-*<;>r";
		sendWIClientMessage(thisClient, outStr, false);
		currentWIDCC = -1;
	}
}

String IoTT_LBServer::getWIMessageString(AsyncClient * thisClient, lnReceiveBuffer thisMsg) //used in client mode
{
	String outStr = "";
	switch (thisMsg.lnData[0])
	{
		case 0xBF : //request Loco Addr
		{
			uint16_t locoAddr =  (thisMsg.lnData[1] << 7) + thisMsg.lnData[2];
			if (currentWIDCC > 0)
				clearWIThrottle(thisClient);
			String addrStr = String(locoAddr);
			String hlpStr = (locoAddr > 127 ? "L" : "S") + addrStr;
			outStr = "M0+" + hlpStr + "<;>" + hlpStr;
		}
		break;
		case 0xA0 : //Set slot speed
		{
			String addrStr = (currentWIDCC > 127 ? "L" : "S") + String(currentWIDCC);
			String hlpStr = String(thisMsg.lnData[2]);
			outStr = "M0A" + addrStr + "<;>V" + hlpStr;
		}
		break;
		case 0xA1 : //Set slot DIRF
		{
			String addrStr = (currentWIDCC > 127 ? "L" : "S") + String(currentWIDCC);
			String hlpStr = String((thisMsg.lnData[2] & 0x20)>>5);
			outStr = "M0A" + addrStr + "<;>R" + hlpStr;
		}
		break;
	}
	return outStr;
}

bool IoTT_LBServer::sendWIServerMessageStringEX(AsyncClient * thisClient, uint8_t replyType)
{
	String outStr;
//	char outBuf[50] = {'\0'};
	switch (replyType)
	{
		case 0: outStr = "iDCC-EX V-4.2.14 / " + String(WiTVersion); break;
		case 1: outStr = "# 50"; break; //number of available loco slots
		case 2: outStr = "p" + String(digitraxBuffer->getPowerStatus() & 0x01); break;
		case 3: 
			outStr = digitraxBuffer->getTurnoutInfo(0, true);
			break;	
		case 4: 
			outStr = digitraxBuffer->getRosterInfo(0, true);
			break;
		case 5: 
			outStr = digitraxBuffer->getRouteInfo(0, true);
			break;
		case 10: break; //reserved for FastClock Update			
	}
//	Serial.printf("%i %s\n", replyType, outStr.c_str());
	if (thisClient && (outStr != ""))
		return(sendWIClientMessage(thisClient, outStr, true));
	else
		return false;
}

bool IoTT_LBServer::sendWIServerMessageStringWI(AsyncClient * thisClient, uint8_t replyType) //server mode
{
	String outStr;
	char outBuf[100];
//	extern String BBVersion;
	switch (replyType)
	{
		case 0: outStr = "VN2.0"; break;
		case 1: switch (defSource)
				{
					case 0: 
					{
						outStr = "RL" + String(locoSupport.size());
						if (locoSupport.size() > 0)
							for (uint16_t i = 0; i < locoSupport.size(); i++)
							{
								sprintf(outBuf, "]\\[%i}|{%i}|{%c", locoSupport[i], locoSupport[i], locoSupport[i] > 127 ? 'L' : 'S');
								outStr += String(outBuf);
							}
					}
					break;
					case 1: outStr = digitraxBuffer->getRosterInfo(0, false); 
					break;
//					default: outStr = ""; break;
				}
				break;
		case 2: outStr = "PPA" + String(digitraxBuffer->getPowerStatus()); break;
		case 3: outStr = "PTT]\\[Turnouts}|{Turnout]\\[Closed}|{2]\\[Thrown}|{4]\\[Unknown}|{1]\\[Inconsistent}|{8"; // break; 
				sendWIClientMessage(thisClient, outStr, false);
				switch (defSource)
				{
					case 0: 
					{
						outStr = "PTL";
						for (uint16_t i = 0; i < turnoutSupport.size(); i++)
							outStr += "]\\[" + String(turnoutSupport[i]) + "}|{" + "IoTT" + "}|{" + ((digitraxBuffer->getSwiPosition(turnoutSupport[i]) > 0) ? "4" : "2");
					}
					break;
					case 1: outStr = digitraxBuffer->getTurnoutInfo(0, false); break;
//					default: outStr = ""; break;
				}
				break;
		case 4: outStr = "PRT]\[Routes}|{Route]\[Active}|{2]\[Inactive}|{4]\[Unknown}|{0]\[Inconsistent}|{8";
				sendWIClientMessage(thisClient, outStr, false);
				switch (defSource)
				{
//					case 1: outStr = digitraxBuffer->getRouteInfo(0, false); break;
					default: outStr = ""; break;
				}
				break;
		case 5: outStr = "RCC0"; break;
		case 6: outStr = "HTDCC-EX"; break;
//		case 6: outStr = "HTIoTT"; break;
		case 7: outStr = "Ht" + String(WiTVersion); break;
//		case 8: outStr = "PW" + String(lbs_Port); break; //web port
		case 9: outStr = "*" + String((int)round(pingInterval/1000)); break;
		case 10: outStr = "PFT" + String(digitraxBuffer->getFCTime()) + "<;>" + String((int)digitraxBuffer->slotBuffer[0x7B][0]); break;
//---------------------->> Init sequence end^
		default: outStr = ""; break;
	}
//	Serial.printf("%i %s\n", replyType, outStr.c_str());
	if (thisClient && (outStr != ""))
		return(sendWIClientMessage(thisClient, outStr, false));
	else
		return false;
}

bool IoTT_LBServer::sendWIClientMessage(AsyncClient * thisClient, String cmdMsg, bool useEXFormat)
{
	if (thisClient)
		if (thisClient->canSend())
		{
			String lnStr = cmdMsg;
			if (useEXFormat)
				lnStr = '<' + lnStr + '>';
//			else
			{
				lnStr += '\r';
				lnStr += '\n';
			}
			if (thisClient->space() > strlen(lnStr.c_str())+2)
			{
				thisClient->add(lnStr.c_str(), strlen(lnStr.c_str()));
				nextPingPoint = millis() + pingInterval - random(500);
				return thisClient->send();
			}
		}
//		Serial.println("failed");
		return false;
}

bool IoTT_LBServer::sendLNClientText(AsyncClient * thisClient, String cmdMsg, String txtMsg)
{
	if (thisClient)
		if (thisClient->canSend())
		{
			String lnStr = cmdMsg + " " + txtMsg;
			lnStr += '\r';
			lnStr += '\n';
//			Serial.print(thisClient->space());
			if (thisClient->space() > strlen(lnStr.c_str())+2)
			{
//				Serial.println(lnStr);
				thisClient->add(lnStr.c_str(), strlen(lnStr.c_str()));
				nextPingPoint = millis() + pingInterval - random(500);
				return thisClient->send();
			}
		}
	return false;
}

bool IoTT_LBServer::sendLNClientMessage(AsyncClient * thisClient, String cmdMsg, lnReceiveBuffer thisMsg)
{
	if (thisClient)
		if (thisClient->canSend())
		{
//			Serial.print("sending... ");
			String lnStr = cmdMsg;
			char hexbuf[13];
			for (uint8_t i = 0; i < thisMsg.lnMsgSize; i++)
			{
				sprintf(hexbuf, " %02X", thisMsg.lnData[i]);
				lnStr += String(hexbuf);
			}
			lnStr += '\r';
			lnStr += '\n';
//			Serial.print(thisClient->space());
			if (thisClient->space() > strlen(lnStr.c_str())+2)
			{
				thisClient->add(lnStr.c_str(), strlen(lnStr.c_str()));
				nextPingPoint = millis() + pingInterval - random(500);
//			Serial.println(" done");
				return thisClient->send();
			}
//			Serial.println(" failed");
		}
	return false;
}

String IoTT_LBServer::getServerIP()
{
	return isServer ? "" : lbs_IP.toString();
}

void IoTT_LBServer::processLoop()
{
	if (isWiThrottle)
		processLoopWI();
	else
		processLoopLN();
}

void IoTT_LBServer::processLoopWI() //process function for WiThrottle
{
	char outBuf[50] = {'\0'};
	if (isServer)
	{
		if (clients.size() > 0)
		{
			for (uint16_t i = 0; i < clients.size(); i++)
			{
				if (clients[i]->useExCmd >= 0) //known format
				{
					if (clients[i]->sendInitSeq >= 0) //only valid after format is clear
					{
//						Serial.printf("%i %i \n", i, clients[i]->sendInitSeq);
						if (clients[i]->useExCmd > 0)
						{
							if (numInitSeq - clients[i]->sendInitSeq <= 2)
								sendWIServerMessageStringEX(clients[i]->thisClient, numInitSeq - clients[i]->sendInitSeq);
						}
						else
						{
//							Serial.printf("Loop %i\n", numInitSeq - clients[i]->sendInitSeq);
							sendWIServerMessageStringWI(clients[i]->thisClient, numInitSeq - clients[i]->sendInitSeq);
//							Serial.println("done");

						}
						clients[i]->sendInitSeq--;
					}
					else
						if (trunc(clients[i]->lastFC/60) != trunc(digitraxBuffer->getFCTime()/60))
						{
//						if (fcUpdate)
							if (clients[i]->useExCmd > 0)
								sendWIServerMessageStringEX(clients[i]->thisClient, 10); //not implemented in DCC EX, does nothing
							else
							{
								sendWIServerMessageStringWI(clients[i]->thisClient, 10);
							}
							clients[i]->lastFC = digitraxBuffer->getFCTime();
						}
				}
			}
			if (que_wrPos != que_rdPos)
			{
				int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
				locoDef * thisLoco = NULL;
				switch (transmitQueue[hlpQuePtr].lnData[0])
				{
					case 0x82:;
					case 0x83:;
					case 0x85:
						{
							for (uint16_t i = 0; i < clients.size(); i++)
							{
								if (clients[i]->useExCmd > 0)
									sendWIServerMessageStringEX(clients[i]->thisClient, 2);
								else
									sendWIServerMessageStringWI(clients[i]->thisClient, 2);
							}
						}
						break;
					case 0xB2:
						{
							uint16_t inpNr = (transmitQueue[hlpQuePtr].lnData[1] << 1) + ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F) << 8) + ((transmitQueue[hlpQuePtr].lnData[2] & 0x20) >> 5);
							sprintf(outBuf, (transmitQueue[hlpQuePtr].lnData[2] & 0x10) > 0 ? "Q %i" : "q %i", inpNr);
							for (uint16_t i = 0; i < clients.size(); i++)
							{
								if (clients[i]->useExCmd > 0)
									sendWIClientMessage(clients[i]->thisClient, outBuf, clients[i]->useExCmd > 0);
							}
						}
						break;

					case 0xB4: //OPC_LONG_ACK
						switch (transmitQueue[hlpQuePtr].lnData[1])
						{
							case 0x3C: //OPC_SW_STATE
								if (lastSwiAddr != 0xFFFF)
								{
									for (uint16_t i = 0; i < clients.size(); i++)
									{
										if (clients[i]->useExCmd > 0) //DCC EX
											sprintf(outBuf, "H %i %i", (lastSwiAddr, transmitQueue[hlpQuePtr].lnData[2] & 0x70) == 0x30 ? 0 : 1);
//											sprintf(outBuf, "H %i %c", transmitQueue[hlpQuePtr].lnData[1] + ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F)<<7), transmitQueue[hlpQuePtr].lnData[2] & 0x20 ? 'C' : 'T');
										else //native Wi
											sprintf(outBuf, "PTA%c%i", (lastSwiAddr, transmitQueue[hlpQuePtr].lnData[2] & 0x70) == 0x30 ? '2' : '4', lastSwiAddr);
										sendWIClientMessage(clients[i]->thisClient, outBuf, clients[i]->useExCmd > 0);
									}
								}
								break;
						}
						lastSwiAddr = 0xFFFF;
						break;
					case 0xBC:  //OPC_SW_STATE Request. Following command is LACK with status
						lastSwiAddr = ((transmitQueue[hlpQuePtr].lnData[1] & 0x7F)) + ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F)<<7); //store switch address for coming LACK
						break;
					case 0xBD:;  //OPC_SW_REQ, SW_ACK
					case 0xB0:
						{
							for (uint16_t i = 0; i < clients.size(); i++)
							{
								if (clients[i]->useExCmd > 0) //DCC EX
									sprintf(outBuf, "H %i %i", transmitQueue[hlpQuePtr].lnData[1] + ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F)<<7), transmitQueue[hlpQuePtr].lnData[2] & 0x20 ? 0 : 1);
//									sprintf(outBuf, "H %i %c", transmitQueue[hlpQuePtr].lnData[1] + ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F)<<7), transmitQueue[hlpQuePtr].lnData[2] & 0x20 ? 'C' : 'T');
								else //native Wi
									sprintf(outBuf, "PTA%c%i", transmitQueue[hlpQuePtr].lnData[2] & 0x20 ? '2' : '4', transmitQueue[hlpQuePtr].lnData[1] + ((transmitQueue[hlpQuePtr].lnData[2] & 0x0F)<<7));
								sendWIClientMessage(clients[i]->thisClient, outBuf, clients[i]->useExCmd > 0);
							}
						}
						break;					
					case 0xA0:; //OPC_LOCO_SPD
					case 0xA1:; //OPC_LOCO_DIRF
					case 0xA2: //OPC_LOCO_SND
						{
							for (uint16_t i = 0; i < clients.size(); i++)
							{
								thisLoco = clients[i]->getLocoBySlot(NULL, transmitQueue[hlpQuePtr].lnData[1], '*');
								while (thisLoco)
								{
//									Serial.printf("LN Slot: %i Mem Slot: %i ThID: %c\n", transmitQueue[hlpQuePtr].lnData[1], thisLoco->slotNum, thisLoco->throttleID);
									uint16_t currFct = (digitraxBuffer->slotBuffer[thisLoco->slotNum][3] << 8) + digitraxBuffer->slotBuffer[thisLoco->slotNum][7];
									uint16_t fctChgMask = currFct ^ thisLoco->dirFct;
									if (transmitQueue[hlpQuePtr].lnData[0] == 0xA0) clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qV");
									if ((fctChgMask & 0x2000))
										if (transmitQueue[hlpQuePtr].lnData[0] == 0xA1) clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qR");
									if ((fctChgMask & 0x1F00))
										if (transmitQueue[hlpQuePtr].lnData[0] == 0xA1) clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qF");
									if ((fctChgMask & 0x000F))
										if (transmitQueue[hlpQuePtr].lnData[0] == 0xA2) clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qF");
									thisLoco = clients[i]->getLocoBySlot(thisLoco, transmitQueue[hlpQuePtr].lnData[1], '*');
								}
							}
						}
						break;
					
					case 0xE7: 
						{
							switch (transmitQueue[hlpQuePtr].lnData[1])
							{
								case 0x0E: //SL_RD
								{
									uint8_t slotNr = transmitQueue[hlpQuePtr].lnData[2];
									if (slotNr <= 0x77)
									{
										uint16_t locoAddr = (transmitQueue[hlpQuePtr].lnData[9] << 7) + transmitQueue[hlpQuePtr].lnData[4];
//										Serial.printf("Slot Read Addr %i\n", locoAddr);
										for (uint16_t i = 0; i < clients.size(); i++)
										{
											thisLoco = clients[i]->getLocoByAddr(thisLoco, locoAddr, '*');
											while (thisLoco)
											{
												clients[i]->confirmLoco(transmitQueue[hlpQuePtr].lnData[2], locoAddr, thisLoco->throttleID, transmitQueue[hlpQuePtr].lnData[3], transmitQueue[hlpQuePtr].lnData[5], (transmitQueue[hlpQuePtr].lnData[6]<<8) + transmitQueue[hlpQuePtr].lnData[10]);
//												Serial.printf("Send Loco %i Throttle %c\n", thisLoco->locoAddr, thisLoco->throttleID);
												//send dir, speed, fcts
												clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qV");
												clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qR");
												clients[i]->setLocoAction(thisLoco->locoAddr, thisLoco->throttleID, "qF", 0xFFFF);
//												Serial.println("Send Loco UpData done");
												
												thisLoco = clients[i]->getLocoByAddr(thisLoco, locoAddr, '*');
											}
										}
									}
									else
									{
										switch (slotNr)
										{
											case 0x7B: //FastClock
												{
//													for (uint16_t i = 0; i < clients.size(); i++)
//													{
//														sendWIServerMessageString(clients[i]->thisClient, 10); //fast clock update
//														clients[i]->lastFC = digitraxBuffer->getFCTime();
//													}
												}
											break;
											case 0x7C: //Prog Slot
												{
													for (uint16_t i = 0; i < clients.size(); i++)
														if (clients[i]->useExCmd > 0) //DCC EX
														{
															bool readOp = (transmitQueue[hlpQuePtr].lnData[3] & 0x40) == 0;
															bool serviceMode = (transmitQueue[hlpQuePtr].lnData[3] & 0x04) == 0;
															bool validData = (transmitQueue[hlpQuePtr].lnData[4] & 0x0F) == 0;
															uint16_t cvNr = (transmitQueue[hlpQuePtr].lnData[9] & 0x7F) + ((transmitQueue[hlpQuePtr].lnData[8] & 0x01) << 7) + ((transmitQueue[hlpQuePtr].lnData[8] & 0x30) << 4);
															uint8_t cvVal = (transmitQueue[hlpQuePtr].lnData[10] & 0x7F) + ((transmitQueue[hlpQuePtr].lnData[8] & 0x02) << 6);
															if (serviceMode)
															{
																if (cvNr == 0)
																	sprintf(outBuf, "%c %i", readOp ? 'r' : 'w', validData ? cvVal : -1);
																else
																	sprintf(outBuf, "r %i %i%", cvNr+1, validData ? cvVal : -1); //read and write have same answer
																sendWIClientMessage(clients[i]->thisClient, outBuf, clients[i]->useExCmd > 0);
															}
														}
												}
											break;
										}
									}		
									break;
								}
								break;
							}
						}
						break;
				}

				que_rdPos = hlpQuePtr; //if not successful, we keep trying
			}
		}
		else
			que_rdPos = que_wrPos; //no client, so reset out queue to prevent overflow
	}
	else
	{
		if (!lntcpClient->connected())
		{
			long now = millis();
			if (now - lastReconnectAttempt > reconnectInterval) 
			{
				reconnectInterval = min(reconnectInterval+10000, 60000); //increae interval
				lastReconnectAttempt = now;
				Serial.print("Trying to connect to WiThrottle server at ");
				Serial.println(lbs_IP);
				lntcpClient->connect(lbs_IP, lbs_Port);
			}
		}
		else
			if (que_wrPos != que_rdPos)
			{
				if (lntcpClient->canSend())
				{
					int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
					String msgStr = getWIMessageString(lntcpClient, transmitQueue[hlpQuePtr]);
					if (msgStr != "")
						if (sendWIClientMessage(lntcpClient, msgStr, false))
							que_rdPos = hlpQuePtr; 
						else
							return; //if not successful, we try next time
					else
						que_rdPos = hlpQuePtr; //invalid message, ignore
				}
			}
			else // periodic pinging of server
			{
				
				if ((millis() > nextPingPoint) || sendID)
				{
					if (pingSent)
						lntcpClient->stop();
					else
					{
						sendWIPing();
						nextPingPoint += 2000; //2 secs ping timeout
					}
				}

			}
		
	}
}

void IoTT_LBServer::processLoopLN() //process function for LN over TCP
{
	if (isServer)
	{
		if (clients.size() > 0)
		{
			if (que_wrPos != que_rdPos)
			{
//				Serial.println("TCP Server send data to clients");
				int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
//				if (thisClient->canSend())
				{
					if (clientTxConfirmation)
					{
						if (sendLNClientMessage(clients[clientTxIndex]->thisClient, "SENT OK", transmitQueue[hlpQuePtr]))
						{
							clientTxConfirmation = false;
							clientTxIndex++;
						}
					}
					else
						if (sendLNClientMessage(clients[clientTxIndex]->thisClient, "RECEIVE", transmitQueue[hlpQuePtr]))
						{
							if ((lastTxClient == clients[clientTxIndex]->thisClient) && ((lastTxData.requestID & 0x00FF) == (transmitQueue[hlpQuePtr].requestID & 0x00FF)) && ((transmitQueue[hlpQuePtr].errorFlags & msgEcho) > 0))
								clientTxConfirmation = true;
							else
								clientTxIndex++;
						}
					if (clientTxIndex == clients.size()) //message sent to all clients
					{
						que_rdPos = hlpQuePtr; //if not successful, we keep trying
						clientTxIndex = 0;
						clientTxConfirmation = false;
					}
				}
			}
		}
		else
			que_rdPos = que_wrPos; //no client, so reset out queue to prevent overflow
	}
	else
	{
		if (!lntcpClient->connected())
		{
			long now = millis();
			if (now - lastReconnectAttempt > reconnectInterval) 
			{
				reconnectInterval = min(reconnectInterval+10000, 60000); //increae interval
				lastReconnectAttempt = now;
				Serial.print("Trying to connect to TCP server ");
				Serial.print(lbs_IP);
				Serial.printf(" Port %i\n", lbs_Port);
//				Serial.println(isWiThrottle);
				lntcpClient->connect(lbs_IP, lbs_Port);
			}
		}
		else
			if (que_wrPos != que_rdPos)
			{
//				Serial.println("Send message to server");
				if (lntcpClient->canSend())
				{
					int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
					if (sendLNClientMessage(lntcpClient, "SEND", transmitQueue[hlpQuePtr]))
					{
						que_rdPos = hlpQuePtr; 
						lastOutMsg = que_rdPos;
					}
					else
						return; //if not successful, we try next time
				}
			}
			else // periodic pinging of server
			{
				if (millis() > nextPingPoint)
				{
					if (pingSent)
					{
						Serial.println("Ping Timeout");
						lntcpClient->stop();
					}
					else
					{
						sendLNPing();
						nextPingPoint += 2000; //2 secs ping timeout
					}
				}
			}
		
	}
	yield();
}

void IoTT_LBServer::sendWIPing()
{
	extern String deviceName;
//    String hlpStr = "IoTT_Stick_M5_" + String((uint32_t)ESP.getEfuseMac());
	sendWIClientMessage(lntcpClient, "N" + 	deviceName, false);
    if (sendID)
    {
		sendID = false;
		sendWIClientMessage(lntcpClient, "HU" + WiFi.localIP().toString(), false);
	}
	pingSent = true;
}

void IoTT_LBServer::sendLNPing()
{
//	Serial.println("LN Ping request");
	lnTransmitMsg txData;
	txData.lnMsgSize = 2;
	txData.lnData[0] = 0x81;
	txData.lnData[1] = 0x7E;
	lnWriteMsg(&txData);
	pingSent = true;
}

void IoTT_LBServer::updateClientList()
{
//	Serial.println("updateClientList");
	int8_t currClient = isWiThrottle ? getWSClientByPage(0,  "pgWiCfg") : getWSClientByPage(0,  "pgLBSCfg");
	if (currClient >= 0)
	{
		DynamicJsonDocument doc(800);
		char myMqttMsg[800];
		doc["Cmd"] = "ClientList";
		JsonObject Data = doc.createNestedObject("Data");
		Data["WIType"] = isWiThrottle;
		JsonArray clArray = Data.createNestedArray("cl");
		for (int i = 0; i < clients.size(); i++)
		{
			clArray[i]["ip"] = clients[i]->thisClient->remoteIP();
			if (isWiThrottle)
			{
				clArray[i]["name"] = clients[i]->wiDeviceName;
				JsonArray thList = clArray[i].createNestedArray("devs");
				char throttleIDList[10] = {0,0,0,0,0,0,0,0,0,0};
				uint8_t numThrottles = clients[i]->getThrottleIDList(&throttleIDList[0]);
//				Serial.println(throttleIDList);
				
				char arrayName[3] = {'T',0,0};
				for (uint8_t j = 0; j < numThrottles; j++)
				{
					arrayName[1]= throttleIDList[j];
					JsonObject tObj = thList.createNestedObject();
					tObj["thID"] = arrayName;
					JsonArray t0List = tObj.createNestedArray("addr");
					locoDef* thisLoco = clients[i]->getLocoFromList(NULL, throttleIDList[j]);
					while (thisLoco)
					{
						t0List.add(thisLoco->locoAddr);
						thisLoco = clients[i]->getLocoFromList(thisLoco, throttleIDList[j]);
					}
				}
			}
		}
		serializeJson(doc, myMqttMsg);
//		Serial.println(myMqttMsg);

		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMqttMsg);
			currClient = isWiThrottle ? getWSClientByPage(currClient+1,  "pgWiCfg") : getWSClientByPage(currClient+1,  "pgLBSCfg");
		}
//		Serial.println("Done");
	}
}

