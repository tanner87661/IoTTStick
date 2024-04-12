/*

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

Adaptation to LocoNet (half-duplex network with DCMA) by Hans Tanner. 
See Digitrax LocoNet PE documentation for more information

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is dstributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <IoTT_TrainSensor.h>

//txFct sensorCallback = NULL;

void myTask(void * thisParam)
{
	IoTT_TrainSensor * thisObj = (IoTT_TrainSensor*)thisParam;
	while(1)
	{
		thisObj->sensorTask(NULL);
//		delay(1000);
	}
}

IoTT_TrainSensor::IoTT_TrainSensor(TwoWire * newWire)
{
	sensorWire = newWire;
	avgMove.sampleIndex = 0;
	for (uint8_t i = 0; i < sampleBufferSize; i++)
		avgMove.sampleArray[i] = 0;
//	sensorWire->begin(sda, scl);
//	delay(10);
}

IoTT_TrainSensor::~IoTT_TrainSensor() 
{
	if (taskHandleSensor)
		vTaskDelete(taskHandleSensor);
}

volatile void IoTT_TrainSensor::sensorTask(void * thisParam)
{
	float rotAngle = -1;
	if ((millis() - lastSampleCtrl) > measuringInterval)
	{
		uint32_t currTime = micros();
		lastSampleCtrl += measuringInterval;

		if (magSensor)
		{

			if (!magCalibrated)
			{
				magSensor->collectCalibData();
				if ((abs(revCtr) > 3))
				{
					magSensor->setFluxOffset();
					magCalibrated = true;
				}
			}

			if (reverseDir)
				rotAngle = magSensor->getAngleData();
			else
				rotAngle = 360 - magSensor->getAngleData();
		}
		else
			return; //no sensor, so don't do anything

		int8_t overFlow = 0;
		float_t relMove = rotAngle - workData.axisAngle;

		if (abs(rotAngle - workData.axisAngle) > 180)
		{
			overFlow = rotAngle > workData.axisAngle ? overFlow = -1 : overFlow = 1;
			if (overFlow > 0)
				relMove += (float_t)magOverflow;
			else
				relMove -= (float_t)magOverflow;
		}
		else
			relMove = (rotAngle  - workData.axisAngle);

		bool dirFwd = relMove >= 0;

		avgMove.sampleArray[avgMove.sampleIndex] = relMove / sampleBufferSize;
		avgMove.sampleIndex = (avgMove.sampleIndex + 1) % sampleBufferSize;
		avgMove.sampleSum = 0;
		for (uint8_t i = 0; i < sampleBufferSize; i++)
			avgMove.sampleSum += avgMove.sampleArray[i];

		workData.avgMove = avgMove.sampleSum; //(0.995 * workData.avgMove) + (0.005 * relMove); //used to detect standstill

		bool isMoving = (abs(workData.avgMove) > 0.25);
//		Serial.printf("%.2f %.2f %.2f \n", rotAngle/100, relMove, workData.avgMove);
		revCtr += overFlow;     
		
		float_t linDistance = 0;
		float_t avgDistance = 0; //used for speed calculation
		
		//calculate time since last speed calculation
		uint32_t speedDiff = currTime - lastSpeedTime;
		workData.currDirFwd = dirFwd;
		workData.axisAngle = rotAngle;

		//calculate travelled distance
		linDistance = (float)relMove * wheelDia * PI / 360;
		avgDistance = workData.avgMove * wheelDia * PI / 360;
		if (isMoving)
			workData.currSpeedTech = avgDistance * 1000000 / speedDiff; //0.995 * workData.currSpeedTech + 0.005 * avgDistance * 1000000 / speedDiff;
		else
			workData.currSpeedTech = 0;
				
		lastSpeedTime = currTime;
		if (isMoving)
			if (workData.avgMove > 0)
				workData.absIntegrator += linDistance;
			else
				workData.absIntegrator -= linDistance;
		workData.relIntegrator += linDistance;

		//here we know travel distance and speed, so we can now read IMU and calculate vector and position, but only if distance > 0
		if (imuSensor)
		{
			if (!imuSensor->getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER)) //get euler vector m IMU
				Serial.println("no event");
			imuSensor->getCalibration(&mySystem, &myGyro, &myAccel, &myMag); //read calibration status as well
		}
		if (reloadOffset)
		{
			headingOffset[0] = -orientationData.orientation.x;
			headingOffset[1] = -orientationData.orientation.y;
			headingOffset[2] = -orientationData.orientation.z;
			reloadOffset = false;
		}

		if (abs(linDistance) > 0)
		{ 
			compOrientationData[0] = orientationData.orientation.x + headingOffset[0]; //yaw
			if (compOrientationData[0] < 0) compOrientationData[0] += 360;
			if (compOrientationData[0] > 360) compOrientationData[0] -= 360;

			compOrientationData[1] = orientationData.orientation.y + headingOffset[1]; //roll
			if (compOrientationData[1] > 90) compOrientationData[1] = 180 - compOrientationData[1];
			if (compOrientationData[1] < -90) compOrientationData[1] = -180 - compOrientationData[1];

			compOrientationData[2] = orientationData.orientation.z + headingOffset[2]; //pitch
			if (compOrientationData[2] < -180) compOrientationData[2] = 360 + compOrientationData[2];
			if (compOrientationData[2] > 180) compOrientationData[2] = -360 + compOrientationData[2];


			
			//convert euler angles to rad
			workData.eulerVectorRad[0] = compOrientationData[0] * TWO_PI  / 360;
			workData.eulerVectorRad[1] = compOrientationData[1] * TWO_PI  / 360;
			workData.eulerVectorRad[2] = compOrientationData[2] * TWO_PI  / 360;
			workData.eulerVectorRad[3] = linDistance;

			if (sin(workData.eulerVectorRad[2]) < 0.01) //min 1% slope
				workData.eulerVectorRad[2] = 0; //otherwise set to 0

			//add to position vector
			workData.posVector_mm[0] -= workData.eulerVectorRad[3] * cos(workData.eulerVectorRad[2]) * cos(workData.eulerVectorRad[0]);
			workData.posVector_mm[1] += workData.eulerVectorRad[3] * cos(workData.eulerVectorRad[2]) * sin(workData.eulerVectorRad[0]);
			workData.posVector_mm[2] += workData.eulerVectorRad[3] * sin(workData.eulerVectorRad[2]);

//			if ((currSpeed != 0) && (overFlow != 0)) //we are moving
			if ((overFlow != 0)) //we are moving
			{ //calculate curve radius
				float_t angleDiff = 0;
				if (abs(compOrientationData[0] - lastOrientationData[0]) > 180)
				{
					float yawOverFlow = compOrientationData[0] > lastOrientationData[0] ? yawOverFlow = -1 : yawOverFlow = 1;
					angleDiff = yawOverFlow > 0 ? compOrientationData[0] + 360 - lastOrientationData[0] : compOrientationData[0] - 360 - lastOrientationData[0];
				}
				else
					angleDiff = (compOrientationData[0] - lastOrientationData[0]);

					
				if (abs(angleDiff) > 0)
					workData.currRadiusTech = 180 * wheelDia / angleDiff;
				else
					workData.currRadiusTech = 0;
//				Serial.printf("%.2f %.2f  \n", angleDiff, workData.currRadiusTech);
//          Serial.println(workData.currRadiusTech);
				memcpy(&lastOrientationData, &compOrientationData, sizeof(compOrientationData));
			}

		}
		if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
		{
			workData.timeStamp = millis();
			dispData = workData;
			xSemaphoreGive(sensorSemaphore);
		}
	}
}

void IoTT_TrainSensor::begin() 
{
//	speedEstimate = new OneDimKalman(8,10,10,10);
	relMoveEstimate = new OneDimKalman(8,10,10,10);
	if (magType == 1)
	{
		Serial.println("Initialize TMAG5273");
		magSensor = new TMAG5273(sensorWire);
		if (!magSensor->setDeviceConfig(mountStyle))
		{
			/* There was a problem detecting the BNO055 ... check your connections */
			Serial.println("Ooops, no TMAG5273 detected ... Check your hardware!");
			delete magSensor;
			magSensor = NULL;
		}
	}
	if (imuType == 1)
	{
		Serial.println("Initialize BNO055");
		imuSensor = new Adafruit_BNO055(55, 0x29);
		if (!imuSensor->begin())
		{
			/* There was a problem detecting the BNO055 ... check your connections */
			Serial.println("Ooops, no BNO055 detected ... Check your hardware!");
			delete imuSensor;
			imuSensor = NULL;
		}
		else
		{
			displayIMUSensorDetails();
			displayIMUSensorStatus();
		}
	}
	if (!imuSensor) //use Stick imu sensor
	{
		Serial.println("Using internal IMU!");
	}
		
	sensorSemaphore = xSemaphoreCreateMutex();
	if (magSensor || imuSensor)
	{
		if (taskHandleSensor == NULL)
		{
//			Serial.println("Create Sensor Task");
//			xTaskCreatePinnedToCore(myTask, 
			xTaskCreate(myTask,        ///* Task function. 
					"SensorTask",      //* String with name of task. 
                    16000,            //* Stack size in bytes. 
                    (void*)this,     //* Parameter passed as input of the task 
                    1,                //* Priority of the task. 
                    &taskHandleSensor);            //* Task handle, core Number
        }
	}
}

