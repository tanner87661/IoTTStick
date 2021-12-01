#include <SPI.h>
#include <TMAG5170.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>
#include <utility/imumaths.h>

/* Connections
 *  Nano Board  MOSI 11  MISO 12  SCK 13 SS 10
 */

/* Set the delay between fresh samples */
#define magOverflow 720 //1/2 degreee resolution
#define measuringInterval 10 //ms
#define reportingInterval 1000 //ms
#define recordingInterval 100 //ms

float seaLevelPressure = 1013.25;
float wheelDiameter = 31; //mm from config settings
uint8_t magThreshold = 2;

String trackFileName = "";
volatile bool trackMode = false;

float calibFlux[6] = {0,0,0,0,0,0};
bool magCalibrated = false;
int revCtr = 0;
int calibCtr = 0;
uint32_t lastOverFlow = micros();

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address

Adafruit_BME280 * bme = NULL; // I2C
Adafruit_BNO055 * bno = NULL;
TMAG5170 * magSensor = NULL;

volatile uint32_t lastSampleCtrl = millis();
uint32_t lastReport = millis();
volatile uint32_t lastTrackRecord = millis();
sensors_event_t orientationData;
volatile bool reloadOffset = false;
volatile float location[3] = {0,0,0};
float headingOffset[3] = {0,0,0};
float lastOrientationData[3] = {0,0,0};
float compOrientationData[3] = {0,0,0};
uint8_t mySystem, myGyro, myAccel, myMag;
bool reportSensor = false;

volatile uint16_t lastAngle;
volatile uint16_t diffAngle;
volatile uint32_t lastAngleUpdate;
volatile uint32_t absIntegrator = 0; //absolute value, direction independent
volatile int32_t relIntegrator = 0; //for pos calc
volatile bool    currDirFwd = true;
volatile float angleDiff = 0;

volatile float currSpeed;
volatile float currRadius;
//OneDimKalman * speedAvg = NULL; //Kalman filtered analog reading

float travelDistance;

float thisAltitude;

TaskHandle_t taskHandleSensor = NULL;
SemaphoreHandle_t sensorSemaphore = NULL;

void sensorTask(void * thisParam)
{
  lastSampleCtrl = millis();
  lastReport = millis();
  lastAngleUpdate = micros();
  while (true)
  {
    sensorLoop();
  }
}

