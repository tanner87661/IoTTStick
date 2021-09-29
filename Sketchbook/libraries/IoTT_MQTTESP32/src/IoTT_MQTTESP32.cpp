/*

MQTT Access library to send LocoNet commands to an MQTT broker and recieve tem
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

#include <IoTT_MQTTESP32.h>


#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

char lnPingTopic[100] = "lnPing";  //ping topic, do not change. This is helpful to find Gateway IP Address if not known. 
char lnBCTopic[100] = "lnIn";  //default topic, can be specified in mqtt.cfg. Useful when sending messages from 2 different LocoNet networks
char lnEchoTopic[100] = "lnEcho"; //default topic, can be specified in mqtt.cfg

char thisNodeName[60] = ""; //default topic, can be specified in mqtt.cfg

cbFct mqttCallback = NULL;
mqttFct nativeCallback = NULL;
//cbFct mqttappCallback = NULL;

uint8_t workMode = 0; //0: LN; 1: DCC; 2: NativeMQTT; 3: DCC from MQTT

MQTTESP32::MQTTESP32():PubSubClient()
{
	setCallback(psc_callback);
	nextPingPoint = millis() + pingDelay;
}

MQTTESP32::~MQTTESP32()
{
}

MQTTESP32::MQTTESP32(Client& client):PubSubClient(client)
{
	setCallback(psc_callback);
	nextPingPoint = millis() + pingDelay;
}

void MQTTESP32::psc_callback(char* topic, byte* payload, unsigned int length)
{
	payload[length] = 0;
	if (nativeCallback)
	{
		nativeCallback(topic, payload, length);
		return;
	}
	else
	{
		DynamicJsonDocument doc(4 * length);
		DeserializationError error = deserializeJson(doc, payload);
		switch(workMode)
		{
			case 0: //LocoNet, OLCB
				if (!error)
				{
					if (doc.containsKey("From"))
					{
						lnReceiveBuffer recData;
						String sentFrom = doc["From"];
						if ((strcmp(topic, lnPingTopic) == 0) && (strcmp(sentFrom.c_str(), thisNodeName) != 0))
						{
							//this is a ping command from another node
						}
						if (strcmp(topic, lnBCTopic) == 0) //can be new message or echo from our own message
						{
							if (doc.containsKey("Data"))
							{
								recData.lnMsgSize = doc["Data"].size();
								for (int j=0; j < recData.lnMsgSize; j++)  
									recData.lnData[j] = doc["Data"][j];
								if (doc.containsKey("ReqID"))
									recData.reqID = (uint16_t) doc["ReqID"];
								else
									recData.reqID = 0;
								if (doc.containsKey("ReqRespTime"))
									recData.reqRespTime = doc["ReqRespTime"];
								else
									recData.reqRespTime = 0;
								if (doc.containsKey("ReqRecTime"))
									recData.reqRecTime = doc["ReqRecTime"];
								else
									recData.reqRecTime = 0;
								if (doc.containsKey("EchoTime"))
									recData.echoTime = doc["EchoTime"];
								else
									recData.echoTime = 0;
								recData.msgType = doc["MsgType"];
								recData.errorFlags = 0;
								if (strcmp(sentFrom.c_str(), thisNodeName) == 0)
								{
									recData.errorFlags |= msgEcho;
									recData.echoTime = micros() - recData.reqRecTime;
									recData.reqID &= 0x3FFF; //clear flag to transmit to App
//									Serial.println(recData.errorFlags);
								}
								else
									recData.reqID |= 0xC000;
//								Serial.printf("MQTT Rx %2X\n", recData.lnData[0]);
								if (mqttCallback != NULL)
									mqttCallback(&recData);
//							else
//								if (onMQTTMessage) 
//									onMQTTMessage(&recData);
							}
						}
					}
				}
		}
	}//else
}

void MQTTESP32::setNodeName(char * newName, bool newUseMAC)
{
	strcpy(&thisNodeName[0], newName);
	useMAC = newUseMAC;
//	strcpy(&thisNodeName[0], nodeName);
	if (useMAC)
	{
		String hlpStr = String(ESP_getChipId());
	    strcpy(&thisNodeName[strlen(thisNodeName)], hlpStr.c_str());
	}
}

void MQTTESP32::loadMQTTCfgJSON(DynamicJsonDocument doc)
{
	if (doc.containsKey("MQTTServer"))
		strcpy(mqtt_server, doc["MQTTServer"]);
    if (doc.containsKey("MQTTPort"))
        mqtt_port = doc["MQTTPort"];
    if (doc.containsKey("MQTTUser"))
		strcpy(mqtt_user, doc["MQTTUser"]);
    if (doc.containsKey("MQTTPassword"))
        strcpy(mqtt_password, doc["MQTTPassword"]);
    if (doc.containsKey("NodeName"))
        strcpy(nodeName, doc["NodeName"]);
    if (doc.containsKey("inclMAC"))
        includeMAC = doc["inclMAC"];
    if (doc.containsKey("pingDelay"))
        setPingFrequency(doc["pingDelay"]);
    if (doc.containsKey("BCTopic"))
        strcpy(appBCTopic, doc["BCTopic"]);
    if (doc.containsKey("DCCTopic"))
        strcpy(appDCCTopic, doc["DCCTopic"]);
    if (doc.containsKey("EchoTopic"))
        strcpy(appEchoTopic, doc["EchoTopic"]);
    if (doc.containsKey("PingTopic"))
        strcpy(appPingTopic, doc["PingTopic"]);

    setServer(mqtt_server, mqtt_port);
    setNodeName(nodeName, includeMAC);
    setBCTopicName(appBCTopic);
    setEchoTopicName(appEchoTopic);
    setPingTopicName(appPingTopic);
}

void MQTTESP32::setMQTTCallback(cbFct newCB, uint8_t newMode)
{
//	Serial.println("setMQTTCallback");
	nativeCallback = NULL; //make sure only one callback is active
	mqttCallback = newCB;
	workMode = newMode; //0: LocoNet, 3: DCC from MQTT
}

void MQTTESP32::setNativeMQTTCallback(mqttFct newCB, uint8_t newMode)
{
	mqttCallback = NULL; //make sure only one callback is active
	nativeCallback = newCB;
	workMode = newMode;
	switch (workMode)
	{
		case 3: //DCC from MQTT
			setBCTopicName(appDCCTopic);
			break;
	}
}

void MQTTESP32::setDCCMode()
{
	mqttCallback = NULL; //make sure only one callback is active
	nativeCallback = NULL;
	workMode = 1;
}

void MQTTESP32::sendDCCMsg(char * msgStr)
{
	if (workMode == 1)
		publish(appDCCTopic, msgStr);
}

/*
void MQTTESP32::setAppCallback(cbFct newCB)
{
	Serial.println("setAppCallback");
	mqttappCallback = newCB;
}
*/