void IoTT_TrainSensor::displayIMUSensorDetails()
{
	if (imuSensor)
	{
		Adafruit_BNO055::adafruit_bno055_rev_info_t myInfo;
		sensor_t sensor;
		imuSensor->getRevInfo(&myInfo);
		imuSensor->getSensor(&sensor);
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
	}
	else
		Serial.print("IMU Sensor not available");
}

/**************************************************************************/
/*
    Display some basic info about the sensor status
    */
/**************************************************************************/
void IoTT_TrainSensor::displayIMUSensorStatus()
{
	if (imuSensor)
	{
		/* Get the system status values (mostly for debugging purposes) */
		uint8_t system_status, self_test_results, system_error;
		system_status = self_test_results = system_error = 0;
		imuSensor->getSystemStatus(&system_status, &self_test_results, &system_error);

		/* Display the results in the Serial Monitor */
		Serial.println("");
		Serial.print("System Status: 0x");
		Serial.println(system_status, HEX);
		Serial.print("Self Test:     0x");
		Serial.println(self_test_results, HEX);
		Serial.print("System Error:  0x");
		Serial.println(system_error, HEX);
		Serial.println("");
	}
	else
		Serial.print("IMU Sensor not available");
}

void IoTT_TrainSensor::loadLNCfgJSON(DynamicJsonDocument doc)
{
//	Serial.println("Call JSON Serial");
	if (doc.containsKey("UseMag"))
		magType = doc["UseMag"];
	if (doc.containsKey("UseIMU"))
		imuType = doc["UseIMU"];
	if (doc.containsKey("WheelDia"))
		wheelDia = doc["WheelDia"];
	if (doc.containsKey("ReverseDir"))
		reverseDir = doc["ReverseDir"];
	if (doc.containsKey("MountStyle"))
		mountStyle = doc["MountStyle"];
	if ((doc.containsKey("ScaleList")) && (doc.containsKey("ScaleIndex")))
	{
		uint8_t scaleIndex = doc["ScaleIndex"];
//		JsonArray ScaleArray = doc["ScaleList"];
//		JsonArray ScaleInfo = ScaleArray[scaleIndex];
		workData.modScale = doc["ScaleList"][scaleIndex]["Scale"];
//		Serial.println(workData.modScale);
		strcpy(workData.scaleName, doc["ScaleList"][scaleIndex]["Name"]);
//		Serial.println(doc["ScaleList"][scaleIndex]["Name"]);
	}
	else
	{
		workData.modScale = 87;
		strcpy(workData.scaleName, "HO");
	}
	if (doc.containsKey("Units"))
		workData.dispDim = (uint8_t)doc["Units"]; //0: metric; 1: imperial
	if (doc.containsKey("MagThreshold"))
		magThreshold = doc["MagThreshold"]; //0: metric; 1: imperial
	begin();
}
/*
void IoTT_TrainSensor::setTxCallback(txFct newCB)
{
	sensorCallback = newCB;
}
*/
void IoTT_TrainSensor::resetDistance()
{
//	Serial.println("resetDistance");
	if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
	{
		workData.absIntegrator = 0;
		workData.relIntegrator = 0;
		dispData = workData;
		xSemaphoreGive(sensorSemaphore);
	}
}