void loadSensorCfgJSON(DynamicJsonDocument doc)
{
  if (doc.containsKey("WheelSize"))
    wheelDiameter = (float)doc["WheelSize"];
  if (doc.containsKey("SeaLevel"))
    seaLevelPressure = (float)doc["SeaLevel"];
  if (doc.containsKey("MagThreshold"))
    magThreshold = (uint8_t)doc["MagThreshold"];
//  Serial.printf("Dia: %f Sea: %f Mag: %i\n", wheelDiameter, seaLevelPressure, magThreshold);
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
    */
/**************************************************************************/
void displaySensorDetails(void)
{
    Adafruit_BNO055::adafruit_bno055_rev_info_t myInfo;
    sensor_t sensor;
    bno->getRevInfo(&myInfo);
    bno->getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print("Sensor:       "); Serial.println(sensor.name);
    Serial.print("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
    Serial.print("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
    Serial.print("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
    Serial.printf("Firmware:     %X.%2X\n", myInfo.sw_rev >> 8, myInfo.sw_rev & 0x00FF); 
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

/**************************************************************************/
/*
    Display some basic info about the sensor status
    */
/**************************************************************************/
void displaySensorStatus(void)
{
    /* Get the system status values (mostly for debugging purposes) */
    uint8_t system_status, self_test_results, system_error;
    system_status = self_test_results = system_error = 0;
    bno->getSystemStatus(&system_status, &self_test_results, &system_error);

    /* Display the results in the Serial Monitor */
    Serial.println("");
    Serial.print("System Status: 0x");
    Serial.println(system_status, HEX);
    Serial.print("Self Test:     0x");
    Serial.println(self_test_results, HEX);
    Serial.print("System Error:  0x");
    Serial.println(system_error, HEX);
    Serial.println("");
    delay(500);
}

bool initSensors()
{

  Wire.begin(27, 26, 400000);
  delay(500); // give the sensor time to set up:

//  speedAvg = new OneDimKalman(8,10,10,10);
//  speedAvg->setInitValues(); //initialize with default values

  Serial.println("Initializing BNO055 IMU");
  bno = new Adafruit_BNO055(55, 0x29);
  if (!bno->begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your hardware!");
    return false;
  }
  Serial.println("Set BNO055 Calibration data");
  adafruit_bno055_offsets_t calibrationData = {-42, -7, -23, -635, 205, 861, 0, -1, 0, 1000, 724};
//  bno->setSensorOffsets(calibrationData);
  displaySensorDetails();
  displaySensorStatus();
  bno->setExtCrystalUse(true);

  Serial.println("Initializing BME280 Environmental Sensor");
  bme = new Adafruit_BME280();
  if (!bme->begin(0x76, &Wire))
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BME280 detected ... Check your hardware!");
    return false;
  }

  Serial.println("Initializing TMAG5170 3 axis hall effect sensor");
  magSensor = new TMAG5170(25, 12, 14, 13);
  if (!magSensor)
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no TMAG5170 detected ... Check your hardware!");
    return false;
  }
  magSensor->initTMAG5170_forEval();

  delay(500); // give the sensor time to set up:

  sensorSemaphore = xSemaphoreCreateMutex();

  if (taskHandleSensor == NULL)
    xTaskCreate(    sensorTask,        /* Task function. */
                    "SensorTask",      /* String with name of task. */
                    16000,            /* Stack size in bytes. */
                    NULL,     /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    &taskHandleSensor);            /* Task handle. */


  startCalib();
  return true;
}

/*
Calibration Results: 
Accelerometer: -42 -7 -23 
Gyro: 0 -1 0 
Mag: -635 205 861 
Accel Radius: 1000
Mag Radius: 724

Storing calibration data to EEPROM...
Data stored to EEPROM.

 */

void sensorLoop()
{
  if ((millis() - lastSampleCtrl) > measuringInterval)
  {
    if (!magCalibrated)
    {
      if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
      {
        for (uint8_t i = 0; i < 3; i++)
        {
          float fluxRes = magSensor->getFluxdensity(i);
          if (fluxRes < calibFlux[2 * i])
            calibFlux[2 * i] = fluxRes;
          if (fluxRes > calibFlux[(2 * i)+1])
            calibFlux[(2 * i)+1] = fluxRes;
        }
        if (abs(revCtr - calibCtr) > 2)
        {
          float xGain = calibFlux[1] - calibFlux[0];
          float zGain = calibFlux[5] - calibFlux[4];
          xGain = zGain / xGain;
          if (xGain < 1.0)
          {
            magSensor->regConfig(writeReg, MAG_GAIN_CONFIG, GAIN_SELECTION_XisSelected || (uint16_t) trunc(xGain * 1024));
            Serial.printf("Calibrated X: %f %i \n", (uint16_t) xGain, trunc(xGain * 1024));
          }
          else
          {
            magSensor->regConfig(writeReg, MAG_GAIN_CONFIG, GAIN_SELECTION_ZisSelected || (uint16_t) trunc((1024/xGain)));
            Serial.printf("Calibrated Z: %f %i \n", 1/xGain,(uint16_t) trunc(1024/xGain));
          }
          magCalibrated = true;
        }
        xSemaphoreGive(sensorSemaphore);
      }
      else
        return; //to retry immediately
    }
    uint16_t angleData;
    if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
    {
      angleData =  magSensor->getAngledataRaw() >> 3;
      xSemaphoreGive(sensorSemaphore);
    }
    else
      return; //to retry immediately
    int8_t overFlow = 0;
    int relMove = 0;
    if (abs(angleData - lastAngle) > 360)
    {
      overFlow = angleData > lastAngle ? overFlow = -1 : overFlow = 1;
      relMove = overFlow > 0 ? angleData + magOverflow - lastAngle : angleData - magOverflow - lastAngle;
    }
    else
      relMove = (angleData  - lastAngle);
    revCtr += overFlow;     
    bool dirFwd = relMove >= 0;

    float vdist;
    float hdist;

//    if (magCalibrated)
//      Serial.println(angleData);

    if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
    {
      bno->getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
      bno->getCalibration(&mySystem, &myGyro, &myAccel, &myMag);
      thisAltitude = bme->readAltitude(seaLevelPressure);
      xSemaphoreGive(sensorSemaphore);

//      Serial.printf("Yaw: %f Roll: %f Pitch: %f \n", orientationData.orientation.x, orientationData.orientation.y, orientationData.orientation.z);

      if (reloadOffset)
      {
        headingOffset[0] = -orientationData.orientation.x;
        headingOffset[1] = -orientationData.orientation.y;
        headingOffset[2] = -orientationData.orientation.z;
        reloadOffset = false;
      }

      compOrientationData[0] = orientationData.orientation.x + headingOffset[0]; //yaw
      if (compOrientationData[0] < 0) compOrientationData[0] += 360;
      if (compOrientationData[0] > 360) compOrientationData[0] -= 360;

      compOrientationData[1] = orientationData.orientation.y + headingOffset[1]; //roll
      if (compOrientationData[1] > 90) compOrientationData[1] = 180 - compOrientationData[1];
      if (compOrientationData[1] < -90) compOrientationData[1] = -180 - compOrientationData[1];

      compOrientationData[2] = orientationData.orientation.z + headingOffset[2]; //pitch
      if (compOrientationData[2] < -180) compOrientationData[2] = 360 + compOrientationData[2];
      if (compOrientationData[2] > 180) compOrientationData[2] = -360 + compOrientationData[2];
    
      if ((dirFwd == currDirFwd) || (abs(relMove) > magThreshold))
      {
        uint32_t currTime = micros();
        uint32_t timeDiff = currTime - lastAngleUpdate;
        uint32_t overFlowDiff = currTime - lastOverFlow;
        currDirFwd = dirFwd;
        lastAngleUpdate = currTime;
        diffAngle = (lastAngle - angleData);
        lastAngle = angleData;
        absIntegrator += abs(relMove);
        relIntegrator += relMove;

          //calculate location
          float linDistance = (float)relMove * wheelDiameter * PI / 720;
          //calculate horizontal distance and vertical distance considering pitch

        float pitchRad = compOrientationData[2] * TWO_PI  / 360;
        float rollRad = compOrientationData[1] * TWO_PI  / 360;
        float yawRad  = compOrientationData[0] * TWO_PI  / 360;

        if (sin(pitchRad) < 0.01) //min 1% slope
        {
          vdist = 0;
          hdist = linDistance;
        }
        else
        {
          vdist = linDistance * sin(pitchRad);
          hdist = linDistance * cos(pitchRad);
        }

        
      
        //calculate x and y comnponents of horizontal distance considering yaw
        float xdist = hdist * cos(yawRad);
        float ydist = hdist * sin(yawRad);

        //add to position vector
        location[0] -= xdist;
        location[1] += ydist;
        location[2] += vdist;

        if ((trackMode) && (millis() > lastTrackRecord + recordingInterval))
        {
          char trackEntry[100];
          sprintf(trackEntry, "{\"Coord\":[%f,%f,%f,%f]},", location[0], location[1], location[2], yawRad); 
          sprintf(trackEntry, "{\"Move\":[%f,%f,%f,%f]},", yawRad, rollRad, pitchRad, linDistance); 
          if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
          {
            appendJSONFile(trackFileName, trackEntry);
            xSemaphoreGive(sensorSemaphore);
//            Serial.println(trackEntry);
            lastTrackRecord += recordingInterval;
          }
          else
            Serial.println("No semaphore");
        }

        //updat Speed info
        if (overFlow != 0)
        {
          currSpeed = (float)overFlow * (1000000 * wheelDiameter * PI / (float)overFlowDiff);
          lastOverFlow = currTime;
        }
//      Serial.printf("Incr: %i Speed: %f Dist: %f Pos: %i\n", relMove, currSpeed, linDistance, relIntegrator);
      }
      else
      {
        if ((abs(relMove) < magThreshold) && ((micros() - lastOverFlow) > 1000000))
          currSpeed = 0; //speedAvg->getEstimate(0);
      }
      if ((currSpeed != 0) && (overFlow != 0)) //we are moving
      { //calculate curve radius
        if (abs(compOrientationData[0] - lastOrientationData[0]) > 180)
        {
          float yawOverFlow = compOrientationData[0] > lastOrientationData[0] ? yawOverFlow = -1 : yawOverFlow = 1;
          angleDiff = yawOverFlow > 0 ? compOrientationData[0] + 360 - lastOrientationData[0] : compOrientationData[0] - 360 - lastOrientationData[0];
        }
        else
          angleDiff = (compOrientationData[0] - lastOrientationData[0]);
          
        if (abs(angleDiff) > 0)
//          currRadius = hdist / (angleDiff * TWO_PI  / 360);
          currRadius = 180 * wheelDiameter / angleDiff;
        else
          currRadius = 0;
        // (R * 2Pi) / 360 * angleDiff = Distance
        //R = Distance / (2Pi / 360 * angleDiff)  
        memcpy(&lastOrientationData, &compOrientationData, sizeof(compOrientationData));
      }
      xSemaphoreGive(sensorSemaphore);
//      Serial.printf("%f %f %f %f \n", currSpeed, angleDiff, hdist, currRadius);
    }
    else
      return;

//  bme->readHumidity();
    lastSampleCtrl += measuringInterval;
  }
}

void sensorResetDistance()
{
  Serial.println("Reset Distances");
  absIntegrator = 0;
  relIntegrator = 0;
}

void sensorResetIMU()
{
  Serial.println("Reset Euler Offsets");
  reloadOffset = true;
}

void sensorResetPos()
{
  Serial.println("Reset Position");
  location[0] = 0;
  location[1] = 0;
  location[2] = 0;
}

void startCalib()
{
  calibCtr = revCtr;
  for (uint8_t i = 0; i < 6; i++)
    calibFlux[i] = 0;
  magCalibrated = false;
  Serial.println("Start Calibration");  
}

void sensorReport()
{
    if ((millis() - lastReport) > reportingInterval) 
    {
//      Serial.printf("Abs: %f Rel: %f Speed %f\n", (float)absIntegrator * wheelDiameter * PI / 720, (float)relIntegrator * wheelDiameter * PI / 720, currSpeed);
      if ((globalClient != NULL) && reportSensor)
      {
//        Serial.print("Send...");
        processDataToWebClient("PosData");
//        Serial.println(" done");
      }
//      else
//        Serial.println("No Client");
      lastReport += reportingInterval;
    }
}

void processDataToWebClient(String thisCmd)  //if a web browser is conneted, all LN messages are sent via Websockets
                                                                        //this is the hook for a web based LcooNet viewer
{
    DynamicJsonDocument doc(400);
    char myMqttMsg[400];
    doc["Cmd"] = thisCmd;
    JsonObject Data = doc.createNestedObject("Data");
    JsonArray eulerData = Data.createNestedArray("Euler");
    JsonArray calibData = Data.createNestedArray("Calib");
    JsonArray locData = Data.createNestedArray("Pos");
    if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
    {
      Data["Speed"] = currSpeed;
      Data["Radius"] = currRadius;
//      Data["AngleDiff"] = angleDiff;
//    Data["Dia"] = wheelDiameter;
      Data["AbsDist"] = round((float)absIntegrator * wheelDiameter * PI / 720);
      Data["RelDist"] = round((float)relIntegrator * wheelDiameter * PI / 720);
//      Data["Angle"] = (float)lastAngle / 2;
      Data["Alti"] = thisAltitude;

      for (int8_t i = 0; i < 3; i ++)
        locData.add(round(location[i]));
    
      calibData.add(mySystem);
      calibData.add(myGyro);
      calibData.add(myAccel);
      calibData.add(myMag);
      calibData.add((int)magCalibrated);

      eulerData.add(compOrientationData[0]);
      eulerData.add(compOrientationData[1]);
      eulerData.add(compOrientationData[2]);

      Data["Tracking"] = trackMode;
/*
    JsonArray eulerOffs = Data.createNestedArray("EulerOff");
    eulerOffs.add(headingOffset[0]);
    eulerOffs.add(headingOffset[2]);
    eulerOffs.add(headingOffset[1]);
*/
      xSemaphoreGive(sensorSemaphore);
    }
    else
      return; 

    serializeJson(doc, myMqttMsg);
    globalClient->text(myMqttMsg);
//    Serial.println(myMqttMsg);
    lastWifiUse = millis();
}
