#ifndef IoTT_SecurityElements_h
#define IoTT_SecurityElements_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IoTT_DigitraxBuffers.h>

enum sigAddrType : uint8_t  {swiDyn=0, swiStat=1, sigNMRA=3};
enum ctrlType: int8_t {manual=-1, ABSS=0, ABSD=1, APB=2, CTC=3};
enum fallbackMode: int8_t {nofallback = 0, fbOnEnter = 1, fbOnExitPrev = 2, fbOnExit = 3}; //trigger for signal fallback

typedef struct{
	char entryName[30];
	uint16_t entrySpeed;
} IoTT_SpeedTableEntry;

class IoTT_SpeedTable
{
	public:
		IoTT_SpeedTable();
		~IoTT_SpeedTable();
		
		char speedSetName[50];
		IoTT_SpeedTableEntry * tableEntries = NULL;
		uint16_t numSpeedLines = 0;
		void loadSpeedTableEntryJSON(JsonObject thisObj);
		void getSpeedByName(String speedEntryName);
	private:
		void freeObjects();
	
};

typedef struct{
	char aspectName[30];
	uint8_t aspectNr = 0;
	char staticSpeedName[30];
	uint16_t staticSpeed;
	char dynamicSpeedName[30];
	uint16_t dynamicSpeed;
} IoTT_AspectEntry;

class IoTT_AspectGenerator
{
	public:
		IoTT_AspectGenerator();
		~IoTT_AspectGenerator();
		void loadAspectCfgJSON(JsonObject thisObj);
	private:
		void freeObjects();
	public:
		char aspectSetName[50] = "";
//		sigAddrType addrType = sigNMRA;
//		uint8_t numAddr = 1;
		IoTT_AspectEntry * aspectEntries = NULL;
		uint8_t numAspectEntries = 0;
		uint8_t getAspectFromSpeed(uint8_t dynSpeedLevel, uint8_t statSpeedLevel);
		uint8_t getDynLevelFromAspect(uint8_t thisAspect);
};


class IoTT_SecurityElement; //forward declaration
class IoTT_SecurityElementSection; //forward declaration
class IoTT_SecurityElementList; //forward declaration

class IoTT_SecElLeg //one single leg (A,B or C) with entry signal and connector to neighboringleg
{
	public:
		IoTT_SecElLeg();
		~IoTT_SecElLeg();
		void loadSELegCfgJSON(JsonObject thisObj);
		uint8_t getLegPos();
		void resolveLegConnector();
		void setDynSpeed(uint16_t newSpeedLevel);
		uint8_t getCurrentDynSpeed();
		bool hasEntrySignal();
		uint8_t getNextSignalDynCtr();
		IoTT_SecElLeg * getConnectedLeg(); //returns pointer to A, B, or C or Null depending on calling leg, switch position and available switch and logic
		IoTT_SecElLeg * getOpenLeg(); //returns pointer to B or C or NULL
		void sendAspectCommand(uint8_t aspectVal);
		int calculateAspect();
		uint8_t updateDynSpeedFromSignal();
//		void processLocoNetMsg(lnReceiveBuffer * newData);
	private:
		void freeObjects();
	public:
		IoTT_SecurityElement * parentSE = NULL;
		uint8_t legPos; //0=A, 1=B, if present, 2=C
		uint8_t legTechSpeedIn; //index of static model
		uint8_t legTechSpeedOut; //index of static model
		
		sigAddrType addrType = sigNMRA;
		uint8_t numAddr = 1;

		uint16_t destSENr; //to be resolved at startup
		uint8_t destSELeg; //to be resolved at startup
		IoTT_SecElLeg * destSE; //pointer tp leg
		
		IoTT_AspectGenerator * entryAspectGenerator;
		uint16_t entrySignalAddr[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
		
		//runtime data
		uint8_t dynUptickCtr = 0; //count increments from occupied  block to determin dynSpeed under various control models (i.e. ABSS/ABSD)
		uint8_t lastSigAspect = 0xFF;
		uint8_t lastDynIndex = 0xFF; //in case of DynSwi type the index of the last address that came in
		
};

class IoTT_SecurityElement //one single element with up to 3 legs
{
	public:
		IoTT_SecurityElement();
		~IoTT_SecurityElement();
		void loadSecElCfgJSON(JsonObject thisObj);
		void resolveLegConnectors();
		IoTT_SecElLeg * getLegPtr(uint8_t destSELeg);
		void clearDirectionFlags();
		void setDirection(bool inBound);
		void processElement(); //the core processing routine
		void processABSS(uint8_t newEvents);
		void processABSD(uint8_t newEvents);
		void processAPB(uint8_t newEvents);
		void processCTC(uint8_t newEvents);
		void processManualSE(uint8_t newEvents);
//		void processLocoNetMsg(lnReceiveBuffer * newData);
	private:
		void freeObjects();
	