void IoTT_TrainSensor::resetHeading()
{
//	Serial.println("resetHeading");
	if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
	{
		reloadOffset = true;
		xSemaphoreGive(sensorSemaphore);
	}
}

float_t IoTT_TrainSensor::getPercOfAngle(float_t gForce)
{
	float_t radAngle = asin(gForce);
	return reverseDir ? (-100 * tan(radAngle)) : (100 * tan(radAngle));
}

sensorData IoTT_TrainSensor::getSensorData()
{
	if (xSemaphoreTake(sensorSemaphore, portMAX_DELAY) == pdPASS) 
	{
		sensorData cpyData = dispData;
		xSemaphoreGive(sensorSemaphore);
		if (!imuSensor)
		{
			cpyData.currRadiusTech = 0;
			for (uint8_t i = 0; i < 3; i++)
				cpyData.posVector_mm[i] = 0;
			for (uint8_t i = 0; i < 4; i++)
				cpyData.eulerVectorRad[i] = 0;
		}
		if (mountStyle == 0)
			M5.Imu.getAccel(&cpyData.imuVal[0], &cpyData.imuVal[1], &cpyData.imuVal[2]);
		else
			M5.Imu.getAccel(&cpyData.imuVal[2], &cpyData.imuVal[1], &cpyData.imuVal[0]);
		M5.Imu.getGyro(&cpyData.imuVal[3], &cpyData.imuVal[4], &cpyData.imuVal[5]);
		return cpyData;
	}
}

/*
 * Flat:  Left  Center  Right Forward
 * x		-1	0	1	
 * z		0	-1	0
 * 			Down	Flat	Up
 * y		-1		0		1
 * 
 * Upright Left  Center  Right Forward
 * x		0	1	0
 * z		-1	0	1

 * 			Down	Flat	Up
 * y		-1		0		1
 * 
 * Dir Chg * -1
*/

//void IoTT_TrainSensor::setRepRate(AsyncWebSocketClient * newClient, int newRate)
void IoTT_TrainSensor::setRepRate(int newRate)
{
//	Serial.println("setRepRate");
	refreshRate = newRate;
	lastWebRefresh = millis();
}

//void IoTT_TrainSensor::reqDCCAddrWatch(AsyncWebSocketClient * newClient, int16_t dccAddr, bool simulOnly)
void IoTT_TrainSensor::reqDCCAddrWatch(int16_t dccAddr, bool simulOnly)
{
//	Serial.printf("reqDCCAddrWatch %i\n", dccAddr);
	digitraxBuffer->awaitFocusSlot(dccAddr, simulOnly); //set DigitraxBuffers to watch for next speed or fct command and memorize loco
	waitForNewDCCAddr = true;
	clrSpeedTable();
}