void MQTTESP32::setBCTopicName(char * newName)
{
	strcpy(&lnBCTopic[0], newName);
}

void MQTTESP32::setEchoTopicName(char * newName)
{
	strcpy(&lnEchoTopic[0], newName);
}

void MQTTESP32::setPingTopicName(char * newName)
{
	strcpy(&lnPingTopic[0], newName);
}

bool MQTTESP32::connectToBroker()
{
	uint32_t startTime = millis();
	if (connect(thisNodeName, mqtt_user, mqtt_password)) 
	{
		// ... and resubscribe
		if (mqttCallback)
			subscribeTopics();
		else //or cause main app to have libraries to resubscribe
			subscriptionsOK = false;
	}
	else
	{
		reconnectInterval = min(reconnectInterval+10000, 60000);
		startTime = round((millis() - startTime) / 1000);
		Serial.printf("Connection failed after %i secs, error code %i\n", startTime, state());
	}
	return connected();
}

void MQTTESP32::subscribeTopics()
{
	if ((workMode == 0) || (workMode == 3)) //LN or DCC from MQTT
	{
		subscribe(lnBCTopic);
		subscribe(lnPingTopic);
		subscribe(lnEchoTopic);
	}
	subscriptionsOK = true;
}

bool MQTTESP32::mustResubscribe()
{
	return !subscriptionsOK;
}

int16_t MQTTESP32::lnWriteMsg(lnTransmitMsg txData)
{
// Serial.printf("MQTT Tx %2X\n", txData.lnData[0]);
   uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (connected() && (hlpQuePtr != que_rdPos)) //override protection
    {
//		Serial.println();
//		Serial.printf("MQTT Tx %2X", txData.lnData[0]);
//		for (int i = 1; i < txData.lnMsgSize; i++)
//			Serial.printf(", %2X", txData.lnData[i]);
//		Serial.println();
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
//		Serial.println("MQTT Write Error. Too many messages in queue");
		return -1;
	}
}

