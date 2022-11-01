#include <OneDimKalman.h>

//#define useNano

uint16_t pinValA = 0;
uint16_t pinValB = 0;
double chAMin = 0;
double chAMax = 0;
double chAAvg = 0;
double chBMin = 0;
double chBMax = 0;
double chBAvg = 0;
uint32_t timePoint = millis();
int16_t counter = 0;
int16_t nzcounterA = 0;
int16_t nzcounterB = 0;
int16_t dzcounter = 0;
int16_t dnzcounter = 0;
OneDimKalman* filterA = NULL;
OneDimKalman* filterB = NULL;

double sensorValueA = 0;  // variable to store the value coming from the sensor
double sensorValueB = 0;  // variable to store the value coming from the sensor

#ifdef useNano
int sensorPinA = A0;    // select the input pin for the potentiometer
int sensorPinB = A1;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int pwmPin = 9;      // select the pin for the PWM output
#else
#include <avr/io.h>        // Adds useful constants
#include <util/delay.h>    // Adds delay_ms and delay_us functions
#include <PWM.h>

int sensorPinA = A2;    // select the input pin for the potentiometer
int sensorPinB = A1;    // select the input pin for the potentiometer
int ledPin = 0;      // select the pin for the LED
int pwmPin = 1;      // select the pin for the LED
int ctrPin = 3;      // select the pin for the LED
#endif

void setup() {
  // put your setup code here, to run once:
  #ifdef useNano
    Serial.begin(115200);
  #endif
  filterA = new OneDimKalman(8,10,10,10);
  filterB = new OneDimKalman(8,10,10,10);
  pinMode(0, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
  chAMin = analogRead(sensorPinA);
  chAMax = chAMin;
  chAAvg = chAMin;
  chBMin = analogRead(sensorPinB);
  chBMax = chBMin;
  chBAvg = chBMin;
}

int pos = 0;
void loop() { //AtTiny85: ca. 3300 runs per second
  analogWrite(ledPin, pos);
  analogWrite(pwmPin, 255 - pos);
  pos = (pos + 1) % 255;
  delay(100);
  return;
  pinValA = analogRead(sensorPinA);
  pinValB = analogRead(sensorPinB);
//  sensorValueA = filterA->getEstimate(4.15 * (double)pinValA); // 5 / 1024 * analogRead(sensorPinA) / 10000 * 8500; //mA
//  sensorValueB = filterB->getEstimate(4.63 * (double)pinValB); // 5 / 1024 * analogRead(sensorPinA) / 10000 * 9500; //mA

  uint8_t readFlags = ((pinValA > 0) << 1) + (pinValB > 0);

  switch (readFlags)
  {
    case 0: //no current
      sensorValueB = filterB->getEstimate(0); // 5 / 1024 * analogRead(sensorPinA) / 10000 * 9500; //mA
      sensorValueA = filterA->getEstimate(0); // 5 / 1024 * analogRead(sensorPinA) / 10000 * 9500; //mA
      dzcounter++;
      break;
    case 1: //chB only
      sensorValueB = filterB->getEstimate(4.63 * (double)pinValB); // 5 / 1024 * analogRead(sensorPinA) / 10000 * 9500; //mA
      chBMax = max(chBMax, sensorValueB);
      chBAvg += sq(sensorValueB);
      nzcounterB++;
      break;
    case 2: //chA only
      sensorValueA = filterA->getEstimate(4.15 * (double)pinValA); // 5 / 1024 * analogRead(sensorPinA) / 10000 * 8500; //mA
      chAMax = max(chAMax, sensorValueA);
      chAAvg += sq(sensorValueA);
      nzcounterA++;
      break;
    case 3:
      dnzcounter++;
      break;
  }
  counter++;
  #ifndef useNano
    digitalWrite(ctrPin,!digitalRead(0));
  #endif
  if (millis() > (timePoint))
  {
    timePoint += 100;
    uint16_t pwmOut;
    #ifdef useNano    
      Serial.print(counter);
      Serial.print(" ");
      Serial.print(dzcounter);
      Serial.print(" ");
      Serial.print((float)(nzcounterA)/(float)counter);
      Serial.print(" ");
      Serial.print((float)(nzcounterB)/(float)counter);
      Serial.print(" ");
      Serial.print((float)(nzcounterA-nzcounterB)/(float)counter);
      Serial.print(" ");
//      Serial.print(dnzcounter);
//      Serial.print(" ");
      Serial.print(nzcounterA);
      Serial.print(" ");
      Serial.print(nzcounterB);

//      Serial.print(" A ");
//      Serial.print(pinValA);
//      Serial.print(" B ");
//      Serial.print(pinValB);
      Serial.print(" ");
      Serial.print(chAMax);
      Serial.print(" ");
      Serial.print(chBMax);
      Serial.print(" ");
      double avgA = nzcounterA > 0 ? sqrt(chAAvg/nzcounterA) : 0;
      Serial.print(avgA);
      Serial.print(" ");
      double avgB = nzcounterB > 0 ? sqrt(chBAvg/nzcounterB) : 0;
      pwmOut = round((max(avgA, avgB) * 255) / 2000);
      double sampleRatio = max(avgA, avgB) / (double)(nzcounterA + nzcounterB);
      Serial.print(avgB);
      Serial.print(" ");
//      Serial.print((chAMax + chBMax)/2);
      Serial.print(" ");
//      Serial.print(chAAvg + chBAvg);
      Serial.print(pwmOut);
      Serial.print(" ");
      Serial.println(sampleRatio);
    #endif
    analogWrite(pwmPin, pwmOut); 
    counter = 0;
    nzcounterA = 0;
    nzcounterB = 0;
    dzcounter = 0;
    dnzcounter = 0;
    chAMax = 0;
    chBMax = 0;
    chAAvg = 0;
    chBAvg = 0;
  }
}