bool IoTT_TrainSensor::proceedToTrackEnd(bool origin)
{
//	if (speedSample.adminData.upDir == origin)
//		toggleDirCommand();
	return true;
}

bool IoTT_TrainSensor::reverseTestDir()
{
	return true;
}

void IoTT_TrainSensor::startTest(float_t trackLen, float_t vMax, std::vector<float> pMode)
{
//	Serial.printf("%f %f\n", trackLen, vMax);
	if (speedSample.adminData.speedTestRunning) return; //do not restart
	//verify that focusSlot is defined
	int8_t currSlot = digitraxBuffer->getFocusSlotNr();
	if ((currSlot > 0) || (digitraxBuffer->getLocoNetMode() == false))
	{
		speedSample.adminData.testTrackLen = trackLen; // mm
		speedSample.adminData.sampleMinDistance = wheelDia * PI * minTestWheelTurns; //always mm
		speedSample.adminData.crawlSpeedMax = wheelDia * PI * crawlTurns; //always mm/s
//		Serial.printf("%.2f \n", speedSample.adminData.crawlSpeedMax);
		speedSample.adminData.testSteps = 127; //pMode == 0 ? 28 : 127;
//		Serial.printf("%i %i\n", speedSample.adminData.testSteps, pMode);
		speedSample.adminData.upDir = true;
		speedSample.adminData.testState[0].testPhase = 0; //start test
		speedSample.adminData.testState[1].testPhase = 0; 
		speedSample.adminData.masterPhase = 0; //run test
		speedSample.adminData.testState[0].lastSpeedStep = 0;
		speedSample.adminData.testState[1].lastSpeedStep = 0;
		speedSample.adminData.testState[0].crawlSpeedStep = 1;
		speedSample.adminData.testState[1].crawlSpeedStep = 1;
		speedSample.adminData.testState[0].vMaxComplete = false;
		speedSample.adminData.testState[1].vMaxComplete = false;
		speedSample.adminData.testPOI.clear();
		speedSample.adminData.testPOI.push_back(vMax);
//		for (uint8_t i = 0; i < pMode.size(); i++)
//			speedSample.adminData.testPOI.push_back(pMode.at(i));
		speedSample.adminData.testState[0].poiIndex = 0;
		speedSample.adminData.testState[1].poiIndex = 0;
		speedSample.adminData.testState[0].testSpeedMax = 0;
		speedSample.adminData.testState[1].testSpeedMax = 0;
		speedSample.adminData.testState[0].maxSpeedCtr = 0;
		speedSample.adminData.testState[1].maxSpeedCtr = 0;
		speedSample.adminData.testState[0].maxTestSpeedStep = 0;
		speedSample.adminData.testState[1].maxTestSpeedStep = 0;
//		speedSample.adminData.testState[0].trackLimitViolation = false;
//		speedSample.adminData.testState[1].trackLimitViolation = false;
		speedSample.adminData.testState[0].testError = 0;
		speedSample.adminData.testState[1].testError = 0;
		//reset speed recording data
		clrSpeedTable();
		//reset distance counter
		resetDistance();
		//start test process
		speedSample.adminData.speedTestTimer = millis();
		speedSample.adminData.currSpeedStep = 0;
		speedSample.adminData.validSample = false;
		speedSample.adminData.speedTestRunning = true;
	}
	else
		Serial.println("No focus slot assigned");
}

void IoTT_TrainSensor::sendSpeedCommand(uint8_t newSpeed)
{
	int8_t currSlot = digitraxBuffer->getFocusSlotNr();
	if ((currSlot > 0) || (digitraxBuffer->getLocoNetMode() == false))
	{
//		Serial.printf("Set Speed Slot %i to %i\n", currSlot, newSpeed);
		lnTransmitMsg txBuffer;
		txBuffer.lnData[0] = 0xA0; //OPC_LOCO_SPD 
		txBuffer.lnData[1] = currSlot;
		txBuffer.lnData[2] = newSpeed == 0 ? 0 : min(127, newSpeed + 1); //map(newSpeed, 0, speedSample.adminData.testSteps, 0, maxSpeedSteps-1);
		txBuffer.lnMsgSize = 4;
		setXORByte(&txBuffer.lnData[0]);
		sendMsg(txBuffer);
	}
}

void IoTT_TrainSensor::toggleDirCommand()
{
	int8_t currSlot = digitraxBuffer->getFocusSlotNr();
	if ((currSlot > 0) || (digitraxBuffer->getLocoNetMode() == false))
	{
		slotData * focusSlot = digitraxBuffer->getSlotData(currSlot);
		lnTransmitMsg txBuffer;
		txBuffer.lnData[0] = 0xA1; //OPC_LOCO_DIRF 
		txBuffer.lnData[1] = currSlot;
		txBuffer.lnData[2] = ((*focusSlot)[3] ^ 0x20);
//		Serial.printf("Slot %2X Sent %2X\n", (*focusSlot)[3], txBuffer.lnData[2]);
		txBuffer.lnMsgSize = 4;
		setXORByte(&txBuffer.lnData[0]);
		sendMsg(txBuffer);
		speedSample.adminData.upDir = (!speedSample.adminData.upDir);
	}
}

