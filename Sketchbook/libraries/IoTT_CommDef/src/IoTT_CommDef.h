#ifndef IoTT_CommDef_h
#define IoTT_CommDef_h

#define LNDebug

#define DebugPrint

#ifdef DebugPrint
	#define SerialDebug(x) Serial.println(x)
	#define SerialDebugf(x) Serial.printf(x)
#else
	#define SerialDebug(x)
	#define SerialDebugf(x)
#endif

#include <Arduino.h>
#include <inttypes.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//Error Flags
#define errorCollision    0x01
#define errorFrame        0x02
#define errorTimeout      0x04
#define errorCarrierLoss  0x08

#define msgEcho		  	  0x10
#define msgIncomplete	  0x20		
#define msgXORCheck       0x40
#define msgStrayData	  0x80	

#define dirUpstream 	  0x0200

#define fromLNSubnet	  0x1000
#define fromMQTTGW		  0x2000
#define fromWiServer	  0x3000
#define fromLBServer	  0x4000

#define lnMaxMsgSize 48 //this is the maximum length of LocoNet Messages as defined by the standard. Do not change.

enum messageType : uint8_t {LocoNet=0, OpenLCB=1, DCCEx=2, DCCBoost=3};
enum nodeType : uint8_t {standardMode = 0, limitedMaster = 1, fullMaster = 2};

//this type is used to send a new message to Loconet
typedef struct {
	messageType msgType = LocoNet;
    uint8_t    lnMsgSize = 0;
    uint8_t    lnData[lnMaxMsgSize];
    uint16_t   requestID = 0; //temporarily store reqID while waiting for message to get to head of buffer Bits 0..7 are random number, bits 8..11 are ctrl flags, bits 12..15 encode message origin
/*
 * Ctrl Flags
 * 	0x0200 Upstream message
 *  everything else reserved
 * 
 * Message Origin
 * 	0x0000 not set
 *  0x1000 Loconet Subnet
 *  0x2000 MQTT Gateway
 *  0x3000 WiServer
 *  0x4000 lbServer
 *  everything else reserved
*/ 
				//Loconet Subnet: 1; MQTT: 2; WiServer: 3; LNTCP: 4;
    uint32_t   reqRecTime = 0;
} lnTransmitMsg;

//The call back function of the library provides a pointer to this structure with the incoming message and status information 
typedef struct {  
	messageType msgType = LocoNet;
    uint8_t    lnMsgSize = 0;
    uint8_t    lnData[lnMaxMsgSize];
    uint16_t   requestID = 0; //contains reqID if echo or response to previous request
							//also used as change mask for DCC Generator in IoTT_DigitraxBuffers
    uint32_t   reqRecTime = 0;
    uint32_t   echoTime = 0; //time in microsecs between request and echo (same) message
    uint32_t   reqRespTime = 0; //time in microsecs between request and response message
    uint8_t	   errorFlags = 0;	
} lnReceiveBuffer;    

typedef struct {
	uint8_t priority;
	uint8_t frameType;
	uint8_t canFrameType;
	uint16_t MTI;
	uint16_t srcAlias;
	uint16_t dstAlias;
	uint8_t dlc;
	union 
	{
		uint8_t u8[8];   /**< \brief Payload byte access*/
		uint32_t u32[2]; /**< \brief Payload u32 access*/
		uint64_t u64;    /**< \brief Payload u64 access*/
	} olcbData;
} olcbMsg;

typedef struct
{
	char topicName[50];
	bool inclAddr;
} topicStruct;

typedef union 
{
	int 	 longVal;
	float_t  floatVal;
	char     strVal[4];	
	char *   strPtr;	
} ppParam;

typedef struct
{
	uint8_t paramNr = 0; //unused
	uint8_t numParams = 0; //unused
	uint8_t dataType = 0xFF; //unused
	ppParam payload;
} ppElement;

typedef struct
{
	uint8_t cmdName = 0;
	uint32_t execTime = 0;
	uint16_t cvNr = 0;
	uint8_t cvVal = 0;
	uint8_t execStat = 0;
} progCmd;

typedef struct
{
	char pageName[50];
	AsyncWebSocketClient * wsClient;
} wsClientInfo;

bool parseDCCExNew(char* inpStr, lnTransmitMsg* inpStatus, std::vector<ppElement> * paramList);
int  parseDCCExParamNew(lnTransmitMsg* thisEntry, std::vector<ppElement> * paramList);

typedef uint16_t (*txFct) (lnTransmitMsg);
typedef void (*cbFct) (lnReceiveBuffer *);
typedef void (*mqttFct) (char*, byte*, unsigned int);
typedef void (*mqttTxFct) (byte, char*, char*); //mode (0:send, 1:subscribe), topic, payload
typedef void (*dccCbFct) (std::vector<ppElement>*);

void setXORByte(uint8_t * msgData);
bool getXORCheck(uint8_t * msgData, uint8_t targetLen = 0);
int8_t getWSClient(int8_t withID);
int8_t getWSClientByPage(uint8_t startFrom, char * toWebPage);
void untokstr(char* strList[], uint8_t listLen, char* inpStr, const char* token); 
bool isSameMsg(lnReceiveBuffer* msgA, lnReceiveBuffer* msgB);

	void dispMsg(uint8_t * msgData, uint8_t targetLen = 0);
	void dispSlot(uint8_t * slotBytes);
	bool verifySyntax(uint8_t * msgData);

class rmsBuffer
{
	public:
		rmsBuffer(uint8_t bufSize);
		void addVal(uint16_t newVal);
		float_t getRMSVal();
		void clrBuffer();
	private:
		float_t* rmsData = NULL;
		uint8_t bufferSize;
		int8_t wrIndex = -1;
};

#endif
