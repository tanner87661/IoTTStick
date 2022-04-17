#include <Wire.h>
#include <TMAG5273.h>

#define pinSDA 26 //changes between input and output by I2C master
#define pinSCL 0 //pull SCL low while sending non I2C data over SDA

TMAG5273 * magSensor = NULL;
float sensorData[3] = {0,0,0};


void setup() {
  Serial.begin(115200);
  Wire.begin(pinSDA, pinSCL, 400000); //initialize the I2C interface 400kHz
  delay(500);
  Serial.println("Initializing TMAG5273 3 axis hall effect sensor");

  magSensor = new TMAG5273(&Wire);
  if (!magSensor->setDeviceConfig())
  {
      /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println("Ooops, no TMAG5273 detected ... Check your hardware!");
    delete magSensor;
    magSensor = NULL;
    while (1) {}
  }
  delay(500); // give the sensor time to set up:
}

void loop() 
{
  // put your main code here, to run repeatedly:
  // Read X and Z, do math, then print
    sensorData[0] = magSensor->getFluxDensity(axis_X);
    sensorData[1] = magSensor->getFluxDensity(axis_Y);
    sensorData[2] = magSensor->getFluxDensity(axis_Z);

    float angleData =  magSensor->getAngleData();
    char dispText[120];
    sprintf(dispText, "%.2f %.2f %.2f %.2f \n", sensorData[0], sensorData[1], sensorData[2], angleData/360);
    Serial.print(dispText);
    delay(50);

}
