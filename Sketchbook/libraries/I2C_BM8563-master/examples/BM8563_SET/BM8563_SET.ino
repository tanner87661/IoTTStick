#include "I2C_BM8563.h"

// RTC BM8563 I2C port

// I2C pin definition for M5Stick & M5Stick Plus & M5Stack Core2
#define BM8563_I2C_SDA 21
#define BM8563_I2C_SCL 22

// I2C pin definition for M5Stack TimerCam
// #define BM8563_I2C_SDA 12
// #define BM8563_I2C_SCL 14

I2C_BM8563 rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire1);

void setup() {
  // Init Serial
  Serial.begin(115200);
  delay(50);

  // Init I2C
  Wire1.begin(BM8563_I2C_SDA, BM8563_I2C_SCL);

  // Init RTC
  rtc.begin();

  // Set RTC Date
  I2C_BM8563_DateTypeDef dateStruct;
  dateStruct.weekDay = 3;
  dateStruct.month = 3;
  dateStruct.date = 22;
  dateStruct.year = 2019;
  rtc.setDate(&dateStruct);

  // Set RTC Time
  I2C_BM8563_TimeTypeDef timeStruct;
  timeStruct.hours   = 18;
  timeStruct.minutes = 56;
  timeStruct.seconds = 10;
  rtc.setTime(&timeStruct);
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

  // Wait
  delay(1000);
}