void IoTT_TrainSensor::stopTest()
{
//	if (runSpeedTest)
	{
		//stop locomotive
		sendSpeedCommand(0);
		speedSample.adminData.speedTestRunning = false;
		sendSpeedTableDataToWeb(true);
	}
}

void IoTT_TrainSensor::clrSpeedTable()

{
//	Serial.println("Reset Table");
	for (uint8_t i = 0; i < maxSpeedSteps; i++)
	{
		speedSample.fw[i] = 0;
		speedSample.bw[i] = 0;
	}
	sendSpeedTableDataToWeb(false);
}

bool IoTT_TrainSensor::processTestStep(sensorData * sensStatus)
{
	uint32_t timeSince;
	bool hasProgress = (abs(sensStatus->relIntegrator - speedSample.adminData.testStartLinIntegrator) > 0) && (abs(sensStatus->relIntegrator - speedSample.adminData.lastLinIntegrator) > 0);
	speedSample.adminData.lastLinIntegrator = sensStatus->relIntegrator;
	if (speedSample.adminData.validSample)
	{
		timeSince = micros() - speedSample.adminData.measureStartTime;
		if ((timeSince > (uint32_t)maxSampleTime) || (abs(sensStatus->relIntegrator - speedSample.adminData.testStartLinIntegrator) > speedSample.adminData.sampleMinDistance))
		{
//			Serial.println("s cmpl");
			return true;
		}
	}
	else
	{
		timeSince = millis() - speedSample.adminData.testStartTime;
		if (timeSince > speedSample.adminData.accelTime) //(uint32_t)accel2Steps)
		{
//			Serial.println("s start");
			speedSample.adminData.testStartLinIntegrator = sensStatus->relIntegrator;
			speedSample.adminData.lastLinIntegrator = speedSample.adminData.testStartLinIntegrator;
			speedSample.adminData.measureStartTime = micros();
			speedSample.adminData.validSample = true;
		}
	}
	return false;
}

int8_t IoTT_TrainSensor::getNextPrecStep(speedTable * dataTable, bool forwardDir) //only called for investigation of POI
{
/*
	float_t * speedArray = forwardDir ? &(dataTable->fw[0]) : &(dataTable->bw[0]); 
	
	for (uint8_t i = 0; i < dataTable->adminData.testPOI.size(); i++) //process for each POI
	{
		//if next lower entry = 0, set speed test, until two values received below poi value
		uint8_t stepIndex = 0;
		while (speedArray[stepIndex] <= dataTable->adminData.testPOI.at(i)) //find first speed entry just above looped poi
			stepIndex++;
		uint8_t upperLimit = stepIndex + 2;
		uint8_t lowerLimit = stepIndex - 3;
		for (uint8_t j = lowerLimit; j < upperLimit; j++)
			if (speedArray[j] == 0) //no value set right now
				return (j);
	}
*/
	return -1;
}

