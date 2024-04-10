#ifndef BoosterNode_h
#define BoosterNode_h
#include "Arduino.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "SVMgr.h"

#define numSamples 600
#define resSamples 800
#define scTestSamples 1
#define smplBufSize 3 //size of sample buffer for current reading
#define readIntv 1000
#define olfThreshold 120 //%
#define fuseTripPoint 1.35
#define olfRestoreThreshold 60 //%
#define limWaitTime 500 //wait time in case of limited reset
#define limRetry 5 //# of retrys in limited reset mode

#define quadByteResponse   "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i,%i,%i,%i]}}\n"
#define singleByteResponse "{\"Cmd\":\"SVR\",\"Prm\":{\"Src\":%i,\"Opc\":%i,\"Addr\":%i,\"Vals\":[%i]}}\n"
#define DCCAmpResponse "{\"Cmd\":\"DCCAmp\",\"I\":[%i,%i,%i,%i,%i,%i,%i,%i]}\n"
#define LNResponse "{\"Cmd\":\"LN\",\"OPC\":%i,\"Addr\":%i, \"Stat\":%i}\n"

enum extNodeStatus : uint8_t {extStop=0, extRun=1, extScDetect=2}; //short circuit detect
enum extSignalStatus : uint8_t {noSig=0, dccSig=1, pwmSig=2}; //external signal status
enum intNodeStatus : uint8_t {allok=0, scAR0=1, scAR1= 2, scOff=3, olOff=4, olCool=5, awReset=6}; //short circuit detect
enum resetModes : uint8_t {manual=0, limeted=1, fullauto=2}; //reset modes

typedef struct
{
  uint8_t trigDef; //high nibble: type low nibble trigger event
  uint16_t devAddr;  
} lnActivatorDef;

typedef struct
{
//  uint16_t lnAddress;
  uint8_t ctrlPin;
  uint8_t sensePin;
  uint8_t reversePin;
  
  float senseFactor; //Current Factor: 8500 A/A, Rsense = 5kOhm Umax at 8.5 Amps = 5V = Analog Value 1023 Sense Factor = 8500 / 1023 = 8.31
  //make it uint16_t * 1000
  uint16_t currNominal; //[mA] set trip current, must be smaller than achievable short circuit current. 80% of this is nominal current (infinite RMS load) if higher, warmup is calculated
  uint16_t scCurr; //short circuit current in mA
  uint8_t fuseMode; //10,20,30,..,100
  resetModes autoResetMode; //0: manual only 1: limited 2: full autoreset
  bool autoReverseMode;
  lnActivatorDef actSetOn; //type-nibble addr-word state-nibble  byte 0: type {input, swreq, swack, btn  byte 1,2: 16bit addr byte 3:trig state 0,1,2,3,4,5 
  lnActivatorDef actSetOff; //type-nibble addr-word state-nibble
  lnActivatorDef actResetNode; //type-nibble addr-word state-nibble
  lnActivatorDef actARPolarityPos; //type-nibble addr-word state-nibble
  lnActivatorDef actARPolarityNeg; //type-nibble addr-word state-nibble
  lnActivatorDef repShortCircuit; //type-nibble addr-word state-nibble
  lnActivatorDef repOverload; //type-nibble addr-word state-nibble
  uint8_t checkByte;
} nodeConfigData;

typedef struct
{
  float currData[smplBufSize] = {0,0,0}; //adders for squared current reading values
  uint16_t currCtr = 0; //counter for number of samples in currData
  uint16_t currTemp = 0; //centigrade
  uint16_t currRMS = 0;
  uint16_t currOLF = 0; //0..120%
  uint8_t rdPtr = 0;
  uint8_t wrPtr = 0;
  uint32_t lastCurrRead; //last time current values were read

  //calculate in initNode function
  uint16_t nominalTemp; //in centigrade, based on fuseMode, temp at nominal current. tripTemp is 110% of that
  uint16_t tripCurrVal = 0; //120% of nominal current
  uint8_t scTrigVal = 2; //short circuit trigger value for scCtr
  
  uint16_t scCtr; //short circuit counter
  uint16_t scVal; //short circuit current reading value
  uint32_t scTimer; //start time when shutdown occurred
  uint16_t scWaitTime = 2000;

  intNodeStatus intStat = allok; 
  extNodeStatus extStat = extStop;
  extSignalStatus sigStat = noSig;
} bOpsCtrlData;

class Booster {
  public:
  Booster();
  ~Booster();
  void writeEEPROM(bool writeEE);
  nodeConfigData readEEPROM();
  void initNode(uint8_t nNr, uint16_t lnAddr); //, nodeConfigData* initData);
  void setLNAddr(uint16_t lnAddr);

  //config commands
  void setSensorFactor(double newSensFact);
  void setNominalCurrent(uint16_t newCurrent);
  void setFuseValue(uint8_t newFuseMode);
  void setResetMode(uint8_t newResetMode);
  void setARMode(bool newARMode);
  void setSignalStatus(extSignalStatus newStatus);

  //runtime commands
  void setExtStatus(extNodeStatus newStatus);
  void setARPolarity(uint8_t newPolarity); //0: forward 1: reverse; 0xFF: toggle

  void resetBooster(); //conditional call restartNode 
  void restartNode();  //reset status machine, set output active
  
  void requestSVData(uint8_t svCmd, int16_t memLoc); //set report flags
  void writeSVData(uint8_t svCmd, int16_t memLoc, uint8_t svData[]); //write SV data
//  void requestBoosterReport(uint16_t reportType); //set report flags
  void startSCTest();
  
  bool processLoop();
  bool processReportRequest();

  void processTimerInterrupt(); //main interrupt service
  void processTimerOps(); //called from ISR
  void processTimerSCDetect(); //called from ISR

  void processExtCommand(lnActivatorDef newCmd);

  void sendLNMsg(uint8_t opCode, uint16_t lnAddr, uint8_t lnStatus);
  void sendIntStatLN();
  void sendOLFStatLN();
  
  void updateRMS_T(uint8_t currBuffer); //update current and temp data, called from processLoop

  uint8_t getOutputStatus(); //return status of ctrl pin

  public:  
  nodeConfigData bCfg;
  bOpsCtrlData bOps;
  uint8_t nodeNr = 0;
  private:
  uint16_t reportFlags;
  svBuffer reportQueue;
  uint16_t lnAddress;
  intNodeStatus intStatTx = 0;
  uint8_t olfTx = 0;
//  uint8_t sendCfgMask = 0; replaced by reportFlags
  void tripLine(bool newStat); //set ctrl pin, if low, start timer for short circuit
  void updateNominalTemp();
  void configPins();
  void releasePins();
};

//extern void getBoosterOpsReport(Booster* thisBooster);
//extern void getBoosterHwCfgReport(Booster* thisBooster);
//extern void getBoosterElCfgReport(Booster* thisBooster);

#endif
