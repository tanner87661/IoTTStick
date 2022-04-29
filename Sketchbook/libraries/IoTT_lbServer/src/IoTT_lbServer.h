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
#include <IoTT_DigitraxBuffers.h>
#include <ArduinoJSON.h>
#include <AsyncTCP.h>
#include <vector>


#define lbs_reconnectStartVal 10000
#define queBufferSize 50 //messages that can be written in one burst before buffer overflow

extern IoTT_DigitraxBuffers * digitraxBuffer;
extern void prepSlotReadMsg(lnTransmitMsg * msgData, uint8_t slotNr);

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
	void initWIServer(bool serverMode = false); //server mode not supported at this time
	void startServer();
	void processLoop();
	uint16_t lnWriteMsg(lnTransmitMsg txData);
	uint16_t lnWriteMsg(lnReceiveBuffer txData);
	void setLNCallback(cbFct newCB);
	void loadLBServerCfgJSON(DynamicJsonDocument doc);
	String getServerIP();
	uint8_t getConnectionStatus();
  
private:
   // Member functions
	bool sendLNMessage(lnReceiveBuffer txData);

	static void onConnect(void *arg, AsyncClient *client);
	static void handleNewClient(void* arg, AsyncClient* client);
	/* clients events */
	static void handleError(void* arg, AsyncClient* client, int8_t error);
	static void handleDataFromServer(void* arg, AsyncClient* client, void *data, size_t len);
	static void handleDataFromClient(void* arg, AsyncClient* client, void *data, size_t len);
	static void handleData(void* arg, AsyncClient* client, char *data, size_t len);
	static void handleDisconnect(void* arg, AsyncClient* client);
	static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time);
    static void handleLNPoll(void *arg, AsyncClient *client);        //every 125ms when connected
    static void handleWIPoll(void *arg, AsyncClient *client);        //every 125ms when connected

	static void tcpToLN(char * str, lnReceiveBuffer * recData);
	void strToWI(char * str, lnReceiveBuffer * recData);

	void processLoopLN(); //process function for LN over TCP
	void processLoopWI(); //process function for WiThrottle
   // Member variables
    AsyncServer * lntcpServer = NULL;
    bool isServer = true;
	
	uint32_t lastReconnectAttempt = millis();
	lnReceiveBuffer transmitQueue[queBufferSize];
	uint8_t que_rdPos = 0, que_wrPos = 0;
    bool sendLNClientMessage(AsyncClient * thisClient, String cmdMsg, lnReceiveBuffer thisMsg);
	String getWIMessageString(AsyncClient * thisClient, lnReceiveBuffer thisMsg);
    static bool sendWIClientMessage(AsyncClient * thisClient, String cmdMsg);
    void sendLNPing();
	void sendWIPing();
	void clearWIThrottle(AsyncClient * thisClient);
	static void processLNServerMessage(AsyncClient* client, char * data);
	static bool processWIServerMessage(AsyncClient* client, char * data);
	uint8_t numWrite, numRead;
   
	uint32_t respTime;
	uint8_t  respOpCode;
	uint16_t respID;
	
	uint16_t clientTxIndex = 0;
	bool clientTxConfirmation = false;
};

#endif
