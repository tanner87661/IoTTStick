#include <Wire.h>               // Used to establish serial communication on the I2C bus
#include "SparkFun_TMAG5273_Arduino_Library.h"  // Used to send and recieve specific information from our sensor

TMAG5273 sensor;  // Initialize hall-effect sensor

// I2C default address
uint8_t i2cAddress = TMAG5273_I2C_ADDRESS_INITIAL;

// Interrupt pin used
// NOTE: This pin is specific to the SparkFun IoT Motor Driver
uint8_t intPin = 4;

// Start the flag as false
bool interruptFlag = false;

// ISR to print what interrupt was triggered
void isr1() 
{
  interruptFlag = true;
}


void setup() 
{
  Wire.begin();
  // Start serial communication at 115200 baud
  Serial.begin(115200);

  // Configure Interrupt Pin
  pinMode(intPin, INPUT);
  // Attach interrupt to pin 4 as a digital, falling pin
  attachInterrupt(digitalPinToInterrupt(intPin), isr1, CHANGE);

  // Begin example of the magnetic sensor code (and add whitespace for easy reading)
  Serial.println("TMAG5273 Example 2: Interrupts");
  Serial.println("");
  // If begin is successful (0), then start example
  if (sensor.begin(i2cAddress, Wire) == true)
  {
    Serial.println("Begin");
  } 
  else  // Otherwise, infinite loop
  {
    Serial.println("Device failed to setup - Freezing code.");
    while (1);  // Runs forever
  }

  // Set interrupt through !INT
  sensor.setInterruptMode(TMAG5273_INTERRUPT_THROUGH_INT);

  // Set the !INT pin state - pulse for 10us
  sensor.setIntPinState(true);

  // Enable the interrupt response for the thresholds
  sensor.setThresholdEn(true);

  //int pinStatus = sensor.getInterruptPinStatus();
  pinMode(intPin, INPUT);

  // Set X, Y, Z, and T Thresholds for interrupt to be triggered
  sensor.setXThreshold(5);            // mT
  //sensor.setYThreshold(5);            // mT
  //sensor.setZThreshold(5);            // mT
  //sensor.setTemperatureThreshold(50);  // C

  Serial.print("X Threshold Set: ");
  Serial.println(sensor.getXThreshold());
}

/* To use the other thresholds, simply change the names and use the other functions: 
- sensor.getYThreshold();
- sensor.getZThreshold();
- sensor.getTemperatureThreshold();
*/

void loop()
{
  if (interruptFlag == true) 
  {
    interruptFlag = false;
    Serial.println("X Threshold Reached!");

    int xThresh = sensor.getXThreshold();
    Serial.print("X Threshold: ");
    Serial.println(xThresh);

    if (sensor.getMagneticChannel() != 0)  // Checks if mag channels are on - turns on in setup
    {
      float magX = sensor.getXData();
      float magY = sensor.getYData();
      float magZ = sensor.getZData();
      float temp = sensor.getTemp();

      Serial.print("(");
      Serial.print(magX);
      Serial.print(", ");
      Serial.print(magY);
      Serial.print(", ");
      Serial.print(magZ);
      Serial.println(") mT");
      Serial.print(temp);
      Serial.println(" °C");
    } 
    else 
    {
      Serial.println("Mag Channels disabled, stopping..");
      while (1);
    }
  }
  else
  {
    Serial.println("Checking for Interrupts...");
  }

  delay(500);

}
