#include "SVMgr.h"

  svBuffer::svBuffer()
  {
    
  }
  
  svBuffer::~svBuffer()
  {
    
  }

  void svBuffer::setMemLookup(memLookup* newFct)
  {
    lookupFct = newFct;
  }

  uint8_t svBuffer::getMemoryPos(uint16_t memLoc)
  {
    
  }
  
  void svBuffer::setMemoryPos(uint16_t memLoc, uint8_t memVal)
  {
    
  }
  
  void svBuffer::addRequest(uint16_t memLoc) //lsb of HighByte has flag for 4 byte command
  {
    uint8_t wrPtr = (rwPtr & 0x0F) >> 2;
    svReqLoc[wrPtr] = memLoc & 0x00FF;
    if (memLoc & 0x0100)
      rwPtr |= (0x10 << wrPtr);
    else
      rwPtr &= (~(0x10 << wrPtr));
    wrPtr++;
    rwPtr = (rwPtr & 0xF3) + ((wrPtr % bufSize) << 2);
  }
  
  bool svBuffer::hasRequest()
  {
    return (rwPtr & 0x03) != ((rwPtr & 0x0F) >> 2);
  }
  
  uint16_t svBuffer::getRequest()
  {
    if (hasRequest())
    {
      uint8_t wrPtr = rwPtr & 0x03;
      rwPtr = (rwPtr & 0xFC) + ((wrPtr + 1) % bufSize);
      return svReqLoc[wrPtr] + (((rwPtr >> wrPtr) & 0x10)<<4);
    }
  }
  
//  private:
//  uint8_t rwPtr = 0;
//  uint8_t svReqLoc[3];
