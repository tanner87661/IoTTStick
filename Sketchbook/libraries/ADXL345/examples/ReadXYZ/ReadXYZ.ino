#include <Wire.h>
#include <ADXL345.h>

ADXL345 accel(ADXL345_STD);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  byte deviceID = accel.readDeviceID();
  if (deviceID != 0) {
    Serial.print("0x");
    Serial.print(deviceID, HEX);
    Serial.println("");
  } else {
    Serial.println("read device id: failed");
    while(1) {
      delay(100);
    }
  }

  // Data Rate
  // - ADXL345_RATE_3200HZ: 3200 Hz
  // - ADXL345_RATE_1600HZ: 1600 Hz
  // - ADXL345_RATE_800HZ:  800 Hz
  // - ADXL345_RATE_400HZ:  400 Hz
  // - ADXL345_RATE_200HZ:  200 Hz
  // - ADXL345_RATE_100HZ:  100 Hz
  // - ADXL345_RATE_50HZ:   50 Hz
  // - ADXL345_RATE_25HZ:   25 Hz
  // - ...
  if (!accel.writeRate(ADXL345_RATE_200HZ)) {
    Serial.println("write rate: failed");
    while(1) {
      delay(100);
    }
  }

  // Data Range
  // - ADXL345_RANGE_2G: +-2 g
  // - ADXL345_RANGE_4G: +-4 g
  // - ADXL345_RANGE_8G: +-8 g
  // - ADXL345_RANGE_16G: +-16 g
  if (!accel.writeRange(ADXL345_RANGE_16G)) {
    Serial.println("write range: failed");
    while(1) {
      delay(100);
    }
  }

  if (!accel.start()) {
    Serial.println("start: failed");
    while(1) {
      delay(100);
    }
  }
}

void loop() {
  if (accel.update()) {
    Serial.print(accel.getX());
    Serial.print(",");
    Serial.print(accel.getY());
    Serial.print(",");
    Serial.print(accel.getZ());
    Serial.println("");
  } else {
    Serial.println("update failed");
    while(1) {
      delay(100);
    }
  }
  delay(300);
}
