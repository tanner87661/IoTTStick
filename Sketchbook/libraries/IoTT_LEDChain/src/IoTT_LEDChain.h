#ifndef IoTT_LEDChain_h
#define IoTT_LEDChain_h

#define FASTLED_INTERRUPT_RETRY_COUNT 1
//#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>
#include <ArduinoJson.h>
//#include <PubSubClient.h>
#include <IoTT_CommDef.h>
#include <IoTT_ButtonTypeDef.h>
#include <IoTT_DigitraxBuffers.h>

//#define useRTOS

#define i2cMaxChainLength 525

enum chainModeType : byte {hatDirect=0, hatI2C = 1}; //, hatSerComm = 2};
enum transitionType : byte {soft=0, direct=1, merge=2};
enum colorMode : byte {constlevel=0, localblinkpos=1, localblinkneg=2, globalblinkpos=3, globalblinkneg=4, localrampup=5, localrampdown=6, globalrampup=7, globalrampdown=8};
enum displayType : byte {discrete=0, linear=1};

class IoTT_ledChain;
class IoTT_LEDHandler;

class IoTT_ColorDefinitions
{
	public:
		IoTT_ColorDefinitions();
		~IoTT_ColorDefinitions();
		void loadColDefJSON(JsonObject thisObj);
	private:
	public:
		char colorName[50];
		CRGB RGBVal;
		CHSV HSVVal;
	private:
};

class IoTT_LEDCmdList
{
	public:
		IoTT_LEDCmdList();
		~IoTT_LEDCmdList();
		void loadCmdListJSON(JsonObject thisObj);
		void updateLEDs();
	private:
	public:
		IoTT_LEDHandler* parentObj = NULL;
		uint8_t upToValLen =0;
		uint16_t * upToVal = NULL;
		IoTT_ColorDefinitions** colOn = NULL;
		IoTT_ColorDefinitions** colOff = NULL;
		uint8_t * dispMode = NULL;
		uint16_t * blinkRate = NULL;
		uint8_t * transType = NULL;
	
};

class IoTT_LEDHandler
{
	public:
		IoTT_LEDHandler();
		~IoTT_LEDHandler();
		void loadLEDHandlerJSON(JsonObject thisObj);
		void updateLEDs();
		void updateLocalBlinkValues();
		bool identifyLED(uint16_t ledNr);
		void processTranspEvent(uint16_t btnAddr, uint16_t eventValue);
	private:
		void freeObjects();
		void updateBlockDet();
		void updateSwitchPos();
		void updateSwSignalPos(bool isDynamic = true);
		void updateSignalPos();
		void updateButtonPos();
		void updateAnalogValue();
		void updateTransponder();
		void updatePowerStatus();
		void updateConstantLED();
		void updateChainData(IoTT_LEDCmdList * cmdDef, IoTT_LEDCmdList * cmdDefLin = NULL, uint8_t distance = 0);
		void updateChainDataForColor(uint8_t colorNr, IoTT_LEDCmdList * cmdDef, IoTT_LEDCmdList * cmdDefLin = NULL, uint8_t distance = 0);
	public:
		IoTT_ledChain* parentObj = NULL;
	public:
		IoTT_LEDCmdList** cmdList = NULL;
		uint16_t cmdListLen = 0;
		uint16_t * ledAddrList = NULL;
		uint8_t ledAddrListLen = 0;
		bool multiColor = false;
		uint16_t * ctrlAddrList = NULL;
		uint8_t ctrlAddrListLen = 0;
		uint16_t * condAddrList = NULL;
		uint8_t condAddrListLen = 0;
		sourceType ctrlSource = evt_nosource;
		uint8_t displType = 0;

		CHSV * currentColor = NULL; //CHSV(0,0,0);
		uint16_t blinkInterval = 500;
		uint32_t blinkTimer = millis();
		bool     blinkStatus = false;
		float_t  locFaderValue = 0;
		uint16_t lastValue = 0xFFFF;
		uint16_t lastStatValue = 0xFFFF;
		uint32_t lastActivity = 0xFFFFFFFF;
		uint8_t  lastExtStatus = 1; //used for transponder events whioch are not in (yet) in Digitrax Buffer)
};

