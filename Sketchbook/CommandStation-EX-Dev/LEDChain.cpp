#include "LEDChain.h"

CRGB ledChain[LED_COUNT]; 

LEDChain::LEDChain()
{
  
}

void LEDChain::begin()
{
  pinMode(LED_PIN, OUTPUT);
  FastLED.setBrightness(128);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(ledChain, LED_COUNT);
}

void LEDChain::setLED(uint8_t ledNr, CHSV ledColor)
{
 if (ledNr < LED_COUNT)
   ledChain[ledNr] = ledColor; 
}

void LEDChain::setLEDBrightness(uint8_t percentLevel)
{
  uint8_t newLevel = round(255 * percentLevel / 100);
  FastLED.setBrightness(newLevel);
  FastLED.show();
}
