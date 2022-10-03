
// Testing interrupt-based analog reading
// ATMega328p

// Note, many macro values are defined in <avr/io.h> and
// <avr/interrupts.h>, which are included automatically by
// the Arduino interface

#include <LiquidCrystal.h>

LiquidCrystal lcd(8,9,4,5,6,7);

int ctrlPin = -1;

// High when a value is ready to be read
volatile int readFlag;

// Value to store analog result
volatile int analogVal;

#define millisamples 52 //26
#define reportIntv 50

typedef uint16_t sampleBuffer[millisamples+1];

volatile sampleBuffer sampleAccu[2];
volatile uint8_t selBuffer = 0;
//volatile float sampleAccu = 0;
volatile uint8_t sampleCtr = 0;
volatile float msVal[2] = {0,0};
volatile float intvAccu = 0;
volatile uint16_t intvCtr = 0;
volatile float trackVal[2] = {0,0};
volatile byte trackSel = 0; //0: main track 1: prog track

bool sampleAvail = false;
// Initialization
void setup(){
  Serial.begin(115200);
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Main:         mA");  
  lcd.setCursor(0,1);
  lcd.print("Prog:         mA");  
  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  ADMUX &= B11011111;
 
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  ADMUX |= B01000000;
 
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input
  ADMUX &= B11110001;
 
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
  readFlag = 0;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
  if (ctrlPin >= 0)
    pinMode(ctrlPin, OUTPUT);
  
}



// Processor loop
uint32_t lastDisp = millis() + 500;
void loop()
{
    Serial.println(micros());
  if(sampleAvail)
  {
    Serial.println("r");
//    receiveSamples(&(sampleAccu[selBuffer ^ 0x01][0]));
    receiveSamples();//&sampleAccu[0]);
    sampleAvail = false;
  }

  if (millis() > lastDisp)
  {
//    Serial.println("disp");
    lastDisp += 500;
    char txtBuf[5];
//    lcd.setCursor(6,0);
//    dtostrf(2.75 * trackVal[0],7,1,txtBuf);
//    lcd.print(txtBuf);  
//    lcd.setCursor(6,1);
//    dtostrf(2.75 * trackVal[1],7,1,txtBuf);
//    lcd.print(txtBuf);  
//    Serial.print(millis());
//    Serial.print(" ");
//    Serial.print(2.75 * trackVal[0]);
//    Serial.print(" ");
//    Serial.println(2.75 * trackVal[1]);
  }
}

void receiveSamples()//sampleBuffer* floatVals)
{
  float thisSample = 0;
  uint8_t currBuffer = selBuffer ^ 0x01;
  uint8_t currTrack = sampleAccu[currBuffer][0] & 0x0001;
  uint8_t sampleNr =  sampleAccu[currBuffer][0] >> 8;
    Serial.print(selBuffer);
    Serial.print(" ");
    Serial.println(currBuffer);
//    Serial.print(" ");
//    Serial.print(currTrack);
//    Serial.print(" ");
//  Serial.println(sampleNr);
//  for (int i = 0; i < millisamples; i++)
//    thisSample += sq(sampleAccu[currBuffer][i+1]);
  intvAccu += thisSample;
  msVal[currTrack] = sqrt(thisSample/millisamples); //prog pulse

//  if (sampleNr >= reportIntv)
  {
//    trackVal[currTrack] = sqrt(intvAccu/((float)sampleNr * millisamples));
//    Serial.print(millis());
//    Serial.print(thisSample);
//    Serial.print(" ");
//    Serial.print(intvAccu);
//    Serial.print(" ");
//    Serial.print(trackVal[0]);
//    Serial.print(" ");
//    Serial.println(trackVal[1]);
  }
}

// Interrupt service routine for the ADC completion
ISR(ADC_vect){
  uint16_t thisVal = ADCL | (ADCH << 8);
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
//  ADCSRA |= B01000000;
  if (ctrlPin >= 0)
    digitalWrite(ctrlPin, !digitalRead(ctrlPin));
  // Done reading
  // Must read low first
  sampleAccu[selBuffer][sampleCtr+1] = thisVal;
  sampleCtr++;
  if (sampleCtr >= millisamples)
  {
    sampleCtr = 0;
    sampleAccu[selBuffer][0] = (intvCtr<<8) + trackSel;
//    Serial.print("r");
    selBuffer ^= 0x01;
//    Serial.print(selBuffer);
    intvCtr++;
    sampleAvail = true;
    return;
    if (intvCtr >= reportIntv)
    {
      trackSel ^= 0x01;
      ADMUX &= B11110000;
      ADMUX |= (trackSel); //use A0, A1
//      ADMUX |= (trackSel+1); //use A1, A2
      intvAccu = 0;
      intvCtr = 0; 
    }
  }
  // Not needed because free-running mode is enabled.
}
