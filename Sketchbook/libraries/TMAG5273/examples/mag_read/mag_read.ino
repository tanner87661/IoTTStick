#include <SPI.h>
#include <TMAG5170.h>

TMAG5170 * magSensor = NULL;
float sensorData[3] = {0,0,0};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Initializing TMAG5170 3 axis hall effect sensor");

  magSensor = new TMAG5170(13);
  if (!magSensor)
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no TMAG5170 detected ... Check your hardware!");
    while (1) {}
  }
  magSensor->initTMAG5170_forEval();
  delay(500); // give the sensor time to set up:
}

void loop() {
  // put your main code here, to run repeatedly:
  // Read X and Z, do math, then print
    sensorData[0] = magSensor->getFluxdensity(axis_X);
    sensorData[1] = magSensor->getFluxdensity(axis_Y);
    sensorData[2] = magSensor->getFluxdensity(axis_Z);

    uint16_t angleData =  magSensor->getAngledataRaw() >> 3;
    char dispText[120];
    sprintf(dispText, "%f %f\n", sensorData[1], sensorData[2]);
    Serial.print(dispText);
    delay(10);

}
