//Version 1.0.0
#ifndef IoTT_DigitraxBuffers_h
#define IoTT_DigitraxBuffers_h

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <IoTT_CommDef.h>
#include <IoTT_SerInjector.h>
#include <IoTT_RemoteButtons.h>
#include <IoTT_ButtonTypeDef.h>
#include <SPIFFS.h>

#define numSigs 2048
#define numSwis 512 //=2048/4
#define numBDs 512 //=4096/8
#define numAnalogVals 4096
#define numButtons 4096
#define numSlots 128 //total system slots
#define maxSlots 120 //locomotive slots

#define bufferUpdateInterval 1000
#define slotRequestInterval 1900
#define switchProtLen 20
#define progTimeout 10000
#define fcRefreshInterval 1000
//#define fcBroadcastInterval 75000
#define purgeLimitShort 30 //count for 300 s purge time
#define purgeLimitLong 60 // and for 60 sec
#define purgeInterval 10000 //run purge timer every 10 seconds
#define statusInterval 1000 //run status timer every 5 seconds
#define queryInterval 1000

//Digitrax CS
#define opSwPurgeTimeExt 12
#define opSwDisablePurging 13
#define opSwPurgeForce0 14
#define opSwSpeedStepDefault 20
#define opSwForceSwiAck 26
#define opSwDisableInterrogate 27
#define opSwTrackPwrRestore 32
#define opSwDisablePwrUpdate 42
#define opSwDisableSwiStateReply 44
#define opSwProgIsMain 46
//IoTT opSw
#define opSwSuperSlotRefresh 48
#define opEchoSwi 56
#define opEchoInp 57
#define opEchoLoco 58

//slot bytes
#define STAT 0
#define ADR 1
#define SPD 2
#define DIRF 3
#define TRK 4 //this is used as purge counter
#define SS2 5
#define ADR2 6
#define SND 7
#define ID1 8
#define ID2 9

//refresh status defines
#define slotFree 0
#define slotCommon 1
#define slotIdle 2
#define slotActive 3

//consist status defines
#define cnFree 0
#define cnUplink 1
#define cnDownlink 2
#define cnMiddle 3


typedef void (*dccFct) (uint8_t, uint8_t *); //slot nr, fct depending value array

typedef uint8_t blockDetBuffer[numBDs]; //4096 input bits, 8 per byte, lsb is lowest number
typedef uint8_t switchBuffer[numSwis]; //current status of switches, 4 per byte. First bit indicates coil state, second is position
typedef uint8_t signalBuffer[numSigs]; //current status of aspects, 1 per byte values 0..31, 3 MSB reserved
typedef uint16_t analogValBuffer[numAnalogVals]; //current status of analog values, 1 per word, running from 0 to 4096
typedef uint8_t buttonValBuffer[numButtons]; //current status of buttons, 2 per byte, statuses down, up, click, hold, dblclick
typedef uint8_t powerStatusBuffer;
typedef uint8_t slotData[10]; //slot data 0 is slot number, this is given by position in array, so we only need 10 bytes
typedef slotData slotDataBuffer[numSlots];

extern DynamicJsonDocument * getDocPtr(String cmdFile, bool duplData);

typedef struct
{
	uint32_t lastActivity;
	uint16_t devAddr;
}protocolEntry;

/*
typedef struct
{
	uint16_t lnAddr;
	outputType lnType;
	uint8_t lnMsg;
} activatorMsg;
*/

typedef struct
{
	uint16_t ardID = 0;
	uint8_t ardPin = 0;
	uint8_t ardType;
	uint16_t lnAddr;
	uint8_t posLogic;
	uint16_t numPar1;
	uint16_t numPar2;
	bool confOK = false;
	outputType lnType;
	char Descr[30] = {'\0'};
} arduinoPins;

typedef struct
{
	int8_t currUsePin = 0; //4MSB ture/false 4LSB analog pinNr
	float_t currMultiplier = 1.0;
	float_t currOffset = 0.0;
	uint8_t currBuffSize = 40;
} trackerData;

typedef struct
{
	uint16_t ID;
	char automationType[2] = {'\0'};
	char Descr[50] = {'\0'};
} automationEntry;

