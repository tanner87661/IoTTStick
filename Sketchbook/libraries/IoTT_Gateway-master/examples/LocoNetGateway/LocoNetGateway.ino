#include <arduino.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/tzapu/WiFiManager or install using library manager

AsyncWebServer server(80);
DNSServer dnsServer;
AsyncWiFiManager wifiManager(&server,&dnsServer);
WiFiClient wifiClient;

#include <IoTT_LocoNetHBESP32.h> //use hybrid LocoNet library as Wifi is active
#include <IoTT_MQTTESP32.h> //this is the MQTT access library to send and receive LocoNet messages to/from MQTT broker
#include <IoTT_Gateway.h>

/////////USER CONFIGURATION//////////////////////////////////////////
#define pinRx    22  //pin used to receive LocoNet signals
#define pinTx    23  //pin used to transmit LocoNet signals
#define InverseLogic true
#define showPerformance

const char* mqtt_server = "broker.hivemq.com"; //replace with the ip of your (local) MQTT broker or use this free one
uint16_t mqtt_port = 1883; // = Mosquitto port number, standard is 1883, 8883 for SSL connection (use WiFiSecureClient in this case);
char mqtt_user[50] = "";
char mqtt_password[50] = "";
char mqtt_NodeName[50] = "myGateway";

/////////END OF USER CONFIGURATION//////////////////////////////////////////

//this implementation is using Hardware Serial port 2 by default
LocoNetESPSerial lnSerial(pinRx, pinTx, InverseLogic); //true is inverted signals
MQTTESP32 lnMQTT(wifiClient); //true is inverted signals
ln_mqttGateway commGateway(&lnSerial, &lnMQTT, &callbackLocoNetMessage);

//used for performance analysis
#ifdef showPerformance
  uint16_t loopCtr = 0;
  uint32_t myTimer = millis() + 1000;
#endif

void callbackLocoNetMessage(lnReceiveBuffer * newData)
{
   Serial.printf("LN Msg Error Flags %2X ReqID %i with %i bytes requested %i response time %i echo time %i: ", newData->errorFlags, newData->reqID, newData->lnMsgSize, newData->reqRecTime, newData->reqRespTime, newData->echoTime); 
   for (int i=0; i<newData->lnMsgSize; i++)
   {
     Serial.print(newData->lnData[i],16);
     Serial.print(" ");
   }
   Serial.println();
}
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  wifiManager.autoConnect();
  lnSerial.setBusyLED(LED_BUILTIN);

  lnMQTT.setBCTopicName("lnIn");
  lnMQTT.setEchoTopicName("lnEcho");
  lnMQTT.setPingTopicName("lnPing");
  lnMQTT.setServer(mqtt_server, mqtt_port);
  lnMQTT.setNodeName(mqtt_NodeName);
  lnMQTT.connectToBroker();

  commGateway.setCommMode(GW); //set to Gateway Mode. Could also be LN or MQTT if used as interface
  Serial.println("Init Done");
  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
}

void sendBDInput(uint16_t bdNum, bool bdStat)
{
  lnTransmitMsg myMsg;
  myMsg.lnMsgSize = 4;
  myMsg.reqID = random(2000);
  myMsg.lnData[0] = 0xB2; //B2 1A 50 7
  myMsg.lnData[1] = (bdNum & 0x7E) >> 1;
  myMsg.lnData[2] = ((uint8_t)bdStat<<5) | ((bdNum & 0x01)<<4);
  myMsg.lnData[3] = myMsg.lnData[0] ^ myMsg.lnData[1] ^ myMsg.lnData[2] ^ 0xFF;
  if (lnSerial.carrierOK())
  {
    int numBytes = lnSerial.lnWriteMsg(myMsg);
    Serial.printf("Write %i bytes for ID %i Target %i\n", numBytes, myMsg.reqID, myMsg.lnMsgSize);
  }
  else
    Serial.println("LocoNet not connected");
}

void sendLocoReq(uint8_t LocoNum)
{
  lnTransmitMsg myMsg;
  myMsg.lnMsgSize = 4;
  myMsg.reqID = random(2000);
  myMsg.lnData[0] = 0xBF; //B2 1A 50 7
  myMsg.lnData[1] = 0;
  myMsg.lnData[2] = LocoNum;
  myMsg.lnData[3] = myMsg.lnData[0] ^ myMsg.lnData[1] ^ myMsg.lnData[2] ^ 0xFF;
  if (lnSerial.carrierOK())
  {
    int numBytes = lnSerial.lnWriteMsg(myMsg);
    Serial.printf("Write %i bytes for ID %i Target %i\n", numBytes, myMsg.reqID, myMsg.lnMsgSize);
  }
  else
    Serial.println("LocoNet not connected");
}

void loop() {
  // put your main code here, to run repeatedly:

#ifdef showPerformance
  loopCtr++;
  if (millis() > myTimer)
  {
    Serial.printf("Timer Loop: %i\n", loopCtr);
    loopCtr = 0;
    myTimer += 1000;
  }
#endif

  if (Serial.available())
  {
    char c;
    while (Serial.available())
      c = Serial.read();
    Serial.println("Sending LN Messages:");
    for (int i = 0; i < 8; i++)
    {
      sendLocoReq(50+i);
      sendBDInput(i, false);
      sendBDInput(i, true);
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  { 
    commGateway.processLoop();
  }
  else
    WiFi.reconnect();

  yield();
}
