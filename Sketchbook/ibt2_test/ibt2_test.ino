
#define useNano

uint16_t pinValA = 0;
uint16_t pinValB = 0;
double chAMin = 0;
double chAMax = 0;
double chAAvg = 0;
double chBMin = 0;
double chBMax = 0;
double chBAvg = 0;
uint32_t timePoint = millis();
uint16_t counter = 0;
uint16_t nzcounterA = 0;
uint16_t nzcounterB = 0;
uint16_t dzcounter = 0;
uint16_t dnzcounter = 0;
double sensorValueA = 0;  // variable to store the value coming from the sensor
double sensorValueB = 0;  // variable to store the value coming from the sensor

#ifdef useNano
int sensorPinA = A0;    // select the input pin for the potentiometer
int sensorPinB = A1;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
#else
int sensorPinA = A2;    // select the input pin for the potentiometer
int ledPin = 1;      // select the pin for the LED
int pwmPin = 1;      // select the pin for the LED
int ctrPin = 0;      // select the pin for the LED
#endif

void setup() {
  // put your setup code here, to run once:
  #ifdef useNano
    Serial.begin(115200);
  #endif
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

void loop() { //AtTiny85: ca. 3300 runs per second
  pinValA = analogRead(sensorPinA);
  pinValB = analogRead(sensorPinB);
  sensorValueA = 3.66 * pinValA; // 5 / 1024 * analogRead(sensorPinA) / 10000 * 7500; //mA
  sensorValueB = 4.39 * pinValB; // 5 / 1024 * analogRead(sensorPinA) / 10000 * 9000; //mA
//  sensorValueB = (0.8689 * sensorValueB) + 242.75; //y=0.8689x+242.7460
  uint8_t readFlags = ((sensorValueA > 0) << 1) + (sensorValueB > 0);
  switch (readFlags)
  {
    case 0: //no current
      chBMax = max(chBMax, sensorValueB);
      chBAvg = (999 * chBAvg + sensorValueB)/1000; 
      chAMax = max(chAMax, sensorValueA);
      chAAvg = (999 * chAAvg + sensorValueA)/1000; 
      dzcounter++;
      break;
    case 1: //chB only
      chBMax = max(chBMax, sensorValueB);
      chBAvg = (999 * chBAvg + sensorValueB)/1000; 
      nzcounterB++;
      break;
    case 2: //chA only
      chAMax = max(chAMax, sensorValueA);
      chAAvg = (999 * chAAvg + sensorValueA)/1000; 
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
  if (millis() > (timePoint + 50))
  {
    timePoint += 50;
    #ifdef useNano    
      Serial.print(counter);
      Serial.print(" ");
      Serial.print(dzcounter);
      Serial.print(" ");
      Serial.print(dnzcounter);
      Serial.print(" ");
      Serial.print(nzcounterA);
      Serial.print(" ");
      Serial.print(4.882 * pinValA);
      Serial.print(" ");
//      Serial.print(chAMax);
      Serial.print(" ");
      Serial.print(chAAvg);
      Serial.print(" ");
      Serial.print(nzcounterB);
      Serial.print(" ");
      Serial.print(4.882 * pinValB);
      Serial.print(" ");
//      Serial.print(chBMax);
      Serial.print(" ");
      Serial.print(chBAvg);
      Serial.print(" ");
      Serial.print((chAMax + chBMax)/2);
      Serial.print(" ");
      Serial.print(chAAvg + chBAvg);
      Serial.println();
    #else
      analogWrite(pwmPin, 10 * chA1); 
    #endif
    counter = 0;
    nzcounterA = 0;
    nzcounterB = 0;
    dzcounter = 0;
    dnzcounter = 0;
    chAMax = 0;
    chBMax = 0;
  }
}
