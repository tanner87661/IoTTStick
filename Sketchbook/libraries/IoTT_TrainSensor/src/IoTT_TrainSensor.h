/*
IoTT_SerInjector.h

Based on SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

Adaptation to LocoNet (half-duplex network with DCMA) by Hans Tanner. 
See Digitrax LocoNet PE documentation for more information

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef IoTT_TrainSensor_h
#define IoTT_TrainSensor_h

#include <inttypes.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <IoTT_CommDef.h>
#include <ArduinoJson.h>
#include <IoTT_DigitraxBuffers.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <TMAG5273.h>
//#include <SparkFun_TMAG5273_Arduino_Library.h>
#include <utility/imumaths.h>
#include <M5Unified.h>
//#include <OneDimKalman.h>

extern uint16_t sendMsg(lnTransmitMsg txData);

// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.

//#define lnBusy 0
//#define lnAwaitBackoff 1
//#define lnNetAvailable 2

#define rxBufferSize 64
#define txBufferSize 64
#define verBufferSize 48
#define maxSpeedSteps 127
#define accel2Steps 500 //start sampling
#define maxSampleTime 10000000 //sampling complete after micros()
#define minTestWheelTurns	6 //a successful sampling must last at least x wheel turns
#define crawlTurns 5 //wheel turns per second for crawl speed

#define queBufferSize 50 //messages that can be written in one burst before buffer overflow
#define measuringInterval 5 //ms 200 samples per second
#define sampleBufferSize 50 //last mag readings to build average, max 255
#define speedTestInterval 100 //ms call test function 5 times per second
#define magOverflow 360.0
#define speedChangeTimeout 500 //ms
#define noIncreaseLimit 3 //number of measurements without increasing speed to determine end test

typedef struct 
{
	float_t x;
	float_t y;
} coordPt;

const uint8_t corrLen = 4;
const coordPt linCorr[corrLen] = {0,0,110,150,280,190,360,360};

typedef struct
{
	float sampleArray[sampleBufferSize];
	uint8_t sampleIndex;
	float sampleSum;
} sampleBuffer;

typedef struct 
{
	uint32_t timeStamp = millis();
	float_t currSpeedTech = 0; //[mm/s]
	float_t currRadiusTech = 0; //[m]
	float_t axisAngle = 0; //[deg]
	float_t absIntegrator = 0; //[m] absolute value, direction independent
	float_t relIntegrator = 0; //[m] for pos calc
	bool    currDirFwd = true;
    float_t modScale = 87; //HO by default
    uint8_t dispDim = 0; //metric by default
    char scaleName[5];
	float_t imuVal[6] = {0,0,0,0,0,0};
	float_t eulerVectorRad[4] = {0,0,0,0}; //0: yaw rad  1: roll rad 2: pitch rad 3: distance
	float_t posVector_mm[3] = {0,0,0};	//0 dimX 1: dimY 2: dimZ
//	uint8_t Error = 0;
	float_t avgMove = 0;
	float_t avgDir = 0;
}sensorData;

typedef struct
{
	uint8_t testPhase = 0; //0: not started 1: increasing speeds 2: decreasing speeds 3: end test
    uint32_t sampleTime;
	uint8_t lastSpeedStep = 0;
	uint8_t crawlSpeedStep = 1;
	uint8_t poiIndex = 0; //val 0 = initial table test val 1 = refinement phase
	float_t testSpeedMax = 0;
	uint8_t maxSpeedCtr = 0;
	uint8_t maxTestSpeedStep = 0;
//	bool trackLimitViolation = false;
	uint8_t testError = 0; //0x01: no speed increase  0x02: no step increase
	bool vMaxComplete = false;
}dirData;

typedef struct
{
	bool speedTestRunning = false; //test in progress
	uint8_t masterPhase = 0; //0: test running 1: crawl to end of track 2: reverse direction
	float_t testTrackLen = 0; //available track len in mm
	uint8_t testSteps = 127; //0; // steps by default
	float sampleMinDistance = 150; //mm, calculated based on wheel diameter and # of turns
	bool upDir = true; //forward test
	int8_t upDirPos = -1; //0 false 1 true means positive rel integrator 0xFF undefined
	float_t lastLinIntegrator = 0; //position of last measurement
	uint8_t currSpeedStep = 0; //the current speed test that is sampled. Values from 2 - 127
	uint32_t accelTime = accel2Steps;
	float_t crawlSpeedMax = 30;
	uint32_t speedTestTimer = millis(); //timer to call speedTest from processLoop
	uint32_t testStartTime = millis(); //the time the testSpeed command was sent, used to determin accel end
	uint32_t measureStartTime = micros();
	bool validSample = false; //tell the task to record sample data in speed table
	float_t testStartLinIntegrator = 0; //the distance reading the testSpeed was considered achieved
	std::vector<float> testPOI;
	dirData testState[2]; //0: fw 1: bw encoded by upDir
}testAdminData;

typedef struct
{
	testAdminData adminData;
	float_t fw[maxSpeedSteps];
	float_t bw[maxSpeedSteps];
} speedTable;

extern std::vector<wsClientInfo> globalClients; // a list to hold all clients when in server mode
//extern char* wsTxBuffer; //use the existing buffer 16kb to send mqtt messages

class IoTT_TrainSensor
{
public:
	IoTT_TrainSensor(TwoWire * newWire);
	~IoTT_TrainSensor();
	void begin();
	void processLoop();
	sensorData getSensorData();
	void resetDistance();
	void resetHeading();
	float_t getPercOfAngle(float_t gForce);
	void setTxCallback(txFct newCB);
	void loadLNCfgJSON(DynamicJsonDocument doc);
	volatile void sensorTask(void * thisParam);
	void setRepRate(int newRate);
//	void setRepRate(AsyncWebSocketClient * newClient, int newRate);
	void reqDCCAddrWatch(int16_t dccAddr, bool simulOnly);
//	void reqDCCAddrWatch(AsyncWebSocketClient * newClient, int16_t dccAddr, bool simulOnly);
	void startTest(float_t trackLen, float_t vMax, std::vector<float> pMode);
	void stopTest();
	void sendSpeedCommand(uint8_t newSpeed);
	void toggleDirCommand();
	void programmerReturn(uint8_t * programmerSlot);
   
private:
	void sendSpeedTableDataToWeb(bool isFinal);
//	void sendPosDataToWeb();
	void sendSensorDataToWeb(uint16_t updateRate);
	void clrSpeedTable();
	int8_t getNextPrecStep(speedTable * dataTable, bool forwardDir);
	bool processTestStep(sensorData * sensStatus);
	bool processSpeedTest(); //returns false if complete
	void displayIMUSensorDetails();
	void displayIMUSensorStatus();
	bool proceedToTrackEnd(bool origin);
	bool reverseTestDir();
    TwoWire * sensorWire = NULL;  
    uint8_t magType = 0; 
    uint8_t imuType = 0;
    float_t wheelDia = 10;
    float_t magThreshold = 5;
	SemaphoreHandle_t sensorSemaphore = NULL;
	TaskHandle_t taskHandleSensor = NULL;
	Adafruit_BNO055 * imuSensor = NULL;
	sensors_event_t orientationData;
	float compOrientationData[3] = {0,0,0};
	float headingOffset[3] = {0,0,0};
	float lastOrientationData[3] = {0,0,0};
	uint8_t mySystem, myGyro, myAccel, myMag;
	TMAG5273 * magSensor = NULL;
	uint32_t lastSampleCtrl = millis();
	bool magCalibrated = false;
	bool reverseDir = false;
	int revCtr = 0;
	uint32_t lastSpeedTime = micros();
//	uint32_t lastOverFlow = micros();
//	float_t overflowDistance = 0;
	sensorData workData;
	sensorData dispData;
//	OneDimKalman * speedEstimate = NULL;
	OneDimKalman * relMoveEstimate = NULL;
	uint16_t refreshRate = 0;
	uint32_t lastWebRefresh = millis();
	bool waitForNewDCCAddr = false; //flag to listen for a DCC address
	bool reloadOffset = false; //flag the task to reset the IMU offsets
	uint8_t mountStyle = 0; //flat mount
	speedTable speedSample;
	sampleBuffer avgMove;
   // Member functions
};

//this is the callback function. Provide a function of this name and parameter in your application and it will be called when a new message is received
//extern void onLocoNetMessage(lnReceiveBuffer * recData) __attribute__ ((weak));

#endif
