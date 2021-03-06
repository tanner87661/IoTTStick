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

//this type is used to send a new message to Loconet
typedef struct {
    uint8_t    lnMsgSize = 0;
    uint8_t    lnData[lnMaxMsgSize];
    uint16_t   reqID = 0; //temporarily store reqID while waiting for message to get to head of buffer
    uint32_t   reqRecTime = 0;
} lnTransmitMsg;

//The call back function of the library provides a pointer to this structure with the incoming message and status information 
typedef struct {  
    uint8_t    lnMsgSize = 0;
    uint8_t    lnData[lnMaxMsgSize];
    uint16_t   reqID = 0; //contains reqID if echo or response to previous request
    uint32_t   reqRecTime = 0;
    uint32_t   echoTime = 0; //time in microsecs between request and echo (same) message
    uint32_t   reqRespTime = 0; //time in microsecs between request and response message
    uint8_t	   errorFlags = 0;	
} lnReceiveBuffer;    

typedef void (*cbFct) (lnReceiveBuffer *);

#endif
