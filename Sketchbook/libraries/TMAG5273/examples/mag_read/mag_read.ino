#include <SPI.h>
#include <TMAG5273.h>
#include <Wire.h>  

TMAG5273 * magSensor = NULL;
float sensorData[3] = {0,0,0};

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin();
  Serial.println("Initializing TMAG5273 3 axis hall effect sensor");

  TwoWire* wire = new TwoWire();
  magSensor = new TMAG5273(wire);
  if (!magSensor)
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no TMAG5273 detected ... Check your hardware!");
    while (1) {}
  }
  magSensor->setDeviceConfig(2);
  delay(500); // give the sensor time to set up:
}

void loop() {
  // put your main code here, to run repeatedly:
  // Read X and Z, do math, then print
    sensorData[0] = magSensor->getFluxDensity(axis_X);
    sensorData[1] = magSensor->getFluxDensity(axis_Y);
    sensorData[2] = magSensor->getFluxDensity(axis_Z);

    uint16_t angleData =  magSensor->getAngleData();
    char dispText[120];
    sprintf(dispText, "%f %f %d\n", sensorData[1], sensorData[2], angleData);
    Serial.print(dispText);
    delay(10);

}
