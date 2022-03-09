#ifndef InputPorts_h
#define InputPorts_h

#include <Arduino.h>

#define PortBank1 A2
#define PortBank2 A7

class InputPorts {
  public:
    void begin();
    void portLoop();
    void selInput(uint8_t inpNr = 0);
    
  private:
  uint32_t lastStatus = 0;
  uint8_t  addrSel = 0;
};


#endif
