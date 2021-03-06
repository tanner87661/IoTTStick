#include <Wire.h>
#include "I2C_BM8563.h"

// I2C pin definition for M5Stick & M5Stick Plus & M5Stack Core2
// RTC BM8563 I2C port
#define BM8563_I2C_SDA 21
#define BM8563_I2C_SCL 22
// Grove I2C port
#define GROVE_I2C_SDA 32
#define GROVE_I2C_SCL 33

// I2C pin definition for M5Stack TimerCam
// RTC BM8563 I2C port
// #define BM8563_I2C_SDA 12
// #define BM8563_I2C_SCL 14
// Grove I2C port
// #define GROVE_I2C_SDA 4
// #define GROVE_I2C_SCL 13

I2C_BM8563 rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire1);

void setup() {
  // Init Serial
  Serial.begin(115200);
  delay(50);

  // Init I2C
  Wire.begin(GROVE_I2C_SDA, GROVE_I2C_SCL);
  Wire1.begin(BM8563_I2C_SDA, BM8563_I2C_SCL);

  // Init RTC
  rtc.begin();
}

void grove_i2c_scan()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ ) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("done\n");
  }
}

void loop() {
  I2C_BM8563_DateTypeDef dateStruct;
  I2C_BM8563_TimeTypeDef timeStruct;

  // Get RTC
  rtc.getDate(&dateStruct);
  rtc.getTime(&timeStruct);

  // Print RTC
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
                dateStruct.year,
                dateStruct.month,
                dateStruct.date,
                timeStruct.hours,
                timeStruct.minutes,
                timeStruct.seconds
               );

  grove_i2c_scan();

  // Wait
  delay(1000);
}
