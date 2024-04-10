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
  Serial.println("TMAG5273 Example 1: Basic Readings");
  Serial.println("");

  // If begin is successful (0), then start example
  if(sensor.begin(i2cAddress, Wire) == 1)
  {
    Serial.println("Begin");
  }
  else // Otherwise, infinite loop
  {
    Serial.println("Device failed to setup - Freezing code.");
    while(1); // Runs forever
  }

}


void loop() 
{
  // Checks if mag channels are on - turns on in setup
  if(sensor.getMagneticChannel() != 0) 
  {
    sensor.setTemperatureEn(true);

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
    Serial.println(" C");
  }
  else
  {
    // If there is an issue, stop the magnetic readings and restart sensor/example
    Serial.println("Mag Channels disabled, stopping..");
    while(1);
  }

  delay(100);
}