typedef struct
{
	uint16_t ID;
	char Description[50] = {'\0'};
	char FunctionMap[70] = {'\0'};
} rosterEntry;


uint32_t millisElapsed(uint32_t since);
uint32_t microsElapsed(uint32_t since);

class IoTT_SerInjector;

/*
typedef struct
{
	uint8_t sensStatType; //Bit 7 1=reversed 0: not used; 1: block detector; 2: switch report 3: button
	uint16_t sensAddr;
}sensorEntry;
*/

class IoTT_Mux64Buttons;

class IoTT_DigitraxBuffers
{
	public:
		//general functions
		IoTT_DigitraxBuffers(txFct lnOut = NULL);
		~IoTT_DigitraxBuffers();
		void loadRHCfgJSON(DynamicJsonDocument doc);
		void setRedHatMode(txFct lnReply, DynamicJsonDocument doc);
		void setLocoNetMode(bool newMode);
		void clearSlotBuffer(bool hardReset);
		bool cnTreeValid(uint8_t thisSlot, uint8_t cnLevel);
		uint8_t getConsistStatus(uint8_t ofSlot);
		uint8_t getRefreshStatus(uint8_t ofSlot);
		uint8_t getConsistTopSlot(uint8_t ofSlot);
		void setConsistStatus(uint8_t forSlot, uint8_t newStatus);
		void setRefreshStatus(uint8_t forSlot, uint8_t newStatus);
		bool getLocoNetMode();
		bool saveToFile(String fileName);
		void loadFromFile(String fileName);
		void processLoop(); //run loop to see if something needs to be sent out
		void processLocoNetMsg(lnReceiveBuffer * newData); //process incoming Loconet messages
		void writeProg(uint16_t dccAddr, uint8_t progMode, uint8_t progMethod, uint16_t cvNr, uint8_t cvVal);
		void readProg(uint16_t dccAddr, uint8_t progMode, uint8_t progMethod, uint16_t cvNr);
		void readAddrOnly();
		void setPowerStatus(uint8_t newStatus);
		void localPowerStatusChange(uint8_t newStatus);
//		void sendSwiReq(bool useAck, uint16_t swiAddr, uint8_t newPos);
		uint8_t getOpSw(uint8_t opSwNr, uint8_t numBits);
		void setOpSw(uint8_t opSwNr, uint8_t numBits, uint8_t newVal);
		void getRedHatConfig(uint16_t filterMask);
		void sendRedHatCmd(char * cmdStr);
		//read and write buffer values
		uint8_t getPowerStatus();
		uint8_t getButtonValue(uint16_t buttonNum);
		uint8_t getBDStatus(uint16_t bdNum);
		uint8_t getSwiPosition(uint16_t swiNum);
		uint8_t getSwiCoilStatus(uint16_t swiNum);
		uint8_t getSwiStatus(uint16_t swiNum);
		void setSwiStatus(uint16_t swiNum, bool swiPos, bool coilStatus); //called from DCC or WiThrottle notification
		uint32_t getLastSwiActivity(uint16_t swiNum);
		uint8_t getSignalAspect(uint16_t sigNum);
		void setSignalAspect(uint16_t sigNum, uint8_t sigAspect);
		uint16_t getAnalogValue(uint16_t analogNum);
		void setAnalogValue(uint16_t analogNum, uint16_t analogValue);
		bool getBushbyWatch();
		void enableBushbyWatch(bool enableBushby);
		uint32_t getFCTime();
		uint32_t getFCRate();
		void setFCTime(uint32_t newTime, bool updateLN);
		void setFCRate(uint8_t newRate, bool updateLN);
		void enableLissyMod(bool enableLissy);
		void enableFCRefresh(bool useFC, uint16_t fcRefreshRate);
		uint8_t getUpdateReqStatus();
		void clearUpdateReqFlag(uint8_t clrFlagMask);
//		void addActor(uint16_t Id, uint8_t pinType, uint8_t pinNr, uint8_t flags);
		void sendDCCCmdToWeb(std::vector<ppElement> * myParams);
		void sendFCCmdToWeb();
		void sendTrackCurrent(uint8_t trackId);
		uint16_t receiveDCCGeneratorFeedbackNew(std::vector<ppElement> * txData);
//		uint16_t receiveDCCGeneratorFeedback(lnTransmitMsg txData);
		//LocoNet Management functions mainly for Command Station mode
		//from incoming DCC command
		//sensor slot finding functions
		void awaitFocusSlot(int16_t dccAddr, bool simulOnly);
//		slotData * getFocusSlotData();
		slotData * getSlotData(uint8_t slotNum);
		int8_t getFocusSlotNr();
		uint8_t getSlotOfAddr(uint8_t locoAddrLo, uint8_t locoAddrHi);
		String getRouteInfo(uint16_t id, bool exFormat);
		String getRosterInfo(uint16_t id, bool exFormat);
		String getTurnoutInfo(uint16_t id, bool exFormat);
		String getSensorInfo(uint16_t id, bool exFormat);

