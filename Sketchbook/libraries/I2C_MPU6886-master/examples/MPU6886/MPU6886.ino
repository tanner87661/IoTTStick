#include "I2C_MPU6886.h"

I2C_MPU6886 imu(I2C_MPU6886_DEFAULT_ADDRESS, Wire1);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire1.begin(21, 22);

  imu.begin();
  Serial.printf("whoAmI() = 0x%02x\n", imu.whoAmI());
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
