#include <Wire.h>
#include "I2C_BM8563.h"

// pin definitions for M5Stack TimerCam
//#define POWER_HOLD_PIN 33
//#define LED_PIN 2
// RTC BM8563 I2C port
//#define BM8563_I2C_SDA 12
//#define BM8563_I2C_SCL 14

// pin definitions for M5Stack CoreInk
#define POWER_HOLD_PIN 12
#define LED_PIN 10
// RTC BM8563 I2C port
#define BM8563_I2C_SDA 21
#define BM8563_I2C_SCL 22

I2C_BM8563 rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire1);

void bat_init() {
  pinMode(POWER_HOLD_PIN, OUTPUT);
  digitalWrite(POWER_HOLD_PIN, 1);
}

void bat_disable_output() {
  digitalWrite(POWER_HOLD_PIN, 0);
}

void setup() {
  // Init Serial
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  // will hold battery output
  bat_init();

  // Init I2C
  Wire1.begin(BM8563_I2C_SDA, BM8563_I2C_SCL);

  // Init RTC
  rtc.begin();
  rtc.clearIRQ();
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

  digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                   // wait for a second
  digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW

  rtc.SetAlarmIRQ(5);
  delay(10);

  // disable battery output, will wake up after 5 sec, Sleep current is 1~2μA
  bat_disable_output();

  // if usb is not connected, following code will not be executed;
  esp_deep_sleep(5000000);
  esp_deep_sleep_start();
}