int16_t MQTTESP32::lnWriteMsg(lnReceiveBuffer txData)
{
// 	Serial.printf("MQTT Tx %2X\n", txData.lnData[0]);
    uint8_t hlpQuePtr = (que_wrPos + 1) % queBufferSize;
    if (connected() && (hlpQuePtr != que_rdPos)) //override protection
    {
//		Serial.println();
//		Serial.printf("MQTT Tx %2X", txData.lnData[0]);
//		for (int i = 1; i < txData.lnMsgSize; i++)
//			Serial.printf(", %2X", txData.lnData[i]);
//		Serial.println();
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
//		Serial.println("MQTT Write Error. Too many messages in queue");
		return -1;
	}
}

void MQTTESP32::setPingFrequency(uint16_t pingSecs)
{
	pingDelay = 1000 * pingSecs;
	nextPingPoint = millis() + pingDelay;
}

bool MQTTESP32::sendPingMessage()
{
    DynamicJsonDocument doc(1200);
    char myMqttMsg[200];
    String hlpStr = thisNodeName;
    doc["From"] = hlpStr; //NetBIOSName + "-" + ESP_getChipId();
	doc["IP"] = WiFi.localIP().toString();
	long rssi = WiFi.RSSI();
	doc["SigStrength"] = rssi;
	doc["Mem"] = ESP.getFreeHeap();
	doc["Uptime"] = round(millis()/1000);
    serializeJson(doc, myMqttMsg);
    if (connected())
    {
      if (!publish(appPingTopic, myMqttMsg)) 
      {
//        	Serial.println(F("Ping Failed"));
			return false;
      } else 
      {
//        	Serial.println(F("Ping OK!"));
			return true;
      }
    }
    else
		return false;
}

bool MQTTESP32::sendMQTTMessage(lnReceiveBuffer txData)
{
    DynamicJsonDocument doc(1200);
    char myMqttMsg[400];
    String jsonOut = "";
    String hlpStr = thisNodeName;
    doc["From"] = hlpStr; //NetBIOSName + "-" + ESP_getChipId();
    doc["ReqRecTime"] = txData.reqRecTime;
    doc["ReqRespTime"] = txData.reqRespTime;
    doc["EchoTime"] = txData.echoTime;
    doc["ReqID"] = txData.reqID;
    doc["ErrorFlags"] = txData.errorFlags;
    switch (txData.msgType)
    {
		case 0: doc["MsgType"] = "LN"; break;
		case 1: doc["MsgType"] = "LCB"; break;
	}
    doc["Valid"] = 1; //legacy data, do not use in new designs
    JsonArray data = doc.createNestedArray("Data");
    
    switch (txData.msgType)
    {
		case 0: for (byte i=0; i < txData.lnMsgSize; i++)
					data.add(txData.lnData[i]);
				break;
		case 1: byte i = 0;
				while (i < lnMaxMsgSize)
				{
					data.add(char(txData.lnData[i]));
					if (char(txData.lnData[i]) == ';')
						break;
					i++;
				}
	}
    serializeJson(doc, myMqttMsg);
    if (connected())
    {
      if ((txData.errorFlags & msgEcho) > 0)  //send echo message if echo flag is set 
//        if (!publish(lnEchoTopic, myMqttMsg))
//        {
//			return false;
//        } else 
//        {
			return true;
//        }
      else  //otherwise send BC message (in direct mode, meaning the command came in via lnOutTopic)
        if (!publish(lnBCTopic, myMqttMsg))
        {
			return false; //changed from true
        } else 
        {
			return true;
        }
    }
	return false; //changed from true
}

bool MQTTESP32::mqttPublish(char * topic, char * payload)
{
    if (connected())
        return publish(topic, payload);
    else
		return false;
}

void MQTTESP32::processLoop()
{
    if (!connected()) 
    {
      long now = millis();
      if (now - lastReconnectAttempt > reconnectInterval) 
      {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        Serial.println("Reconnect MQTT Broker");
        if (connectToBroker()) 
        {
			reconnectInterval = reconnectStartVal;
			lastReconnectAttempt = 0;
			Serial.println("Success");
        }
        else
          Serial.println("Failure");
      }
    } 
    else
    {
      // Client connected
		loop();
		if (mqttCallback)
			if (que_wrPos != que_rdPos)
			{
				int hlpQuePtr = (que_rdPos + 1) % queBufferSize;
				if (sendMQTTMessage(transmitQueue[hlpQuePtr]))
					que_rdPos = hlpQuePtr; //if not successful, we keep trying
			}
		if (pingDelay > 0)
			if (millis() > nextPingPoint)
			{
				if (sendPingMessage())
					nextPingPoint += (pingDelay);
			}
	}
	yield();
}

