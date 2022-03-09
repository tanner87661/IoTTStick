#include "LedChain.h"

CRGB ledChain[LED_COUNT]; 


void LEDChain::begin()
{
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(ledChain, LED_COUNT);
}
