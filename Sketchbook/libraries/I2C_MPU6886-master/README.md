# I2C MPU6886 IMU Library

This is a library of MPU6886, the IMU for I2C connectivity.

## Support Devices

- [M5Stack M5Stick](https://docs.m5stack.com/#/en/core/m5stickc)
- [M5Stack M5Stick Plus](https://docs.m5stack.com/#/en/core/m5stickc_plus)

## Usage
```c
#include "I2C_MPU6886.h"

I2C_MPU6886 imu(I2C_MPU6886_DEFAULT_ADDRESS, Wire1);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire1.begin(21, 22);

  imu.begin();
  Serial.printf("WhoAmI() = 0x%02x\n", imu.WhoAmI());
}

void loop() {
  float ax;
  float ay;
  float az;
  float gx;
  float gy;
  float gz;
  float t;

  imu.getAccel(&ax, &ay, &az);
  imu.getGyro(&gx, &gy, &gz);
  imu.getTemp(&t);

  Serial.printf("%f,%f,%f,%f,%f,%f,%f\n", ax, ay, az, gx, gy, gz, t);

  delay(100);
}
```
