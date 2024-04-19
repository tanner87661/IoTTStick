//#define DialKnob

#include <M5Dial.h>
#define PowerSys M5.Power.Axp2101

//#define measurePerformance //uncomment this to display the number of loop cycles per second
//#define useAI
//#define useSecEl

//Arduino published libraries. Install using the Arduino IDE or download from Github and install manually
#include <Arduino.h>
//#include <SD.h>
#include <FS.h>
#define FORMAT_SPIFFS_IF_FAILED true
#include <SPIFFS.h>
//#include <M5Stack.h>
#include <M5Unified.h>
#include <math.h>
//#include <Wire.h>
#include <WiFi.h>
#include <ArduinoUniqueID.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE. Make sure to use version 6.x
#include <TJpg_Decoder.h>

//following libraries can be downloaded from https://github.com/tanner87661?tab=repositories
#include <IoTT_CommDef.h>
#include <IoTT_DigitraxBuffers.h> //as introduced in video # 30
//#include <IoTT_LocoNetHBESP32.h> //this is a hybrid library introduced in video #29
#include <IoTT_MQTTESP32.h> //as introduced in video # 29
//#include <IoTT_RemoteButtons.h> //as introduced in video # 29, special version for Wire connection via 328P
#include <IoTT_LocoNetButtons.h> //as introduced in video # 29
//#include <IoTT_LEDChain.h> //as introduced in video # 30
//#include <IoTT_SerInjector.h> //Serial USB Port to make it an interface for LocoNet and OpenLCB
//#include <IoTT_OpenLCB.h> //CAN port for OpenLCB
#include <IoTT_Switches.h>

#if defined useSecEl
//#include <IoTT_SecurityElements.h> //not ready yet. This is the support for ABS/APB as described in Videos #20, 21, 23, 24
#endif

//include <NmraDcc.h> //install via Arduino IDE
//#include <OneDimKalman.h>
#include <IoTT_lbServer.h>
//nclude <IoTT_TrainSensor.h>
#ifdef useAI
  #include <IoTT_VoiceControl.h>
#endif

String BBVersion = "0.0.1Dev";

//library object pointers. Libraries will be dynamically initialized as needed during the setup() function
AsyncWebServer * myWebServer = NULL; //(80)
DNSServer * dnsServer = NULL;
//WiFiClientSecure * wifiClientSec = NULL;
WiFiClient * wifiClient = NULL;
AsyncWebSocket * ws = NULL; //("/ws");
//AsyncWebSocketClient * globalClient = NULL;
std::vector<wsClientInfo> globalClients; // a list to hold all clients when in server mode

uint32_t wsBufferSize = 16384;
uint32_t wsRxReadPtr = 0;
uint32_t wsRxWritePtr = 0;
char * wsRxBuffer; //[16384, 32768]; //should this be dynamic?
uint32_t wsTxReadPtr = 0;
uint32_t wsTxWritePtr = 0;
char * wsTxBuffer; //[32768]; //should this be dynamic?
bool execLoop = true; //used to stop loop execution if update files are coming in. Must result in restart

//global variables
bool useStaticIP = false;
IPAddress static_ip;
IPAddress static_gw;
IPAddress static_nm;
IPAddress static_dns;
IPAddress ap_ip;
IPAddress ap_nm(255,255,255,0);
IPAddress dyn_ip;

String mySSID;

uint8_t wifiCfgMode = 0x00; //1: STA, 2: AP, 3: STA+AP
String deviceName; //device name used for AP, will load from node.cfg
String apPassword; //AP password, will load from node.cfg

bool wifiAlwaysOn = false; //set to false to shut Wifi after some time of inactivity. Gateway and MQTT should be disabled, though
bool wifiCancelled = false; //true if Wifi was switched off due to no use
uint32_t lastWifiUse = millis();

#define wifiShutTimeout 120000 //after 2 Mins of not using, Wifi is closed
#define apShutTimeout 180000 //after 3 Mins of not using, AP is shut. 3 Min to give enough time to connect to the AP from a PC

#define i2cClock 400000

String bufferFileName = "/buffers.dat";

//more library object pointers. Libraries will be dynamically initialized as needed during the setup() function
//IoTT_Mux64Buttons * myButtons = NULL;
IoTT_LocoNetButtonList * eventHandler = NULL; 
#ifdef useSecEl
//IoTT_SecurityElementList * secElHandlerList = NULL;
#endif
#ifdef useAI
//  IoTT_VoiceControl * voiceWatcher = NULL;
#endif
//LocoNetESPSerial * lnSerial = NULL;
//LocoNetESPSerial * lnSubnet = NULL;
//nodeType subnetMode = standardMode;
//IoTT_SerInjector * usbSerial = NULL;
IoTT_LBServer * lbClient = NULL;
IoTT_LBServer * lbServer = NULL;
IoTT_LBServer * wiThServer = NULL;
IoTT_DigitraxBuffers * digitraxBuffer = NULL; //pointer to DigitraxBuffers
//IoTT_OpenLCB *olcbSerial = NULL;
//HardwareSerial *wireSerial = NULL;
//ln_mqttGateway * commGateway = NULL;
//IoTT_ledChain * myChain = NULL;
IoTT_SwitchList * mySwitchList = NULL;
MQTTESP32 * lnMQTTClient = NULL;
MQTTESP32 * lnMQTTServer = NULL;
//NmraDcc  * myDcc = NULL;
//IoTT_TrainSensor * trainSensor = NULL;
//some variables used for performance measurement
#ifdef measurePerformance
uint16_t loopCtr = 0;
uint32_t myTimer = millis() + 1000;
#endif

