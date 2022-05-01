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

cbFct lbsCallback = NULL;
//cbFct mqttappCallback = NULL;

uint8_t lbsMode = 0; //0: LN; 

IoTT_LBServer::IoTT_LBServer()
{
//	setCallback(psc_callback);
	nextPingPoint = millis() + pingInterval + random(4500);
}

IoTT_LBServer::~IoTT_LBServer()
{
	if (lntcpClient.thisClient)
		lntcpClient.thisClient->close();
}

IoTT_LBServer::IoTT_LBServer(Client& client)
{
//	setCallback(psc_callback);
	nextPingPoint = millis() + pingInterval + random(4500);
}

void IoTT_LBServer::initLBServer(bool serverMode)
{
//	Serial.println("Init LB Server");
	isServer = serverMode;
	if (isServer)
	{
		lntcpServer = new AsyncServer(WiFi.localIP(), lbs_Port);
		lntcpServer->onClient(&handleTopNewClient, this);
	}
	else //client mode
	{
		lntcpClient.thisClient = new AsyncClient();
		lntcpClient.thisClient->onData(handleTopDataFromClient, this);
		lntcpClient.thisClient->onConnect(handleTopConnect, this);
		lntcpClient.thisClient->onPoll(handleTopLNPoll, this);
	}
}

void IoTT_LBServer::initWIServer(bool serverMode)
{
//	Serial.println("WI");
	isServer = serverMode;
	isWiThrottle = true;
	if (isServer) //add code here to support server mode
	{
		lntcpServer = new AsyncServer(WiFi.localIP(), lbs_Port);
		lntcpServer->onClient(&handleTopNewClient, lntcpServer);
	}
	else //client mode
	{
//		Serial.println("Init WiThrottle");
		lntcpClient.thisClient = new AsyncClient();
		lntcpClient.thisClient->onData(handleTopDataFromClient, this);
		lntcpClient.thisClient->onConnect(handleTopConnect, this);
		lntcpClient.thisClient->onPoll(handleTopWIPoll, this);
	}
}

void IoTT_LBServer::startServer()
{
	if (isServer && lntcpServer)
		lntcpServer->begin();
}

void IoTT_LBServer::handleNewClient(AsyncClient* client)
{
	Serial.printf("A new client has been connected to server, ip: %s with timeout %i %i\n", client->remoteIP().toString().c_str(), client->getAckTimeout(), client->getRxTimeout());
	
	tcpDef newClientData;
	// add to list
	newClientData.thisClient = client;
	clients.push_back(newClientData);
	Serial.printf("New total is %i client(s)\n", clients.size());
  
	for (int i = 0; i < clients.size(); i++)
	{
		Serial.println(clients[i].thisClient->remoteIP());
	}
	// register events
	client->onData(&handleTopDataFromServer, this);
	client->onError(&handleTopError, this);
	client->onDisconnect(&handleTopDisconnect, this);
	client->onTimeout(&handleTopTimeOut, this);
}

 /* clients events */
void IoTT_LBServer::handleError(AsyncClient* client, int8_t error) 
{
	Serial.printf("\n%s\n", client->errorToString(error));
}

void IoTT_LBServer::handleDisconnect(AsyncClient* client) 
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].thisClient == client)
		{
			clients.erase(clients.begin() + i);
			break;
		}
	}
	Serial.printf("Client disconnected. %i clients remaining \n", clients.size());
	yield();
}

void IoTT_LBServer::handleTimeOut(AsyncClient* client, uint32_t time) 
{
  Serial.printf("Client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

void IoTT_LBServer::setLNCallback(cbFct newCB)
{
	lbsCallback = newCB;
}

void IoTT_LBServer::loadLBServerCfgJSON(DynamicJsonDocument doc)
{
	if (doc.containsKey("PortNr"))
		lbs_Port = doc["PortNr"];
	if (doc.containsKey("ServerIP"))
	{
		String thisIP = doc["ServerIP"];
		lbs_IP.fromString(thisIP);
	}
}

uint8_t IoTT_LBServer::getConnectionStatus()
{
	if (isServer)
		return clients.size();
	else
		if (lntcpClient.thisClient->connected())
			return 1;
		else
			return 0;
}

uint16_t IoTT_LBServer::lnWriteMsg(lnTransmitMsg txData)
{
//	Serial.printf("LN over TCP Tx %02X\n", txData.lnData[0]);
	uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].msgType = txData.msgType;
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].reqID = txData.reqID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, lnMaxMsgSize); //txData.lnMsgSize);
		transmitQueue[hlpQuePtr].reqRespTime = 0;
		transmitQueue[hlpQuePtr].echoTime = 0;
		transmitQueue[hlpQuePtr].errorFlags = 0;
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
	}
	else
	{	
		Serial.println("LN over TCP Write Error. Too many messages in queue");
		return -1;
	}
}

