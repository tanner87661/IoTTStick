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

uint32_t nextPingPoint;
uint16_t reconnectInterval = lbs_reconnectStartVal;  //if not connected, try to reconnect every 10 Secs initially, then increase if failed
bool pingSent = false;

static std::vector<tcpDef> clients; // a list to hold all clients

IPAddress lbs_IP;
uint16_t lbs_Port = 1234; // = LocoNet over TCP port number, must be set the same in JMRI or other programs

AsyncClient * lastTxClient = NULL; 
lnReceiveBuffer lastTxData;
tcpDef lntcpClient;

#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

//char thisNodeName[60] = ""; //default topic, can be specified in mqtt.cfg

cbFct lbsCallback = NULL;
//cbFct mqttappCallback = NULL;

uint8_t lbsMode = 0; //0: LN; 

IoTT_LBServer::IoTT_LBServer()
{
//	setCallback(psc_callback);
	nextPingPoint = millis() + pingInterval + random(5000);
}

IoTT_LBServer::~IoTT_LBServer()
{
	if (lntcpClient.thisClient)
		lntcpClient.thisClient->close();
}

IoTT_LBServer::IoTT_LBServer(Client& client)
{
//	setCallback(psc_callback);
	nextPingPoint = millis() + pingInterval + random(5000);
}

void IoTT_LBServer::initLBServer(bool serverMode)
{
	isServer = serverMode;
	if (isServer)
	{
		lntcpServer = new AsyncServer(WiFi.localIP(), lbs_Port);
		lntcpServer->onClient(&handleNewClient, lntcpServer);
	}
	else //client mode
	{
		lntcpClient.thisClient = new AsyncClient();
		lntcpClient.thisClient ->onData(handleDataFromClient, lntcpClient.thisClient );
		lntcpClient.thisClient ->onConnect(onConnect, lntcpClient.thisClient );
//		lntcpClient.thisClient ->onPoll(handlePoll, lntcpClient.thisClient );
	}
}

void IoTT_LBServer::startServer()
{
	if (isServer && lntcpServer)
		lntcpServer->begin();
}

void IoTT_LBServer::handleNewClient(void* arg, AsyncClient* client)
{
	Serial.printf("A new client has been connected to server, ip: %s\n", client->remoteIP().toString().c_str());

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
	client->onData(&handleDataFromServer, NULL);
	client->onError(&handleError, NULL);
	client->onDisconnect(&handleDisconnect, NULL);
	client->onTimeout(&handleTimeOut, NULL);
}

 /* clients events */
void IoTT_LBServer::handleError(void* arg, AsyncClient* client, int8_t error) 
{
  Serial.printf("\n%s\n", client->errorToString(error));
}

void IoTT_LBServer::handleDisconnect(void* arg, AsyncClient* client) 
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

void IoTT_LBServer::handleTimeOut(void* arg, AsyncClient* client, uint32_t time) 
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

uint16_t IoTT_LBServer::lnWriteMsg(lnTransmitMsg txData)
{
//	Serial.printf("LN over TCP Tx %2X\n", txData.lnData[0]);
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

void IoTT_LBServer::handleDataFromServer(void* arg, AsyncClient* client, void *data, size_t len) 
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
			handleData(arg, currClient->thisClient, (char*) data, len);
		}
	}
}

void IoTT_LBServer::handleDataFromClient(void* arg, AsyncClient* client, void *data, size_t len) 
{
//	Serial.println("handleDataFromClient");
//	Serial.write((uint8_t *)data, len);
//	Serial.println();
	if (lntcpClient.thisClient == client) 
	{
		
		if ((((char*)data)[len-1] == '\n') || (((char*)data)[len-1] == '\r'))
		{
			//if command is complete, call handle data
//			lntcpClient.rxBuffer[lntcpClient.rxPtr] = '\0';
			handleData(arg, lntcpClient.thisClient, (char*) data, len);
//			lntcpClient.rxPtr = 0;
		}
		else
			Serial.println("No CRLF in data");
	}
//	else
//		Serial.println("Not for us");
}