	private: //functions
		//write buffer values
		void processBufferUpdates(lnReceiveBuffer * newData); //process incoming Loconet messages to the buffer
		void setButtonValue(uint16_t buttonNum, uint8_t buttonValue);
		void setBDStatus(uint16_t bdNum, bool bdStatus);
		void setProgStatus(bool progBusy);
		void processDCCSwitch(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStatus);
		arduinoPins* findPeripherialItemById(arduinoPins * itemList, uint16_t listLen, uint8_t devType, uint16_t itemID);

		void processDCCInput(uint16_t sensID, bool sensStatus);
		//LocoNet functions for Cmd Stn Client mode
		void requestNextSlotUpdate();
		//LocoNet Management functions mainly for Command Station mode
		uint8_t getBushbyStatus(); //read from config slot
		uint8_t getSlotStatus(uint8_t slotNr);
		void updateSlotStatus(uint8_t slotNr, uint8_t newStatus);
		uint16_t getAddrOfSlot(uint8_t slotNr);
		uint8_t getTopSlot(uint8_t masterSlot);

		uint8_t getFirstSlave(uint8_t masterSlot);
//		void updateTrackByte(bool setOp, uint8_t trackBits);

		void processSlotManager(lnReceiveBuffer * newData); //process incoming Loconet messages to the buffer
		void initArduinoBoard();
		
		//DCC Generator functions
		bool processDCCGenerator(lnReceiveBuffer * newData);
//		void processDCCGeneratorFeedback(lnTransmitMsg txData);
		void iterateMULinks(uint8_t thisSlot, uint8_t dirSpeedData);
		void setSlotDirfSpeed(lnReceiveBuffer * newData);
//		void generateSpeedCmd(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t topSpeed);
//		void generateFunctionCmd(lnTransmitMsg * txBuffer, lnReceiveBuffer * newData);
		void purgeUnusedSlots();

	public:
		slotDataBuffer slotBuffer;
		powerStatusBuffer trackByte = 1; //OPC_IDLE, Em. Stop, Power ON

