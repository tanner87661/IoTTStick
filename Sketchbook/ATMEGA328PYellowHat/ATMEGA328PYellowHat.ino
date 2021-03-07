  #include <Arduino.h>
//  #include <Wire.h>
  #include <TinyWireS.h>

  #define readProc receive
  #define writeProc send
  #define i2cConnection TinyWireS
  
  #define clockFreq 400000

  #define devType 0x55 //yellow hat, used to request data

/*
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
*/

/************************************************************************************************************************************
 * Firmware for YellowHat ATMEGA 328P
 * Compile for Arduino Uno or Arduino Nano
 * Download to YellowHat either
 * - using ICSP Programmer (using installed Bootloader) or
 * - Arduino as ISP (no bootloader)
 * 
 * See http://myiott.org for more information
 * 
 */


#include <Adafruit_NeoPixel.h>    //needed for the WS2812
#include <avr/pgmspace.h>         //needed for PROGMEM
#include <avr/eeprom.h>

// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (16)
#endif

//SDA and SCL for I2C are on pins A4 and A5 as set in the library
//Pin 1 is the onboard LED on the standard ATTiny85 board. Can be reused in our own design
#define LED_PIN    3
#define debugPin   LED_BUILTIN
#define muxAddrReg PORTB
#define I2C_SLAVE_ADDRESS 0x18 // the 7-bit address (remember to change this when adapting this example)

// How many NeoPixels are attached to the Arduino?
const uint16_t LED_COUNT = 25;

Adafruit_NeoPixel * strip; // = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//CRGB ledChain[LED_COUNT]; // = NULL;
uint16_t chainLength = LED_COUNT;
uint8_t wdtTimer = 0;
uint32_t lastCol;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
  if (eeprom_read_word ((uint16_t *) 0) == 0) //newly programmed chip, initialize with basic setup
  {
    eeprom_update_word((uint16_t *) 0, 8); //chain length
    eeprom_update_word((uint16_t *) 2, 0102); //chain type
  }
  initLEDChain();
//  fillStrip(CHSV(0,0,0)); //initialize dark
//  initMUX();
  initI2C();
  wdtTimer = 0;
}

void loop()
{
  delay(100);
  wdtTimer++;
  if (wdtTimer > 50) 
    resetFunc(); //if communication with Hat breaks down, we reset the Arduino to rearbitrate the I2C bus
  TinyWireS_stop_check();
}
