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
#include <ESPmDNS.h>
#include <vector>


#define lbs_reconnectStartVal 10000
#define queBufferSize 50 //messages that can be written in one burst before buffer overflow

extern IoTT_DigitraxBuffers * digitraxBuffer;
extern void prepSlotReadMsg(lnTransmitMsg * msgData, uint8_t slotNr);
extern void prepLocoAddrReqMsg(lnTransmitMsg * msgData, uint16_t dccAddr);
extern void prepSlotMoveMsg(lnTransmitMsg * msgData, uint8_t slotSRC, uint8_t slotDEST);
extern void prepSlotStat1Msg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t stat);
extern void prepSlotSpeedMsg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t speed);
extern void prepSlotDirFMsg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t dirfdata);
extern void prepSlotSndMsg(lnTransmitMsg * msgData, uint8_t slotNr, uint8_t snddata);
extern void prepTurnoutMsg(lnTransmitMsg * msgData, bool useACK, uint16_t swiAddr, uint8_t swiPos);
extern void prepTrackPowerMsg(lnTransmitMsg * msgData, uint8_t pwrStatus);

extern void callbackLocoNetMessage(lnReceiveBuffer * newData);
extern uint16_t sendMsg(lnTransmitMsg txData);

typedef struct
{
	uint16_t locoAddr = 0;
	uint8_t slotNum = 0;
	uint16_t dirFct = 0;
	uint16_t noLatchFct = 0x0200; //Fct 2 is momentary
	uint8_t slotStatus = 0;
	bool activeSlot = false;
	char throttleID;
} locoDef;

#define numInitSeq 11

//typedef struct
class tcpDef
{
public:
	tcpDef();
	~tcpDef();
	void addLoco(uint16_t locoAddr, char thID);
	void removeLoco(uint16_t locoAddr, char thID);
	void stealLoco(uint16_t locoAddr, char thID);
	void confirmLoco(uint8_t slotAddr, uint16_t locoAddr, char thID, uint8_t slotStat, uint8_t slotSpeed, uint16_t dirFctFlags);
	void setTurnout(char pos, char* addr);
	locoDef* getLocoByAddr(locoDef* startAt, uint16_t locoAddr, char thID);
	locoDef* getLocoBySlot(locoDef* startAt, uint8_t slotAddr, char thID);
	void setLocoAction(uint16_t locoAddr, char thID, char* ActionCode);
	void setTrackPowerStatus(uint8_t newStatus);
public:
	AsyncClient * thisClient = NULL;
	char * wiHWIdentifier = NULL;
	char * wiDeviceName = NULL;
	uint32_t nextPing = millis();
	int8_t sendInitSeq = -1;
private:	
	uint8_t currSpeed;
	std::vector<locoDef> slotList;
//	uint8_t slotListLen = 0;
};

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
	void initMDNS();
	uint16_t lnWriteMsg(lnTransmitMsg* txData);
	uint16_t lnWriteMsg(lnReceiveBuffer* txData);
//	void setLNCallback(cbFct newCB);
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
//	bool sendLNMessage(lnReceiveBuffer txData);

	void handleData(AsyncClient* client, char *data, size_t len);
	/* clients events */

	void tcpToLN(char * str, lnReceiveBuffer * recData);
	void strToWI(char * str, lnReceiveBuffer * recData);

	void processLoopLN(); //process function for LN over TCP
	void processLoopWI(); //process function for WiThrottle
   // Member variables
    AsyncServer * lntcpServer = NULL;
    AsyncClient * lntcpClient = NULL;
    bool isServer = true;
	
	uint32_t lastReconnectAttempt = millis();
	lnReceiveBuffer transmitQueue[queBufferSize];
	uint8_t que_rdPos = 0, que_wrPos = 0;
    bool sendLNClientMessage(AsyncClient * thisClient, String cmdMsg, lnReceiveBuffer thisMsg);
	String getWIMessageString(AsyncClient * thisClient, lnReceiveBuffer thisMsg);
	bool sendWIServerMessageString(AsyncClient * thisClient, uint8_t replyType);
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

	std::vector<tcpDef*> clients; // a list to hold all clients when in server mode
	std::vector<uint16_t> turnoutSupport; //a list holding the turnout numbers to be initialized in WiThrottle Server

	IPAddress lbs_IP;
	uint16_t lbs_Port = 1234; // = LocoNet over TCP port number, must be set the same in JMRI or other programs
	uint16_t lbs_ServerPort = 1234; // = LocoNet over TCP port number, must be set the same in JMRI or other programs

	uint16_t wiVersion = 0;
	char * wiServerType = NULL;
	char * wiServerMessage = NULL;
	char * wiServerDescription = NULL;
	uint8_t allowPwrChg = 0;
	AsyncClient * lastTxClient = NULL; 
	lnReceiveBuffer lastTxData;
};

#endif
