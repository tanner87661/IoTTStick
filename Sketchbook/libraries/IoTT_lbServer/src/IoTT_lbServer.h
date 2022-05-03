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
//extern void callbackLocoNetMessage(lnReceiveBuffer * newData);

typedef struct
{
	AsyncClient * thisClient = NULL;
	char * wiHWIdentifier = NULL;
	char * wiDeviceName = NULL;
	uint32_t nextPing = millis();
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

	void handleError(AsyncClient* client, int8_t error);
	void handleNewClient(AsyncClient* client);
	void handleDataFromClient(AsyncClient* client, void *data, size_t len);
	void handleConnect(AsyncClient *client);

	void handleDataFromServer(AsyncClient* client, void *data, size_t len);
	void handleTimeOut(AsyncClient* client, uint32_t time);
    void handleLNPoll(AsyncClient *client);        //every 125ms when connected
    void handleWIPoll(AsyncClient *client);        //every 125ms when connected
	void handleDisconnect(AsyncClient* client);
  
private:
   // Member functions
	bool sendLNMessage(lnReceiveBuffer txData);

	void handleData(AsyncClient* client, char *data, size_t len);
	/* clients events */

	void tcpToLN(char * str, lnReceiveBuffer * recData);
	void strToWI(char * str, lnReceiveBuffer * recData);

	void processLoopLN(); //process function for LN over TCP
	void processLoopWI(); //process function for WiThrottle
   // Member variables
    AsyncServer * lntcpServer = NULL;
    tcpDef lntcpClient;
    bool isServer = true;
	
	uint32_t lastReconnectAttempt = millis();
	lnReceiveBuffer transmitQueue[queBufferSize];
	uint8_t que_rdPos = 0, que_wrPos = 0;
    bool sendLNClientMessage(AsyncClient * thisClient, String cmdMsg, lnReceiveBuffer thisMsg);
	String getWIMessageString(AsyncClient * thisClient, lnReceiveBuffer thisMsg);
    bool sendWIClientMessage(AsyncClient * thisClient, String cmdMsg);
    void sendLNPing();
	void sendWIPing();
	void clearWIThrottle(AsyncClient * thisClient);
	void processLNServerMessage(AsyncClient* client, char * data);
	bool processWIMessage(AsyncClient* client, char * data);
	bool processWIClientMessage(AsyncClient* client, char * data);
	bool processWIServerMessage(AsyncClient* client, char * data);
	uint8_t numWrite, numRead;
   
	uint32_t respTime;
	uint8_t  respOpCode;
	uint16_t respID;
	
	uint16_t clientTxIndex = 0;
	bool clientTxConfirmation = false;
	uint32_t nextPingPoint;
	uint16_t reconnectInterval = lbs_reconnectStartVal;  //if not connected, try to reconnect every 10 Secs initially, then increase if failed
	bool pingSent = false;
	bool sendID = false;
	bool isWiThrottle = false;
	int16_t currentWIDCC = 0;
	uint16_t pingInterval = 10000; //ping every 5-10 secs if there is no other traffic

	std::vector<tcpDef> clients; // a list to hold all clients when in server mode

	IPAddress lbs_IP;
	uint16_t lbs_Port = 1234; // = LocoNet over TCP port number, must be set the same in JMRI or other programs

	uint16_t wiVersion = 0;
	char * wiServerType = NULL;
	char * wiServerMessage = NULL;
	char * wiServerDescription = NULL;
	
	AsyncClient * lastTxClient = NULL; 
	lnReceiveBuffer lastTxData;
};

#endif