bool IoTT_TrainSensor::processSpeedTest() //returns false if complete
{
	//get distance data
	int8_t currSlot = digitraxBuffer->getFocusSlotNr();
	if ((currSlot > 0) || (digitraxBuffer->getLocoNetMode() == false))
	{
		slotData * focusSlot = digitraxBuffer->getSlotData(currSlot);
		bool forwardDir = ((*focusSlot)[3] & 0x20) == 0; //forward = bit cleared
		sensorData cpyData = getSensorData();
		uint8_t upDirIndex = speedSample.adminData.upDir ? 1 : 0;
		bool recordData = true;
		switch (speedSample.adminData.masterPhase)
		{
			case 0: //proceed with directional test
			{
				//verify there is enough room more next step, otherwise change direction
				float_t remDist = speedSample.adminData.upDir ? speedSample.adminData.testTrackLen - abs(cpyData.relIntegrator) : abs(cpyData.relIntegrator);
				float_t brakeDist = sq(cpyData.currSpeedTech) / 300;
				if ((remDist < brakeDist) && (speedSample.adminData.testTrackLen > 0))
				{
					speedSample.adminData.masterPhase = 1;
					speedSample.adminData.testState[upDirIndex].testPhase = 0; //prepare for reentry
					//verify track length limit condition
					if (speedSample.adminData.currSpeedStep <= speedSample.adminData.testState[upDirIndex].maxTestSpeedStep)
					{
//						speedSample.adminData.testState[upDirIndex].trackLimitViolation = true;
						speedSample.adminData.testState[upDirIndex].testError |= 0x01;

					}
					else
						speedSample.adminData.testState[upDirIndex].maxTestSpeedStep = speedSample.adminData.currSpeedStep;
					return true;
				}
				//if there's room, proceed
				switch (speedSample.adminData.testState[upDirIndex].testPhase)
				{
					case 0: //set initial speed
					{
//						Serial.printf("Start Meas %i %i\n", upDirIndex, speedSample.adminData.testState[upDirIndex].lastSpeedStep);
						uint8_t oldSpeed = speedSample.adminData.currSpeedStep; 
						if (speedSample.adminData.testState[upDirIndex].lastSpeedStep == 0)
						{
							speedSample.adminData.currSpeedStep = 1; //speedSample.adminData.testSteps > 28 ? 2 : 1;
							speedSample.adminData.testState[upDirIndex].testPhase++; //this is a valid test, so set testPhase to 2 be incrementing twice
						}
						else
							speedSample.adminData.currSpeedStep = speedSample.adminData.testState[upDirIndex].lastSpeedStep;
						speedSample.adminData.accelTime = (uint32_t)accel2Steps + (4 * (speedSample.adminData.currSpeedStep - oldSpeed));
						speedSample.adminData.validSample = false;
						sendSpeedCommand(speedSample.adminData.currSpeedStep); //get started with speed step
						speedSample.adminData.testStartTime = millis();
						speedSample.adminData.testState[upDirIndex].testPhase++;
					}
					break;
					case 1: //intermediate step for reentering routing, cancels recording of speed in the next step
						recordData = false; 
					//no break, just proceed
					case 2: //increase speed and measure up to about 30mm/s, then determine direction and speed step for test direction, then move on
						if (processTestStep(&cpyData))
						{
//							Serial.printf("Dist: %.2f\n", cpyData.relIntegrator);
							uint32_t timeSince = micros() - speedSample.adminData.measureStartTime;
							float_t distSince = cpyData.relIntegrator - speedSample.adminData.testStartLinIntegrator;
							if ((speedSample.adminData.upDirPos == -1) && (abs(distSince) > 5))
							{
								speedSample.adminData.upDirPos = (distSince > 0) ^ (!speedSample.adminData.upDir);
//								Serial.printf("UpDirPos: %i\n", speedSample.adminData.upDirPos);
							}
							float_t * dataTable = forwardDir ? &speedSample.fw[0] : &speedSample.bw[0];
							float_t * dataEntry = &(dataTable[speedSample.adminData.currSpeedStep]); //forwardDir ? &speedSample.fw[speedSample.adminData.currSpeedStep] : &speedSample.bw[speedSample.adminData.currSpeedStep];
							(*dataEntry) = abs(1000000 * distSince / timeSince); //[mm/s]
							
//							Serial.printf("spd: %.2f %.2f %i %i\n", (*dataEntry), speedSample.adminData.crawlSpeedMax, speedSample.adminData.testState[upDirIndex].crawlSpeedStep, speedSample.adminData.currSpeedStep);
							if (((*dataEntry) < speedSample.adminData.crawlSpeedMax) && (speedSample.adminData.testState[upDirIndex].crawlSpeedStep < speedSample.adminData.currSpeedStep))
							{
								speedSample.adminData.testState[upDirIndex].crawlSpeedStep = speedSample.adminData.currSpeedStep;
//								Serial.printf("set crawl speed %i\n", speedSample.adminData.currSpeedStep);
							}
							if (((*dataEntry) > speedSample.adminData.testState[upDirIndex].testSpeedMax) || (speedSample.adminData.testState[upDirIndex].testSpeedMax == 0)) //must have moved first
							{
								speedSample.adminData.testState[upDirIndex].testSpeedMax = (*dataEntry);
								speedSample.adminData.testState[upDirIndex].maxSpeedCtr = 0;
							}
							else
								speedSample.adminData.testState[upDirIndex].maxSpeedCtr++;
							speedSample.adminData.testState[upDirIndex].lastSpeedStep = speedSample.adminData.currSpeedStep;

							if (speedSample.adminData.testState[upDirIndex].maxSpeedCtr > noIncreaseLimit)
								speedSample.adminData.testState[upDirIndex].testError |= 0x02;

							bool endTest = (speedSample.adminData.currSpeedStep >= maxSpeedSteps) || (speedSample.adminData.testState[upDirIndex].testError > 0);
//							bool endTest = (speedSample.adminData.currSpeedStep >= maxSpeedSteps) || (speedSample.adminData.testState[upDirIndex].maxSpeedCtr > noIncreaseLimit) || speedSample.adminData.testState[upDirIndex].trackLimitViolation;
							if (!endTest)
							{
								if (speedSample.adminData.testState[upDirIndex].poiIndex == 0) //regular phase
								{
//									Serial.printf("%i %i %f %f\n", speedSample.adminData.currSpeedStep, speedSample.adminData.testSteps, (*dataEntry), speedSample.adminData.testPOI.at(0));
									if ((speedSample.adminData.currSpeedStep < speedSample.adminData.testSteps) && (*dataEntry) <= (1.25 * speedSample.adminData.testPOI.at(0)))
									{
										if ((*dataEntry) < (1.2 * speedSample.adminData.crawlSpeedMax))
											speedSample.adminData.currSpeedStep++;
										else
											speedSample.adminData.currSpeedStep = min(speedSample.adminData.currSpeedStep + 5, maxSpeedSteps);
									}
									else
									{
//										Serial.println("poi incr");
										speedSample.adminData.testState[upDirIndex].poiIndex++; //switch to refining mode
									}
								}
								if (speedSample.adminData.testState[upDirIndex].poiIndex > 0) //currently not doing poi testing
								{
//									int8_t nextStep = getNextPrecStep(&speedSample, forwardDir);
//									if (nextStep > 0)
//										speedSample.adminData.currSpeedStep = nextStep;
//									else
										endTest = true;
//										Serial.println("End Test");
								}
							}
							
							if (!endTest)
							{
								speedSample.adminData.validSample = false;
								sendSpeedCommand(speedSample.adminData.currSpeedStep); //set speed step
								speedSample.adminData.testStartTime = millis();
							}
							else
							{
								speedSample.adminData.testState[upDirIndex].vMaxComplete = true;
								speedSample.adminData.testState[upDirIndex].testPhase = 0;
								speedSample.adminData.masterPhase = 1;
							}
							sendSpeedTableDataToWeb(false);

//							Serial.println(speedSample.adminData.testState[upDirIndex].testPhase);
						}
					break;
				}
				return true;
			}
			break;
			case 1: //slow move to track end
			{
				uint8_t newSpeed = 0;
				float_t distFromOrigin = speedSample.adminData.upDir ? speedSample.adminData.testTrackLen - abs(cpyData.relIntegrator) : speedSample.adminData.upDirPos == 1 ? cpyData.relIntegrator : -cpyData.relIntegrator; //negative if overshooting
//				Serial.printf("Dist: %0.2f %i %i \n", distFromOrigin, speedSample.adminData.upDir, speedSample.adminData.upDirPos);
				if ((distFromOrigin < 0) || (speedSample.adminData.testTrackLen == 0))
				{
					newSpeed = 0;
				}
				else
					newSpeed = speedSample.adminData.testState[upDirIndex].crawlSpeedStep;
				if (newSpeed != speedSample.adminData.currSpeedStep)
				{
					speedSample.adminData.currSpeedStep = newSpeed;
					sendSpeedCommand(speedSample.adminData.currSpeedStep); //set speed step
				}
				if (newSpeed == 0)
					speedSample.adminData.masterPhase++;
			}
			break;
			case 2: //slow down and toggle direction
				toggleDirCommand();
				if (speedSample.adminData.upDir && speedSample.adminData.testState[0].vMaxComplete && speedSample.adminData.testState[1].vMaxComplete)
					return false; //back to origin and both curves complete, so end test
				speedSample.adminData.testStartTime = millis();
				speedSample.adminData.masterPhase++;
			break;
			case 3:
				if ((millis() - speedSample.adminData.testStartTime) > 1000)
					speedSample.adminData.masterPhase = 0; //back to testing
				break;
		}
		return true;
	}
	return true;
}

