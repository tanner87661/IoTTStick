#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
  initInterrupts();
  
}
void initInterrupts()
{
  noInterrupts();          
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 928;// = (16*10^6) / (17241*1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
//  TCCR1B |= (1 << CS12) | (1 << CS10);  
  TCCR1B |= (1 << CS10);  // Mode 0, clock select 1
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  interrupts();          
}

/*
 *     ADCSRA = (ADCSRA & 0b11111000) | 0b00000100;   // speed up analogRead sample time 
    TCCR1A = 0;
    ICR1 = CLOCK_CYCLES;
    TCNT1 = 0;   
    TCCR1B = _BV(WGM13) | _BV(CS10);     // Mode 8, clock select 1
    TIMSK1 = _BV(TOIE1); // Enable Software interrupt

 */
static void interruptHandler()
{
    if (OCR1A == 464)     
      OCR1A = 1856;
    else   
      OCR1A = 464;
  PORTD ^= 0x60;
//  digitalWrite(5, !digitalRead(5));
//  digitalWrite(6, !digitalRead(6));
}

ISR(TIMER1_COMPA_vect){ interruptHandler(); }

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available())
  {
    while(Serial.available())
      char c = Serial.read();
//    noInterrupts();  
    if (OCR1A == 928)     
      OCR1A = 1856;
    else   
      OCR1A = 928;
//    interrupts();          
  }
}
