// Compiler settings
// ATTiny85
// Internal 16MHz

//#define useFastLED  //does noit work currently on ATTiny :-(

  #include <Arduino.h>
  #include <TinyWire.h> 

  #define readProc receive
  #define writeProc send
  #define i2cConnection TinyWire
  
//  #define clockFreq 400000

  #define devType 0x56 //GreenHat, used to request data

#define BRIGHTNESS 255

/*
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
*/

/************************************************************************************************************************************
 * Firmware for GreenHat ATMEGA ATTiny85
 * Compile for ATTiny
 * Download to GreenHat either
 * - Arduino as ISP (no bootloader)
 * 
 * See http://myiott.org for more information
 * 
 */
#if defined(useFastLED)
do not use
//  #include <FastLED.h>
#else
  #include <Adafruit_NeoPixel.h>    //needed for the WS2812
#endif

//#include <avr/pgmspace.h>         //needed for PROGMEM
//#include <avr/eeprom.h>
#include <EEPROM.h>

// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (16)
#endif

//SDA and SCL for I2C are on pins A4 and A5 as set in the library
//Pin 1 is the onboard LED on the standard ATTiny85 board. Can be reused in our own design
#define LED_PIN    1
//#define debugPin   LED_BUILTIN
//#define muxAddrReg PORTB
#define I2C_SLAVE_ADDRESS 0x30 // 2 lsb bits will be taken from PB3/PB4

// How many NeoPixels are attached to the Arduino?
const uint16_t LED_COUNT = 33;

#ifdef useFastLED
do not use
//  #include <FastLED.h>
#else
  Adafruit_NeoPixel * strip; // = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

//CRGB ledChain[LED_COUNT]; // = NULL;
uint16_t chainLength = LED_COUNT;
uint32_t wdtTimer = 0;
uint8_t i2cAddr = 0;
uint8_t memPtr = 0; //memory location for next byte request
bool startMode = true;

#ifdef useFastLED
do not use
//  CRGB ledChain[LED_COUNT];
//  CRGB lastCol;
#else
  uint32_t lastCol;
#endif

#define wdtInterval 500

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
  if (eeprom_read_word ((uint16_t *) 0) == 0xFFFF) //newly programmed chip, initialize with basic setup
  {
    eeprom_update_word((uint16_t *) 0, LED_COUNT); //chain length
    eeprom_update_word((uint16_t *) 2, 0102); //chain type
  }
  initLEDChain();
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  initI2C();
  wdtTimer = 0;
}

//uint16_t oldHue = 0;
void loop()
{
/*
  uint32_t rgbcolor = strip->ColorHSV(oldHue, 255, 50);
  oldHue = oldHue + 1;
  strip->setPixelColor(0, rgbcolor);
  fillStrip(rgbcolor);
  strip->show()
delay(10);
*/


  if (startMode)
  {
    wdtTimer++;
    if ((wdtTimer > 2500) && startMode) 
      resetFunc(); //if communication with Hat breaks down, we reset the Arduino to rearbitrate the I2C bus
  }
  else
  {
    if (millis() > wdtTimer)
      resetFunc(); //if communication with Hat breaks down, we reset the Arduino to rearbitrate the I2C bus
  }
  if (i2cAddr != getI2CAddr())
    resetFunc(); //if communication with Hat breaks down, we reset the Arduino to rearbitrate the I2C bus

}