//********************************************************Hardware Configuration******************************************************
//#define groveRxD 33 //pin 1 yellow RxD for LocoNet, DCC
//#define groveTxD 32 //pin 2 white TxD for LocoNet, DCC
//constexpr gpio_num_t groveRxCAN = GPIO_NUM_33; //for OpenLCB CAN
//constexpr gpio_num_t groveTxCAN = GPIO_NUM_32; //for OpenLCB CAN

  #define stickLED 19 //red LED on stick plus2
  #define  LedON true
//#define hatSDA 26 //changes between input and output by I2C master
//#define hatSCL 0 //pull SCL low while sending non I2C data over SDA
//#define hatRxD 0
//#define hatRxDAlt 36 //read only Input
//#define hatTxD 26
//#define hatInputPin 36 //25+ //36
//#define hatDataPin 26 //26 used to send LED data from FastLED to BlueHat. First, set to output low, then call show
//#define rhDataPin 0 //used to send LED data from FastLED to the RedHat. First, set to output low, then call show
//#define yellowWireAddr 0x18 //I2C Address used on hats with I2C slave 328P
//uint8_t hatData[] = {1, hatDataPin}; //pin definition for LED library
//uint8_t analogPins[] = {hatInputPin, 2}; //analog pin used for button reading, number of button MUX (connected via I2C)

//***********************************************************************************************************************************

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
//  uint8_t * validInterfaces = NULL;
//  uint8_t numValid = 0;
} configType;

configType useHat;
configType useInterface;
uint32_t useServer = 0; //bit mask configuration list
uint32_t useALM = 0; //bit mask configuration list

uint8_t m5CurrentPage = 0;
uint8_t m5ThrottleMenu = 0;
uint16_t oldWifiStatus = 0;
uint8_t useM5Viewer = 0;
uint8_t m5DispLine = 0;

bool darkScreen = false;
bool hatPresent = false;
bool pwrUSB = false;
bool pwrDC = false;

uint16_t wifiMode = 0;
uint16_t wifiStatus = 0;
bool dataValid = false;

File uploadFile; //used for web server to upload files

//this is the outgoing communication function for IoTT_DigitraxBuffers.h, routing the outgoing messages to the correct interface
uint16_t sendMsg(lnTransmitMsg txData)
{
//  Serial.printf("verifySyntax %i\n", useInterface.devId);
//  if (!verifySyntax(&txData.lnData[0]))
//  {
//    Serial.printf("ERROR: Call sendMsg to %i: %i, %2X, %2X, %2X, %2X \n", useInterface.devId, txData.lnMsgSize, txData.lnData[0], txData.lnData[1], txData.lnData[2], txData.lnData[3]);
//    Serial.printf("Outgoing Callback to %i ID %2X\n", useInterface.devId, txData.requestID);
//    return 0;
//  }
  switch (useInterface.devId)
  {
    case 0: break; //none
//    case 10:; //DCC from MQTT
//    case 1: //DCC 
//            if (lnSerial) return lnSerial->lnWriteMsg(&txData); //send to Loopback interface to make hats tick
//            break; 
    case 16:; //LocoNet Loopback
//    case 2: if (lnSerial) return lnSerial->lnWriteMsg(&txData); break;
    case 3: if (lnMQTTClient) return lnMQTTClient->lnWriteMsg(&txData); break;
    case 17:; //WiThrottle native Mode
    case 18:; //WiThrottle DCC EX Mode
    case 12:if (lbClient) //LocoNet over TCP or WiClient
            {
//               Serial.printf("Send to lbServer ID %2X\n", txData.requestID);
               return lbClient->lnWriteMsg(&txData); break; //this is message to lbServer or WiServer 
            }
/*            
    case 5: //OpenLCB
            if (olcbSerial) olcbSerial->lnWriteMsg(txData);
            break;
*/            
  }
  Serial.printf("No Command Source selected %i\n", useInterface.devId);
  return 0;
}

uint16_t sendMQTTMsg(char * topic, char * payload) //used for native MQTT only
{
//  switch (useInterface.devId)
  {
//    case 8: //native MQTT
//      if (lnMQTTClient) return lnMQTTClient->mqttPublish(topic, payload);
//      break; 
  }
  return 0;
}

/*
void resetPin(uint8_t pinNr)
{
  pinMode(pinNr, OUTPUT);
  digitalWrite(pinNr,0);
  pinMode(pinNr, INPUT);
}
*/

