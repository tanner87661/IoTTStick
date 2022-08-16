#if defined ( ARDUINO )
#include <Arduino.h>
#include <SD.h>
#include <SPIFFS.h>
#endif
#include <M5AtomDisplay.h>
#include <M5UnitLCD.h>
#include <M5UnitOLED.h>
#include <M5Unified.h>

String BBVersion = "1.5.13";

//#define measurePerformance //uncomment this to display the number of loop cycles per second
//#define useAI
//Arduino published libraries. Install using the Arduino IDE or download from Github and install manually
#include <Math.h>

//#include <Wiroe.h>
//#include <esp_int_wdt.h>
//#include <esp_task_wdt.>

#include <WiFi.h>
#include <ArduinoUniqueID.h>
#include <time.h>
#include <FS.h>
#define FORMAT_SPIFFS_IF_FAILED true
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/alanswx/ESPAsyncWiFiManager
#include <ArduinoJson.h> //standard JSON library, can be installed in the Arduino IDE. Make sure to use version 6.x

//following libraries can be downloaded from https://github.com/tanner87661?tab=repositories
#include <IoTT_CommDef.h>
#include <IoTT_DigitraxBuffers.h> //as introduced in video # 30
#include <IoTT_LocoNetHBESP32.h> //this is a hybrid library introduced in video #29
#include <IoTT_MQTTESP32.h> //as introduced in video # 29
//#include <IoTT_Gateway.h> //LocoNet Gateway as introduced in video # 29
#include <IoTT_RemoteButtons.h> //as introduced in video # 29, special version for Wire connection via 328P
#include <IoTT_LocoNetButtons.h> //as introduced in video # 29
#include <IoTT_LEDChain.h> //as introduced in video # 30
#include <IoTT_SerInjector.h> //Serial USB Port to make it an interface for LocoNet and OpenLCB
//#include <IoTT_OpenLCB.h> //CAN port for OpenLCB
#include <IoTT_Switches.h>
#include <IoTT_SecurityElements.h> //not ready yet. This is the support for ABS/APB as described in Videos #20, 21, 23, 24
#include <NmraDcc.h> //install via Arduino IDE
//#include <OneDimKalman.h>
#include <IoTT_lbServer.h>
#include <IoTT_TrainSensor.h>
#ifdef useAI
  #include <IoTT_VoiceControl.h>
#endif

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
bool execLoop = true; //used to stop loop execution if update files are coming in. Must result in restart

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
#ifdef useAI
  IoTT_VoiceControl * voiceWatcher = NULL;
#endif
LocoNetESPSerial * lnSerial = NULL;
nodeType subnetMode = standardMode;
IoTT_SerInjector * usbSerial = NULL;
IoTT_LBServer * lbClient = NULL;
IoTT_LBServer * lbServer = NULL;
IoTT_LBServer * wiServer = NULL;
IoTT_DigitraxBuffers * digitraxBuffer = NULL; //pointer to DigitraxBuffers
//IoTT_OpenLCB *olcbSerial = NULL;
//HardwareSerial *wireSerial = NULL;
//ln_mqttGateway * commGateway = NULL;
IoTT_ledChain * myChain = NULL;
IoTT_SwitchList * mySwitchList = NULL;
MQTTESP32 * lnMQTTClient = NULL;
MQTTESP32 * lnMQTTServer = NULL;
NmraDcc  * myDcc = NULL;
IoTT_TrainSensor * trainSensor = NULL;
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
#define hatDataPin 26 //26 used to send LED data from FastLED to BlueHat. First, set to output low, then call show
#define rhDataPin 0 //used to send LED data from FastLED to the RedHat. First, set to output low, then call show
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
//  uint8_t * validInterfaces = NULL;
//  uint8_t numValid = 0;
} configType;

configType useHat;
configType useInterface;
uint32_t useServer = 0; //bit mask configuration list
uint32_t useALM = 0; //bit mask configuration list

uint8_t m5CurrentPage = 0;
uint16_t oldWifiStatus = 0;
uint8_t useM5Viewer = 0;
uint8_t m5DispLine = 0;

bool darkScreen = false;
bool hatPresent = false;
bool pwrUSB = false;
bool pwrDC = false;

File uploadFile; //used for web server to upload files


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
