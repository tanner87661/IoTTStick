
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
  }
  strip->begin();
  strip->setBrightness(10); // set full brightness
//  strip->show(); // Initialize all pixels to 'off'
  fillStrip(0x000010); //initialize dark
//  strip->show(); // Initialize all pixels to 'off'
}

/*
  RGB=0012,
  RBG=0021,
  GRB=0102,
  GBR=0120,
  BRG=0201,
  BGR=0210
*/

void fillStrip(uint32_t newCol)
{
  strip->fill(newCol);
  strip->show(); 
}
