#include <Wire.h> // Used to establish serial communication on the I2C bus
#include "SparkFun_TMAG5273_Arduino_Library.h" // Used to send and recieve specific information from our sensor

// To download the Arduino Simple FOC Library, use this link: https://docs.simplefoc.com/arduino_simplefoc_library_showcase
#include <SimpleFOC.h> 

/* For easy reading with this example, try opening the serial plotter! */

// Initialize hall-effect sensor
TMAG5273 sensor; 

// I2C default address
uint8_t i2cAddress = TMAG5273_I2C_ADDRESS_INITIAL;

// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(8);

// BLDCDriver6PWM(int phA_h,int phA_l,int phB_h,int phB_l,int phC_h,int phC_l, int en)
BLDCDriver6PWM driver = BLDCDriver6PWM(16,17, 18,23, 19,33, 34);

//target variable in rad/s (radians per second)
float target_velocity_rps = 3;

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&target_velocity_rps, cmd); }
void doLimit(char* cmd) { command.scalar(&motor.voltage_limit, cmd); }


void setup() 
{  
  Wire.begin();
  // Start serial communication at 115200 baud
  Serial.begin(115200);   

  // Begin example of the magnetic sensor code
  Serial.println("TMAG5273 Example 5: Basic Readings with the IoT Motor Driver");

  // ********* Driver config *********
  // power supply voltage [V]
  driver.voltage_power_supply = 3.3; // ESP32 3.3V supply pin
  // PWM Configuration
  driver.pwm_frequency = 20000;
  // this value is fixed on startup
  driver.voltage_limit = 4;
  driver.init();
  // link the motor and the driver to enable usage
  motor.linkDriver(&driver);
  // limit the voltage to be set to the motor
  motor.voltage_limit = 4;   // [V]
  // open loop control config
  motor.controller = MotionControlType::velocity_openloop;
  // init motor hardware
  motor.init();
 
  // add target command T
  command.add('T', doTarget, "target velocity");
  command.add('L', doLimit, "voltage limit");

  Serial.println("Motor ready!");
  Serial.print("Set target velocity [rad/s]: ");
  Serial.println(target_velocity_rps);
  _delay(1000);

  // If the TMAG5273begin is successful (1), then start example
  if(sensor.begin(i2cAddress, Wire) == true)
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
  // Basic motor movement
  motor.move(target_velocity_rps);

  // user communication
  command.run();

  // Hall Effect Code
  if(sensor.getMagneticChannel() != 0) // Checks if mag channels are on - turns on in begin()
  {
    float magX = sensor.getXData();
    float magY = sensor.getYData();
    float magZ = sensor.getZData();

    Serial.print("(");
    Serial.print(magX);
    Serial.print(", ");
    Serial.print(magY);
    Serial.print(", ");
    Serial.print(magZ);
    Serial.println(") mT");
  }
  else
  {
    Serial.println("Mag Channels disabled, stopping..");
    while(1);
  }

}
