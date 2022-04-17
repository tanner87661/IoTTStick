#ifndef BoardManager_h
#define BoardManager_h
#include <Arduino.h>
#include "config.h"
#include "DCC.h"
#include "EEStore.h"
#include "DIAG.h"
#include "LEDChain.h"
#include "Sensors.h"

#define pinCurrentSensorRailSync A0
#define pinCurrentSensorIBT2 A1
//#define PortBank1 A2 Defined in Sensors.h
#define pinRawPwrSupply A3
#define pinLN_A A4
#define pinCurrentSensorProg A5
#define pinLN_B A6
//#define PortBank2 A7

#define pinProgTrackActive 6
#define pinIBT2Enable 7
#define pinSetSupplyPwr 8
#define pinS1 9
#define pinS2 10
#define pinS3 11
#define pinCmdStationMode 12
#define pinLowPwrDCC 13

#define numSamples 500 //this is about 3 sinus cycles of 60Hz AC, just in case
#define minVoltage 250 //450 //12V  (1024 / 3.3) * (16/11)
#define maxVoltage 650 //735 //26V  (1024 / 3.3) * (26/11)
#define diffVoltage 10 //28  //1V (1024 / 3.3 * (1 / 11)
#define pwrCheckInterval 500 //check power status every 500 ms

class BoardManager
{ 
  public:
    BoardManager();
    void intializeBoard();
    void setDeviceMode(uint8_t newMode); //0: Off 1: Cmd Stn 2: Booster with LN 3: Booster no LN 65535: Reboot 328P
    void setDeviceOutput(uint8_t flagNr, uint8_t flagStatus);
    void setOutputCurrent(uint8_t outMod, uint16_t newCurrent);
    bool verifyPowerSignal(uint16_t numChecks = numSamples);
    void processLoop();
    
    uint8_t getDeviceMode();

    void setProgTrack(bool setOn);
    Sensor * getSensor();
    void checkAllSensors(Print *stream);
    void printAllSensors(Print *stream);
    LEDChain * getLEDChain();
    void setLED(uint8_t ledNr, CHSV ledColor);
    void setLEDBrightness(uint8_t percentLevel);
    void setLEDDispStatus();
    uint16_t getExtPwrStatus();

  private:
    uint8_t  deviceConfig = 0; //config byte to set all operations modes 0 : Cmd Station 1: Booster with LocoNet support 2: Booster without LocoNet
                               //Bit 0: use DCC Generator
                               //Bit 1: use Input Ports
    uint16_t powerStatus = 0xFFFF;
    uint16_t minVal = 0xFFFF; //used for power analysis
    uint16_t maxVal = 0x0000;
    uint32_t lastPwrCheck = millis();
    LEDChain * RHCtrlLEDs = NULL;
    Sensor * RHSensorBlock = NULL;

  private:
    void initPins();
    void setIBT2Enable(bool setEnable);
    void setProgOutputEnable(bool setEnable);
    void setRailSyncEnable(bool setEnable);
    void setDCCSource(bool setInternal);
    void setPowerRelay(bool newStat);
    void setBoardModeOff();
    void setBoardModeCmdStn();
    void setBoardModeBooster(bool useLN = true);
    void resetBoard();
    
};


#endif
