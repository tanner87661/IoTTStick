/*
IoTT_MQTTESP32.h

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

#ifndef IoTT_MQTTESP32_h
#define IoTT_MQTTESP32_h

#include <stdlib.h>
#include <arduino.h>
#include <Math.h>
#include <inttypes.h>
#include <WiFi.h>
#include <IoTT_CommDef.h>
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE
#include <PubSubClient.h> //standard library, install using library manager

#define reconnectStartVal 10000
#define queBufferSize 50 //messages that can be written in one burst before buffer overflow

typedef struct
{
	char * bcTopic = NULL;
	char * echoTopic = NULL;
	char * pingTopic = NULL;
	char * dccTopic = NULL;
	char * thisNodeName = NULL;
} topicList;


class MQTTESP32 : public PubSubClient
{
public:
//	MQTTESP32();
	~MQTTESP32();
	MQTTESP32(Client& client);
	void initializeMQTT(uint8_t newMode);
	void processLoop();
	int16_t lnWriteMsg(lnTransmitMsg* txData);
	int16_t lnWriteMsg(lnReceiveBuffer* txData);
	void setNodeName(char * newName, bool newUseMAC = true);
//	void setBCTopicName(char * newName);
//	void setEchoTopicName(char * newName);
//	void setPingTopicName(char * newName);
	void setPingFrequency(uint16_t pingSecs);
	bool connectToBroker();
	void subscribeTopics();
	bool mustResubscribe();
//	void setDCCMode();
	void sendDCCMsg(char * msgStr);
//	void setMQTTCallback(cbFct newCB, uint8_t newMode = 0);
//	void setNativeMQTTCallback(mqttFct newCB, uint8_t newMode = 2);
//	void setAppCallback(cbFct newCB);
	void loadMQTTCfgJSON(DynamicJsonDocument doc);
    bool mqttPublish(char * topic, char * payload);
	topicList myTopics;
  
//	char lnPingTopic[100] = "lnPing";  //ping topic, do not change. This is helpful to find Gateway IP Address if not known. 
	uint8_t workMode = 0xFF; //0: LN; 1: DCC; 2: NativeMQTT; 3: DCC from MQTT

private:
   // Member functions
	Client * wifiClientMQTT = NULL;
	bool sendMQTTMessage(lnReceiveBuffer txData);
	bool sendPingMessage();
	bool subscriptionsOK = false;
	uint16_t reconnectInterval = reconnectStartVal;  //if not connected, try to reconnect every 10 Secs initially, then increase if failed
	uint32_t lastReconnectAttempt = millis();
//	static void psc_callback(char* topic, byte* payload, unsigned int length);
 
   // Member variables
	lnReceiveBuffer transmitQueue[queBufferSize];
	uint8_t que_rdPos = 0, que_wrPos = 0;
	lnReceiveBuffer lnInBuffer;
   
	char nodeName[50] = "IoTT-MQTT";	
	bool useMAC = true;

	uint8_t numWrite, numRead;
   
	uint32_t nextPingPoint;
	uint32_t pingDelay = 300000; //5 Mins
	uint32_t respTime;
	uint8_t  respOpCode;
	uint16_t respID;
   
	
	char mqtt_server[50] = "broker.hivemq.com"; // = Mosquitto Server IP "192.168.xx.xx" as loaded from mqtt.cfg
	uint16_t mqtt_port = 1883; // = Mosquitto port number, standard is 1883, 8883 for SSL connection;
	char mqtt_user[50] = "";
	char mqtt_password[50] = "";

//	char appPingTopic[100] = "lnPing";  //ping topic, do not change. This is helpful to find Gateway IP Address if not known. 
//	char appBCTopic[100] = "lnIn";  //default topic, can be specified in mqtt.cfg. Useful when sending messages from 2 different LocoNet networks
//	char appEchoTopic[100] = "lnEcho"; //default topic, can be specified in mqtt.cfg
//	char appDCCTopic[100] = "dccBC";  //default topic, can be specified in mqtt.cfg. Useful when sending messages from 2 different LocoNet networks
	bool includeMAC = true;

};

//this is the callback function. Provide a function of this name and parameter in your application and it will be called when a new message is received
//extern void onMQTTMessage(lnReceiveBuffer * recData) __attribute__ ((weak));

extern uint16_t sendMsg(lnTransmitMsg txData);
extern void callbackLocoNetMessage(lnReceiveBuffer * newData);
extern void dccClientCallback(char* topic, byte *  payload, unsigned int length);
extern void nativeClientCallback(char* topic, byte *  payload, unsigned int length);

#endif
