void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
}

double chA0 = 0;
double chA1 = 0;
uint32_t timePoint = millis();
uint32_t counter = 0;

void loop() {
  // put your main code here, to run repeatedly:
    chA0 = max(chA0, analogRead(0));
//  chA0 = (999 * chA0 + analogRead(0))/1000; 
  chA1 = (999 * chA1 + analogRead(1))/1000; 
  counter++;
  if (millis() > (timePoint + 100))
  {
    timePoint += 100;
//    Serial.println(counter);
    Serial.print(chA0);
    Serial.print(" ");
    Serial.println(chA1);
    counter = 0;
    chA0 = 0;
  }
//  if ((chA0 > 0.5) && (chA1 > 0.5))
  {
//    Serial.print(chA0);
//    Serial.print(" ");
//    Serial.println(chA1);
  }
}
