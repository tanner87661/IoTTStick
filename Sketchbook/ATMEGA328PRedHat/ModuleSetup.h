#ifndef ModuleSetup_h
#define ModuleSetup_h

#include <Arduino.h>

#define rawPwrSupplyPin A3

#define numSamples 500 //this is about 3 sinus cycles of 60Hz AC, just in case
#define minVoltage 450 //16V  (1024 / 3.3) * (16/11)
#define maxVoltage 735 //26V  (1024 / 3.3) * (26/11)
#define diffVoltage 28  //1V (1024 / 3.3 * (1 / 11)

class BoardManager
{
  public:
    uint8_t deviceConfig = 0; //config byte to set all operations modes 
                              //Bit 0: use DCC Generator
                              //Bit 1: use Input Ports
    
    void setDeviceMode(uint8_t newMode);
    uint8_t getDeviceMode();
    bool verifyPowerSignal();
    
  private:
    bool verifyPowerSignal();
    void setBoosterMode(bool useLoconet);
    void setCmdStationMode();
  
  
};


#endif
