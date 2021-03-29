
void initLEDChain()
{
  chainLength = eeprom_read_word ((uint16_t *) 0);
  if (chainLength > LED_COUNT)
    chainLength = LED_COUNT;
  uint16_t chainType = eeprom_read_word ((uint16_t *) 2);
  switch (chainType)
  {
    case 12: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_RGB + NEO_KHZ800); break;
    default: strip = new Adafruit_NeoPixel(chainLength, LED_PIN, NEO_GRB + NEO_KHZ800); break;
  }
  strip->begin();
  strip->setBrightness(100); // set full brightness
  strip->show(); // Initialize all pixels to 'off'
  fillStrip(0x000010); //initialize dark
  strip->show(); // Initialize all pixels to 'off'
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
