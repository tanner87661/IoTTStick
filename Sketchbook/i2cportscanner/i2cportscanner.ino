/* 
* i2c_port_address_scanner for the IoTT Stick to verify I2C bus on pins 0 / 26
* Based on the original code
* available on Arduino.cc and later improved by user Krodal and Nick Gammon (www.gammon.com.au/forum/?id=10896)
* 
*/
#if defined ( ARDUINO )
#include <Arduino.h>
#include <SD.h>
#include <SPIFFS.h>
#endif
#include <M5AtomDisplay.h>
//#include <M5UnitLCD.h>
//#include <M5UnitOLED.h>
#include <M5Unified.h>

#include <Wire.h>

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;   // default=115200. if "Serial" is not needed, set it to 0.
  cfg.clear_display = true;  // default=true. clear the screen when begin.
  cfg.output_power  = true;  // default=true. use external port 5V output.
  cfg.internal_imu  = true;  // default=true. use internal IMU.
  cfg.internal_rtc  = true;  // default=true. use internal RTC.
  cfg.internal_spk  = false;  // default=true. use internal speaker.
  cfg.internal_mic  = true;  // default=true. use internal microphone.
  cfg.external_imu  = false;  // default=false. use Unit Accel & Gyro.
  cfg.external_rtc  = false;  // default=false. use Unit RTC.
  cfg.external_spk  = false; // default=false. use SPK_HAT / ATOMIC_SPK
  cfg.led_brightness = 0;   // default= 0. system LED brightness (0=off / 255=max) (※ not NeoPixel)

  M5.begin(cfg);

//  Serial.begin(115200);
//  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\n\nI2C Scanner to scan for devices on each port pair D0 to D7\n");
  scanPorts();
}

//uint8_t portArray[] = {22, 21}; //scl 0 SDA 26 //LC2021
//uint8_t portArray[] = {33, 32}; //scl 33 SDA 32 //IoTT Stick Grove
uint8_t portArray[] = {0, 26}; //scl 0 SDA 26 //IoTT Stick
//uint8_t portArray[] = {26, 27}; //scl 26 SDA 27 //Sensor Test
//uint8_t portArray[] = {A5, A4}; //scl A5 SDA A4 //Arduino Uno, Nano
//String portMap[] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}; //for Wemos
//String portMap[] = {"GPIO 22", "GPIO 21"}; //LC2021
//String portMap[] = {"GPIO 33", "GPIO 32"}; //IoTT Stick Grove
String portMap[] = {"GPIO 0", "GPIO 26"}; //IoTT Stick
//String portMap[] = {"Analog IO A5", "Analog IO A4"}; //Arduino Uno, Nano
//String portMap[] = {"GPIO 26", "GPIO 27"}; //Sensor Test

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
        Serial.println(Wire.getClock());
        Serial.println(Wire.setClock(400000));
        Serial.println(Wire.getClock());
        delay(500);
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
  for (address = 1; address < 127; address++ )  
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    delay(10);
    error = Wire.endTransmission();

    if (error == 0){
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  } //for loop
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("**********************************\n");
  //delay(1000);           // wait 1 seconds for next scan, did not find it necessary
}

void loop() {
//  if (Serial.read())
    scanPorts();  
    delay(2000);
}
