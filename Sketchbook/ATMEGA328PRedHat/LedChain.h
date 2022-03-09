#ifndef LedChain_h
#define LedChain_h


#include <Arduino.h>
#include <avr/eeprom.h>
#include <FastLED.h>

#define LED_PIN    3

const uint16_t LED_COUNT = 35;

class LEDChain
{
  private:
  public:
    void begin();
};

#endif
