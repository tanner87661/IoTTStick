char BBVersion[] = {'1','5','D'};

//#define measurePerformance //uncomment this to display the number of loop cycles per second
#define useM5Lite
//Arduino published libraries. Install using the Arduino IDE or download from Github and install manually
#include <arduino.h>
#include <Math.h>
//#include <Wire.h>
//#include <esp_int_wdt.h>
//#include <esp_task_wdt.>

#ifdef useM5Lite
  #include "M5Lite.h"
  #define M5 M5Lite
#else
  #include <M5StickC.h>
#endif
//#include <M5StickCPlus.h>
#include <ArduinoUniqueID.h>
#include <time.h>
#include <FS.h>
#include <SPIFFS.h>
#define FORMAT_SPIFFS_IF_FAILED true
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE. Make sure to use version 6.x

//following libraries can be downloaded from https://github.com/tanner87661?tab=repositories
#include <IoTT_DigitraxBuffers.h> //as introduced in video # 30
#include <IoTT_LocoNetHBESP32.h> //this is a hybrid library introduced in video #29
#include <IoTT_MQTTESP32.h> //as introduced in video # 29
#include <IoTT_Gateway.h> //LocoNet Gateway as introduced in video # 29
#include <IoTT_RemoteButtons.h> //as introduced in video # 29, special version for Wire connection via 328P
#include <IoTT_LocoNetButtons.h> //as introduced in video # 29
#include <IoTT_LEDChain.h> //as introduced in video # 30
#include <IoTT_SerInjector.h> //Serial USB Port to make it an interface for LocoNet and OpenLCB
//#include <IoTT_OpenLCB.h> //CAN port for OpenLCB
#include <IoTT_Switches.h>
//#include <IoTT_SecurityElements.h> //not ready yet. This is the support for ABS/APB as described in Videos #20, 21, 23, 24
#include <NmraDcc.h> //install via Arduino IDE
#include <OneDimKalman.h>
#include <IoTT_lbServer.h>

//library object pointers. Libraries will be dynamically initialized as needed during the setup() function
AsyncWebServer * myWebServer = NULL; //(80)
DNSServer * dnsServer = NULL;
//WiFiClientSecure * wifiClientSec = NULL;
WiFiClient * wifiClient = NULL;
AsyncWebSocket * ws = NULL; //("/ws");
AsyncWebSocketClient * globalClient = NULL;
uint32_t wsBufferSize = 16384;
uint32_t wsRxReadPtr = 0;
uint32_t wsRxWritePtr = 0;
char * wsRxBuffer; //[32768]; //should this by dynamic?
uint32_t wsTxReadPtr = 0;
uint32_t wsTxWritePtr = 0;
char * wsTxBuffer; //[32768]; //should this by dynamic?

//global variables
bool useStaticIP = false;
IPAddress static_ip;
IPAddress static_gw;
IPAddress static_nm;
IPAddress static_dns;
IPAddress ap_ip;
IPAddress ap_nm(255,255,255,0);

uint8_t wifiCfgMode = 0x00; //1: STA, 2: AP, 3: STA+AP
String deviceName; //device name used for AP, will load from node.cfg
String apPassword; //AP password, will load from node.cfg

bool wifiAlwaysOn = false; //set to false to shut Wifi after some time of inactivity. Gateway and MQTT should be disabled, though
bool wifiCancelled = false; //true if Wifi was switched off due to no use
uint32_t lastWifiUse = millis();
#define wifiShutTimeout 120000 //after 2 Mins of not using, Wifi is closed
#define apShutTimeout 180000 //after 3 Mins of not using, AP is shut. 3 Min to give enough time to connect to the AP from a PC

String bufferFileName = "/buffers.dat";

//more library object pointers. Libraries will be dynamically initialized as needed during the setup() function
IoTT_Mux64Buttons * myButtons = NULL;
IoTT_LocoNetButtonList * eventHandler = NULL; 
//IoTT_SecurityElementList * secElHandlerList = NULL;
LocoNetESPSerial * lnSerial = NULL;
IoTT_SerInjector * usbSerial = NULL;
IoTT_LBServer * lbServer = NULL;
//IoTT_OpenLCB *olcbSerial = NULL;
//HardwareSerial *wireSerial = NULL;
ln_mqttGateway * commGateway = NULL;
IoTT_ledChain * myChain = NULL;
IoTT_SwitchList * mySwitchList = NULL;
MQTTESP32 * lnMQTT = NULL;
NmraDcc  * myDcc = NULL;

//some variables used for performance measurement
#ifdef measurePerformance
uint16_t loopCtr = 0;
uint32_t myTimer = millis() + 1000;
#endif

