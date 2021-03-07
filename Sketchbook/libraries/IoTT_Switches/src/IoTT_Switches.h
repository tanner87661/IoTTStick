/*
IoTT_Switches.h


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

#ifndef IoTT_Switches_h
#define IoTT_Switches_h

#include <inttypes.h>
#include <ArduinoJson.h>
#include <Adafruit_PWMServoDriver.h>
#include <IoTT_DigitraxBuffers.h>
#include <IoTTCommDef.h>
#include <IoTT_ButtonTypeDef.h>
#include <IoTT_RemoteButtons.h> //as introduced in video # 29, special version for Wire connection via 328P
#include <IoTT_LocoNetButtons.h> //as introduced in video # 29
#include <IoTT_LEDChain.h> //as introduced in video # 30

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

enum driveType : byte {dualcoilAC=0, dualcoilDC=1, bipolarcoil=2, stallmotor=3, rcservo=4, dcmotor=5};
enum greenHatType : byte {comboModule=1, servoModule=0};

const uint16_t refreshInterval = 5000; //microseconds
const uint8_t ledChainAddr = 0x33;
const uint8_t ioExtAddr = 0x06;
const uint8_t pwmDriverAddr = 0x43;
const uint8_t ghInterval = 5; //ms delay between calls to process function
const uint8_t wdtInterval = 100; //ms delay between calls to process function

#define minPos 210
#define initPos 215

typedef struct
{
	bool    isUsed;
	uint8_t aspectID;
	int16_t aspectPos;
	uint8_t moveCfg;
}aspectEntry;

class IoTT_SwitchList;
class IoTT_GreenHat;

class IoTT_SwitchBase
{
public:
	IoTT_SwitchBase();
	~IoTT_SwitchBase();
	void freeObjects();
	void begin(IoTT_GreenHat * ownerObj, uint8_t listIndex);
	virtual void processExtEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue);
	virtual void processSwitch();
	void processServo(uint8_t servoNr);
	void processServoSimple();
	void processServoComplex();
	virtual void loadSwitchCfgJSON(JsonObject thisObj);
   
private:
   
   // Member functions
public:
   // Member variables
	uint8_t modIndex = 0;
	driveType driverType = dualcoilAC;
	sourceType srcType = evt_trackswitch;
	uint8_t switchAddrListLen = 0;
	uint16_t * switchAddrList = NULL;
	uint8_t condDataListLen = 0;
	uint16_t * condDataList = NULL;
	uint16_t accelRate = 1500;
	uint16_t decelRate = 1500;
	uint16_t upSpeed = 320;
	uint16_t downSpeed = 320;
	uint8_t lambda = 3;
	uint8_t frequency = 5;
	uint16_t hesPoint = 550;
	uint16_t hesSpeed = 0;
	uint16_t activationTime = 1500; //ms, used for coil based drivers
	uint8_t aspectListLen = 0;
	aspectEntry * aspectList = NULL;
	
//runtime variables
	uint8_t  currMoveMode; //runtime data 0: at target; 1: accelerating; 2: linear movememnt; 3: hesitating; 4: stopping
	uint16_t  extSwiPos = 0xFFFF;
//	uint16_t targetPos = minPos;
	aspectEntry * targetMove = NULL;
	float_t currentPos = initPos;
	uint32_t nextMoveWait = refreshInterval;
	uint32_t lastMoveTime = micros();
	float_t currSpeed = 0;
	uint32_t timeNull; //runtime data
public:
	IoTT_GreenHat * parentObj = NULL;
};

class IoTT_ServoDrive: public IoTT_SwitchBase
{
public:
	IoTT_ServoDrive();
	~IoTT_ServoDrive();
	virtual void processExtEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue);
	virtual void processSwitch();
	virtual void loadSwitchCfgJSON(JsonObject thisObj);
private:
};

class IoTT_ComboDrive: public IoTT_SwitchBase
{
public:
	IoTT_ComboDrive();
	~IoTT_ComboDrive();
	virtual void processSwitch();
	virtual void loadSwitchCfgJSON(JsonObject thisObj);
private:
};

class IoTT_GreenHat
{
public:
	IoTT_GreenHat();
	~IoTT_GreenHat();
	void freeObjects();
	void begin(IoTT_SwitchList * ownerObj, uint8_t listIndex);
	void setGreenHatType(greenHatType newType);
	void loadGreenHatCfgJSON(uint8_t fileNr, JsonObject thisObj, bool resetList);
	void processSwitch();
	void processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue);
	void setPWMValue(uint8_t lineNr, uint16_t pwmVal);
	bool isVerified();
	void moveServo(uint8_t servoNr, uint16_t servoPos);
private:
	Adafruit_PWMServoDriver * ghPWM = NULL;
	IoTT_SwitchBase ** switchModList = NULL;
	uint16_t switchModListLen = 0;
	IoTT_Mux64Buttons * myButtons = NULL;
	IoTT_LocoNetButtonList * buttonHandler = NULL; 
	IoTT_ledChain * myChain = NULL;
	greenHatType modType = servoModule;
	uint8_t hatIndex = 0;
	uint8_t oddCtr = 0;
	uint32_t ghUpdateTimer = millis() + ghInterval;
	uint32_t wdtResetTime = millis() + wdtInterval;
	int16_t startUpCtr = 50;
public:
	IoTT_SwitchList * parentObj = NULL;
};

class IoTT_SwitchList
{
public:	
	IoTT_SwitchList();
	~IoTT_SwitchList();
	void freeObjects();
	void begin(TwoWire * newWire);
	void setMQTTMode(mqttTxFct txFct);
	void setLocalMode();
	void processLoop();
	void processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue);
	void configModMem(uint8_t numModules);
	void setGreenHatType(uint8_t modNr, greenHatType modType);
	void loadSwCfgJSON(uint8_t ghNr, uint8_t fileNr, DynamicJsonDocument doc, bool resetList = true);
	bool isVerified();
	void moveServo(uint8_t servoNr, uint16_t servoPos);
private:
	IoTT_GreenHat ** greenHatList = NULL;
	uint16_t greenHatListLen = 0;
public:
	TwoWire * swiWire = NULL;
};


#endif
