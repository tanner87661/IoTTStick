char BBVersion[] = {'1','0','2'};

//#define measurePerformance //uncomment this to display the number of loop cycles per second
//Arduino published libraries. Install using the Arduino IDE or download from Github and install manually
#include <arduino.h>
#include <Math.h>
#include <Wire.h>

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
//#include <IoTT_MQTTESP32.h> //as introduced in video # 29
//#include <IoTT_LEDChain.h> //as introduced in video # 30
#include <OneDimKalman.h>

//library object pointers. Libraries will be dynamically initialized as needed during the setup() function
AsyncWebServer * myWebServer = NULL; //(80)
DNSServer * dnsServer = NULL;
//WiFiClientSecure * wifiClientSec = NULL;
WiFiClient * wifiClient = NULL;
AsyncWebSocket * ws = NULL; //("/ws");
AsyncWebSocketClient * globalClient = NULL;
uint32_t wsBufferSize = 32768;
uint32_t wsRxReadPtr = 0;
uint32_t wsRxWritePtr = 0;
char * wsRxBuffer; //[32768]; //should this by dynamic?
uint32_t wsTxReadPtr = 0;
uint32_t wsTxWritePtr = 0;
char * wsTxBuffer; //[32768]; //should this by dynamic?
bool execLoop = true; //used to stop loop execution if update files are coming in. Must result in restart

//global variables
bool useStaticIP = false;
IPAddress static_ip;
IPAddress static_gw;
IPAddress static_nm;
IPAddress static_dns;
IPAddress ap_ip;
IPAddress ap_nm(255,255,255,0);

uint8_t wifiCfgMode = 0x02; //1: STA, 2: AP, 3: STA+AP
String deviceName; //device name used for AP, will load from node.cfg
String apPassword; //AP password, will load from node.cfg

bool wifiAlwaysOn = false; //set to false to shut Wifi after some time of inactivity. Gateway and MQTT should be disabled, though
bool wifiCancelled = false; //true if Wifi was switched off due to no use
uint32_t lastWifiUse = millis();
#define wifiShutTimeout 120000 //after 2 Mins of not using, Wifi is closed
#define apShutTimeout 180000 //after 3 Mins of not using, AP is shut. 3 Min to give enough time to connect to the AP from a PC

String bufferFileName = "/buffers.dat";

//more library object pointers. Libraries will be dynamically initialized as needed during the setup() function
//MQTTESP32 * lnMQTT = NULL;

//some variables used for performance measurement
#ifdef measurePerformance
uint16_t loopCtr = 0;
uint32_t myTimer = millis() + 1000;
#endif

//********************************************************Hardware Configuration******************************************************
#define groveRxD 2 //pin 1 yellow RxD for LocoNet, DCC
//#define groveTxD 12 //pin 2 white TxD for LocoNet, DCC
//constexpr gpio_num_t groveRxCAN = GPIO_NUM_33; //for OpenLCB CAN
//constexpr gpio_num_t groveTxCAN = GPIO_NUM_32; //for OpenLCB CAN
 
#define stickLED 2 //red LED on stick
#define hatSDA 21 //changes between input and output by I2C master
#define hatSCL 22 //pull SCL low while sending non I2C data over SDA
//#define hatRxD 22
//#define hatTxD 21
//#define hatInputPin 36 //25+ //36
//#define hatDataPin 26 //used to send LED data from FastLED. First, set to output low, then call show
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

String configDir = "/configdata";
String dataDir = "/www/trackdata/";
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

uint16_t oldWifiStatus = 0;
bool hatPresent = false;
bool pwrUSB = false;
bool pwrDC = true;

File uploadFile; //used for web server to upload files

/*
uint16_t sendMQTTMsg(char * topic, char * payload) //used for native MQTT only
{
  switch (useInterface.devId)
  {
    case 8: //native MQTT
      if (lnMQTT) return lnMQTT->mqttPublish(topic, payload);
      break; 
  }
}
*/

void setup() {
  // put your setup code here, to run once:

  wsRxBuffer = (char*) malloc(wsBufferSize); 
  wsTxBuffer = (char*) malloc(wsBufferSize); 
  Serial.begin(115200);

  char time_output[30];
  strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  Serial.println(time_output);
  Serial.println("Init SPIFFS");
  SPIFFS.begin(); //File System. Size is set to 1 MB during compile time and loaded with configuration data and web pages
  UniqueIDdump(Serial);
  myWebServer = new AsyncWebServer(80);
  dnsServer = new DNSServer();
  wifiClient = new WiFiClient();
  uint16_t wsRequest = 0;
  ws = new AsyncWebSocket("/ws");
  //verify some library settings
//  if (MQTT_MAX_PACKET_SIZE < 480)
//  {
//    Serial.printf("PubSubClient.h MQTT_MAX_PACKET_SIZE only %i. Should be >= 480\n", MQTT_MAX_PACKET_SIZE);
//    delay(5000);
//    hard_restart();
//  ESP.restart(); //configuration update requires restart to be sure dynamic allocation of objects is not messed up
//  }
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

    {
      Serial.println("No Interface defined");
//      useInterface.devName = "LocoNet";
      strcpy(useHat.devName, "LocoNet");
      useInterface.devCommMode = 1;
      useInterface.devId = 2;
    }
    //verify that hat and interface go together
    bool interfaceOK = false;

    Serial.printf("Initialize measuring system");
    jsonDataObj = getDocPtr("/configdata/trmon.cfg", false);
    if (jsonDataObj != NULL)
    {
        loadSensorCfgJSON(*jsonDataObj);
        delete(jsonDataObj);    }

    if (!initSensors())
    {
      delay(2000);
      ESP.restart();
    }
//-------------------------------------------------------------------------------------------------------------------------------------------------------
    Serial.println("Connect WiFi");  
    establishWifiConnection(myWebServer,dnsServer);
    delay(1000);    
//    if (lnMQTT)
//    {
//      Serial.println("Connect MQTT");  
//      establishMQTTConnection();
//      Serial.println("Connect MQTT done");  
//    }


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
//    if (lbServer)
//      lbServer->startServer();
      
    Serial.println(String(ESP.getFreeHeap()));
  }
  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
//  pinMode(0, OUTPUT);
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

    sensorReport();
      
    checkWifiTimeout(); //checks if wifi has been inactive and disables it after timeout
    if (wifiCfgMode == 1)
  // || (wifiCfgMode == 3)) //STA active, Internet connection
    {
      if (!wifiCancelled) //handles keep alive updates as long connection is valid
      {
        if (WiFi.status() == WL_CONNECTED)
        { 
          sendKeepAlive();
//          if (useNTP)
//            getInternetTime(); //gets periodic updates of date and time from NTP server
        }
        else
        {
          Serial.println("Reconnect WiFi");
          establishWifiConnection(myWebServer,dnsServer);
          startWebServer();
        }
      }
/*
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
*/
    }
    else
      sendKeepAlive();
  }
  else
    Serial.println("No ExecLoop");
  processDisplay();
}
