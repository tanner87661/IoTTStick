//Version 1.0.0
#ifndef IoTT_DigitraxBuffers_h
#define IoTT_DigitraxBuffers_h

#include <Arduino.h>
#include <IoTT_CommDef.h>
#include <IoTT_SerInjector.h>
#include <IoTT_RemoteButtons.h>
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
#define purgeInterval 65000

typedef void (*dccFct) (lnTransmitMsg *, uint8_t, uint8_t); //slot nr, fct depending value

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

uint32_t millisElapsed(uint32_t since);
uint32_t microsElapsed(uint32_t since);

class IoTT_SerInjector;

typedef struct
{
	uint8_t sensStatType; //Bit 7 1=reversed 0: not used; 1: block detector; 2: switch report 3: button
	uint16_t sensAddr;
}sensorEntry;

class IoTT_Mux64Buttons;

class IoTT_DigitraxBuffers
{
	public:
		//general functions
		IoTT_DigitraxBuffers(txFct lnOut = NULL);
		~IoTT_DigitraxBuffers();
		void loadRHCfgJSON(DynamicJsonDocument doc);
		void setRedHatMode(txFct lnReply, DynamicJsonDocument doc);
		bool saveToFile(String fileName);
		void loadFromFile(String fileName);
		void processLoop(); //run loop to see if something needs to be sent out
		void processLocoNetMsg(lnReceiveBuffer * newData); //process incoming Loconet messages

		//read and write buffer values
		uint8_t getPowerStatus();
		uint8_t getButtonValue(uint16_t buttonNum);
		uint8_t getBDStatus(uint16_t bdNum);
		uint8_t getSwiPosition(uint16_t swiNum);
		uint8_t getSwiCoilStatus(uint16_t swiNum);
		uint8_t getSwiStatus(uint16_t swiNum);
		void setSwiStatus(uint16_t swiNum, bool swiPos, bool coilStatus); //called from DCC noitification
		uint32_t getLastSwiActivity(uint16_t swiNum);
		uint8_t getSignalAspect(uint16_t sigNum);
		void setSignalAspect(uint16_t sigNum, uint8_t sigAspect);
		uint16_t getAnalogValue(uint16_t analogNum);
		void setAnalogValue(uint16_t analogNum, uint16_t analogValue);
		bool getBushbyWatch();
		void enableBushbyWatch(bool enableBushby);
		uint16_t receiveDCCGeneratorFeedback(lnTransmitMsg txData);
		//LocoNet Management functions mainly for Command Station mode
		//from incoming DCC command
		//sensor slot finding functions
		void awaitFocusSlot();
//		slotData * getFocusSlotData();
		slotData * getSlotData(uint8_t slotNum);
		int8_t getFocusSlotNr();

	private: //functions
		//write buffer values
		void processBufferUpdates(lnReceiveBuffer * newData); //process incoming Loconet messages to the buffer
		void setPowerStatus(uint8_t newStatus);
		void setButtonValue(uint16_t buttonNum, uint8_t buttonValue);
		void setBDStatus(uint16_t bdNum, bool bdStatus);
		void setProgStatus(bool progBusy);

		//LocoNet functions for Cmd Stn Client mode
		void requestNextSlotUpdate();
		//LocoNet Management functions mainly for Command Station mode
		uint8_t getBushbyStatus(); //read from config slot
		uint8_t getSlotStatus(uint8_t slotNr);
		void updateSlotStatus(uint8_t slotNr, uint8_t newStatus);
		uint8_t getSlotOfAddr(uint8_t locoAddrLo, uint8_t locoAddrHi);
		uint16_t getAddrOfSlot(uint8_t slotNr);
		uint8_t getTopSlot(uint8_t masterSlot);

		uint8_t getFirstSlave(uint8_t masterSlot);
		void updateTrackByte(bool setOp, uint8_t trackBits);

		void processSlotManager(lnReceiveBuffer * newData); //process incoming Loconet messages to the buffer

		//DCC Generator functions
		bool processDCCGenerator(lnReceiveBuffer * newData);
//		void processDCCGeneratorFeedback(lnTransmitMsg txData);
		void iterateMULinks(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t templData, dccFct procFunc);
		void setSlotDirfSpeed(lnReceiveBuffer * newData, bool sendDCC);
		void generateSpeedCmd(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t topSpeed);
		void generateFunctionCmd(lnTransmitMsg * txBuffer, lnReceiveBuffer * newData);
		void purgeUnusedSlots();

	private: //variables
		sensorEntry sensorTable[32];
		IoTT_Mux64Buttons * rhButtons = NULL;
		bool isCommandStation = false;
		bool isRedHat = false;
//		bool useLocoNet = true;
		bool bushbyWatch = false;
		int8_t focusSlot = -1; //used by purple hat to identify slot to be monitored
		bool   focusNextAddr = false;
		uint8_t swWrPtr = 0;
		uint16_t lastSwiAddr = 0xFFFF;
		uint32_t nextBufferUpdate = millis();
		uint32_t nextSlotUpdate = millis();
		protocolEntry switchProtocol[switchProtLen];
		bool progMode = false;
		uint32_t progSent = millis();
		uint32_t fcRefresh = millis();
		uint32_t purgeSlotTimer = millis();
		slotData progSlot;
		uint8_t rxPin = 36;
		uint8_t txPin = 26; 
		
		//RedHat                  
		uint8_t ledLevel = 15; //0-100%
};

extern IoTT_DigitraxBuffers* digitraxBuffer; //pointer to DigitraxBuffers

#endif

//callback functions to notify application about buffer updates
extern void handlePowerStatus() __attribute__ ((weak)); //power status change
extern void handleSwiEvent(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStat) __attribute__ ((weak));
extern void handleInputEvent(uint16_t inpAddr, uint8_t inpStatus) __attribute__ ((weak));
extern void handleSignalEvent(uint16_t sigAddr, uint8_t sigAspect) __attribute__ ((weak));
extern void handleAnalogValue(uint16_t analogAddr, uint16_t inputValue) __attribute__ ((weak));
extern void handleButtonValue(uint16_t btnAddr, uint8_t inputValue) __attribute__ ((weak));
extern void handleTranspondingEvent(uint16_t zoneAddr, uint16_t locoAddr, uint8_t eventVal) __attribute__ ((weak));
