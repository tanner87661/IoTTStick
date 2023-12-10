/* 
* i2c_port_address_scanner for the IoTT Stick to verify I2C bus on pins 0 / 26
* Based on the original code
* available on Arduino.cc and later improved by user Krodal and Nick Gammon (www.gammon.com.au/forum/?id=10896)
* 
*/
#if defined ( ARDUINO )
#include <Arduino.h>
//#include <SD.h>
//#include <SPIFFS.h>
#endif
//#include <M5Unified.h>
#include <TMAG5273.h>
#include <Wire.h>

TMAG5273 * magSensor = NULL;

void setup() {
  Serial.begin(115200);
  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\n\nPurpleHaty I2C Scanner and TMAG Test\n");
  scanPorts();
}

uint8_t portArray[] = {0, 26}; //scl 0 SDA 26 //IoTT Stick
String portMap[] = {"GPIO 0", "GPIO 26"}; //IoTT Stick

void scanPorts() { 
  for (uint8_t i = 1; i < sizeof(portArray); i++) {
    for (uint8_t j = 0; j < sizeof(portArray)-1; j++) {
      if (i != j)
      {
        Serial.println("Scanning (SDA : SCL) - " + portMap[i] + " : " + portMap[j] + " - ");
        Serial.println(Wire.setPins(26,0));
//        Serial.println(Wire.setPins(portArray[i], portArray[j]));
//        Wire.begin(portArray[i], portArray[j]); //for ESP32
        Serial.println(portArray[i], portArray[j]);
        Wire.begin(); //for Arduino, Nano
        delay(10);
//        Serial.println(Wire.getClock());
//        Serial.println(Wire.setClock(400000));
//        Serial.println(Wire.getClock());
//        delay(500);
        check_if_exist_I2C();
      }
    }
  }
}

void check_if_exist_I2C() 
{
  byte error, address;
  int nDevices;
  nDevices = 0;
//for (address = 0x36; address < 0x37; address++ )  
  for (address = 0x35; address < 0x36; address++ )  
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    delay(10);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.printf("I2C device found at address %2X\n", address);
      if (address == 0x35)
        readTMAG();
    }
  } //for loop
}

void readTMAG()
{
  magSensor = new TMAG5273(&Wire);
  magSensor->setDeviceConfig(0);
  for (uint8_t i = 0; i < 5; i++)  
  {
    float rotAngle = magSensor->getAngleData();
    Serial.printf("Current angle %f deg \n ", rotAngle);
    delay(500);
  }
  delete magSensor;
}

void loop() 
{
    scanPorts();  
    delay(500);
}