void setup() 
{
// put your setup code here, to run once:
//  resetPin(0);
//  resetPin(26);
//  resetPin(32);
//  resetPin(33);
//  resetPin(36);

  wsRxBuffer = (char*) malloc(wsBufferSize); 
  wsTxBuffer = (char*) malloc(wsBufferSize); 

  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;   // default=115200. if "Serial" is not needed, set it to 0.
  cfg.clear_display = true;  // default=true. clear the screen when begin.
  cfg.output_power  = true;  // default=true. use external port 5V output.
  cfg.internal_imu  = true;  // default=true. use internal IMU.
  cfg.internal_rtc  = true;  // default=true. use internal RTC.
  cfg.internal_spk  = false;  // default=true. use internal speaker.
  cfg.internal_mic  = true;  // default=true. use internal microphone.
  cfg.external_imu  = false;  // default=false. use Unit Accel & Gyro.
  cfg.external_rtc  = false;  // default=false. use Unit RTC.
  cfg.external_spk  = false; // default=false. use SPK_HAT / ATOMIC_SPK
  cfg.led_brightness = 0;   // default= 0. system LED brightness (0=off / 255=max) (※ not NeoPixel)

  M5Dial.begin(cfg, true, false);
  Serial.println(M5.getBoard());
  Serial.println("Init SPIFFS");
  SPIFFS.begin(); //File System. Size is set to 1 MB during compile time and loaded with configuration data and web pages
  initDisplay();
  digitalWrite(stickLED, !LedON);
  getRTCTime();
  char time_output[30];
  strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  Serial.println(time_output);
  Serial.println("IoTT Dial Version " + BBVersion);
  UniqueIDdump(Serial);
  digitraxBuffer = new IoTT_DigitraxBuffers(sendMsg); //initialization with standard LocoNet communication function
  //load switch status data from file. If not Cmd Stn mode, slot buffer is cleared, otherwise, load slot buffer from previous session
  digitraxBuffer->loadFromFile(bufferFileName); //load previous dataset
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

    //load digitraxBuffer settings
    if (jsonConfigObj->containsKey("useBushby"))
      if ((bool)(*jsonConfigObj)["useBushby"])
        digitraxBuffer->enableBushbyWatch(true); //defined in IoTT_DigitraxBuffers.h
      else
        digitraxBuffer->enableBushbyWatch(false);
    if (jsonConfigObj->containsKey("useLissy"))
      if ((bool)(*jsonConfigObj)["useLissy"])
        digitraxBuffer->enableLissyMod(true); //defined in IoTT_DigitraxBuffers.h
      else
        digitraxBuffer->enableLissyMod(false);
    if (jsonConfigObj->containsKey("broadcastFC"))
    {
      bool fcStat = (*jsonConfigObj)["broadcastFC"];
      uint16_t fcRate = (*jsonConfigObj)["broadcastFCRate"];
      digitraxBuffer->enableFCRefresh(fcStat, fcRate); //defined in IoTT_DigitraxBuffers.h
    }
    else
      digitraxBuffer->enableFCRefresh(true, 75);
    if (jsonConfigObj->containsKey("refreshFct"))
    {
      bool fctRefresh = (*jsonConfigObj)["refreshFct"];
      digitraxBuffer->enableFctRefresh(fctRefresh);
    }

    //load the interface type
    if (jsonConfigObj->containsKey("InterfaceTypeList") && jsonConfigObj->containsKey("InterfaceIndex"))
    {
      int listIndex = (*jsonConfigObj)["InterfaceIndex"];
      JsonObject commType = (*jsonConfigObj)["InterfaceTypeList"][listIndex];
      strcpy(useInterface.devName, commType["Name"]);
      useInterface.devCommMode = commType["Type"];
      useInterface.devId = commType["IntfId"];
    }
    else
    {
      Serial.println("No Interface defined, use LocoNet");
      strcpy(useInterface.devName, "LocoNet");
      useInterface.devCommMode = 1;
      useInterface.devId = 2;
    }
   //load the hat type
    strcpy(useHat.devName, "none");
    bool validHat = false;
    useHat.devCommMode = 0;
    useHat.devId = 0;
    if (jsonConfigObj->containsKey("HatTypeList") && jsonConfigObj->containsKey("HatIndex"))
    {
      int listIndex = (*jsonConfigObj)["HatIndex"];
      JsonObject hatData = (*jsonConfigObj)["HatTypeList"][listIndex];
     
//      JsonArray devTypes = (*jsonConfigObj)["HatTypeList"];
      JsonArray IntfList = hatData["InterfaceList"];
      if (IntfList)
      {
        for (int i = 0; i < IntfList.size(); i++)
          if (IntfList[i] == useInterface.devId) //this hat is ok with the command source
          {
            strcpy(useHat.devName, hatData["Name"]);
            useHat.devId = hatData["HatId"];
            useHat.devCommMode = hatData["Type"];
            validHat = true;
            break;
          }
      }
    }
    else
    {
      Serial.println("No HAT defined, go with none");
//      useHat.devName = "none";
//      useHat.validInterfaces = (uint8_t*) realloc (useHat.validInterfaces, sizeof(uint8_t));
//      useHat.validInterfaces[0] = 1;
//      useHat.numValid = 1;
    }
      
    //load the server list
    if (jsonConfigObj->containsKey("ServerTypeList") && jsonConfigObj->containsKey("ServerIndex"))
    {
      JsonArray serverIndex = (*jsonConfigObj)["ServerIndex"];
      for (int j = 0; j < serverIndex.size(); j++)
      {
        byte shiftBy = serverIndex[j];
        JsonObject serverData = (*jsonConfigObj)["ServerTypeList"][shiftBy];
        JsonArray IntfList = serverData["InterfaceList"];
        if (IntfList)
        {
          for (int i = 0; i < IntfList.size(); i++)
            if (IntfList[i] == useInterface.devId) //this server is ok with the command source
            {
              useServer |= (0x01 << shiftBy);
//              strcpy(useServer[j].devName, serverData["Name"]);
//                useServer[j].devId = serverData["ServerId"];
//                useServer[j].devCommMode = serverData["Type"];
//                validServer = true;
              break;
            }
        }
      }
    }

    //Load ALM list
    if (jsonConfigObj->containsKey("ALMTypeList") && jsonConfigObj->containsKey("ALMIndex"))
    {
      JsonArray almIndex = (*jsonConfigObj)["ALMIndex"];
      for (int i = 0; i < almIndex.size(); i++)
      {
        byte shiftBy = almIndex[i];
        JsonObject almData = (*jsonConfigObj)["ALMTypeList"][shiftBy];
        JsonArray IntfList = almData["InterfaceList"];
        if (IntfList)
        {
          for (int i = 0; i < IntfList.size(); i++)
            if (IntfList[i] == useInterface.devId) //this alm is ok with the command source
            {
              useALM |= (0x01 << shiftBy);
              break;
            }
        }
      }
    }
  
    if (!validHat)
    {
      Serial.println("Invalid Hat/CommNode combination. Reload Node Config File");
      useHat.devCommMode = 0;
      useHat.devId = 0;
      useInterface.devCommMode = 0;
      useInterface.devId = 0;
    }
  }  
    Serial.printf("Initialize with %s %s Servers: %2X ALM: %2X \n", useHat.devName, useInterface.devName, useServer, useALM);
