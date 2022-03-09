#ifndef LedChain_h
#define LedChain_h


#include <Arduino.h>
#include <avr/eeprom.h>
#include <FastLED.h>

#define LED_PIN    3

const uint16_t LED_COUNT = 3;

class LEDChain
{
  private:
  public:
    LEDChain();
    void begin();
    void setLED(uint8_t ledNr, CHSV ledColor);
    void setLEDBrightness(uint8_t percentLevel);
};

#endif
