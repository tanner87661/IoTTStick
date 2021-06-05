#ifndef IoTT_LocoNetButtons_h
#define IoTT_LocoNetButtons_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IoTT_ButtonTypeDef.h>
#include <IoTT_DigitraxBuffers.h>

class IoTT_LocoNetButtonList;
class IoTT_LocoNetButtons;
class IoTT_BtnHandler;


class IoTT_BtnHandlerCmd
{
	public: //
		IoTT_BtnHandlerCmd();
		~IoTT_BtnHandlerCmd();
		void loadButtonCfgJSON(JsonObject thisObj);
		void executeBtnEvent();
		IoTT_BtnHandler * parentObj = NULL;
		
//	private:
		outputType targetType; //switch, input, button, signal, analog, power, analoginp
		uint16_t targetAddr; //address for target type
	public:
		uint8_t cmdType; //value parameter
		uint8_t cmdValue; //value parameter
		uint16_t execDelay; //time in ms to wait before sending next command
		   
};

class IoTT_BtnHandler
{
	public:
		IoTT_BtnHandler();
		~IoTT_BtnHandler();
		void loadButtonCfgJSON(JsonObject thisObj);
//		uint8_t getEventType();
		void processBtnEvent();
		uint16_t getCondAddr(uint8_t addrIndex = 0);
		int16_t getCondAddrIndex(uint16_t ofAddr); //index if found, -1 if not there
		IoTT_LocoNetButtons * parentObj = NULL;
		IoTT_BtnHandlerCmd * getCmdByIndex(uint16_t thisCmd);
		IoTT_BtnHandlerCmd * getCmdByTypeAddr(uint8_t cmdType, uint16_t cmdAddr);
		uint8_t numCmds = 0;
	private:
//		uint8_t eventType  = noevent;
		
		uint16_t * btnCondAddr = NULL;
		uint16_t btnCondAddrLen = 0;
//		uint8_t currEvent = 0;
		IoTT_BtnHandlerCmd ** cmdList = NULL;
};

class IoTT_LocoNetButtons
{
	public:
		IoTT_LocoNetButtons();
		~IoTT_LocoNetButtons();
		void loadButtonCfgJSON(JsonObject thisObj);
		void processBtnEvent(uint8_t inputValue);
		void processSimpleEvent(uint8_t inputValue);
		void processBlockDetEvent(uint8_t inputValue);
		void processDynEvent(uint8_t inputValue);
		void processSignalEvent(uint8_t inputValue);
		void processAnalogEvent(uint16_t inputValue);
		void processAnalogScaler(uint16_t inputValue);
		void processTransponderEvent(uint16_t inputValue);
		void processPowerEvent(uint16_t inputValue);
		uint16_t getBtnAddr(uint8_t index);
		int8_t hasBtnAddr(uint16_t thisAddr);
		sourceType getEventSource();
		sourceType hasEventSource(sourceType thisSource);
		bool getEnableStatus();
		int16_t getCondDataIndex(uint16_t ofData);
		uint8_t getLastRecEvent();
		uint8_t getLastComplEvent();
		IoTT_LocoNetButtonList * parentObj = NULL;
		
	private:
		uint8_t lastRecButtonEvent; //last event received
		uint8_t lastComplButtonEvent; //last event that was executed. If successful, those two are identical
		enableType enableInput = ent_alwayson;
		uint16_t enableAddr = 0;
		uint8_t enableStatus = 0;
		sourceType eventInput = evt_nosource;
		uint16_t * btnAddrList = NULL;
		uint8_t btnAddrListLen = 0;
		uint16_t * condDataList = NULL;
		uint8_t condDataListLen = 0;
		uint8_t eventTypeListLen = 0;
		IoTT_BtnHandler ** eventTypeList = NULL;
		uint8_t lastEvent = 0xFF;
};

typedef struct 
{
	IoTT_BtnHandlerCmd * nextCommand = NULL;
	uint32_t execTime = 0;
	bool tbd = false; //to be done. if true, we need to send, otherwise, it can be overwritten
} cmdPtr;

#define cmdBufferLen 50

typedef struct
{
	cmdPtr cmdOutBuffer[cmdBufferLen];
//	uint8_t readPtr = 0;
//	uint8_t writePtr = 0;
} cmdBuffer;

class IoTT_LocoNetButtonList
{
	public:
		IoTT_LocoNetButtonList();
		~IoTT_LocoNetButtonList();
		void processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue);
		void loadButtonCfgJSON(DynamicJsonDocument doc, bool resetList = true);
		void loadButtonCfgJSONObj(JsonObject doc, bool resetList = true);
		void processButtonHandler();
		void addCmdToBuffer(IoTT_BtnHandlerCmd * newCmd);
		IoTT_LocoNetButtons * getButtonByAddress(uint16_t btnAddr);
		uint16_t getButtonIndexByAddress(sourceType inputEvent, uint16_t btnAddr, uint16_t startIndex = 0);
	private:
		void freeObjects();
		IoTT_LocoNetButtons ** btnList = NULL;
		uint16_t numBtnHandler = 0;
		
	public:
		cmdBuffer outBuffer;
};

//these are the execute functions. Provide a function with this name and parameter in your application and it will be called when a command must be sent to LocoNet
extern void sendSwitchCommand(uint8_t opCode, uint16_t swiNr, uint8_t swiTargetPos, uint8_t coilStatus) __attribute__ ((weak)); //switch
extern void sendSignalCommand(uint16_t signalNr, uint8_t signalAspect) __attribute__ ((weak)); //signal
extern void sendPowerCommand(uint8_t cmdType, uint8_t pwrStatus) __attribute__ ((weak)); //power
extern void sendBlockDetectorCommand(uint16_t bdNr, uint8_t bdStatus) __attribute__ ((weak)); //block detector
extern void sendAnalogCommand(uint16_t btnNr, uint16_t analogVal) __attribute__ ((weak)); //analog value
extern void sendButtonCommand(uint16_t btnNr, uint8_t  btnEvent) __attribute__ ((weak)); //button command

#endif