//-------------------------------------------------------------------------------------------------------------------------------------------------------
/*
    if (useInterface.devId == 1) //|| (useInterface.devId == 9)) //DCC Interface or DCC to MQTT
    {
      Serial.println("Load DCC Interface");  
      digitraxBuffer->setLocoNetMode(false);
      myDcc = new NmraDcc();
      pinMode(groveTxD, OUTPUT);
      digitalWrite(groveTxD, 0);
      // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up 
      myDcc->pin(groveRxD, 1);
      // Call the main DCC Init function to enable the DCC Receiver
      myDcc->init(MAN_ID_DIY, 10, FLAGS_CV29_BITS, 0 );
//  myDcc->init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0 );
      Serial.println("DCC Init Done");
    }
    else 
      Serial.println("DCC Interface not activated");
*/
/*
    if ((useInterface.devId == 2) || (useInterface.devId == 16)) //LocoNet or LocoNet Loopback
    {
      Serial.println("Init LocoNet");  
      lnSerial = new LocoNetESPSerial(); //UART2 by default
      if (useInterface.devId == 16)
      {
        lnSerial->begin(); //Initialize as Loopback
        digitraxBuffer->setLocoNetMode(false); //switch off Slot query

      }
      else
      {
        lnSerial->begin(groveRxD, groveTxD, true, true); //true is inverted signals on Rx, Tx
        lnSerial->setNetworkType(subnetMode); 
        Serial.printf("LocoNet Prio Mode: %i\n", subnetMode);
      }
      lnSerial->setBusyLED(stickLED, LedON);
//      lnSerial->setLNCallback(callbackLocoNetMessage);
    } 
    else 
      Serial.println("LocoNet not activated");
*/
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

    if (useInterface.devId == 12) // LocoNet lbServer Client
    {
      Serial.println("Load LocoNet over TCP / lbServer Client");  
      jsonDataObj = getDocPtr("/configdata/lbserver.cfg", false);
      if (jsonDataObj != NULL)
      {
        lbClient = new IoTT_LBServer();
        lbClient->loadLBServerCfgJSON(*jsonDataObj);
        //LocoNet over TCP client mode
        lbClient->initLBServer(false);
//        lbClient->setLNCallback(callbackLocoNetMessage);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("LocoNet over TCP / lbServer Client not activated");

    if ((useInterface.devId == 17) || (useInterface.devId == 18)) // WiThrottle, client
    {
      Serial.println("Load WiThrottle Client");  
      jsonDataObj = getDocPtr("/configdata/wiclient.cfg", false);
      if (jsonDataObj != NULL)
      {
        lbClient = new IoTT_LBServer();
        lbClient->loadLBServerCfgJSON(*jsonDataObj);
        lbClient->initWIServer(false, useInterface.devId == 18);
        digitraxBuffer->setLocoNetMode(false);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("WiThrottle not activated");

    if ((useInterface.devId == 3) || (useInterface.devId == 8) || (useInterface.devId == 10))
        //LN/OLCB via MQTT or LN Gateway/ALM or OpenLCB with Gateway, or native MQTT, or DCC /w Gateway, or DCC from Gateway, or LocoNet with MQTT/TCP
    {
      Serial.println("Init MQTT Client");  
      jsonDataObj = getDocPtr("/configdata/mqtt.cfg", false);
      if (jsonDataObj != NULL)
      {
//        lnMQTTClient = new MQTTESP32(*wifiClient); 
        WiFiClient * thisClient = new WiFiClient();
        lnMQTTClient = new MQTTESP32(*thisClient); 
        switch (useInterface.devId)
        {
          case 3: lnMQTTClient->initializeMQTT(3); break; //lnClient //setMQTTCallback(callbackLocoNetMessage, 0); break;
          case 8: lnMQTTClient->initializeMQTT(2); break; //setNativeMQTTCallback(nativeClientCallback, 2); break;
          case 10: lnMQTTClient->initializeMQTT(1); break; //setNativeMQTTCallback(dccClientCallback, 3); break; //DCC from MQTT, read only
        }
        lnMQTTClient->loadMQTTCfgJSON(*jsonDataObj);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("MQTT Client not activated");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
// loading servers

    if (useServer & 0x01) // lnMQTT Gateway
    {
      Serial.println("Init MQTT Gateway");  
      jsonDataObj = getDocPtr("/configdata/mqtt.cfg", false);
      if (jsonDataObj != NULL)
      {
        WiFiClient * thisClient = new WiFiClient();
        lnMQTTServer = new MQTTESP32(*thisClient); 
//        lnMQTTServer = new MQTTESP32(*wifiClient); 
        switch (useInterface.devId)
        {
          case 1:;
          case 10: lnMQTTServer->initializeMQTT(4); break; //no callback as DCC is one way only
//          case 7: lnMQTTServer->setMQTTCallback(callbackOpenLCBMessage); break;
          case 2:;
          case 3:;
          case 12: lnMQTTServer->initializeMQTT(0); break; //LN broker callback
        }
        lnMQTTServer->loadMQTTCfgJSON(*jsonDataObj);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("MQTT Gateway not activated");

    if (useServer & 0x02) // && (lnSerial || lbServer || lnMQTT || commGateway)) //Button Handler only with LocoNet or lbServer
//    if ((useInterface.devId == 11) || (useInterface.devId == 12) || (useInterface.devId == 13) || (useInterface.devId == 14 || (useInterface.devId == 15)))  // LocoNet with lbServer or lbServer Client or lbServer with MQTT
    {
      Serial.println("Load LocoNet over TCP / lbServer");  
      jsonDataObj = getDocPtr("/configdata/lbserver.cfg", false);
      if (jsonDataObj != NULL)
      {
        lbServer = new IoTT_LBServer();
        lbServer->loadLBServerCfgJSON(*jsonDataObj);
//        lbServer->setLNCallback(callbackLocoNetMessage);
        lbServer->initLBServer(true);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("LocoNet over TCP / lbServer not activated");

    if (useServer & 0x04) //WiThrottle Server
    {
      Serial.println("Load WiThrottle Server");  
      jsonDataObj = getDocPtr("/configdata/wiclient.cfg", false);
      if (jsonDataObj != NULL)
      {
        wiThServer = new IoTT_LBServer();
        wiThServer->loadLBServerCfgJSON(*jsonDataObj);
        wiThServer->initWIServer(true);
        wifiAlwaysOn = true;
        delete(jsonDataObj);
      }
    }
    else 
      Serial.println("WIThrottle Server not activated");

/*
    if ((useServer & 0x08) && ((useInterface.devId == 3) || (useInterface.devId == 12))) //Loconet Subnet goes with lnClient or MQTTClient
    {
      Serial.println("Install Loconet Subnet");  
      subnetMode = fullMaster;
      lnSubnet = new LocoNetESPSerial(); //UART2 by default
      if (lnSubnet)
      {
        lnSubnet->begin(groveRxD, groveTxD, true, true); //true is inverted signals on Rx, Tx
        digitraxBuffer->setLocoNetMode(false); //switch off Slot query
        lnSubnet->setNetworkType(subnetMode); 
        lnSubnet->setUpstreamMode(true);
        lnSubnet->setBusyLED(stickLED, LedON);
        Serial.printf("LocoNet Prio Mode: %i\n", subnetMode);
      }
    }
    else 
      Serial.println("Loconet Subnet not installed");
*/

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
    if ((useHat.devId == 7) || (useHat.devId == 3) || (useHat.devId == 4)) //PurpleHat or YellowHat or GreenHat
    {
        Serial.println("Init TwoWire");  
//        Wire.setClock(i2cClock);
        Wire.begin(hatSDA, hatSCL, 400000); //initialize the I2C interface 400kHz
//        Serial.println(Wire.getClock());

//        check_if_exist_I2C();
        
    }
*/
// initialize selected Hat
/*
    if ((useHat.devId == 1) || (useHat.devId == 3) || (useHat.devId == 6) || (useHat.devId == 8)) //BlueHat or YellowHat or RedHat or SilverHat
    {
      {
        Serial.printf("Load LED Chain Data Hat Id: %i\n", useHat.devId); 
        jsonDataObj = getDocPtr("/configdata/led.cfg", false);
        if (jsonDataObj != NULL)
        {
        Serial.println("Create LED Chain"); 
          if (useHat.devId == 3) //YellowHat
            myChain = new IoTT_ledChain(&Wire, yellowWireAddr); // set for using I2C Bus address 0x18
          else
            myChain = new IoTT_ledChain(); // ... construct now, and call setup later
          myChain->loadLEDChainJSON(*jsonDataObj);
          delete(jsonDataObj);
          uint16_t subFileCtr = 1;
          while (SPIFFS.exists("/configdata/led" + String(subFileCtr) + ".cfg"))
          {
            jsonDataObj = getDocPtr("/configdata/led" + String(subFileCtr) + ".cfg", false);
            if (jsonDataObj)
            {
              myChain->loadLEDChainJSON(*jsonDataObj, false);
              delete(jsonDataObj);
            }
            subFileCtr++;
          }
          if (useInterface.devCommMode == 3)
            myChain->setMQTTMode(mqttTransmit);
          Serial.printf("Init LED Chain on Pin %i, %i LEDs long\n", useHat.devId == 1 ? hatDataPin : rhDataPin, myChain->getChainLength());
          if (myChain->colTypeNum == 0x66)
          {
            if (useHat.devId == 1) //BlueHat
              FastLED.addLeds<WS2812B, hatDataPin, GRB>(myChain->getChain(), myChain->getChainLength()); 
            else
              FastLED.addLeds<WS2812B, rhDataPin, GRB>(myChain->getChain(), myChain->getChainLength()); 
          }
          if (myChain->colTypeNum == 0x0C)
          {
            if (useHat.devId == 1) //BlueHat
              FastLED.addLeds<WS2812B, hatDataPin, RGB>(myChain->getChain(), myChain->getChainLength()); 
            else
              FastLED.addLeds<WS2812B, rhDataPin, RGB>(myChain->getChain(), myChain->getChainLength()); 
          }
          if (useHat.devId == 1) //BlueHat
            myChain->setFastPin(hatDataPin);
          else
            myChain->setFastPin(rhDataPin);
        }
      }
    }
    else 
      Serial.println("LED Chain not activated");
*/

/*
    if (useHat.devId == 2) //BrownHat USB Serial Injector
    {
      jsonDataObj = getDocPtr("/configdata/usb.cfg", false);
      if (jsonDataObj != NULL)
      {
        Serial.printf("Load Serial communication interface %i\n", useInterface.devId); 
        usbSerial = new IoTT_SerInjector(hatRxDAlt, hatTxD, false, 1);
        usbSerial->setTxCallback(sendMsg);
        usbSerial->loadLNCfgJSON(*jsonDataObj);
        switch (useInterface.devId)
        {
          case 2: ; // LocoNet
          case 3: ; //LocoNet over MQTT
          case 4: usbSerial->setProtType(LocoNet); break; //LocoNet with Gateway
          case 5: ; //OpenLCB
          case 6: ; //OpenLCB over MQTT
          case 7: usbSerial->setProtType(OpenLCB); break; //OpenLCB with Gateway
          //usbSerial->setProtType(DCCEx); break; //DCC++Ex DCC Generator
        }
        delete(jsonDataObj); 
        Serial.println("Serial communication loaded"); 
      }
    }
    else
      Serial.println("Serial communication not activated");
*/

/*
    if (useHat.devId == 3) //YellowHat
    {
        Serial.println("Init YellowHat");  
        Serial.println("Load Btn Config Data");  
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
*/

/*
    if (useHat.devId == 4) //GreenHat
    {
        Serial.println("Init GreenHat");  
        //if Comm Interfae = DCC or DCC from MQTT, we also initialize LocoNet Loopback top enable local buttons
        if ((useInterface.devId == 1) || (useInterface.devId == 10)) //DCC or DCC from MQTT
        {
          Serial.println("Init LocoNet Loopback for local buttons");  
          lnSerial = new LocoNetESPSerial(); //UART2 by default
          lnSerial->begin(); //Initialize as Loopback
          lnSerial->setBusyLED(stickLED, LedON);
//          lnSerial->setLNCallback(callbackLocoNetMessage);
        } 
        uint8_t numBoards = 0;
        while (checkI2CPort(0x33 - numBoards) == 0)
          numBoards++;
        Serial.printf("Detected %i GreenHat Modules\n", numBoards);
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
//              if (modDefList[modLoop]["Type"] == "servo")
//                mySwitchList->setGreenHatType(modLoop, servoModule);
//              else
//                mySwitchList->setGreenHatType(modLoop, comboModule);
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
                      mySwitchList->loadSwCfgJSON(modLoop, cfgLoop, *jsonCfgObj, false);
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
*/

/*
    if (useHat.devId == 5) //BlackHat
    {
        jsonDataObj = getDocPtr("/configdata/throttle.cfg", false);
        if (jsonDataObj != NULL)
        {
          Serial.println("Load BlackHat Button Data");  
          myButtons = new IoTT_Mux64Buttons();
          myButtons->initButtonsDirect(false); //GPIO mode
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
*/

/*
    if (useHat.devId == 7) //PurpleHat Trainside sensor
    {
      jsonDataObj = getDocPtr("/configdata/phcfg.cfg", false);
      if (jsonDataObj != NULL)
      {
        Serial.println("Load Trainside Sensor"); 
        trainSensor = new IoTT_TrainSensor(&Wire);
//        trainSensor->setTxCallback(sendMsg);
        trainSensor->loadLNCfgJSON(*jsonDataObj);
        delete(jsonDataObj); 
        Serial.println("Purple Sensor loaded"); 
      }
    }
    else
      Serial.println("Purple Sensor not activated");
*/

/*
    if (useHat.devId == 6) // || (useHat.devId == 8)) //RedHat++ Shield
    {
      jsonDataObj = getDocPtr("/configdata/rhcfg.cfg", false);
      if (jsonDataObj != NULL)
      {
        Serial.println("Load DCC++Ex communication interface"); 
        subnetMode = fullMaster;
        digitraxBuffer->setRedHatMode(sendLocoNetReply, *jsonDataObj); //function hooks in DigitraxBuffers
        if (lnSerial)
        {
          lnSerial->setNetworkType(subnetMode); 
//          delay(1000);
//          lnSerial->sendLineBreak(50); //3ms
//          delay(1000);
        }
        delete(jsonDataObj); 
        Serial.println("DCC++Ex loaded"); 
      }
      digitraxBuffer->clearSlotBuffer(false); //do not erase, just fix problems
    }
    else
    {
      Serial.println("DCC++Ex not activated");
      digitraxBuffer->clearSlotBuffer(true); //erase slot data (slave)
    }
*/

/*
    if (useHat.devId == 8) //SilverHat USB Serial Injector
    {
      jsonDataObj = getDocPtr("/configdata/usb.cfg", false);
      if (jsonDataObj != NULL)
      {
        Serial.printf("Load SilverHat Serial communication interface %i\n", useInterface.devId); 
        usbSerial = new IoTT_SerInjector(hatRxDAlt, hatTxD, false, 1);
        usbSerial->setProtType(DCCBoost);
        usbSerial->begin();
        usbSerial->setTxCallback(sendMsg);
//        usbSerial->loadLNCfgJSON(*jsonDataObj);
/*
        switch (useInterface.devId)
        {
          case 2: ; // LocoNet
          case 3: ; //LocoNet over MQTT
          case 4: usbSerial->setProtType(LocoNet); break; //LocoNet with Gateway
          case 5: ; //OpenLCB
          case 6: ; //OpenLCB over MQTT
          case 7: usbSerial->setProtType(OpenLCB); break; //OpenLCB with Gateway
          //usbSerial->setProtType(DCCEx); break; //DCC++Ex DCC Generator
        }
        delete(jsonDataObj); 
        Serial.println("Serial communication for Booster loaded"); 
      }
    }
    else
      Serial.println("Silver not activated");
*/

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Initialize ALMs
    if (useALM & 0x01) //&& (lnSerial || lbServer || lnMQTT || commGateway)) //Button Handler only with LocoNet or lbServer
    {
      Serial.println("Load Event Handler Data");  
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

#ifdef useSecEl
/*
    if (useALM & 0x02)
    {
      Serial.println("Load Security Element Data");  
      jsonDataObj = getDocPtr("/configdata/secel.cfg", false);
      if (jsonDataObj != NULL)
      {
        secElHandlerList = new(IoTT_SecurityElementList);
        secElHandlerList->loadSecElCfgJSON(*jsonDataObj);
        delete(jsonDataObj);
        uint16_t subFileCtr = 1;
        while (SPIFFS.exists("/configdata/secel" + String(subFileCtr) + ".cfg"))
        {
          jsonDataObj = getDocPtr("/configdata/secel" + String(subFileCtr) + ".cfg", false);
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
#endif

#ifdef useAI
/*
    if (useALM & 0x04) // && ((useHat.devId == 0) ||(useHat.devId == 1) ||(useHat.devId == 6)))  //RedHat Serial Injector
    {
      Serial.println("Initialize VoiceWatcher");  
      voiceWatcher = new(IoTT_VoiceControl);
      voiceWatcher->beginKeywordRecognition();
      voiceWatcher->setTxCallback(sendMsg);
      jsonDataObj = getDocPtr("/configdata/vwcfg.cfg", false);
      if (jsonDataObj != NULL)
      {
        voiceWatcher->loadKeywordCfgJSON(*jsonDataObj);
        delete(jsonDataObj);
      }
      Serial.println("VoiceWatcher running");  
    }
    else
      Serial.println("VoiceWatcher not activted");  
*/
#endif

    Serial.println("Connect WiFi");  
    establishWifiConnection(myWebServer,dnsServer);
    delay(1000);    
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
    
    startWebServer();
//    if (useNTP) getInternetTime();
    if (wiThServer)
      wiThServer->startServer();
    if (lbServer)
      lbServer->startServer();
     
    Serial.println(String(ESP.getFreeHeap()));
  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
  pinMode(0, INPUT);
  
}

void loop() 
{
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

  if (execLoop)
  {
#ifdef useSecEl
//    if (secElHandlerList) secElHandlerList->processLoop(); //calculates speeds in all blocks and sets signals accordingly
#endif
#ifdef useAI
//    if (voiceWatcher) voiceWatcher->processKeywordRecognition(); //listens for STOP and GO keywords
#endif
//    if (myDcc) myDcc->process(); //receives and decodes track signals
    if (eventHandler) eventHandler->processButtonHandler(); //drives the outgoing buffer and time delayed commands
//    if (usbSerial) usbSerial->processLoop(); //drives the USB interface serial traffic
    if (lbClient) lbClient->processLoop(); //drives the LocoNet over TCP and WiThrottle interface client traffic
    if (lbServer) lbServer->processLoop(); //drives the LocoNet over TCP interface server traffic
    if (wiThServer) wiThServer->processLoop(); //drives the WiThrottle interface server traffic
//    if (lnSerial) lnSerial->processLoop(); //handling all LocoNet communication
//    if (lnSubnet) lnSubnet->processLoop(); //handling all LocoNet Subnet communication
//    if (olcbSerial) olcbSerial->processLoop(); //handling all OpenLCB communication
//    if (trainSensor) trainSensor->processLoop(); //getting the data fromn the speed sensor

//  if (myChain)
//    hatVerified = myChain->isVerified();
//  else
//    hatVerified = true;
//  if (hatVerified)
//    if (myButtons) myButtons->processButtons(); //checks if a button was pressed and sends button messages
//  if (mySwitchList) mySwitchList->processLoop(pwrDC);
//  if (myChain) 
//    myChain->processChain(); //updates all LED's based on received status information for switches, inputs, buttons, etc.

  if (dataValid)
  {
    checkWifiTimeout(); //checks if wifi has been inactive and disables it after timeout
    if ((wifiCfgMode == 1))// || (wifiCfgMode == 3)) //STA active, Internet connection
    {
      if (!wifiCancelled) //handles keep alive updates as long connection is valid
      {
        if (wifiStatus == WL_CONNECTED)
        { 
          sendKeepAlive();
          if (useNTP)
            getInternetTime(); //gets periodic updates of date and time from NTP server
        }
        else
        {
          Serial.println("Reconnect WiFi");
          establishWifiConnection(myWebServer,dnsServer);
          startWebServer();
        }
      }
    
      if ((!wifiCancelled) && ((lnMQTTServer) || (lnMQTTClient))) //handles all wifi communication for MQTT
        if (wifiStatus == WL_CONNECTED)
        { 
            if (lnMQTTClient) 
            {
              if (lnMQTTClient->mustResubscribe()) //true after reset of the MQTT connection
              {
                //add code to resubscribe topics of all libraries that support native MQTT, e.g. LED, Buttons
                if (useInterface.devCommMode == 3) //native MQTT
                {
//                  if (myChain) myChain->subscribeTopics();
//                  if (myButtons) myButtons->subscribeTopics();
                }
//                lnMQTTClient->subscribeTopics();
              }
              lnMQTTClient->processLoop(); //LN or OpenLCB over MQTT
            }
            if (lnMQTTServer) 
            {
              if (lnMQTTServer->mustResubscribe()) //true after reset of the MQTT connection
                lnMQTTServer->subscribeTopics();
              lnMQTTServer->processLoop(); //LN or LCC Gateway
            }
        }
        else
        {
          Serial.println("Reconnect WiFi");
          establishWifiConnection(myWebServer,dnsServer);
          startWebServer();
        }
    }
    else
      sendKeepAlive();
    }
//    if (subnetMode != standardMode)
//      if (lnSerial) lnSerial->processLoop(); //increase frequency to reduce time lag
  }
  M5.update(); //read button states
  processDisplay();
  
  yield();
  digitraxBuffer->processLoop(); //updating DigitraxBuffers by querying information from LocoNet, e.g. slot statuses
}