//this is called when data is received, either in server or client mode
void IoTT_LBServer::handleData(void* arg, AsyncClient* client, char *data, size_t len)
{
	char *p = data;
    char *subStr;
    char *strEnd = subStr + len;
    if (strchr(p, '\n') != NULL)
		while ((subStr = strtok_r(p, "\n", &p)) != NULL) // delimiter is the new line
		{
			while((*subStr=='\n') || (*subStr=='\r') && (subStr < strEnd))
				subStr++;
			processServerMessage(client, subStr);
		}
	else
		if (strchr(p, '\r') != NULL)
			while ((subStr = strtok_r(p, "\r", &p)) != NULL) // delimiter is the carriage return
			{
				while((*subStr=='\n') || (*subStr=='\r') && (subStr < strEnd))
					subStr++;
				processServerMessage(client, subStr);
			}
		else
			Serial.println("No delimiter");

}

void IoTT_LBServer::tcpToLN(char * str, lnReceiveBuffer * thisData)
{
	char * p = str;
	uint8_t xorCheckByte = 0;
//	Serial.println(p);
	while ((str = strtok_r(p, " ", &p)) != NULL) // delimiter is the space
	{
		uint8_t thisByte = strtol(str, NULL, 16) & 0x000000FF;
		thisData->lnData[thisData->lnMsgSize] = thisByte;
		xorCheckByte = xorCheckByte ^ thisByte;
		thisData->lnMsgSize++;
	}
	if (xorCheckByte != 0xFF)
		thisData->errorFlags = msgXORCheck;
}

void IoTT_LBServer::processServerMessage(AsyncClient* client, char * data)
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
		nextPingPoint = millis() + pingInterval + random(5000);
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
					Serial.printf("Error %i\n", recData.errorFlags);
//			Serial.println("Process RECEIVE complete");
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

void IoTT_LBServer::onConnect(void *arg, AsyncClient *client)
{
	nextPingPoint = millis() + pingInterval + random(5000);
	reconnectInterval = lbs_reconnectStartVal;  //if not connected, try to reconnect every 10 Secs initially, then increase if failed
	pingSent = false;
	Serial.printf("LocoNet over TCP client is now connected to server %s on port %d \n", client->remoteIP().toString().c_str(), lbs_Port);
}

bool IoTT_LBServer::sendClientMessage(AsyncClient * thisClient, String cmdMsg, lnReceiveBuffer thisMsg)
{
	if (thisClient)
//		if (thisClient->canSend())
		if (thisClient->connected() && thisClient->canSend())
		{
			String lnStr = cmdMsg;
			char hexbuf[13];
			if (thisClient->space() > strlen(lnStr.c_str()) && thisClient->canSend())
			{
				for (uint8_t i = 0; i < thisMsg.lnMsgSize; i++)
				{
					sprintf(hexbuf, " %02X", thisMsg.lnData[i]);
					lnStr += String(hexbuf);
				}
				lnStr += '\r';
				lnStr += '\n';
				thisClient->add(lnStr.c_str(), strlen(lnStr.c_str()));
				nextPingPoint = millis() + pingInterval + random(5000);
				return thisClient->send();
			}
		}
	return false;
}

void IoTT_LBServer::processLoop()
{
	if (isServer)
	{
		if (que_wrPos != que_rdPos)
		{
//			Serial.println("TCP Server send data to clients");
			int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
//			if (thisClient->canSend())
			{
				if (clientTxConfirmation)
				{
					if (sendClientMessage(clients[clientTxIndex].thisClient, "SENT OK", transmitQueue[hlpQuePtr]))
					{
						clientTxConfirmation = false;
						clientTxIndex++;
					}
				}
				else
					if (sendClientMessage(clients[clientTxIndex].thisClient, "RECEIVE", transmitQueue[hlpQuePtr]))
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
	{
		if (!lntcpClient.thisClient->connected())
		{
			long now = millis();
			if (now - lastReconnectAttempt > reconnectInterval) 
			{
				reconnectInterval = min(reconnectInterval+10000, 60000); //increae interval
				lastReconnectAttempt = now;
				Serial.print("Trying to connect to TCP server ");
				Serial.println(lbs_IP);
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
					if (sendClientMessage(lntcpClient.thisClient, "SEND", transmitQueue[hlpQuePtr]))
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
						lntcpClient.thisClient->stop();
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

void IoTT_LBServer::sendLNPing()
{
//	Serial.println("Ping request");
	lnTransmitMsg txData;
	txData.lnMsgSize = 2;
	txData.lnData[0] = 0x81;
	txData.lnData[1] = 0x7E;
	lnWriteMsg(txData);
	pingSent = true;
}