//********************************************************Hardware Configuration******************************************************
#define groveRxD 33 //pin 1 yellow RxD for LocoNet, DCC
#define groveTxD 32 //pin 2 white TxD for LocoNet, DCC
constexpr gpio_num_t groveRxCAN = GPIO_NUM_33; //for OpenLCB CAN
constexpr gpio_num_t groveTxCAN = GPIO_NUM_32; //for OpenLCB CAN
 
#define stickLED 10 //red LED on stick
#define hatSDA 26 //changes between input and output by I2C master
#define hatSCL 0 //pull SCL low while sending non I2C data over SDA
#define hatRxD 0
#define hatTxD 26
#define hatInputPin 36 //25+ //36
#define hatDataPin 26 //used to send LED data from FastLED. First, set to output low, then call show
#define yellowWireAddr 0x18 //I2C Address used on hats with I2C slave 328P
uint8_t hatData[] = {1, hatDataPin}; //pin definition for LED library
uint8_t analogPins[] = {hatInputPin, 2}; //analog pin used for button reading, number of button MUX (connected via I2C)

//***********************************************************************************************************************************

//M5StickCPlus M5;
//M5StickC M5;

//global variables for the NTP module
int ntpTimeout = 5000; //ms timeout for NTP update request
char ntpServer[50] = "us.pool.ntp.org"; //default server for US. Change this to the best time server for your region, or set in node.cfg
char ntpTimeZone[100] = "EST5EDT";  // default for Eastern Time Zone. Enter your time zone from here: (https://remotemonitoringsystems.ca/time-zone-abbreviations.php) into node.cfg
bool ntpOK = false;
bool useNTP = false;
bool hatVerified = false;
tm timeinfo;
time_t now;
const uint32_t ntpIntervallDefault = 86400000; //1 day in milliseconds
const uint32_t ntpIntervallShort = 10000; //10 Seconds in case something went wrong
uint32_t ntpTimer = millis();
int millisRollOver = 0;
unsigned long lastMillis = 0;
//strDateime dateTime;

//uint8_t commMode = 0xFF;
//uint8_t modMode = 0xFF;

String configDir = "/configdata";
String configExt = "cfg";
String configDotExt = ".cfg";

typedef struct
{
  char devName[60];
  uint8_t devId;
  uint8_t devCommMode;
  uint8_t * validInterfaces = NULL;
  uint8_t numValid = 0;
} configType;

configType useHat;
configType useInterface;
uint32_t useALM = 0; //bit mask configuration list

uint8_t m5CurrentPage = 0;
uint16_t oldWifiStatus = 0;
uint8_t useM5Viewer = 0;
uint8_t m5DispLine = 0;

File uploadFile; //used for web server to upload files

//this is the outgoing communication function for IoTT_DigitraxBuffers.h, routing the outgoing messages to the correct interface
uint16_t sendMsg(lnTransmitMsg txData)
{
//  Serial.printf("Call sendMsg to %i: %i, %i, %i, %i, %i \n", useInterface.devId, txData.lnMsgSize, txData.lnData[0], txData.lnData[1], txData.lnData[2], txData.lnData[3]);
//  Serial.printf("Outgoing Callback %i\n", useInterface.devId);
  switch (useInterface.devId)
  {
    case 0: break; //none
    case 10:; //DCC from MQTT
    case 1: //DCC 
      Serial.println("DCC. Not going anywhere");
      if (lnSerial) return lnSerial->lnWriteMsg(txData);
      break; 
    case 16:; //LocoNet Loopback
    case 2: if (lnSerial) return lnSerial->lnWriteMsg(txData); break;
    case 3: if (lnMQTT) return lnMQTT->lnWriteMsg(txData); break;
    case 4:; //LocoNet w/ MQTT Gateway
    case 11:; //LocoNet /w lbServer
    case 14:; //LocoNet Loopback /w lbServer
    case 15:; //LocoNet Loopback /w lbServer and MQTT
    case 13: if (commGateway) //LocoNet w/ Gateway lbServer and MQTT
              return commGateway->lnWriteMsg(txData); 
            break;
/*            
    case 5: //OpenLCB
            if (olcbSerial) olcbSerial->lnWriteMsg(txData);
            break;
    case 6: //OpenLCB over MQTT
            if (lnMQTT) lnMQTT->lnWriteMsg(txData); break;
    case 7: //OpenLCB with Gateway
            if (commGateway) //LocoNet w/ Gateway
              commGateway->lnWriteMsg(txData); 
            break;
*/            
    
    case 12: //LN over TCP Client
            if (lbServer)
              return lbServer->lnWriteMsg(txData); break;
  }
}

uint16_t sendMQTTMsg(char * topic, char * payload) //used for native MQTT only
{
  switch (useInterface.devId)
  {
    case 8: //native MQTT
      if (lnMQTT) return lnMQTT->mqttPublish(topic, payload);
      break; 
  }
}

void resetPin(uint8_t pinNr)
{
  pinMode(pinNr, OUTPUT);
  digitalWrite(pinNr,0);
  pinMode(pinNr, INPUT);
}

