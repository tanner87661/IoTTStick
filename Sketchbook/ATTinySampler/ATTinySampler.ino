#define sigPin A2
#define ledPin 0
#define pwmPin 1

uint16_t analogVal = 0;


void setup() {

  OSCCAL = 250; // Overclocks the MCU to around 30 MHz, set lower if this causes instability, raise if you can/want
  
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2));       // clear ADC prescaler bits
  ADCSRA |= bit (ADPS2);                                      // sets ADC clock in excess of 10kHz
  ADCSRA |= bit (ADPS0);
  pinMode(ledPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(sigPin, !digitalRead(sigPin));
  analogVal = analogRead(sigPin);
}
