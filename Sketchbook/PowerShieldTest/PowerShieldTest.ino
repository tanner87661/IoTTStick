#define pwrPin 11
#define pwmPin 13
#define arPin 12
#define sensePin A0

uint8_t testAR = 0;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) ;
  while (Serial.available()) 
    uint8_t myInp = Serial.read();
  pinMode(pwrPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(arPin, OUTPUT);
  digitalWrite(pwrPin, 0);
  digitalWrite(pwmPin, 1);
  digitalWrite(arPin, 0);
  
  Serial.println("PowerShield Test 1.0");
  Serial.println("Enter 1 to include AR test. Otherwise enter return");  
  while (!Serial.available());
  testAR = Serial.read();
  while (Serial.available()) 
    uint8_t myInp = Serial.read();
}

void showCurrent()
{
  while (!Serial.available())
  {
    uint16_t serInp = analogRead(sensePin);
    Serial.print(serInp);
    Serial.print(" 8.31: ");
    Serial.print(8.31 * serInp);
    Serial.print(" 12.2: ");
    Serial.println(12.2 * serInp);
    delay(500);
  }
}

void loop() 
{
  Serial.println("Press key to start");
  while (!Serial.available());
  while (Serial.available()) 
    uint8_t myInp = Serial.read();

  Serial.println("Test 1: Digital pin functioning left side until key is pressed");  
  digitalWrite(pwrPin, 1);
  digitalWrite(pwmPin, 0);
  digitalWrite(arPin, 0);
  showCurrent();
  Serial.println("Test 2: Digital pin functioning right side until key is pressed");  
  digitalWrite(pwmPin, 1);
  digitalWrite(arPin, 0);
  showCurrent();
  if (testAR)
  {
    Serial.println("Test 3: Digital pin functioning left side with AR until key is pressed");  
    digitalWrite(pwmPin, 0);
    digitalWrite(arPin, 1);
    showCurrent();
    while (Serial.available()) 
      uint8_t myInp = Serial.read();
    Serial.println("Test 4: Digital pin functioning right side with AR until key is pressed");  
    digitalWrite(pwmPin, 1);
    digitalWrite(arPin, 1);
    showCurrent();
    while (Serial.available()) 
      uint8_t myInp = Serial.read();
  }
  digitalWrite(pwrPin, 0);
/*
  Serial.println("Test 2: Current Measuring. Presskey to start");  
  while (!Serial.available());
  while (Serial.available()) 
    uint8_t myInp = Serial.read();
//  while (!Serial.available()) 
  {
    Serial.println("Forward until key is pressed");  
    digitalWrite(pwrPin, 1);
    digitalWrite(pwmPin, 0);
    digitalWrite(arPin, 0);
    while (!Serial.available()) 
    {
      uint16_t serInp = analogRead(sensePin);
      Serial.print(serInp);
      Serial.print(" 8.31: ");
      Serial.print(8.31 * serInp);
      Serial.print(" 12.2: ");
      Serial.println(12.2 * serInp);
      delay(500);
      digitalWrite(pwmPin, !digitalRead(pwmPin));
    }
    while (Serial.available()) 
      uint8_t myInp = Serial.read();
    if (testAR)
    {    
      Serial.println("Backwards until key is pressed");  
//    digitalWrite(pwrPin, 1);
      digitalWrite(pwmPin, 0);
      while (!Serial.available()) 
      {
        uint16_t serInp = analogRead(sensePin);
        Serial.print(serInp);
        Serial.print(" 8.31: ");
        Serial.print(8.31 * serInp);
        Serial.print(" 12.2: ");
        Serial.println(12.2 * serInp);
        delay(500);
        digitalWrite(arPin, !digitalRead(arPin));
      }
    }
  }
*/
    digitalWrite(pwrPin, 0);
  while (Serial.available()) 
    uint8_t myInp = Serial.read();
  digitalWrite(pwrPin, 0);
  digitalWrite(pwmPin, 0);
  digitalWrite(arPin, 0);
}