void setup() {
  // put your setup code here, to run once:
//  resetPin(0);
//  resetPin(26);
//  resetPin(32);
//  resetPin(33);
//  resetPin(36);

  wsRxBuffer = (char*) malloc(wsBufferSize); 
  wsTxBuffer = (char*) malloc(wsBufferSize); 
  M5.begin();
//  M5.Axp.EnableCoulombcounter();
  

  initDisplay();
  getRTCTime();
  char time_output[30];
  strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  Serial.println(time_output);
  Serial.println("Init SPIFFS");
  SPIFFS.begin(); //File System. Size is set to 1 MB during compile time and loaded with configuration data and web pages
  UniqueIDdump(Serial);
  loadFromFile(bufferFileName);
  myWebServer = new AsyncWebServer(80);
  dnsServer = new DNSServer();
  wifiClient = new WiFiClient();
  uint16_t wsRequest = 0;
  ws = new AsyncWebSocket("/ws");
  //verify some library settings
  if (MQTT_MAX_PACKET_SIZE < 480)
  {
    Serial.printf("PubSubClient.h MQTT_MAX_PACKET_SIZE only %i. Should be >= 480\n", MQTT_MAX_PACKET_SIZE);
    delay(5000);
//    hard_restart();
  ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
  }
  DynamicJsonDocument * jsonConfigObj = NULL;
  DynamicJsonDocument * jsonDataObj = NULL;
  jsonConfigObj = getDocPtr("/configdata/node.cfg", true); //read and decode the master config file. See ConfigLoader tab. Make data copy
  if (jsonConfigObj != NULL)
  {
    //first, read all Wifi Paramters
    if (jsonConfigObj->containsKey("wifiMode"))
      wifiCfgMode = (*jsonConfigObj)["wifiMode"];
    Serial.printf("Wifi Config Mode %i\n",wifiCfgMode);
//    if (jsonConfigObj->containsKey("useWifiTimeout"))
//      wifiAlwaysOn =  (!(bool)(*jsonConfigObj)["useWifiTimeout"]);
    if (jsonConfigObj->containsKey("devName"))
    {
      String thisData = (*jsonConfigObj)["devName"];
      deviceName = thisData;
    }
    if ((*jsonConfigObj)["inclMAC"])
    {
      deviceName += '_';
      deviceName += String((uint32_t)ESP.getEfuseMac());
    }
    if ((*jsonConfigObj)["useStaticIP"])
      useStaticIP = (bool)(*jsonConfigObj)["useStaticIP"];
    if (useStaticIP && jsonConfigObj->containsKey("staticConfig"))
    {
      String thisIP = (*jsonConfigObj)["staticConfig"]["staticIP"];
      static_ip.fromString(thisIP);
      String thisGW = (*jsonConfigObj)["staticConfig"]["staticGateway"];
      static_gw.fromString(thisGW);
      String thisNM = (*jsonConfigObj)["staticConfig"]["staticNetmask"];
      static_nm.fromString(thisNM);
      String thisDNS = (*jsonConfigObj)["staticConfig"]["staticDNS"];
      static_dns.fromString(thisDNS);
    }

    if (((wifiCfgMode && 0x02) > 0) && jsonConfigObj->containsKey("apConfig"))
    {
      String thisIP = (*jsonConfigObj)["apConfig"]["apGateway"];
      ap_ip.fromString(thisIP);
      String thisPW = (*jsonConfigObj)["apConfig"]["apPassword"];
      apPassword = thisPW;
    }

    //load the hat type
    if (jsonConfigObj->containsKey("HatTypeList") && jsonConfigObj->containsKey("HatIndex"))
    {
      int listIndex = (*jsonConfigObj)["HatIndex"];
      JsonArray devTypes = (*jsonConfigObj)["HatTypeList"];
//      String thisName = devTypes[listIndex]["Name"];
//      useHat.devName = thisName;
      strcpy(useHat.devName, devTypes[listIndex]["Name"]);
      useHat.devId = devTypes[listIndex]["HatId"];
      JsonArray IntfList = devTypes[listIndex]["InterfaceList"];
      if (IntfList)
      {
        useHat.validInterfaces = (uint8_t*) realloc (useHat.validInterfaces, IntfList.size() * sizeof(uint8_t));
        useHat.numValid = IntfList.size();
        for (int i = 0; i < IntfList.size(); i++)
          useHat.validInterfaces[i] = IntfList[i];
      }
      useHat.devCommMode = devTypes[listIndex]["Type"];
    }
    else
    {
      Serial.println("No HAT defined, go with none");
//      useHat.devName = "none";
      strcpy(useHat.devName, "none");
      useHat.devCommMode = 0;
      useHat.devId = 0;
      useHat.validInterfaces = (uint8_t*) realloc (useHat.validInterfaces, sizeof(uint8_t));
      useHat.validInterfaces[0] = 1;
      useHat.numValid = 1;
    }

    //load the interface type
    if (jsonConfigObj->containsKey("InterfaceTypeList") && jsonConfigObj->containsKey("InterfaceIndex"))
    {
      int listIndex = (*jsonConfigObj)["InterfaceIndex"];
      JsonArray commTypes = (*jsonConfigObj)["InterfaceTypeList"];
//      String thisName = commTypes[listIndex]["Name"];
//      useInterface.devName = thisName;
      strcpy(useInterface.devName, commTypes[listIndex]["Name"]);
      useInterface.devCommMode = commTypes[listIndex]["Type"];
      useInterface.devId = commTypes[listIndex]["IntfId"];
    }
    else
    {
      Serial.println("No Interface defined, use LocoNet");
//      useInterface.devName = "LocoNet";
      strcpy(useHat.devName, "LocoNet");
      useInterface.devCommMode = 1;
      useInterface.devId = 2;
    }
    //verify that hat and interface go together
    bool interfaceOK = false;
    for (int i = 0; i < useHat.numValid; i++) // is array of uint8_t
    {
//      Serial.printf("Testing %i %i of %i\n", useInterface.devId, useHat.validInterfaces[i], useHat.numValid);
      if (useInterface.devId == useHat.validInterfaces[i])
      {
        interfaceOK = true;
        break;
      }
    }

    if ((!interfaceOK) || (useHat.devCommMode > useInterface.devCommMode))
    {
      Serial.println("Invalid Hat/CommNode combination. Reload Node Config File");
//      delay(5000);      

      useHat.devCommMode = 0;
      useHat.devId = 0;
      useInterface.devCommMode = 0;
      useInterface.devId = 0;
//      ESP.restart();
    }
    Serial.printf("Initialize with %s %s \n", useHat.devName, useInterface.devName);
//-------------------------------------------------------------------------------------------------------------------------------------------------------
    if ((useInterface.devId == 1) || (useInterface.devId == 9)) //DCC Interface or DCC to MQTT
    {
      Serial.println("Load DCC Interface");  
      myDcc = new NmraDcc();
      pinMode(groveTxD, OUTPUT);
      // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up 
      myDcc->pin(groveRxD, 1);
      // Call the main DCC Init function to enable the DCC Receiver
      myDcc->init(MAN_ID_DIY, 10, FLAGS_CV29_BITS, 0 );
      Serial.println("DCC Init Done");
    }
    else 
      Serial.println("DCC Interface not activated");

    if ((useInterface.devId == 2) || (useInterface.devId == 4) || (useInterface.devId == 11) || (useInterface.devId == 13) || (useInterface.devId == 14)  || (useInterface.devId == 15) || (useInterface.devId == 16)) //LocoNet or Gateway with LocoNet or LocoNet with lbServer or MQTT/TCP
    {
      Serial.println("Init LocoNet");  
      lnSerial = new LocoNetESPSerial(); //UART2 by default
      if ((useInterface.devId == 14) || (useInterface.devId == 15) || (useInterface.devId == 16))
        lnSerial->begin(); //Initialize as Loopback
      else
        lnSerial->begin(groveRxD, groveTxD, true, true); //true is inverted signals on Rx, Tx
      lnSerial->setBusyLED(stickLED, false);
      lnSerial->setLNCallback(callbackLocoNetMessage);
    } 
    else 
      Serial.println("LocoNet not activated");
/*
    if ((useInterface.devId == 5) || (useInterface.devId == 7))//OpenLCB or Gateway with OpenLCB 
    {
      Serial.println("Init OpenLCB");  
      olcbSerial = new IoTT_OpenLCB(groveRxCAN, groveTxCAN);
      olcbSerial->setOlcbCallback(callbackOpenLCBMessage, true);
      olcbSerial->begin();
    }
    else 
      Serial.println("OpenLCB not activated");
*/
    if ((useInterface.devId == 3) || (useInterface.devId == 4) || (useInterface.devId == 6) || (useInterface.devId == 7) || (useInterface.devId == 8) || (useInterface.devId == 9) || (useInterface.devId == 10) || (useInterface.devId == 13)  || (useInterface.devId == 15))
        //LN/OLCB via MQTT or LN Gateway/ALM or OpenLCB with Gateway, or native MQTT, or DCC /w Gateway, or DCC from Gateway, or LocoNet with MQTT/TCP
    {
      Serial.println("Init MQTT");  
      jsonDataObj = getDocPtr("/configdata/mqtt.cfg", false);
      if (jsonDataObj != NULL)
      {
        lnMQTT = new MQTTESP32(*wifiClient); 
        lnMQTT->loadMQTTCfgJSON(*jsonDataObj);
        switch (useInterface.devId)
        {
          case 3:;
          case 4: lnMQTT->setMQTTCallback(callbackLocoNetMessage, 0); break;
          case 5: ;
          case 6:;
//          case 7: lnMQTT->setMQTTCallback(callbackOpenLCBMessage); break;
          case 8: lnMQTT->setNativeMQTTCallback(callbackMQTTMessage, 2); break;
          case 9: lnMQTT->setDCCMode(); break; //no callback as DCC is one way only
          case 10: lnMQTT->setNativeMQTTCallback(callbackDCCMQTTMessage, 3); break; //DCC from MQTT, read only
        }
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("MQTT not activated");

    if ((useInterface.devId == 11) || (useInterface.devId == 12) || (useInterface.devId == 13) || (useInterface.devId == 14 || (useInterface.devId == 15)))  // LocoNet with lbServer or lbServer Client or lbServer with MQTT
    {
      Serial.println("Load LocoNet over TCP / lbServer");  
      jsonDataObj = getDocPtr("/configdata/lbserver.cfg", false);
      if (jsonDataObj != NULL)
      {
        lbServer = new IoTT_LBServer();
        lbServer->loadLBServerCfgJSON(*jsonDataObj);
        switch (useInterface.devId)
        {
          case 11:; //lbServer Mode
          case 13:; //lbServer Mode with MQTT
          case 14:; //Loopback with lbServer
          case 15:  //Loopback with lbServer and MQTT
            lbServer->initLBServer(true);
            break; 
          case 12: //LocoNet over TCP client mode
            lbServer->initLBServer(false);
            lbServer->setLNCallback(callbackLocoNetMessage);
            break; 
        }
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("LocoNet over TCP / lbServer not activated");

    if ((useInterface.devId == 4) || (useInterface.devId == 7) || (useInterface.devId == 13) || (useInterface.devId == 11) || (useInterface.devId == 14) || (useInterface.devId == 15)) // && (modMode == 1))) //LocoNet or Loopback or OpenLCB Gateway/ALM or lbServer
    {
      Serial.println("Load Gateway");  
      switch (useInterface.devId)
      {
        case 4: commGateway = new ln_mqttGateway(lnSerial, lnMQTT, &callbackLocoNetMessage); break;
//        case 7: commGateway = new ln_mqttGateway(olcbSerial, lnMQTT, &callbackOpenLCBMessage); break;
        case 11:;
        case 14:;
        case 15:;
        case 13: commGateway = new ln_mqttGateway(lnSerial, lnMQTT, lbServer, &callbackLocoNetMessage); break;
      }
    }
    else 
      Serial.println("Gateway not activated");

    setTxFunction(&sendMsg); //defined in IoTT_DigitraxBuffers.h
    if (jsonConfigObj->containsKey("useBushby"))
      if ((bool)(*jsonConfigObj)["useBushby"])
        enableBushbyWatch(true); //defined in IoTT_DigitraxBuffers.h
      else
        enableBushbyWatch(false);
        
    if (useHat.devId == 1) //BlueHat or CTC Hat
    {
      {
        Serial.println("Load BlueHat Data"); 

        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Create JSON Doc"); 

        jsonDataObj = getDocPtr("/configdata/led.cfg", false);
        if (jsonDataObj != NULL)
        {
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Create LED Chain"); 
          myChain = new IoTT_ledChain(); // ... construct now, and call setup later
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Load LED Chain Data"); 
          myChain->loadLEDChainJSON(*jsonDataObj);
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Delete JSON Object"); 
          delete(jsonDataObj);
          uint16_t subFileCtr = 1;
          while (SPIFFS.exists("/configdata/led" + String(subFileCtr) + ".cfg"))
          {
//            Serial.printf("trying File %i\n", subFileCtr);
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Create next JSON Doc"); 
            jsonDataObj = getDocPtr("/configdata/led" + String(subFileCtr) + ".cfg", false);
            if (jsonDataObj)
            {
//              Serial.println("Load Chain File");
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Load LED Chain Data"); 
              myChain->loadLEDChainJSON(*jsonDataObj, false);
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Delete JSON Object"); 
              delete(jsonDataObj);
            }
            subFileCtr++;
          }
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Init LED Chain"); 
          if (useInterface.devCommMode == 3)
            myChain->setMQTTMode(mqttTransmit);
          Serial.printf("Init LED Chain on Pin %i, %i LEDs long\n", hatDataPin, myChain->getChainLength());
//          Serial.println(myChain->colTypeNum);
            if (myChain->colTypeNum == 0x66)
            {
              FastLED.addLeds<WS2811, hatDataPin, GRB>(myChain->getChain(), myChain->getChainLength()); 
            }
            if (myChain->colTypeNum == 0x0C)
            {
              FastLED.addLeds<WS2811, hatDataPin, RGB>(myChain->getChain(), myChain->getChainLength()); 
            }
        Serial.println(String(ESP.getFreeHeap()));
        Serial.println("Delete LED Chain"); 
        delete(myChain);
        myChain = NULL;
        Serial.println(String(ESP.getFreeHeap()));
        }
      }
    }
    else 
      Serial.println("BlueHat not activated");

    if (useHat.devId == 2) //USB Serial Injector
    {
      jsonDataObj = getDocPtr("/configdata/usb.cfg", false);
      if (jsonDataObj != NULL)
      {
        Serial.println("Load Serial communication interface"); 
        usbSerial = new IoTT_SerInjector(hatRxD, hatTxD, false, 1);
        usbSerial->setTxCallback(sendMsg);
        usbSerial->loadLNCfgJSON(*jsonDataObj);
        switch (useInterface.devId)
        {
          case 2: ; // LocoNet
          case 3: ; //LocoNet over MQTT
          case 4: usbSerial->setMsgType(LocoNet); break; //LocoNet with Gateway
          case 5: ; //OpenLCB
          case 6: ; //OpenLCB over MQTT
          case 7: usbSerial->setMsgType(OpenLCB); break; //OpenLCB with Gateway
        }
        delete(jsonDataObj); 
        Serial.println("Serial communication loaded"); 
      }
    }
    else
      Serial.println("Serial communication not activated");

    if (useHat.devId == 3) //YellowHat
    {
        Serial.println("Init YellowHat");  
        Wire.begin(hatSDA, hatSCL, 400000); //initialize the I2C interface 400kHz
        jsonDataObj = getDocPtr("/configdata/led.cfg", false);
        if (jsonDataObj != NULL)
        {
          Serial.println("Load Yellow Hat LED Chain Data");  
          myChain = new IoTT_ledChain(&Wire, yellowWireAddr); // set for using I2C Bus address 0x18
          myChain->loadLEDChainJSON(*jsonDataObj);
          delete(jsonDataObj);
          uint16_t subFileCtr = 1;
          while (SPIFFS.exists("/configdata/led" + String(subFileCtr) + ".cfg"))
          {
//            Serial.printf("trying File %i\n", subFileCtr);
            jsonDataObj = getDocPtr("/configdata/led" + String(subFileCtr) + ".cfg", false);
            if (jsonDataObj)
            {
//              Serial.println("Load Chain File");
              myChain->loadLEDChainJSON(*jsonDataObj, false);
              delete(jsonDataObj);
            }
            subFileCtr++;
          }
          if (useInterface.devCommMode == 3) //MQTT
            myChain->setMQTTMode(mqttTransmit);
        }
        else
          Serial.println("Yellow Hat no led chain defined");

        jsonDataObj = getDocPtr("/configdata/btn.cfg", false);
        if (jsonDataObj != NULL)
        {
          Serial.println("Load Yellow Hat Button Data");  
          myButtons = new IoTT_Mux64Buttons();
          myButtons->initButtonsI2C(&Wire, yellowWireAddr, &analogPins[0], true); //use WiFi with buttons (always). ok for pin 36
          myButtons->loadButtonCfgI2CJSON(*jsonDataObj);
          if (useInterface.devCommMode == 3) //MQTT
            myButtons->setMQTTMode(mqttTransmit);
          delete(jsonDataObj);
        }
        else
          Serial.println("Yellow Hat no buttons defined");
    }
    else 
      Serial.println("YellowHat not activated");

    if (useHat.devId == 4) //GreenHat
    {
        Serial.println("Init GreenHat");  
        //if Comm Interfae = DCC or DCC from MQTT, we also initialize LocoNet Loopback top enable local buttons
        if ((useInterface.devId == 1) || (useInterface.devId == 10)) //DCC or DCC from MQTT
        {
          Serial.println("Init LocoNet Loopback for local buttons");  
          lnSerial = new LocoNetESPSerial(); //UART2 by default
          lnSerial->begin(); //Initialize as Loopback
          lnSerial->setBusyLED(stickLED, false);
          lnSerial->setLNCallback(callbackLocoNetMessage);
        } 
        Wire.begin(hatSDA, hatSCL, 400000); //initialize the I2C interface
        jsonDataObj = getDocPtr("/configdata/greenhat.cfg", true);
        if (jsonDataObj != NULL)
          if (jsonDataObj->containsKey("Modules"))
          {
            JsonArray modDefList = (*jsonDataObj)["Modules"];
            Serial.println("Load GreenHat Module(s)");  
            mySwitchList = new IoTT_SwitchList(); 
            mySwitchList->begin(&Wire); // set for using I2C Bus 
            mySwitchList->configModMem(modDefList.size());
//            mySwitchList->loadRunTimeData();

            for (uint8_t modLoop = 0; modLoop < modDefList.size(); modLoop++)
            {
              if (modDefList[modLoop]["Type"] == "servo")
                mySwitchList->setGreenHatType(modLoop, servoModule);
              else
                mySwitchList->setGreenHatType(modLoop, comboModule);
              JsonArray cfgArray = modDefList[modLoop]["CfgFiles"];
              for (uint8_t cfgLoop = 0; cfgLoop < cfgArray.size(); cfgLoop++) //4 config files: switches, buttons, button handler, LEDs
              {
                String fileNameStr = cfgArray[cfgLoop]["FileName"];
                DynamicJsonDocument * jsonCfgObj = getDocPtr(configDir + "/" + fileNameStr + configDotExt, false);
                if (jsonCfgObj)
                {
                  mySwitchList->loadSwCfgJSON(modLoop, cfgLoop, *jsonCfgObj);
                  delete(jsonCfgObj);
                  uint16_t subFileCtr = 1;
                  while (SPIFFS.exists(configDir + "/"  + fileNameStr + String(subFileCtr) + configDotExt))
                  {
                    jsonCfgObj = getDocPtr(configDir + "/"  + fileNameStr + String(subFileCtr) + configDotExt, false);
                    if (jsonCfgObj)
                    {
                      mySwitchList->loadSwCfgJSON(modLoop, cfgLoop, *jsonDataObj, false);
                      delete(jsonCfgObj);
                    }
                    subFileCtr++;
                  }
                  if (useInterface.devCommMode == 3) //MQTT
                    mySwitchList->setMQTTMode(mqttTransmit);
                  if (useInterface.devCommMode == 0) //no interface, local Mode
                    mySwitchList->setLocalMode();
                }
              }
            }
          }
          else
            Serial.println("No GreenHat Modules");
        else
          Serial.println("GreenHat not activated");
    }

    if (useHat.devId == 5) //BlackHat
    {
        jsonDataObj = getDocPtr("/configdata/throttle.cfg", false);
        if (jsonDataObj != NULL)
        {
          Serial.println("Load BlackHat Button Data");  
          myButtons = new IoTT_Mux64Buttons();
          myButtons->initButtonsDirect();
          myButtons->loadButtonCfgDirectJSON(*jsonDataObj);
          if (useInterface.devCommMode == 3) //MQTT
            myButtons->setMQTTMode(mqttTransmit);
          delete(jsonDataObj);
          hatVerified = true; //these are GPIO buttons, so they are always there
        }
        else
          Serial.println("BlackHat no buttons defined");
    }
    else 
      Serial.println("BlackHat not activated");

//Load ALM list
    if (jsonConfigObj->containsKey("ALMTypeList") && jsonConfigObj->containsKey("ALMIndex"))
    {
      JsonArray almIndex = (*jsonConfigObj)["ALMIndex"];
      for (int i = 0; i < almIndex.size(); i++)
      {
        byte shiftBy = almIndex[i];
        useALM |= (0x01 << shiftBy);
      }
    }

//Initialize ALMs
    if (useALM & 0x01) //Button Handler
    {
      Serial.println("Load Button Handler Data");  
      jsonDataObj = getDocPtr("/configdata/btnevt.cfg", false);
      if (jsonDataObj != NULL)
      {
        eventHandler = new(IoTT_LocoNetButtonList);
        eventHandler->loadButtonCfgJSON(*jsonDataObj);
        delete(jsonDataObj);
        uint16_t subFileCtr = 1;
        while (SPIFFS.exists("/configdata/btnevt" + String(subFileCtr) + ".cfg"))
        {
          jsonDataObj = getDocPtr("/configdata/btnevt" + String(subFileCtr) + ".cfg", false);
          if (jsonDataObj)
          {
            eventHandler->loadButtonCfgJSON(*jsonDataObj, false);
            delete(jsonDataObj);
          }
          subFileCtr++;
        }
      }
    }
    else 
      Serial.println("Button Handler not activated");
/*
    if (useALM & 0x02)
    {
      Serial.println("Load Security Element Data");  
      jsonDataObj = getDocPtr("/configdata/secel.cfg");
      if (jsonDataObj != NULL)
      {
        secElHandlerList = new(IoTT_SecurityElementList);
        secElHandlerList->loadSecElCfgJSON(*jsonDataObj);
        delete(jsonDataObj);
        uint16_t subFileCtr = 1;
        while (SPIFFS.exists("/configdata/secel" + String(subFileCtr) + ".cfg"))
        {
          jsonDataObj = getDocPtr("/configdata/secel" + String(subFileCtr) + ".cfg");
          if (jsonDataObj)
          {
            secElHandlerList->loadSecElCfgJSON(*jsonDataObj, false);
            delete(jsonDataObj);
          }
          subFileCtr++;
        }
      }
      else 
        Serial.println("Security Elements not activated");
      }
    else 
      Serial.println("Security Elements not loaded");
*/
    Serial.println("Connect WiFi");  
    establishWifiConnection(myWebServer,dnsServer);
    
    if (lnMQTT)
    {
      Serial.println("Connect MQTT");  
      establishMQTTConnection();
      Serial.println("Connect MQTT done");  
    }
    if (jsonConfigObj->containsKey("useNTP"))
    {
      useNTP = (bool)(*jsonConfigObj)["useNTP"];
      if ((useNTP) && (wifiCfgMode == 1))
      {
        Serial.println("Create NTP Time Access");  
        JsonObject ntpConfig = (*jsonConfigObj)["ntpConfig"];
        if (ntpConfig.containsKey("NTPServer"))
          strcpy(ntpServer, ntpConfig["NTPServer"]);
        if (ntpConfig.containsKey("ntpTimeZone"))
          if (ntpConfig["ntpTimeZone"].is<const char*>())
            strcpy(ntpTimeZone, ntpConfig["ntpTimeZone"]);
          else
            Serial.println("ntpTimeZone is wrong data type");
        configTime(0, 0, ntpServer);
        setenv("TZ", ntpTimeZone, 1);
      }
      else 
        Serial.println("NTP Module not activated");
    }
    else
      Serial.println("NTP Module not defined");
    delete(jsonConfigObj);
    if (useNTP) getInternetTime();
    startWebServer();
    if (lbServer)
      lbServer->startServer();
      
    Serial.println(String(ESP.getFreeHeap()));
  }
  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
  pinMode(0, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
#ifdef measurePerformance
  loopCtr++;
  if (millis() > myTimer)
  {
    Serial.printf("Timer Loop: %i Heap: %i\n", loopCtr, ESP.getFreeHeap());
    loopCtr = 0;
    myTimer += 1000;
  }
#endif  
  if (millis() < lastMillis)
    millisRollOver++; //update ms rollover counter
  else
    lastMillis = millis();  

//  if (secElHandlerList) secElHandlerList->processLoop(); //calculates speeds in all blocks and sets signals accordingly
  if (myDcc) myDcc->process(); //receives and decodes track signals
  if (eventHandler) eventHandler->processButtonHandler(); //drives the outgoing buffer and time delayed commands
  if (usbSerial) usbSerial->processLoop(); //drives the USB interface serial traffic
  if (lbServer) lbServer->processLoop(); //drives the LocoNet over TCP interface traffic
  if (lnSerial) lnSerial->processLoop(); //handling all LocoNet communication
//  if (olcbSerial) olcbSerial->processLoop(); //handling all OpenLCB communication

  if (myChain)
    hatVerified = myChain->isVerified();
  else
    hatVerified = true;
  if (hatVerified)
    if (myButtons) myButtons->processButtons(); //checks if a button was pressed and sends button messages
  if (mySwitchList) mySwitchList->processLoop();
  if (myChain) 
    myChain->processChain(); //updates all LED's based on received status information for switches, inputs, buttons, etc.

  checkWifiTimeout(); //checks if wifi has been inactive and disables it after timeout
  if ((wifiCfgMode == 1)
  )// || (wifiCfgMode == 3)) //STA active, Internet connection
  {
    if (!wifiCancelled) //handles keep alive updates as long connection is valid
    {
      if (WiFi.status() == WL_CONNECTED)
      { 
        sendKeepAlive();
        if (useNTP)
          getInternetTime(); //gets periodic updates of date and time from NTP server
      }
      else
      {
        Serial.println("Reconnect WiFi");
        establishWifiConnection(myWebServer,dnsServer);
      }
    }
    if ((!wifiCancelled) && ((commGateway) || (lnMQTT))) //handles all wifi communication for MQTT
      if (WiFi.status() == WL_CONNECTED)
      { 
        if (commGateway) 
          commGateway->processLoop();
        else
          if (lnMQTT) 
          {
            if (lnMQTT->mustResubscribe()) //true after reset of the MQTT connection
            {
              //add code to resubscribe topics of all libraries that support native MQTT, e.g. LED, Buttons
              if (useInterface.devCommMode == 3) //native MQTT
              {
                if (myChain) myChain->subscribeTopics();
                if (myButtons) myButtons->subscribeTopics();
              }
              lnMQTT->subscribeTopics();
            }
            lnMQTT->processLoop(); //LN or OpenLCB over MQTT
          }
      }
      else
      {
        Serial.println("Reconnect WiFi");
        establishWifiConnection(myWebServer,dnsServer);
      }
  }
  else
    sendKeepAlive();
  M5.update();
  processDisplay();
//  while (Serial.available())
//  {
//    char c = Serial.read();
//    sendBlockDetectorCommand(150,1);
//    sendAnalogCommand(22,17);
//  }

  processBufferUpdates(); //updating DigitraxBuffers by querying information from LocoNet, e.g. slot statuses
}
