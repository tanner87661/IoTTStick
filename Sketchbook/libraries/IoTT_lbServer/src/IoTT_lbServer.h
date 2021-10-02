/*
IoTT_lbServer.h

MQTT interface to send and receive Loconet messages to and from an MQTT broker

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

#ifndef IoTT_lbServer_h
#define IoTT_lbServer_h

#include <stdlib.h>
#include <arduino.h>
#include <Math.h>
#include <inttypes.h>
#include <WiFi.h>
#include <IoTT_CommDef.h>
#include <ArduinoJSON.h>
#include <AsyncTCP.h>
#include <vector>


#define lbs_reconnectStartVal 10000
#define pingInterval 10000 //ping every 10 secs if there is no other traffic
#define queBufferSize 50 //messages that can be written in one burst before buffer overflow

typedef struct
{
	AsyncClient * thisClient;
//	char rxBuffer[200];
//	uint8_t rxPtr = 0;
} tcpDef;


class IoTT_LBServer
{
public:
	IoTT_LBServer();
	~IoTT_LBServer();
	IoTT_LBServer(Client& client);
	void initLBServer(bool serverMode = true);
	void startServer();
	void processLoop();
	uint16_t lnWriteMsg(lnTransmitMsg txData);
	uint16_t lnWriteMsg(lnReceiveBuffer txData);
	void setLNCallback(cbFct newCB);
	void loadLBServerCfgJSON(DynamicJsonDocument doc);
  
private:
   // Member functions
	bool sendLNMessage(lnReceiveBuffer txData);

	static void onConnect(void *arg, AsyncClient *client);
	static void handleNewClient(void* arg, AsyncClient* client);
	/* clients events */
	static void handleError(void* arg, AsyncClient* client, int8_t error);
	static void handleDataFromServer(void* arg, AsyncClient* client, void *data, size_t len);
	static void handleDataFromClient(void* arg, AsyncClient* client, void *data, size_t len);
//	static void handleData(void* arg, tcpDef * clientData);
	static void handleData(void* arg, AsyncClient* client, char *data, size_t len);
	static void handleDisconnect(void* arg, AsyncClient* client);
	static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time);
//    static void handlePoll(void *arg, AsyncClient *client);        //every 125ms when connected

	static void tcpToLN(char * str, lnReceiveBuffer * recData);

   // Member variables
    AsyncServer * lntcpServer = NULL;
    bool isServer = true;
    
	uint32_t lastReconnectAttempt = millis();
	lnReceiveBuffer transmitQueue[queBufferSize];
	uint8_t que_rdPos = 0, que_wrPos = 0;
    bool sendClientMessage(AsyncClient * thisClient, String cmdMsg, lnReceiveBuffer thisMsg);
    void sendLNPing();
	static void processServerMessage(AsyncClient* client, char * data);
	uint8_t numWrite, numRead;
   
	uint32_t respTime;
	uint8_t  respOpCode;
	uint16_t respID;
	
	uint16_t clientTxIndex = 0;
	bool clientTxConfirmation = false;
};

#endif