	private: //variables
		blockDetBuffer blockDetectorBuffer;
		analogValBuffer analogValueBuffer;
		switchBuffer switchPositionBuffer;
		signalBuffer signalAspectBuffer;
		buttonValBuffer buttonValueBuffer;
		uint8_t dispatchSlot = 0x00;
		uint8_t inpQuery = 0xFF; //used to send out query swi cmds after power on
		uint32_t queryDelay = millis(); 
		slotData stdSlot = {0x03, 0x80, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00}; //Byte 1 0x80 for unused, newly initialized slot
		uint32_t purgeLimit = 30;  //standard purge counter limit
//		sensorEntry sensorTable[32];
//		IoTT_Mux64Buttons * rhButtons = NULL;
		bool isCommandStation = false;
//		bool isRedHat = false;
		bool isLocoNet = true;
		bool initPhase = true;
//		bool useLocoNet = true;
		bool bushbyWatch = false;
		bool translateLissy = true;
		bool broadcastFC = true;
		uint32_t fcBroadcastInterval = 75000; //msecs
		int8_t focusSlot = -1; //used by purple hat to identify slot to be monitored
		bool   focusNextAddr = false;
		uint8_t swWrPtr = 0;
		uint8_t requestInpStatusUpdate = 0x00; //flags for resending input button information
		uint16_t lastSwiAddr = 0xFFFF;
		uint32_t nextBufferUpdate = millis();
		uint32_t nextSlotUpdate = millis();
		protocolEntry switchProtocol[switchProtLen];
		bool progModeActive = false;
		bool readFullAddr = false;
		std::vector<uint32_t> cvBuffer;
		uint32_t progSent = millis();
		uint32_t fcRefresh = millis();
		uint32_t fcLastBroadCast = millis();
		uint32_t purgeSlotTimer = millis();
		uint32_t getStatusTimer = millis();
		slotData progSlot;
		uint8_t rxPin = 36;
		uint8_t txPin = 26; 
		bool progBusy = false;
		uint8_t progCV = 0;
		//RedHat                  
		uint8_t ledLevel = 15; //0-100%
		arduinoPins * sensorInputs = NULL;
		uint16_t sensorInputLen = 0;
		arduinoPins * turnoutOutputs = NULL;
		automationEntry * automationList = NULL;
//		rosterEntry * rosterList = NULL;
		std::vector<rosterEntry> rosterList; //a list holding the turnout numbers to be initialized in WiThrottle Server
		uint16_t turnoutOutputLen = 0;
		uint16_t routeOutputLen = 0;
//		uint16_t rosterOutputLen = 0;
		uint8_t DCCActiveSlots = 20; //number of active slots on the DCC++EX
		uint16_t progLimit = 50;
		uint16_t progPulseMin = 200;
		uint16_t progPulseMax = 20000;
		uint32_t intFastClock = 0; //counts fc seconds per day from 0 to 86400, then loops
		uint8_t progNumTry = 3;
		bool    progBoost = false;
		bool    configPeripheralsPwrUp = true;
		uint32_t webTimeout = millis();
		uint8_t trackGaugesLen = 0;
		trackerData* trackGauges = NULL;
		rmsBuffer** trackData = NULL;
//		rmsBuffer* progData = NULL;
		
};

extern IoTT_DigitraxBuffers* digitraxBuffer; //pointer to DigitraxBuffers
//extern AsyncWebSocketClient * globalClient;
extern std::vector<wsClientInfo> globalClients; // a list to hold all clients when in server mode

#endif

//callback functions to notify application about buffer updates
extern void handlePowerStatus() __attribute__ ((weak)); //power status change
extern void handleSwiEvent(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStat) __attribute__ ((weak));
extern void handleInputEvent(uint16_t inpAddr, uint8_t inpStatus) __attribute__ ((weak));
extern void handleSignalEvent(uint16_t sigAddr, uint8_t sigAspect) __attribute__ ((weak));
extern void handleAnalogValue(uint16_t analogAddr, uint16_t inputValue) __attribute__ ((weak));
extern void handleButtonValue(uint16_t btnAddr, uint8_t inputValue) __attribute__ ((weak));
extern void handleTranspondingEvent(uint16_t zoneAddr, uint16_t locoAddr, uint8_t eventVal) __attribute__ ((weak));
extern void handleProgrammerEvent(uint8_t * programmerSlot) __attribute__ ((weak));


//these are the execute functions. Provide a function with this name and parameter in your application and it will be called when a command must be sent to LocoNet
extern void sendSwitchCommand(uint8_t opCode, uint16_t swiNr, uint8_t swiTargetPos, uint8_t coilStatus) __attribute__ ((weak)); //switch
extern void sendSignalCommand(uint16_t signalNr, uint8_t signalAspect) __attribute__ ((weak)); //signal
extern void sendPowerCommand(uint8_t cmdType, uint8_t pwrStatus) __attribute__ ((weak)); //power
extern void sendBlockDetectorCommand(uint16_t bdNr, uint8_t bdStatus) __attribute__ ((weak)); //block detector
extern void sendAnalogCommand(uint16_t btnNr, uint16_t analogVal) __attribute__ ((weak)); //analog value
extern void sendButtonCommand(uint16_t btnNr, uint8_t  btnEvent) __attribute__ ((weak)); //button command
extern void sendSwiReportMessage(uint16_t inpAddr, uint8_t newPos) __attribute__ ((weak)); //switch report command

//callback function to WiThrottle Server
extern void wiAddrCallback(std::vector<ppElement>* ppList) __attribute__ ((weak));
 
