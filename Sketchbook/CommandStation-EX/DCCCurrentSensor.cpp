#include "DCCCurrentSensor.h"


INTERRUPT_CALLBACK adc_interruptHandler = 0;

static float sampleAccu;
static uint8_t sampleCtr;
static float msVal[2];
static float intvAccu;
static uint16_t intvCtr;
static float trackVal[2];
static byte trackSel; //0: main track 1: prog track

void DCCCurrentSensor::begin()
{
  begin(adcinterruptHandler);
}

void DCCCurrentSensor::begin(INTERRUPT_CALLBACK adc_interrupt)
{
  pinMode(8, OUTPUT);

  adc_interruptHandler = adc_interrupt;

  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  ADMUX &= B11011111;
 
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  ADMUX |= B01000000;
 
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input
  ADMUX &= B11110000;
 
  // Set MUX3..0 in ADMUX (0x7C) to read from AD8 (Internal temp)
  // Do not set above 15! You will overrun other parts of ADMUX. A full
  // list of possible inputs is available in Table 24-4 of the ATMega328
  // datasheet
  ADMUX |= 0;
  // ADMUX |= B00001000; // Binary equivalent
 
  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  // Note, this instruction takes 12 ADC clocks to execute
  ADCSRA |= B10000000;
 
  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  ADCSRA |= B00100000;
 
  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  ADCSRB &= B11111000;
//  ADCSRB |= B00000010; //ext trigger
 
  // Set the Prescaler to 128 (16000KHz/128 = 125kHz)
  // Above 200KHz 10-bit results are not reliable.
  // Set the Prescaler to 32 (16000KHz/32 = 500kHz)
  ADCSRA &= B11111000; //32 -> 500kHz
  ADCSRA |= B00000101; //32 -> 500kHz -> 38461 samples per second -> 1ms is 39 samples
 
  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  ADCSRA |= B00001000;
 
  // Enable global interrupts
  // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  // supplies by default.
  sei();
 
  // Kick off the first ADC
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;

  sampleCtr = 0;
  sampleAccu = 0;
  msVal[0] = 0;
  msVal[1] = 0;
  intvAccu = 0;
  intvCtr = 0;
  trackVal[0] = 0;
  trackVal[1] = 0;
  trackSel = 0; //0: main track 1: prog track
}

float DCCCurrentSensor::getCurrent(byte track)
{
  return trackVal[track];
}

void DCCCurrentSensor::adcinterruptHandler()
{
  uint16_t thisVal = ADCL | (ADCH << 8);
  digitalWrite(8, !digitalRead(8));
  sampleAccu += sq((float)thisVal);
  sampleCtr++;
  if (sampleCtr >= millisamples)
  {
    intvAccu += sampleAccu;
    intvCtr++;
  
    msVal[trackSel] = sqrt(sampleAccu/millisamples); //prog pulse

    if (intvCtr >= reportIntv)
    {
      trackVal[trackSel] = sqrt(intvAccu/((float)intvCtr * millisamples));
      trackSel ^= 0x01;
      ADMUX &= B11110000;
      ADMUX |= trackSel;
      intvAccu = 0;
      intvCtr = 0; 
    }
    sampleCtr = 0;
    sampleAccu = 0;
  }
}

// Interrupt service routine for the ADC completion
/*
ISR(ADC_vect)
{
  adc_interruptHandler();
}
*/
