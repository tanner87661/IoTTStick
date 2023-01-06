void adc_setup()
{
 DDRB|=(1<<PB1);         //PB1 as output to activate LED
 ADCSRA|=(1<<ADEN);      //Enable ADC module
 ADMUX=0x01; // configuring PB2 to take input
 ADCSRB=0x00;           //Configuring free running mode
 ADCSRA|= (1<<ADIE)|(1<<ADSC)|(1<<ADATE);   //Start ADC conversion and enabling Auto trigger
 ADCSRA &= 0xF8;
// ADCSRA |= (1<<ADPS2)|(1<<ADPS1) |(1<<ADPS0);  //set diviser to 8
}

ISR(ADC_vect)
{
  uint16_t thisVal = ((ADCH<<8) | ADCL);
  if (digitalRead(3) == regIO) //(1<<PB3))
  {
//    analogVals[wrPtr] = thisVal;
    if (sampleCtr < 8)
    {
      sampleSum[sampleCtr >> 2] += thisVal;
      sampleCtr++;
//      PORTB ^=(1<<PB1); //LED toggle
    }
//    wrPtr = (wrPtr + 1) % 32;
  }
  else
  {
    regIO = !regIO;
    uint16_t myVal = 0;
    if (sampleCtr >= 4)
      myVal = (sampleSum[0] >> 2); //round(sampleSum / sampleCtr);
    if (sampleCtr >= 8)
    {
      myVal += (sampleSum[1] >> 2); //round(sampleSum / sampleCtr);
      myVal >>= 1;
    }
    analogVals = myVal;
    sampleSum[0] = 0;
    sampleSum[1] = 0;
    sampleCtr = 0;
  }
}
