void initLEDChain()
{
  chainLength = eeprom_read_word ((uint16_t *) 0);
  if (chainLength > LED_COUNT)
    chainLength = LED_COUNT;
  uint16_t chainType = eeprom_read_word ((uint16_t *) 2);
  switch (chainType)
  {
    case 102: FastLED.addLeds<WS2811, LED_PIN, GRB>(ledChain, chainLength); break;
    default: FastLED.addLeds<WS2811, LED_PIN, RGB>(ledChain, chainLength); break;
  }
  fillStrip(CHSV(0,0,0)); //initialize dark
}

/*
  RGB=0012,
  RBG=0021,
  GRB=0102,
  GBR=0120,
  BRG=0201,
  BGR=0210
*/

void fillStrip(CHSV newCol)
{
  for (int i = 0; i < chainLength; i++)
    setCurrColHSV(i, newCol); //initialize dark
  FastLED.show();
}

void setCurrColHSV(uint16_t ledNr, CHSV newCol)
{
//  Serial.printf("Set HSV %i to %i\n", ledNr, newCol);
  if ((ledNr >= 0) && (ledNr < chainLength))
      ledChain[ledNr] = newCol;
}