void IoTT_TrainSensor::programmerReturn(uint8_t * programmerSlot)
{
//									uint16_t cvNr = ((*thisSlot)[6] & 0x7F) + (((*thisSlot)[5] & 0x01) << 7) + (((*thisSlot)[5] & 0x30) << 4);
//									uint8_t cvVal = ((*thisSlot)[7] & 0x7F) + (((*thisSlot)[5] & 0x02) << 6);

	uint16_t opsAddr = (programmerSlot[2]<<7) + (programmerSlot[3] & 0x7F);
	uint16_t cvNr = ((programmerSlot[5] & 0x30)<<4) + ((programmerSlot[5] & 0x01)<<7) + (programmerSlot[6] & 0x7F) + 1;
	uint8_t cvVal = (programmerSlot[7] & 0x7F) + ((programmerSlot[5] & 0x02)<<6);
//	Serial.printf("Prog Stat: %i ps: %i CV: %i Val: %i\n", programmerSlot[1], opsAddr, cvNr, cvVal);
	int8_t currClient = getWSClientByPage(0, "pgPrplHatCfg");
	if (currClient >= 0)
	{
		DynamicJsonDocument doc(200);
		doc["Cmd"] = "ProgReturn";
		JsonObject Data = doc.createNestedObject("Data");
		if (programmerSlot[11] == 0xB4)
		{
			Data["Mode"] = programmerSlot[12];
			programmerSlot[11] = 0;
			programmerSlot[12] = 0;
		}
		else
			Data["Mode"] = 0xFF;
		Data["Status"] = programmerSlot[1];
		Data["OpsAddr"] = opsAddr;
		Data["CVNr"] = cvNr;
		Data["CVVal"]= cvVal;
		char myMQTTMsg[400];
		serializeJson(doc, myMQTTMsg);
//		Serial.println(myMqttMsg);
		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMQTTMsg);
			currClient = getWSClientByPage(currClient + 1, "pgPrplHatCfg");
		}
	}
	else
	{
		refreshRate = 0;
		Serial.println("No web client");
	}
}

void IoTT_TrainSensor::sendSpeedTableDataToWeb(bool isFinal)
{
	int8_t currClient = getWSClientByPage(0, "pgPrplHatCfg");
	if (currClient >= 0)
	{
		DynamicJsonDocument doc(6000);
		doc["Cmd"] = "SpeedTableData";
		JsonObject Data = doc.createNestedObject("Data");
		Data["Dir"] = speedSample.adminData.upDir;
		Data["Mode"] = speedSample.adminData.testState[speedSample.adminData.upDir].poiIndex;
		Data["CurrStep"] = speedSample.adminData.currSpeedStep;
		Data["SlotNr"] = digitraxBuffer->getFocusSlotNr();
		Data["NumSteps"] = speedSample.adminData.testSteps;
		Data["TestError"] = speedSample.adminData.testState[0].testError | speedSample.adminData.testState[1].testError;
		JsonArray fwArray = Data.createNestedArray("fw");
		JsonArray bwArray = Data.createNestedArray("bw");
		if (isFinal)
			Data["final"] = true;
		for (uint8_t i = 0; i < speedSample.adminData.testSteps; i++)
		{
			fwArray.add(speedSample.fw[i]);
			bwArray.add(speedSample.bw[i]);
		}
		char myMQTTMsg[3000];
		serializeJson(doc, myMQTTMsg);
//		Serial.println(strlen(myMQTTMsg));
		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMQTTMsg);
			currClient = getWSClientByPage(currClient + 1, "pgPrplHatCfg");
		}
	}	
	else
	{
		refreshRate = 0;
		Serial.println("No web client");
	}
}	

