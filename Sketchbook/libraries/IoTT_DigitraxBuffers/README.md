# Esp32 LocoNet Digitrax Buffers

Implementation of a databuffer to keep track of control bus data elements like block detector statuses etc.

to include in the application use
#include <IoTT_DigitraxBuffers.h>


In your LocoNet callback function (called from LocoNet, MQTT or Gateway), call the buffer update:

void callbackLocoNetMessage(lnReceiveBuffer * newData)
{
  processLocoNetMsg(newData);
  //do otehr stuff
}

This function will update following buffers depending on the received LocoNet message:

typedef  uint8_t blockDetBuffer[numBDs]; //4096 input bits, 8 per yte, lsb is lowest number
typedef uint8_t switchBuffer[numSwis]; //current status of switches, 4 per byte. First bit indicates correct state, second is position
typedef uint8_t signalBuffer[numSigs]; //current status of aspects, 1 per byte values 0..31, 3 MSB reserved
typedef uint16_t analogValBuffer[numAnalogVals]; //current status of analog values, 1 per word, running from 0 to 4096
typedef uint8_t buttonValBuffer[numButtons]; //current status of buttons, 2 per byte, statuses down, up, click, hold, dblclick
typedef uint8_t powerStatusBuffer;
typedef uint8_t slotData[10]; //slot data 0 is slot number, this is given by position in array, so we only need 10 bytes
typedef slotData slotDataBuffer[numSlots];

After updating, it will call the respective event callback function, which can be implemented in the application:

extern void handlePowerStatus() __attribute__ ((weak)); //power status change
extern void handleSwiEvent(uint16_t swiAddr, uint8_t swiPos, uint8_t coilStat) __attribute__ ((weak));
extern void handleInputEvent(uint16_t inpAddr, uint8_t inpStatus) __attribute__ ((weak));
extern void handleSignalEvent(uint16_t sigAddr, uint8_t sigAspect) __attribute__ ((weak));
extern void handleAnalogValue(uint16_t analogAddr, uint16_t inputValue) __attribute__ ((weak));
extern void handleButtonValue(uint16_t btnAddr, uint8_t inputValue) __attribute__ ((weak));

Alternatively, information can be queried using folling functions:

uint8_t getBDStatus(uint16_t bdNum);
uint8_t getSwiPosition(uint16_t swiNum);
uint8_t getSwiCoilStatus(uint16_t swiNum);
uint8_t getSwiStatus(uint16_t swiNum);
uint8_t getSignalAspect(uint16_t sigNum);
uint16_t getAnalogValue(uint16_t analogNum);
uint8_t getButtonValue(uint16_t buttonNum);
slotData * getSlotData(uint8_t slotNum);
uint8_t getBushbyStatus();
uint8_t getPowerStatus();

For initialization, analog values can be set from the application:
void setAnalogValue(uint16_t analogNum, uint16_t analogValue);

The buffer can also request updates from LocoNet for information that can be queried, e.g. slot information, by calling

void processLocoNetMsg(lnReceiveBuffer * newData);

from the program main loop. However, to make it work, a transmit function must be specified. To set the transmit function, use
void setTxFunction(txFct newFct);
and pass a pointer function with the following format:
uint16_t sendMsg(lnTransmitMsg txData)

Example call from setup():
        setTxFunction(&sendMsg);




