/* Router guidelines
 *  - each call to an interface write function should send a valid LocoNet message and a reqID parameter
 *  - the reqID value should be a unique number from 0 to 16383 (lower 14 bits of reqID
 *  - the reqID msb bits are used as routing table and are set by the router for these gateway routing cases
 *  rtc0: originally received on LocoNet Interface, routed to application and MQTT Broker. reqID of such message is 0 or reply reqID
 *  rtc1: originally received on MQTT interface ("From" is not us, echo flag cleared), routed to LocoNet, and echo of it to application and as echoTopic to MQTT
 *  rtc2: when echo flag is set when receiving from LocoNet: outgoing message from application, routed to LocoNet, and echo of it to application and MQTT
 *        when echo flag is cleared when receiving from LocoNet: reply to earlier outgoing message from application, routed to application and MQTT
 *  rtc3: originally received from a TCP client
 */
#define rtc0 0x0000
#define rtc1 0x4000
#define rtc2 0x8000
#define rtc3 0xC000

#include <IoTT_Gateway.h>

cbFct appCallback = NULL;
LocoNetESPSerial * serialPort = NULL;
IoTT_OpenLCB * olcbPort = NULL;
MQTTESP32 * mqttPort = NULL;
IoTT_LBServer * tcpPort = NULL;
//cmdSourceType workMode = GW;
messageType gwType = LocoNet;

ln_mqttGateway::ln_mqttGateway()
{
}

ln_mqttGateway::ln_mqttGateway(LocoNetESPSerial * newLNPort, MQTTESP32 * newMQTTPort, cbFct newCB)
{
	gwType = LocoNet;
	if (newLNPort)
	  setSerialPort(newLNPort);
	if (newMQTTPort)
  	  setMQTTPort(newMQTTPort);
	if (newCB)
  	  setAppCallback(newCB);
}

ln_mqttGateway::ln_mqttGateway(LocoNetESPSerial * newLNPort, MQTTESP32 * newMQTTPort, IoTT_LBServer * newTCPPort, cbFct newCB)
{
	ln_mqttGateway(newLNPort, newMQTTPort, newCB);
	if (newTCPPort)
  	  setTCPPort(newTCPPort);
}

ln_mqttGateway::ln_mqttGateway(IoTT_OpenLCB * newOLCBPort, MQTTESP32 * newMQTTPort, cbFct newCB)
{
	gwType = OpenLCB;
	if (newOLCBPort)
	  setOLCBPort(newOLCBPort);
	if (newMQTTPort)
  	  setMQTTPort(newMQTTPort);
	if (newCB)
  	  setAppCallback(newCB);
}

ln_mqttGateway::~ln_mqttGateway()
{
}

void ln_mqttGateway::setSerialPort(LocoNetESPSerial * newPort)
{
	serialPort = newPort;
	serialPort->setLNCallback(&onLocoNetMessage);
}

void ln_mqttGateway::setOLCBPort(IoTT_OpenLCB * newPort)
{
	olcbPort = newPort;
	olcbPort->setOlcbCallback(&onOLCBMessage, false);
}

void ln_mqttGateway::setMQTTPort(MQTTESP32 * newPort)
{
	mqttPort = newPort;
	mqttPort->setMQTTCallback(&onMQTTMessage);
}

void ln_mqttGateway::setTCPPort(IoTT_LBServer * newPort)
{
	tcpPort = newPort;
	tcpPort->setLNCallback(&onTCPMessage);
}

void ln_mqttGateway::setAppCallback(cbFct newCB)
{
	appCallback = newCB;
}

/*
void ln_mqttGateway::setCommMode(cmdSourceType newMode)
{
	workMode = newMode;
}
*/

void ln_mqttGateway::onLocoNetMessage(lnReceiveBuffer * newData) //this is the callback function for the LocoNet library
{
//	Serial.printf("GW onLocoNetBMsg %2X %2X %i\n", newData->lnData[0], newData->errorFlags, (newData->reqID & rtc3) >> 14);
      switch ((newData->reqID & rtc3) >> 14)
      {
        case 0: //new message from LocoNet
			if (appCallback)
				appCallback(newData);
			if ((newData->errorFlags & (~msgEcho)) == 0)// && (newData->lnMsgSize > 0))//filter out echo flag
				if (getXORCheck(&newData->lnData[0], newData->lnMsgSize))
				{
					if (mqttPort)
						mqttPort->lnWriteMsg(*newData); //send to MQTT        
					if (tcpPort)
						tcpPort->lnWriteMsg(*newData); // then send to TCP
				}
			break;    
        case 1: //originally received from MQTT, than transmitted on LocoNet, now going to the application and TCP
          newData->reqID &= (~rtc3); //clear routing flags
          if ((newData->errorFlags & msgEcho) == 0) //this is a reply message to a request originating from MQTT
			if (mqttPort)
				mqttPort->lnWriteMsg(*newData); // then send to MQTT
          newData->errorFlags &= ~msgEcho; //clear echo flag as this was an original incoming message
		  if (tcpPort)
			tcpPort->lnWriteMsg(*newData);
		  if (appCallback)
			appCallback(newData);
          break;    
        case 2: //sent by the application to Loconet, now the echo goes to MQTT and TCP lbServer. Also: could be a reply to an earlier message. In this case, send to application as well
          newData->reqID &= (~rtc3); //clear routing flags
		  if (appCallback)
			appCallback(newData);
          newData->errorFlags &= ~msgEcho; //clear echo flag, 
          if (mqttPort)
			mqttPort->lnWriteMsg(*newData); // then send to MQTT
          if (tcpPort)
			tcpPort->lnWriteMsg(*newData); // then send to MQTT
          break;    
        case 3: //originally received from TPC, than transmitted on LocoNet, now going to the application and MQTT 
//          Serial.printf("GW LN Size %i Errors %i\n", newData->lnMsgSize, newData->errorFlags); 
          newData->reqID &= (~rtc3); //clear routing flags
		  if (tcpPort)
			tcpPort->lnWriteMsg(*newData);
          newData->errorFlags &= ~msgEcho; //clear echo flag as this was an original incoming message
		  if (mqttPort)	
			mqttPort->lnWriteMsg(*newData); // then send to MQTT
		  if (appCallback)
			appCallback(newData);
          break;    
      }
}

