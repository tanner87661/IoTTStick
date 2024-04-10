#include <Wire.h>            // Used to establish serial communication on the I2C bus
#include "SparkFun_TMAG5273_Arduino_Library.h" // Used to send and recieve specific information from our sensor

TMAG5273 sensor; // Initialize hall-effect sensor

// I2C default address
uint8_t i2cAddress = TMAG5273_I2C_ADDRESS_INITIAL;

void setup() 
{
  Wire.begin();
  // Start serial communication at 115200 baud
  Serial.begin(115200);   

  // Begin example of the magnetic sensor code (and add whitespace for easy reading)
  Serial.println("TMAG5273 Example 4: I2C Settings - Address Change");
  Serial.println("");
  
  // If begin is successful (1), then start example
  if(sensor.begin(i2cAddress, Wire) == true)
  {
    Serial.println("Begin");
  }
  else // Otherwise, infinite loop
  {
    Serial.println("Device failed to setup - Freezing code.");
   
    while(1); // Runs forever
  }

  Serial.print("Original Address: ");
  Serial.println(sensor.getI2CAddress(), HEX);

  // Change I2C Address - must be 7-bits (bits 1-7)
  // NOTE: The I2C address will reset back to default (0x22) after a power cycle. 
  // Running other examples without a power cycle will fail due to the I2C address being different. 
  int updatedAddress = 0x23;

  // Set the I2C Address  
  if(sensor.setI2CAddress(updatedAddress) != 0)
  {
    Serial.println("Address not set correctly!");
    while(1);
  }
  
}

void loop()
{
  // Read current I2C address from the register after set
  int newAddress = sensor.getI2CAddress();
  Serial.print("I2C Address from Device: ");
  Serial.println(newAddress, HEX);
  Serial.println(); // White space for easy viewing

  // Wait 1 second for next scan
  delay(1000);           

}