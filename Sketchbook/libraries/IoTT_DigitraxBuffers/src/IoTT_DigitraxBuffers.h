//Version 1.0.0
#ifndef IoTT_DigitraxBuffers_h
#define IoTT_DigitraxBuffers_h

#include <Arduino.h>
#include <IoTT_CommDef.h>
#include <SPIFFS.h>

#define numSigs 2048
#define numSwis 512 //=2048/4
#define numBDs 512 //=4096/8
#define numAnalogVals 4096
#define numButtons 4096
#define numSlots 128

#define bufferUpdateInterval 1000

typedef void (*dccFct) (lnTransmitMsg *, uint8_t, uint8_t); //slot nr, fct depending value

typedef uint8_t blockDetBuffer[numBDs]; //4096 input bits, 8 per byte, lsb is lowest number
typedef uint8_t switchBuffer[numSwis]; //current status of switches, 4 per byte. First bit indicates coil state, second is position
typedef uint8_t signalBuffer[numSigs]; //current status of aspects, 1 per byte values 0..31, 3 MSB reserved
typedef uint16_t analogValBuffer[numAnalogVals]; //current status of analog values, 1 per word, running from 0 to 4096
typedef uint8_t buttonValBuffer[numButtons]; //current status of buttons, 2 per byte, statuses down, up, click, hold, dblclick
typedef uint8_t powerStatusBuffer;
typedef uint8_t slotData[10]; //slot data 0 is slot number, this is given by position in array, so we only need 10 bytes
typedef slotData slotDataBuffer[numSlots];

uint32_t millisElapsed(uint32_t since);
uint32_t microsElapsed(uint32_t since);


void setReplyFunction(txFct newFct);
void setDccCmdFunction(txFct newFct);
void setSpeedCmd(lnTransmitMsg * txBuffer, uint8_t slotNr, uint8_t speedVal);
void setProgrammingCmd(lnTransmitMsg * txBuffer, uint8_t progMode, uint8_t OpsAdrHi, uint8_t OpsAdrLo, uint8_t CVNrHi, uint8_t CVNrLo, uint8_t CVVal);
void setDirCmd(lnTransmitMsg * txBuffer, uint8_t slotNr, uint8_t dirMask);

//void iterateMULinks(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t templData, dccFct procFunc);
void generateSpeedCmd(lnTransmitMsg * txBuffer, uint8_t thisSlot, uint8_t topSpeed);
//void generateFunctionCmd(lnTransmitMsg * txBuffer, lnReceiveBuffer * newData);
uint8_t getTopSlot(uint8_t masterSlot);
uint8_t getFirstSlave(uint8_t masterSlot);

void prepLACKMsg(lnTransmitMsg * msgData, uint8_t ackCode, uint8_t ackData);
void prepSlotReadMsg(lnTransmitMsg * msgData, uint8_t slotNr);
void processLocoNetReply(lnReceiveBuffer * newData);
void processCmdGenerator(lnReceiveBuffer * newData);
void processLocoNetMsg(lnReceiveBuffer * newData);
uint8_t getSlotOfAddr(uint8_t locoAddrLo, uint8_t locoAddrHi);

uint8_t getBDStatus(uint16_t bdNum);

uint8_t getSwiPosition(uint16_t swiNum);
uint8_t getSwiCoilStatus(uint16_t swiNum);
uint8_t getSwiStatus(uint16_t swiNum);
uint32_t getLastSwitchActivity(uint16_t swiNum);
uint8_t getSignalAspect(uint16_t sigNum);
uint16_t getAnalogValue(uint16_t analogNum);
uint8_t getButtonValue(uint16_t buttonNum);
slotData * getSlotData(uint8_t slotNum);
uint8_t getBushbyStatus();
uint8_t getPowerStatus();
void enableBushbyWatch(bool enableBushby);
bool getBushbyWatch();

void setSlotDirfSpeed(lnReceiveBuffer * newData);
void setAnalogValue(uint16_t analogNum, uint16_t analogValue);
void setSwitchStatus(uint16_t swiNum, bool swiPos, bool coilStatus);
void setSignalAspect(uint16_t sigNum, uint8_t sigAspect);

bool saveToFile(String fileName);
void loadFromFile(String fileName);

void processBufferUpdates();

#endif

extern void handlePowerStatus() __attribute__ ((weak)); //power status change
extern void handleSwiEvent(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStat) __attribute__ ((weak));
extern void handleInputEvent(uint16_t inpAddr, uint8_t inpStatus) __attribute__ ((weak));
extern void handleSignalEvent(uint16_t sigAddr, uint8_t sigAspect) __attribute__ ((weak));
extern void handleAnalogValue(uint16_t analogAddr, uint16_t inputValue) __attribute__ ((weak));
extern void handleButtonValue(uint16_t btnAddr, uint8_t inputValue) __attribute__ ((weak));
extern void handleTranspondingEvent(uint16_t zoneAddr, uint16_t locoAddr, uint8_t eventVal) __attribute__ ((weak));