	public:
		//SE Data
		uint16_t secelID = 0xFFFF;
		ctrlType   ctrlMode = manual; //-1;
		uint16_t blockdetAddr = 0xFFFF;
		uint16_t switchAddr = 0xFFFF;
		bool     switchLogic = true; //true = closed=1 -> AB, thrown=0 -> AC and vice versa for false

		//station related data
		uint16_t targetBtn = 0xFFFF;
		uint16_t departBtn = 0xFFFF;
		uint16_t terminalID = 0xFFFF;
		uint16_t stationID = 0xFFFF;
		uint8_t  stationTrackNr = 0;
		bool isTerminal = false;
		bool isStation = false;
		char stationName[50];

		//SE Leg Data
		std::vector<IoTT_SecElLeg*> connLeg; //0=A, 1=B, if present, 2=C
		IoTT_SecurityElementSection * parentSEModel = NULL;
		IoTT_SecurityElementList * parentSEL = NULL;
//		uint8_t numLegs = 0;
		uint8_t selectedLeg = 1; //defaults to B, in case of no switch
		
		bool autoProtect = false;
		fallbackMode fbMode = nofallback;
		uint8_t lastSwiPos = 0xFF;
		uint8_t lastBDStatus = 0xFF;
		
	private:
		bool dirIn = false; //from A to B/C
		bool dirOut = false;// from B/C to A
};

class IoTT_SecurityElementSection //one section terminated by stopping tracks at each end, sharing the same control model
{
	public:
		IoTT_SecurityElementSection();
		~IoTT_SecurityElementSection();
		void loadSecElCfgJSON(JsonObject thisObj);
		IoTT_SecElLeg * getLegPtr(uint16_t destSENr, uint8_t destSELeg);
		void resolveLegConnectors();
		void processLoop();
		void clearDirectionFlags();
		void setDirection(bool inBound);
//		void processLocoNetMsg(lnReceiveBuffer * newData);
	private:
		void freeObjects();
		std::vector<IoTT_SecurityElement*> secElList;
	public:
		char modelName[50] = "";
		bool isActive = false;
		IoTT_SecurityElementList * parentSEL = NULL;
		IoTT_SpeedTable* dynamicSpeedModel = NULL;
		IoTT_SpeedTable* staticSpeedModel = NULL;
};

class IoTT_SecurityElementList //admin node holding rulebooks, signal definitions, and list of SESections
{
	public:
		IoTT_SecurityElementList();
		~IoTT_SecurityElementList();
		void loadSecElCfgJSON(DynamicJsonDocument doc, bool resetList = true);
		IoTT_SecElLeg * getLegPtr(uint16_t destSENr, uint8_t destSELeg);
		void processLoop();
		IoTT_SpeedTable* getStaticSpeedByName(String speedName);
		IoTT_SpeedTable* getDynamicSpeedByName(String speedName);
		IoTT_AspectGenerator* getAspectGeneratorByName(String aspName);
//		void processLocoNetMsg(lnReceiveBuffer * newData);
	private:
		void freeObjects();
		IoTT_SecurityElementSection** secModelList = NULL;
		void resolveLinks();
		uint16_t numSecModel = 0;
	public:
		std::vector<IoTT_AspectGenerator*> aspGenList;
		std::vector<IoTT_SpeedTable*> staticSpeedList;
		std::vector<IoTT_SpeedTable*> dynamicSpeedList;
//		uint16_t numStaticSpeedSets = 0;
//		uint16_t numDynamicSpeedSets = 0;
//		uint16_t numAspectGenerators = 0;
};

extern void sendSwitchCommand(uint16_t swiNr, uint8_t swiTargetPos, uint8_t coilStatus) __attribute__ ((weak)); //switch
extern void sendSignalCommand(uint16_t signalNr, uint8_t signalAspect) __attribute__ ((weak)); //signal

#endif