class IoTT_ledChain
{
	public:
		IoTT_ledChain(TwoWire * newWire = NULL, uint16_t useI2CAddr = 0x00, bool multiRequest = true);
		~IoTT_ledChain();
		void loadLEDChainJSON(DynamicJsonDocument doc, bool resetList = true);
		void loadLEDChainJSONObj(JsonObject doc, bool resetList = true);
		void setMQTTMode(mqttTxFct txFct);
		void subscribeTopics();
		void processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue);
	
		uint16_t getChainLength();
		CRGB * getChain();
		float_t getBrightness();
		sourceType getBrightnessControlType();
		uint16_t getBrightnessControlAddr();
		IoTT_ColorDefinitions* getColorByName(String colName);
//		void initI2CLED();
		void resetI2CWDT();
		bool isVerified();

	private:
		void freeObjects();
		void updateLEDs();
		void setBrightness(float_t newVal);
		
		uint16_t I2CAddr = 0x00;
		TwoWire * thisWire = NULL;
		chainModeType chainMode = hatDirect;
		topicStruct subTopicList[2] = {{"LEDSET", false}, {"LEDASK", false}};
		topicStruct pubTopicList[1] = {{"LEDREPLY", false}};
		uint8_t intrCtr = 0; //LED interlacing
		uint16_t chainLength = 0;
		CRGB * ledChain = NULL;
		CHSV lastCol = CHSV(0,0,0);
		
		IoTT_ColorDefinitions** colorDefinitionList = NULL;
		uint16_t colorDefListLen = 0;
		
		IoTT_LEDHandler** LEDHandlerList = NULL;
		uint16_t LEDHandlerListLen = 0;

		float currentBrightness = 0.8;
		sourceType brightnessControlType = evt_nosource; //not defined
		uint16_t brightnessControlAddr = 0;
		
		bool useMultiByte = true;
		
		uint16_t tempLEDBuffer[64];
		uint16_t tempLEDCtr = 0;
		
		void showI2CLED();
		void setI2CLED(uint16_t ledNr, CHSV newCol);
		void setI2CLEDType(uint16_t ledType);
		void setI2CChainLen(uint16_t chainLen);
		void resetI2CDevice(bool forceReset);
		int8_t pingI2CDevice(uint8_t numBytes = 3);
		uint16_t i2cChainLength;
		uint16_t i2cChainType = 0;
		int16_t i2cDevID = -1;
		bool i2cVerified = false;


	public:
		uint16_t blinkInterval = 500;
		uint32_t blinkTimer;
		uint32_t ledUpdateTimer;
		uint32_t procRollover = millis();
		uint16_t ledUpdateInterval = 50; //20Hz refresh
		bool     blinkStatus;
		uint8_t  pingCtr = 0;
		uint8_t  fastPin;
		float_t  globFaderValue;
		uint16_t colTypeNum = 0;
		bool needUpdate;
		SemaphoreHandle_t ledBaton;
		uint8_t refreshAnyway = 2; //refresh LEDs despite color test shows same, we do 2 passes for odd/even

	public: 
		CRGB *  initChain(word numLEDs);
		void setFastPin(uint8_t pinNr);
		void processChain();
		bool processMQTTCmd(char * topic, DynamicJsonDocument doc);
		void sendLEDStatusMQTT(uint16_t ledNr);
		void setCurrColHSV(uint16_t ledNr, CHSV newCol);
		void setBlinkRate(uint16_t blinkVal);
		void identifyLED(uint16_t LEDNr);
		void setRefreshInterval(uint16_t newInterval); //1/frame rate in millis()
		bool getBlinkStatus();
		float_t getFaderValue();
		CHSV getCurrColHSV(uint16_t ledNr);
};

#endif
