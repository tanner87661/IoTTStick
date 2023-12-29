#define pwrPin 11
#define pwmPin 13
#define arPin 12
#define sensePin A1

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) ;
  pinMode(pwrPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(arPin, OUTPUT);
  digitalWrite(pwrPin, 0);
  digitalWrite(pwmPin, 1);
  digitalWrite(arPin, 0);
  Serial.println("PowerShield Test 1.0");
}

void loop() 
{
  Serial.println("Press key to start");
  while (!Serial.available());
  while (Serial.available()) 
    uint8_t myInp = Serial.read();
  Serial.println("Test 1: Digital pin functioning");  
  digitalWrite(pwrPin, 1);
  digitalWrite(pwmPin, 0);
  digitalWrite(arPin, 0);
  delay(1000);
  digitalWrite(pwmPin, 1);
  digitalWrite(arPin, 0);
  delay(1000);
  digitalWrite(pwmPin, 0);
  digitalWrite(arPin, 1);
  delay(1000);
  digitalWrite(pwmPin, 1);
  digitalWrite(arPin, 1);
  delay(1000);
  digitalWrite(pwrPin, 0);
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
    digitalWrite(pwrPin, 0);
  while (Serial.available()) 
    uint8_t myInp = Serial.read();
  digitalWrite(pwrPin, 0);
  digitalWrite(pwmPin, 0);
  digitalWrite(arPin, 0);
  
}
