#ifndef SVMgrNode_h
#define SVMgrNode_h

#include "Arduino.h"

#define bufSize 3

typedef uint8_t* (*memLookup) (uint16_t); 

class svBuffer
{
  public:
  svBuffer();
  ~svBuffer();

  void setMemLookup(memLookup* newFct);
  
  uint8_t getMemoryPos(uint16_t memLoc);
  void setMemoryPos(uint16_t memLoc, uint8_t memVal);

  void addRequest(uint16_t memLoc); //lsb of HighByte has flag for 4 byte command
  bool hasRequest();
  uint16_t getRequest();
  private:
  memLookup* lookupFct = NULL;
  uint8_t rwPtr = 0;
  uint8_t svReqLoc[bufSize];
};

#endif