void IoTT_TrainSensor::sendSensorDataToWeb(uint16_t updateRate)
{
//	sensorData cpyData = getSensorData();
//	for (uint8_t i = 0; i < 50; i++)
//		Serial.println(cpyData.avgMove[i]);
//	Serial.println();
//	Serial.println(cpyData.axisAngle);
	int8_t currClient = getWSClientByPage(0, "pgPrplHatCfg");
	if (currClient >= 0)
	{
		sensorData cpyData = getSensorData();
		DynamicJsonDocument doc(400);
		doc["Cmd"] = "SensorData";
		JsonObject Data = doc.createNestedObject("Data");
		Data["RR"] = updateRate;
		Data["TS"] = cpyData.timeStamp;
		Data["Speed"] = cpyData.currSpeedTech;
		if (updateRate >= 500)
		{
			Data["AbsDist"] = cpyData.absIntegrator;
			Data["RelDist"] = cpyData.relIntegrator;
			Data["AxisAngle"] = cpyData.axisAngle;
			int8_t dirFlag = cpyData.currSpeedTech >= 0 ? 1 : -1;
			Data["Slope"] = dirFlag * getPercOfAngle(cpyData.imuVal[1]);
			Data["Banking"] = dirFlag * getPercOfAngle(cpyData.imuVal[0]);

//			if (imuSensor)
			{
				Data["Radius"] = cpyData.currRadiusTech;
				for (uint8_t i = 0; i < 3; i++)
					Data["PosVect"][i] = cpyData.posVector_mm[i];
				for (uint8_t i = 0; i < 4; i++)
					Data["EulerVect"][i] = cpyData.eulerVectorRad[i];
			}
		}
		int8_t currSlotNr = digitraxBuffer->getFocusSlotNr();
		if (currSlotNr >= 0)
		{
			slotData * currSlot = digitraxBuffer->getSlotData(currSlotNr);
			if (currSlot)
			{
				uint16_t thisAddr = (((*currSlot)[6] & 0x7F) << 7) + ((*currSlot)[1] & 0x7F); //from IoTT_DigitraxBuffers.h
//					Serial.printf("Slot: %i Addr %i\n", currSlotNr, thisAddr);
				if (thisAddr > 0)
				{
					Data["DCCAddr"] = thisAddr;
					Data["SpeedStep"] = (*currSlot)[2]; //current speed step
					Data["DirF"] = (*currSlot)[3]; //current DIRF
					waitForNewDCCAddr = false;
				}
			}
		}
		char myMQTTMsg[800];
		serializeJson(doc, myMQTTMsg);
		while (currClient >= 0)
		{
			globalClients[currClient].wsClient->text(myMQTTMsg);
			currClient = getWSClientByPage(currClient + 1, "pgPrplHatCfg");
		}
		lastWebRefresh += refreshRate;
	}
	else
	{
		refreshRate = 0;
		Serial.println("No web client");
	}
}

void IoTT_TrainSensor::processLoop()
{
	if (refreshRate > 0)
	{
		if ((millis() - refreshRate) > lastWebRefresh)
		{
			sendSensorDataToWeb(refreshRate);
//			Serial.println("sendSensorDataToWeb");
		}
	}
	else
		if (waitForNewDCCAddr)
		{
			int8_t currSlotNr = digitraxBuffer->getFocusSlotNr();
			if (currSlotNr >= 0)
			{
				slotData * currSlot = digitraxBuffer->getSlotData(currSlotNr);
				if (currSlot)
				{
					uint16_t thisAddr = (((*currSlot)[6] & 0x7F) << 7) + ((*currSlot)[1] & 0x7F); //from IoTT_DigitraxBuffers.h
					if (thisAddr > 0)
					{
//						Serial.printf("Slot: %i Addr %i\n", currSlotNr, thisAddr);
						sendSensorDataToWeb(refreshRate);
					}
				}
			}
		}
	
	if (speedSample.adminData.speedTestRunning)
		if (millis() - speedSample.adminData.speedTestTimer > speedTestInterval)
		{
//			Serial.printf("Test %i\n", 0);
			speedSample.adminData.speedTestRunning = processSpeedTest();
			if (!speedSample.adminData.speedTestRunning)
			{
				//process data and send to web client
//				Serial.println("Test complete");
				speedSample.adminData.validSample = false;
				stopTest();
			}
			speedSample.adminData.speedTestTimer += speedTestInterval;
		}
}
