uint8_t getI2CAddr()
{
  return (I2C_SLAVE_ADDRESS & 0x7C) + digitalRead(3) + (2 * digitalRead(4)); 
}

void initI2C()
{
  i2cAddr = getI2CAddr();
  i2cConnection.begin(i2cAddr);
  i2cConnection.onReceive(receiveEvent);
  i2cConnection.onRequest(requestEvent);
}

//void requestEvent(int bytes) //for Nano
void requestEvent() //for ATTiny
{  
  uint8_t thisData = 0;
  wdtTimer = millis() + wdtInterval;
  switch (memPtr)
  {
    case 0:
      i2cConnection.writeProc(devType); 
      break;
    case 1:
      thisData = eeprom_read_byte ((uint8_t) 1);
      i2cConnection.writeProc(thisData);
      break;
    case 2:
      thisData = eeprom_read_byte ((uint8_t) 0);
      i2cConnection.writeProc(thisData);
      break;
    case 3:
      thisData = eeprom_read_byte ((uint8_t) 3);
      i2cConnection.writeProc(thisData);
      break;
    case 4:
      thisData = eeprom_read_byte ((uint8_t) 2);
      i2cConnection.writeProc(thisData);
      break;
  }
  memPtr += 1;
}

/*
 * Command structure for data sent to the chain
 * 1 byte commands > 0x3F
 * 0xFF : call show() (usually sent after updating several pixels
 * 
 * standard LED commands are 5 bytes long
 * 0,1 : # of LED in Chain 0xFFFF is broad cast command
 * 2 : Hue
 * 3 : Sat
 * 4 : Val
 * 
 * Ctrl commands are 4 bytes long
 * if LED# = 254, this is not LED update, but a command, specified in the second byte
 * pos 1 val 0: pos 2/3 determine LED chain length, store in EEPROM pos 0
 * pos 1 val 1: pos 2/3 determine LED chain pixel type, store in EEPROM pos 1
 * 
 * Command structure for setting MUX IO lines on Port B
 * 1 byte commands < 0x3F
 * Low nibble: selected ouput line 0-15
 * Bit 4/5 select MUX. Only one of these can be active (low) at a time. Bit 4/5 both high disable all input
 */

/*
void toggle()
{
    if (digitalRead(debugPin) == 0)
      digitalWrite(debugPin, 1);
    else
      digitalWrite(debugPin, 0);
}
*/

void receiveEvent(uint8_t howMany)
{
//  uint32_t rgbcolor = strip->ColorHSV(oldHue, 255, 50);
//  strip->setPixelColor(9, rgbcolor);

  uint16_t thisData = 0;
  byte thisCommand = 0;
  word thisParam = 0;
  uint16_t hue = 0;
  uint8_t sat = 0;
  uint8_t lval = 0;
  if (startMode)
    wdtTimer = 0;
  else
    wdtTimer = millis() + wdtInterval;
  switch (howMany)
  {
    case 1:
      thisData = i2cConnection.readProc();
      if (thisData > 0x30)
        switch (thisData)
        {
          case 0xFF: showChain(); break;
          default: return; break;
        }
      else
        memPtr = 0;
        startMode = false;
        wdtTimer = millis() + wdtInterval;
      break;
    case 2: //this is just the pixel number, we fill it with the color from the last 5bit command
      thisData = (i2cConnection.readProc() << 8) + i2cConnection.readProc();
      setSinglePixel(thisData, lastCol);
      break;
    case 4:
      thisData = i2cConnection.readProc();
      switch (thisData & 0x00FF)
      {
        case 0x00FE: //command to update data
          thisCommand = i2cConnection.readProc();
          thisParam = (i2cConnection.readProc()<<8) + i2cConnection.readProc();
          switch (thisCommand)
          {
            case 0: 
              if ((eeprom_read_word ((uint16_t *) 0) != thisParam) && (thisParam <= LED_COUNT))
              {
                eeprom_update_word((uint16_t *) 0, thisParam); //chain length
                fillStrip(0); //old length
              }
              break;
            case 1: 
              if (eeprom_read_word ((uint16_t *) 2) != thisParam)
              {
                eeprom_update_word((uint16_t *) 2, thisParam); //pixel type
//                fillStrip(0xFFFF00);
              }
              break;
            case 0xFF: 
                resetFunc();  
              break;
          }
          break; //case 0xFE
      }
      break; //case 4
    case 5:
      thisData = (i2cConnection.readProc() << 8) + i2cConnection.readProc();
      switch (thisData)
      {
        case 0xFFFF: //broadcast
        {
          hue = i2cConnection.readProc()<<8;
          sat = i2cConnection.readProc();
          lval = i2cConnection.readProc();
          lastCol = getColorHSV(hue, sat, lval);
          fillStrip(lastCol);
        }
        break;
        default: //update pixel
        {
          hue = i2cConnection.readProc()<<8;
          sat = i2cConnection.readProc();
          lval = i2cConnection.readProc();
          lastCol = getColorHSV(hue, sat, lval);
//          lastCol = getColorHSV(85, 255, 127);
          setSinglePixel(thisData, lastCol);
        }
        break;
      }
      break;
  }
}