void ln_mqttGateway::onOLCBMessage(lnReceiveBuffer * newData) //this is the callback function for the OLCB library
{
//	Serial.printf("GW onOLCBMsg %2X %2X\n", newData->lnData[0], newData->errorFlags);
	newData->reqID |= 0xC000; //set router flag
	if (appCallback) appCallback(newData); 
	if (mqttPort) mqttPort->lnWriteMsg(*newData);        
}

void ln_mqttGateway::onMQTTMessage(lnReceiveBuffer * newData) //this is the callback function for the MQTT library
{
//	Serial.printf("GW onMQTTMsg %2X %2X\n", newData->lnData[0], newData->errorFlags);
    newData->reqID = (newData->reqID & 0x3FFF) | rtc1; //set router case 1
    switch (gwType)
    {
		case LocoNet: 
			if ((newData->errorFlags & msgEcho) == 0)
			{
				if (serialPort) serialPort->lnWriteMsg(*newData); 
				break;
			}
		case OpenLCB: 
			if ((newData->errorFlags & msgEcho) == 0)
			{
				if (appCallback) appCallback(newData);
				if (olcbPort) olcbPort->lnWriteMsg(*newData); 
			}
			break;
	}
}

void ln_mqttGateway::onTCPMessage(lnReceiveBuffer * newData) //this is the callback function for the MQTT library
{
//	Serial.printf("GW onTCPMsg %2X %2X\n", newData->lnData[0], newData->errorFlags);
    newData->reqID = (newData->reqID & 0x3FFF) | rtc3; //set router case 1
    switch (gwType)
    {
		case LocoNet: 
			if ((newData->errorFlags & msgEcho) == 0)
			{
				if (serialPort) serialPort->lnWriteMsg(*newData); 
				break;
			}
		case OpenLCB: 
			if ((newData->errorFlags & msgEcho) == 0)
			{
				if (appCallback) appCallback(newData);
				if (olcbPort) olcbPort->lnWriteMsg(*newData); 
			}
			break;
	}
}

uint16_t ln_mqttGateway::lnWriteMsg(lnTransmitMsg txData)
{
//	Serial.printf("GW LN Tx %2X no eF\n", txData.lnData[0]);
	if (txData.reqID == 0)
		txData.reqID = random(0x3FFF);
    txData.reqID |= rtc2; //set use case flags for outgoing message

    switch (gwType)
    {
		case LocoNet: if (serialPort) return serialPort->lnWriteMsg(txData); else return 0; break;
		case OpenLCB:
			lnReceiveBuffer txDataCopy;
			txDataCopy.msgType = txData.msgType;
			txDataCopy.lnMsgSize = txData.lnMsgSize;
			txDataCopy.reqID = txData.reqID;
			txDataCopy.reqRecTime = micros();
			memcpy(txDataCopy.lnData, txData.lnData, lnMaxMsgSize); //txData.lnMsgSize);
			if (mqttPort) mqttPort->lnWriteMsg(txData);
			if (appCallback) appCallback(&txDataCopy);
			if (olcbPort) return olcbPort->lnWriteMsg(txData); else return 0; 
			break;
	}
}

uint16_t ln_mqttGateway::lnWriteMsg(lnReceiveBuffer txData)
{
//	Serial.printf("GW LN Tx %2X %2X \n", txData.lnData[0], txData.errorFlags);
	if (txData.reqID == 0)
		txData.reqID = random(0x3FFF);
    txData.reqID |= rtc2; //set use case flags for outgoing message
    switch (gwType)
    {
		case LocoNet: if (serialPort) return serialPort->lnWriteMsg(txData); else return 0; break;
		case OpenLCB: 
			txData.reqID = (txData.reqID & 0x3FFF); //clear router flag
//			txData.reqId |= rtc2;
			if (mqttPort) mqttPort->lnWriteMsg(txData);
			if (appCallback) appCallback(&txData);
			if (olcbPort) return olcbPort->lnWriteMsg(txData); else return 0; 
			break;
	}
}

void ln_mqttGateway::processLoop()
{
    switch (gwType)
    {
		case LocoNet: if (serialPort) serialPort->processLoop(); break;
		case OpenLCB: if (olcbPort) olcbPort->processLoop(); break;
	}
    if (mqttPort)
		mqttPort->processLoop();
    if (tcpPort)
		tcpPort->processLoop();
}
