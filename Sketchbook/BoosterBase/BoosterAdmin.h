#ifndef BoosterAdmin_h
#define BoosterAdmin_h
#include "BoosterNode.h"
#include "Arduino.h"
#include <ArduinoJson.h>
#include "SVMgr.h"

#define manufID 25
#define devID 25
#define prodID 1
#define serialNr 99

#define reportInterval 20

#define qSamples 40  //# of samples to determine DCC signal quality

#define maxDefs 6
#define DCCA_PIN 2
#define DCCB_PIN 3

typedef struct
{
  uint8_t numMods;
  uint8_t serNr;
  uint16_t lnAddress;
  bool useLN;
  uint8_t devMode; //command format hi, startup pwr mode lo
  lnActivatorDef globalOn;
  lnActivatorDef globalOff;
  uint8_t checkByte;
} boosterConfigData;

class BoosterGroup {
  public:
  BoosterGroup();
  ~BoosterGroup();
  void writeEEPROM(int8_t nodeNr, bool writeEE);
  boosterConfigData readEEPROM();

  void initBooster();
  void initNodes(); //nodeConfigData* initData, uint8_t numBoosters);

  void setSensorFactor(int8_t nodeNr, double newSensFact);
  void setNominalCurrent(int8_t nodeNr, uint16_t newCurrent);
  void setFuseValue(int8_t nodeNr, uint8_t newFuseMode);
  void setResetMode(int8_t nodeNr, uint8_t newResetMode);
  void setARMode(int8_t nodeNr, bool newARMode);
  void setSignalStatus(extSignalStatus newStatus);

  void setExtStatus(int8_t boosterNr, uint8_t newStat);
  void setARPolarity(int8_t boosterNr, uint8_t newPolarity); //0: forward 1: reverse; 2: toggle
  void resetBooster(int8_t boosterNr);

  void processLoop();
  bool processReportRequest();
  void processTimerInterrupt();

  void requestSVData(uint8_t svCmd, int16_t memLoc); //set report flags
  void writeSVData(uint8_t svCmd, int16_t memLoc, uint8_t svData[]); //write SV data

  uint16_t getLnAddress(){return bConfig.lnAddress;};
/*
  0x01: manufacturer, developer, product, serial number discovery reply
  0x02: manufacturer, developer, product, serial number ID reply
  0x04: # of modules,  LN address (default 25)
  0x08: power up mode, accepted signal type, LN mode
  0x10: LN Actuator for Device On type, state, addr
  0x20: LN Actuator for Device Off type, state, addr
*/
  void startSCTest(int8_t nodeNr);
  void processExtCommand(int8_t boosterNr, lnActivatorDef newCmd);


  private:
  uint16_t reportFlags;
  svBuffer reportQueue;
  Booster* getBooster(uint8_t ofIndex);
  void checkInputSignal();
  private:
//variables for status management
  boosterConfigData bConfig;
  uint8_t getMemoryPos(uint16_t memLoc);
  void setMemoryPos(uint16_t memLoc, uint8_t memVal);

//variables for node management
  uint8_t devIndex = 0; //pointer to the current booster in the interrupt
  uint32_t lastReportTime = millis();
  Booster bData[6];

//variables for input signal analysis and management
  uint8_t posCtrA = 0;
  uint8_t negCtrA = 0;
  uint8_t posCtrB = 0;
  uint8_t negCtrB = 0;
  uint8_t splCtr = 0; //# of samples to determine DCC signal quality
  uint8_t sigStatus = 0;
  uint8_t pwrStatus = 0;
  uint8_t resetMode = 0;
  
};

#endif
