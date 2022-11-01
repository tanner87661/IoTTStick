#include <Arduino.h>
#include <avr/io.h>
#include <TinyWire.h> 
#include <avr/eeprom.h>

#define i2cConnection TinyWire
#define deviceAddr 0xC0
#define sigPin A2
#define ledPin 1
#define pwmPin 0

int16_t analogVals;
uint8_t wrPtr = 0;

/*Pin Usage
B0: SDA
B1: LED Out / Timing verification 
B2: SCL
B3: Analog A3
B4: DCC
B5: Reset
*/

uint8_t i2cAddr = 0;
bool regIO = false;
volatile uint16_t sampleSum[2] = {0,0};
volatile uint8_t sampleCtr = 0;

void setup() {

//  OSCCAL = 250; // Overclocks the MCU to around 30 MHz, set lower if this causes instability, raise if you can/want
  initI2C();
  verifyI2CConfig();
  pinMode(pwmPin, OUTPUT);
  adc_setup();
}

void loop() {
  PORTB ^=(1<<PB1); //LED toggle
  int16_t dispRes = analogVals - 70;
  sendValue(dispRes);
}