uint16_t IoTT_LBServer::lnWriteMsg(lnReceiveBuffer txData)
{
// 	Serial.printf("LN TCP Tx %02X", txData.lnData[0]);
// 	for (int i = 1; i < txData.lnMsgSize; i++)
//		Serial.printf(" %02X", txData.lnData[i]);
//	Serial.println();
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (hlpQuePtr != que_rdPos) //override protection
    {
		transmitQueue[hlpQuePtr].msgType = txData.msgType;
		transmitQueue[hlpQuePtr].lnMsgSize = txData.lnMsgSize;
		transmitQueue[hlpQuePtr].reqID = txData.reqID;
		transmitQueue[hlpQuePtr].reqRecTime = micros();
		memcpy(transmitQueue[hlpQuePtr].lnData, txData.lnData, lnMaxMsgSize); //txData.lnMsgSize);
		transmitQueue[hlpQuePtr].reqRespTime = txData.reqRespTime;
		transmitQueue[hlpQuePtr].echoTime = txData.echoTime;
		transmitQueue[hlpQuePtr].errorFlags = txData.errorFlags;
		que_wrPos = hlpQuePtr;
		return txData.lnMsgSize;
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
		if (clients[i].thisClient == client)
		{
//			Serial.print("Message from ");
//			Serial.println(clients[i].thisClient->remoteIP());
//			Serial.write((uint8_t *)data, len);
//			Serial.println();
			currClient = &clients[i];
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
	if (lntcpClient.thisClient == client) 
	{
		
		if ((((char*)data)[len-1] == '\n') || (((char*)data)[len-1] == '\r'))
		{
			//if command is complete, call handle data
//			lntcpClient.rxBuffer[lntcpClient.rxPtr] = '\0';
			handleData(lntcpClient.thisClient, (char*) data, len);
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
					processWIServerMessage(client, subStr);
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
					processWIServerMessage(client, subStr);
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
}

//this is called when data is received, currently only client mode
bool IoTT_LBServer::processWIServerMessage(AsyncClient* client, char * c)
{
	while (c[0] == '\'')
		c++;
//	Serial.println(c);
	uint16_t len = strlen(c);
	if (len == 0) return false;

	if (lntcpClient.thisClient == client)  //client mode, handle message from server
	{
		pingSent = false;
		nextPingPoint = millis() + pingInterval + random(4500);

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
			uint8_t startPtr = 1;
			while ((c[startPtr] >= '0') && (c[startPtr] <= '9') && (startPtr < len))
			{
				rdBuf = (10 * rdBuf) + (c[startPtr] - '0');
				startPtr++;
			}
			wiVersion = rdBuf << 8;
			rdBuf = 0;
			startPtr++;
			while ((c[startPtr] >= '0') && (c[startPtr] <= '9') && (startPtr < len))
			{
				rdBuf = (10 * rdBuf) + (c[startPtr] - '0');
				startPtr++;
			}
			wiVersion += rdBuf;
			Serial.println(wiVersion, 16);
			return true;
		}
		else if (len > 2 && c[0]=='H' && c[1]=='T') 
		{
//		Serial.println("Process Server Type");
//        processServerType(c+2, len-2);
			return true;
		}
		else if (len > 2 && c[0]=='H' && c[1]=='t') 
		{
//		Serial.println("Process Server Description");
//        processServerDescription(c+2, len-2);
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
			sendWIClientMessage(lntcpClient.thisClient, outStr);
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
					(*thisSlot)[0] = 0x33; //set slot status to in use, refreshed
//			Serial.printf("Process Add %i \n", dccAddr);
			}
			else
			{
				currentWIDCC = -1;
				if (thisSlot)
					(*thisSlot)[0] = 0x03; //set slot status to not in use, not refreshed
//			Serial.printf("Process Remove %i \n", dccAddr);
			}
			if (thisSlot)
			{
				prepSlotReadMsg(txBuffer, slotNr);
				if (lbsCallback)
					lbsCallback(&recBuffer);
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
					if (lbsCallback)
						lbsCallback(&recBuffer);
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
	else //Server Mode, handle incoming commands from client
	{
		return true;
	}

}

void IoTT_LBServer::processLNServerMessage(AsyncClient* client, char * data)
{
//	Serial.println("processServerMessage");
//	Serial.write(data);
//	Serial.println();
	lnReceiveBuffer recData;
	char *p = data;
    char *str;
	str = strtok_r(p, " ", &p);
	if (str != NULL) //has additional data
	{
		if (strcmp(str,"SEND") == 0) //if this happens, we are in server mode and a client requests sending data to LocoNet
		{
			tcpToLN(p, &recData);
			if (recData.errorFlags == 0)
			{
//				Serial.write(data);
//				Serial.println();
//				Serial.printf("Sending %i bytes to LocoNet\n", recData.lnMsgSize);
				recData.reqID = random(0xC000);
				recData.reqID |= 0xC000;
				if (lbsCallback)
					lbsCallback(&recData);
				lastTxClient = client;
				lastTxData = recData;
			}
			else
				Serial.printf("Cancel Sending %i bytes because of error flags %i\n", recData.lnMsgSize, recData.errorFlags);
			return;
		} //everything below means we are in client mode because only a server is sending these messages
		nextPingPoint = millis() + pingInterval + random(4500);
//		Serial.println("Ping reset");
		pingSent = false;
		if (strcmp(str,"VERSION") == 0)
		{
			Serial.println(data);
			return;
		}
		if (strcmp(str,"RECEIVE") == 0)
		{
//			Serial.println("Process RECEIVE");
			tcpToLN(p, &recData);
			if (lbsCallback)
				if (recData.errorFlags == 0)
					lbsCallback(&recData);
				else
				{
					Serial.printf("Receive Error 0x%02X\n", recData.errorFlags);
//					Serial.write(data);
//					Serial.println();
				}
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
		Serial.printf("WiThrottle client is now connected to server %s on port %d \n", client->remoteIP().toString().c_str(), lbs_Port);
		pingInterval = 5000;
	}
	else
	{
		Serial.printf("LocoNet over TCP client is now connected to server %s on port %d \n", client->remoteIP().toString().c_str(), lbs_Port);
		pingInterval = 10000;
	}
}

void IoTT_LBServer::clearWIThrottle(AsyncClient * thisClient)
{
	if (currentWIDCC > 0)
	{
		String outStr = "M0-*<;>r";
		sendWIClientMessage(thisClient, outStr);
		currentWIDCC = -1;
	}
}

String IoTT_LBServer::getWIMessageString(AsyncClient * thisClient, lnReceiveBuffer thisMsg)
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

bool IoTT_LBServer::sendWIClientMessage(AsyncClient * thisClient, String cmdMsg)
{
//	Serial.print("Out: ");
//	Serial.println(cmdMsg);
	if (thisClient)
		if (thisClient->canSend())
		{
			String lnStr = cmdMsg;
			lnStr += '\r';
			lnStr += '\n';
			if (thisClient->space() > strlen(lnStr.c_str())+2)
			{
				thisClient->add(lnStr.c_str(), strlen(lnStr.c_str()));
				nextPingPoint = millis() + pingInterval + random(4500);
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
				nextPingPoint = millis() + pingInterval + random(4500);
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
	if (isServer)
	{
		if (clients.size() > 0)
		{
			if (que_wrPos != que_rdPos)
			{
//				Serial.println("Withrottle Server send data to one or more client(s)");
				int hlpQuePtr = (que_rdPos + 1) % queBufferSize;

/*
				if (thisClient->canSend())
				{
					if (clientTxConfirmation)
					{
						if (sendLNClientMessage(clients[clientTxIndex].thisClient, "SENT OK", transmitQueue[hlpQuePtr]))
						{
							clientTxConfirmation = false;
							clientTxIndex++;
						}
					}
					else
						if (sendLNClientMessage(clients[clientTxIndex].thisClient, "RECEIVE", transmitQueue[hlpQuePtr]))
						{
							if ((lastTxClient == clients[clientTxIndex].thisClient) && ((lastTxData.reqID & 0x3FFF) == (transmitQueue[hlpQuePtr].reqID & 0x3FFF)) && ((transmitQueue[hlpQuePtr].errorFlags & msgEcho) > 0))
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
*/	
				que_rdPos = hlpQuePtr; //if not successful, we keep trying
/*
				}
*/
			}
		}
		else
			que_rdPos = que_wrPos; //no client, so reset out queue to prevent overflow
	}
	else
	{
		if (!lntcpClient.thisClient->connected())
		{
			long now = millis();
			if (now - lastReconnectAttempt > reconnectInterval) 
			{
				reconnectInterval = min(reconnectInterval+10000, 60000); //increae interval
				lastReconnectAttempt = now;
				Serial.print("Trying to connect to WiThrottle server at ");
				Serial.println(lbs_IP);
				lntcpClient.thisClient->connect(lbs_IP, lbs_Port);
			}
		}
		else
			if (que_wrPos != que_rdPos)
			{
//				Serial.println("Send message to server");

				if (lntcpClient.thisClient->canSend())
				{
					int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
					String msgStr = getWIMessageString(lntcpClient.thisClient, transmitQueue[hlpQuePtr]);
					if (msgStr != "")
						if (sendWIClientMessage(lntcpClient.thisClient, msgStr))
							que_rdPos = hlpQuePtr; 
						else
							return; //if not successful, we try next time
				}
			}
			else // periodic pinging of server
			{
				
				if ((millis() > nextPingPoint) || sendID)
				{
					if (pingSent)
						lntcpClient.thisClient->stop();
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
						if (sendLNClientMessage(clients[clientTxIndex].thisClient, "SENT OK", transmitQueue[hlpQuePtr]))
						{
							clientTxConfirmation = false;
							clientTxIndex++;
						}
					}
					else
						if (sendLNClientMessage(clients[clientTxIndex].thisClient, "RECEIVE", transmitQueue[hlpQuePtr]))
						{
							if ((lastTxClient == clients[clientTxIndex].thisClient) && ((lastTxData.reqID & 0x3FFF) == (transmitQueue[hlpQuePtr].reqID & 0x3FFF)) && ((transmitQueue[hlpQuePtr].errorFlags & msgEcho) > 0))
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
		if (!lntcpClient.thisClient->connected())
		{
			long now = millis();
			if (now - lastReconnectAttempt > reconnectInterval) 
			{
				reconnectInterval = min(reconnectInterval+10000, 60000); //increae interval
				lastReconnectAttempt = now;
				Serial.print("Trying to connect to TCP server ");
				Serial.print(lbs_IP);
				Serial.printf(" Port %i\n", lbs_Port);
				Serial.println(isWiThrottle);
				lntcpClient.thisClient->connect(lbs_IP, lbs_Port);
			}
		}
		else
			if (que_wrPos != que_rdPos)
			{
//				Serial.print("Send message to server");
				if (lntcpClient.thisClient->canSend())
				{
					int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
					if (sendLNClientMessage(lntcpClient.thisClient, "SEND", transmitQueue[hlpQuePtr]))
						que_rdPos = hlpQuePtr; 
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
						lntcpClient.thisClient->stop();
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
    String hlpStr = "IoTT_Stick_M5_" + String((uint32_t)ESP.getEfuseMac());
    if (sendID)
    {
		sendID = false;
		sendWIClientMessage(lntcpClient.thisClient, "N" + WiFi.localIP().toString() + '\r' + '\n' + "HU" + hlpStr);
	}
	else
		sendWIClientMessage(lntcpClient.thisClient, "N" + WiFi.localIP().toString());
	pingSent = true;
}

void IoTT_LBServer::sendLNPing()
{
//	Serial.println("LN Ping request");
	lnTransmitMsg txData;
	txData.lnMsgSize = 2;
	txData.lnData[0] = 0x81;
	txData.lnData[1] = 0x7E;
	lnWriteMsg(txData);
	pingSent = true;
}

