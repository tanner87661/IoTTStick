#ifdef useFastLED
do not use
/*
void initLEDChain()
{
  chainLength = eeprom_read_word ((uint16_t *) 0);
  if (chainLength > LED_COUNT)
    chainLength = LED_COUNT;
  uint16_t chainType = eeprom_read_word ((uint16_t *) 2);
  switch (chainType)
  {
    case 12: FastLED.addLeds<WS2811, LED_PIN, RGB>(ledChain, chainLength);  break;
    case 21: FastLED.addLeds<WS2811, LED_PIN, RBG>(ledChain, chainLength);  break;
    case 102: FastLED.addLeds<WS2811, LED_PIN, GRB>(ledChain, chainLength);  break;
    case 120: FastLED.addLeds<WS2811, LED_PIN, GBR>(ledChain, chainLength);  break;
    case 201: FastLED.addLeds<WS2811, LED_PIN, BRG>(ledChain, chainLength);  break;
    case 210: FastLED.addLeds<WS2811, LED_PIN, BGR>(ledChain, chainLength);  break;
  }
  FastLED.setBrightness(255); // set full brightness
  FastLED.show(); // Initialize all pixels to 'off'
}
*/
#else

void initLEDChain()
{
  chainLength = eeprom_read_word ((uint16_t *) 0);
  if (chainLength > LED_COUNT)
    chainLength = LED_COUNT;
  uint16_t chainType = eeprom_read_word ((uint16_t *) 2);

  switch (chainType)
  {
    case 12: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_RGB + NEO_KHZ800); break;
    case 21: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_RBG + NEO_KHZ800); break;
    case 102: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_GRB + NEO_KHZ800); break;
    case 120: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_GBR + NEO_KHZ800); break;
    case 201: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_BRG + NEO_KHZ800); break;
    case 210: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_BGR + NEO_KHZ800); break;
//    default: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_GRB + NEO_KHZ800); break;
  }

  strip->begin();
//<<<<<<< Prerelease
  strip->clear();
  strip->setBrightness(255); // set full brightness
//  strip->show(); // Initialize all pixels to 'off'
//=======
  strip->setBrightness(255); // set full brightness
  strip->show(); // Initialize all pixels to 'off'
//  fillStrip(0x000000); //initialize dark
//  strip->show(); // Initialize all pixels to 'off'
}
#endif
/*
  RGB=0012,
  RBG=0021,
  GRB=0102,
  GBR=0120,
  BRG=0201,
  BGR=0210
*/

#ifdef useFastLED
do not use
/*
CRGB getColorHSV(uint8_t hue, uint8_t sat, uint8_t lval)
{
  return CHSV(hue, sat, lval);
}

void fillStrip(uint32_t newCol)
{
//  strip->fill(newCol);
  FastLED.show(); 
}

void setSinglePixel(uint16_t ledNr, CRGB thisCol)
{
  ledChain[ledNr] = thisCol;
}

void showChain()
{
  
}
*/
#else

uint32_t getColorHSV(uint16_t hue, uint8_t sat, uint8_t lval)
{
  return strip->ColorHSV(hue, sat, lval);  
}

void fillStrip(uint32_t newCol)
{
  strip->fill(newCol);
  strip->show(); 
}

void setSinglePixel(uint16_t ledNr, uint32_t thisCol)
{
  strip->setPixelColor(ledNr, thisCol);
}

void showChain()
{
  strip->show(); 
}

#endif
