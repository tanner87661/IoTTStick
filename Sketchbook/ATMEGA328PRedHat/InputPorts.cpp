#include "InputPorts.h"
#include <Arduino.h>



void InputPorts::begin()
{
  //set low nibble of PORTB for output
}

void InputPorts::selInput(uint8_t inpNr = 0)
{

}

void InputPorts::portLoop()
{
  uint8_t portMask = PORTB & 0x0F;
  if (analogRead(PortBank1) > 512)
    lastStatus |= (0x0001 << (portMask & 0x0F));
  else
    lastStatus &= !(0x0001 << (portMask & 0x0F));
  if (analogRead(PortBank2) > 512)
    lastStatus |= (0x0100 << (portMask & 0x0F));
  else
    lastStatus &= !(0x0100 << (portMask & 0x0F));
  portMask = (portMask + 1) & 0x0F;
  PORTB = (PORTB & 0xF0) | portMask;
}
