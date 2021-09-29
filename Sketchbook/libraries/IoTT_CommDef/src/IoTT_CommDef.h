#ifndef IoTT_CommDef_h
#define IoTT_CommDef_h

#include <arduino.h>
#include <inttypes.h>

//Error Flags
#define errorCollision    0x01
#define errorFrame        0x02
#define errorTimeout      0x04
#define errorCarrierLoss  0x08

#define msgEcho		  	  0x10
#define msgIncomplete	  0x20		
#define msgXORCheck       0x40
#define msgStrayData	  0x80	


#define lnMaxMsgSize 48 //this is the maximum length of LocoNet Messages as defined by the standard. Do not change.

enum messageType : uint8_t {LocoNet=0, OpenLCB=1, DCCEx=2};
enum nodeType : uint8_t {standardMode = 0, limitedMaster = 1, fullMaster = 2};

//this type is used to send a new message to Loconet
typedef struct {
	messageType msgType = LocoNet;
    uint8_t    lnMsgSize = 0;
    uint8_t    lnData[lnMaxMsgSize];
    uint16_t   reqID = 0; //temporarily store reqID while waiting for message to get to head of buffer
    uint32_t   reqRecTime = 0;
} lnTransmitMsg;

//The call back function of the library provides a pointer to this structure with the incoming message and status information 
typedef struct {  
	messageType msgType = LocoNet;
    uint8_t    lnMsgSize = 0;
    uint8_t    lnData[lnMaxMsgSize];
    uint16_t   reqID = 0; //contains reqID if echo or response to previous request
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

typedef uint16_t (*txFct) (lnTransmitMsg);
typedef void (*cbFct) (lnReceiveBuffer *);
typedef void (*mqttFct) (char*, byte*, unsigned int);
typedef void (*mqttTxFct) (byte, char*, char*); //mode (0:send, 1:subscribe), topic, payload

void setXORByte(uint8_t * msgData);
bool getXORCheck(uint8_t * msgData, uint8_t targetLen = 0);

#endif
