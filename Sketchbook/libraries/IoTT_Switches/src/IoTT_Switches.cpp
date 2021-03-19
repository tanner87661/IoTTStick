/*

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
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

#include <IoTT_Switches.h>
#include <IoTT_DigitraxBuffers.h>
#include <Wire.h>

uint32_t currMem = ESP.getFreeHeap();

/*
void snapMem()
{
	currMem = ESP.getFreeHeap();
}

void reportMem(String Label, uint16_t numRecs)
{
	uint32_t newMem = currMem;
	snapMem();
	Serial.printf("Allocated %i bytes for %i * %s \n", newMem - currMem, numRecs, Label);
} 
*/

sourceType getSourceTypeByNameSW(String transName)
{
  if (transName == "button") return evt_button;
  if (transName == "analogvalue") return evt_analogvalue;
  if (transName == "switch") return evt_trackswitch;
  if (transName == "dccsignal") return evt_signalmastdcc;
  if (transName == "dynsignal") return evt_signalmastdyn;
  if (transName == "blockdetector") return evt_blockdetector;
  if (transName == "transponder") return evt_transponder;
  if (transName == "power") return evt_transponder;
  return evt_nosource;
};

driveType getDriveTypeByName(String transName)
{
  if (transName == "dualcoilAC") return dualcoilAC;
  if (transName == "dualcoilDC") return dualcoilDC;
  if (transName == "bipolarcoil") return bipolarcoil;
  if (transName == "stallmotor") return stallmotor;
  if (transName == "rcservo") return rcservo;
  if (transName == "dcmotor") return dcmotor;
  return dualcoilAC;
}

greenHatType getGreenHatTypeByName(String transName)
{
  if (transName == "servo") return servoModule;
  if (transName == "combo") return comboModule;
  return comboModule; 
}


IoTT_SwitchBase::IoTT_SwitchBase() 
{
//	Serial.println("construct Switch Object");
}

IoTT_SwitchBase::~IoTT_SwitchBase() 
{
	freeObjects();
}

void IoTT_SwitchBase::freeObjects()
{
	Serial.println("Free Switch Object");
}

void IoTT_SwitchBase::begin(IoTT_GreenHat * ownerObj, uint8_t listIndex) 
{
	parentObj = ownerObj;
	modIndex = listIndex;
}

void IoTT_SwitchBase::loadSwitchCfgJSON(JsonObject thisObj)
{
//	Serial.println("IoTT_SwitchBase loadSwitchCfgJSON");
//	if (thisObj.containsKey("DriveType"))
//		driverType = getDriveTypeByName(thisObj["DriveType"]);
	if (thisObj.containsKey("CmdSource"))
		srcType = getSourceTypeByNameSW(thisObj["CmdSource"]);
	if (thisObj.containsKey("Addr"))
	{
		JsonArray thisAddrList = thisObj["Addr"];
		switchAddrListLen = thisAddrList.size();
		switchAddrList = (uint16_t*) realloc (switchAddrList, switchAddrListLen * sizeof(uint16_t));
		for (uint8_t i = 0; i < switchAddrListLen; i++)
			switchAddrList[i] = thisAddrList[i];
	}
	if (thisObj.containsKey("CondData"))
	{
		JsonArray thisCondDataList = thisObj["CondData"];
		condDataListLen = thisCondDataList.size();
		condDataList = (uint16_t*) realloc (condDataList, condDataListLen * sizeof(uint16_t));
		for (uint8_t i = 0; i < condDataListLen; i++)
			condDataList[i] = thisCondDataList[i];
	}
	if (thisObj.containsKey("AccelRate"))
		accelRate = thisObj["AccelRate"];
	if (thisObj.containsKey("DecelRate"))
		decelRate = thisObj["DecelRate"];
	if (thisObj.containsKey("UpSpeed"))
		upSpeed = thisObj["UpSpeed"];
	if (thisObj.containsKey("DownSpeed"))
		downSpeed = thisObj["DownSpeed"];
	if (thisObj.containsKey("Lambda"))
		lambda = thisObj["Lambda"];
	if (thisObj.containsKey("Frequency"))
		frequency = thisObj["Frequency"];
	if (thisObj.containsKey("HesPoint"))
		hesPoint = thisObj["HesPoint"];
	if (thisObj.containsKey("HesSpeed"))
		hesSpeed = thisObj["HesSpeed"];
	if (thisObj.containsKey("PowerOff"))
		endMovePwrOff = thisObj["PowerOff"];



//	if (thisObj.containsKey("ActivationTime"))
//		activationTime = thisObj["ActivationTime"];
	currentPos = initPos;
	if (thisObj.containsKey("Positions"))
	{
		JsonArray thisParams = thisObj["Positions"];
        aspectListLen = thisParams.size();
        if (aspectListLen > 0)
			aspectList = (aspectEntry*) realloc (aspectList, aspectListLen * sizeof(aspectEntry));
		currentPos = 0;
        for (int i=0; i < aspectListLen; i++)
        {
			aspectList[i].isUsed = thisParams[i]["Used"];
			aspectList[i].aspectID = thisParams[i]["AspVal"];
			aspectList[i].aspectPos = thisParams[i]["PosPt"];
			aspectList[i].moveCfg = thisParams[i]["MoveCfg"];
			currentPos += (aspectList[i].aspectPos / aspectListLen);
		}
		Serial.printf("Init %i with %f\n", switchAddrList[0], currentPos);
	}
}

void IoTT_SwitchBase::processExtEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
}

void IoTT_SwitchBase::processSwitch()
{
/*
	switch (thisDrive)
	{
		case dualcoilAC:
			break;
		case dualcoilDC:
			break;
		case bipolarcoil:
			break;
		case stallmotor:
			break;
		case rcservo:
			break;
		case dcmotor:
			break;
	}
	
*/
}

void IoTT_SwitchBase::processServoComplex()
{
	if (currMoveMode == 0) //linear movement mode
	{
		uint16_t currPos = round(currentPos);
		if (targetMove)
		{
			if (targetMove->aspectPos != currPos)
			{
				if (microsElapsed(lastMoveTime) > nextMoveWait)
				{
					lastMoveTime = micros();
					//analyze the current status for further decision making
					bool moveUp = targetMove->aspectPos > currPos;
					uint8_t adjMode = moveUp ? (targetMove->moveCfg & 0x0F) : ((targetMove->moveCfg & 0xF0) >> 4);
					float_t linSpeed = moveUp ? (float_t) upSpeed : (float_t)(-1) * downSpeed; //set linSpeed to incr/s
					bool correctDir = (sgn(currSpeed) == sgn(linSpeed)) || (currSpeed == 0.0);
					bool softStop = ((adjMode & 0x03) == 1); //soft stop
					bool softStart = (adjMode & 0x04); //overshoot or soft stop
					bool beforeHesitate = adjMode & 0x08 ? (moveUp ? currPos < hesPoint : currPos > hesPoint) : false; 
					uint16_t moveEndPoint = beforeHesitate ? hesPoint : targetMove->aspectPos; //end point or hesitate
					float moveEndSpeed = (beforeHesitate ? (sgn(linSpeed) * hesSpeed) : (adjMode & 0x02 ? (linSpeed) : (adjMode == 0 ? (linSpeed) : (0)))); //speed when arriving at end point, incr/s
					int32_t breakDistance = round(sq(linSpeed - moveEndSpeed) / (2 * (int32_t)decelRate)); //s = v2 /2a
					uint16_t breakPoint = moveEndPoint - (sgn(linSpeed) * breakDistance);
					bool beforeBreakPoint = ((int)(breakPoint - currPos) * (int)sgn(linSpeed)) > 0;

					//calculate dynamic data for step duration and width 
					uint32_t stepDelay = currSpeed == 0 ? refreshInterval : round(1000000 / abs(currSpeed)); //calculating the duration of 1 step in micros/incr
					float stepFactor = currSpeed == 0 ? 1 : 1 + (refreshInterval / stepDelay);  //calculate how many steps to take assuming 5ms cycle time

//					Serial.printf("Correct Dir: %i Lin Speed %f Curr Sp %f before break: %i \n", correctDir, linSpeed, currSpeed, beforeBreakPoint);
					if (correctDir)
					{
						if (linSpeed != 0)
						{
							if (beforeBreakPoint)
							{
								if (abs(currSpeed) < abs(linSpeed))
								{
									if (softStart)
									//accelerate
										currSpeed += round((sgn(linSpeed) * stepFactor * (stepDelay * (int32_t)accelRate) / 1000000)); //v = v0 + at
									else
										currSpeed = linSpeed;
									if (abs(currSpeed) > abs(linSpeed))
										currSpeed = linSpeed;
								}
								//keep moving
								currentPos += round(stepFactor * sgn(linSpeed)); //set the position
								nextMoveWait = round(stepFactor * stepDelay); //set the delay time
							}
							else
							{
								if (currPos == targetMove->aspectPos) //final position
								{
									currentPos = targetMove->aspectPos;
									nextMoveWait = refreshInterval; //1ms wait                
								}
								else
								{
									//accel-/decel to moveEndSpeed
									bool endAccel = false;
									if (abs(moveEndSpeed) > abs(currSpeed))
									//accelerate
									{
										//add comparison to target speed at position, adjust acceleration if needed
										currSpeed += round((sgn(linSpeed) * stepFactor * (stepDelay * (int32_t)accelRate) / 1000000)); //v = v0 + at
										if (abs(moveEndSpeed) <= abs(currSpeed))
											currSpeed = moveEndSpeed;
										endAccel = currSpeed == moveEndSpeed;
									}
									else
									//decelerate
									{
										//add comparison to target speed at position, adjust deceleration if needed
										int sgnSpeed = sgn(currSpeed);
										currSpeed -= round((sgn(linSpeed) * stepFactor * (stepDelay * (int32_t)decelRate) / 1000000)); //v = v0 - at
										if (sgn(currSpeed) != sgnSpeed)
											currSpeed = 0;
										endAccel = currSpeed == 0;
									}
									//advance to moveEndPoint
									if (endAccel)
									{
										currentPos = moveEndPoint;
										nextMoveWait = refreshInterval; //1ms wait                
									}
									else
									{
										currentPos += round(stepFactor * sgn(linSpeed)); //set the position
										nextMoveWait = round(stepFactor * stepDelay); //set the delay time
									}
								}
							}
							//when there, execute move end
							bool currMoveUp = targetMove->aspectPos > currentPos;
							if ((currentPos == targetMove->aspectPos) || (moveUp != currMoveUp)) //overshooting when speed > 1 incr per cycle
								if ((adjMode & 0x03) > 1) //bounce back or overshooting
								{
									currentPos = targetMove->aspectPos;
									currMoveMode = 1; //enter oscillation phase
									timeNull = micros();
								}
								else
									currSpeed = 0;
						}
						else
						{
							currentPos = targetMove->aspectPos;
							nextMoveWait = 0;
						}
					}
					else
					{
						if (softStop)
						{
							//decelerate and change direction. Speed is incr/s, accel/decel is incr/s2, time intervl is 1ms
							int sgnSpeed = sgn(currSpeed);
							currSpeed += round((sgn(linSpeed) * stepFactor * (stepDelay * (int32_t)decelRate) / 1000000)); //v = v0 - at
							if (sgn(currSpeed) != sgnSpeed)
							currSpeed = 0;
						}
						else
							currSpeed = 0;
						if (currSpeed != 0)
						{
							currentPos += round(stepFactor * 64 * sgn(linSpeed)); //set the position
							nextMoveWait = round(stepFactor * stepDelay); //set the delay time
						}
						else
							nextMoveWait = refreshInterval; //standard 1ms wait
					}
					parentObj->setPWMValue(modIndex, round(currentPos));
//					endMoveTimeout = millis() + endMoveDelay;
				}
			}
			else
			{
				if ((endMovePwrOff) && (endMoveTimeout < millis()) && (endMoveTimeout > 0))
				{
					endMoveTimeout = 0;
					parentObj->setPWMValue(modIndex, 0);
				}
			}
		}
//		else
//		{
//			if ((endMovePwrOff) && (endMoveTimeout < millis()))
//				parentObj->setPWMValue(modIndex, 0);
//		}
	}
	else //oscillator mode
	{
		bool moveUp = currentPos == targetMove->aspectPos;
		uint8_t adjMode = moveUp ? (targetMove->moveCfg & 0x0F) : ((targetMove->moveCfg & 0xF0) >> 4);
		float thisLambda = (float)((moveUp ? (lambda & 0x0F) : ((lambda & 0xF0) >> 4)) + 1) / 2;
		float thisFreq = (float)((moveUp ? (frequency & 0x0F) : ((frequency & 0xF0) >> 4)) + 1) / 2;
		float timePassed2 = (float)(micros() - timeNull) / 1000;
		float timePassed = timePassed2 / 1000;
		//calculate y(t)
		float origAmpl = currSpeed / (TWO_PI * thisFreq);
		float currAmpl = origAmpl  * exp(thisLambda * timePassed * -1);
		float currVal = round(currAmpl * sin(TWO_PI * thisFreq * timePassed));
		uint16_t pwmVal = currentPos; //moveUp ? (uint16_t)SERVOMAX : (uint16_t)SERVOMIN;

		if ((currAmpl / origAmpl) > 0.1) //stop oscillator if amplitude < 10% of original value
		{
			//PWM to targetPos
			if ((adjMode & 0x03) == 3) //bounce back
				//PWM to targetPos + newAmpl
				pwmVal = pwmVal + currVal;
			else //2, overshoot
				//PWM to targetPos - abs(newAmpl)
				pwmVal = moveUp ? pwmVal - abs(currVal) : pwmVal + abs(currVal);
			parentObj->setPWMValue(modIndex, pwmVal);
//			endMoveTimeout = millis() + endMoveDelay;
		} 
		else //done, back to linear mode
		{
			if (endMovePwrOff)
				parentObj->setPWMValue(modIndex, 0);
			else
				parentObj->setPWMValue(modIndex, round(currentPos));
			currMoveMode = 0; 
			currSpeed = 0; 
//			endMoveTimeout = millis() + endMoveDelay;
		}
		nextMoveWait = refreshInterval; //standard 1ms wait
	}
}

void IoTT_SwitchBase::processServoSimple()
{
	if (targetMove)
		if (targetMove->aspectPos != currentPos)
		{
			if (microsElapsed(lastMoveTime) > nextMoveWait)
			{
				lastMoveTime = micros();
				bool moveUp = targetMove->aspectPos > currentPos;
				uint16_t stepSpeed = moveUp ? upSpeed : downSpeed; //incr/s 
				if (stepSpeed > 0) //valid speed settings
				{
					uint32_t stepDelay = round(1000000 / stepSpeed); //calculating the duration of 1 step
					float stepFactor = 1 + (refreshInterval / stepDelay);  //calculate how many steps to take assuming 5ms cycle time
					currentPos += stepFactor * (moveUp ? 1 : (-1)); //set the position
					nextMoveWait = round(stepFactor * stepDelay); //set the delay time
					bool nextDir = targetMove->aspectPos > round(currentPos);
					if (moveUp != nextDir) //reached targetPos, so break the movement
					{
						currentPos = targetMove->aspectPos; //make sure to stop in case of overshoot nextMoveWait = refreshInterval;
					}
				}
				else //no settings, go with maximum speed to the target
				{
					currentPos = targetMove->aspectPos; //make sure to stop in case of overshooting
					nextMoveWait = refreshInterval;
				}
				parentObj->setPWMValue(modIndex, round(currentPos));
//				endMoveTimeout = millis() + endMoveDelay;
			}
		}
		else
			if ((endMovePwrOff) && (endMoveTimeout < millis()))
				parentObj->setPWMValue(modIndex, 0);
}

//----------------------------------------------------------------------------------------------------------------
IoTT_ServoDrive::IoTT_ServoDrive():IoTT_SwitchBase()
{
	
}
	
IoTT_ServoDrive::~IoTT_ServoDrive()
{
}

void IoTT_ServoDrive::processExtEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
	switch (inputEvent)
	{
		case evt_transponder:
		{
			if (switchAddrList[0] == btnAddr)
			{
				uint16_t locoAddr = eventValue & 0x7FFF;
				uint8_t  evtType = (eventValue & 0x8000) >> 15;
				for (uint8_t i = 0; i < condDataListLen; i++)
					if (condDataList[i] == locoAddr)
					{
						targetMove = &aspectList[evtType];
						break;
					}
			}
			break;
		}
	}
	if (targetMove)
		if (targetMove->moveCfg == 0)
			processServoSimple();
		else
			processServoComplex();
}

void IoTT_ServoDrive::processSwitch()
{
	switch (srcType)
	{
		case evt_trackswitch:
		{
			uint16_t swiStatus = 0;
			for (int8_t i = (switchAddrListLen-1); i >= 0; i--) //check for the latest position
				swiStatus = (2 * swiStatus) + ((getSwiPosition(switchAddrList[i]) >> 5) & 0x01);
			if ((swiStatus != extSwiPos) && aspectList[swiStatus].isUsed)
			{
				targetMove = &aspectList[swiStatus];
				extSwiPos = swiStatus;
//				Serial.printf("Updating Static Switch %i Addr  for Status %i  \n", switchAddrListLen, swiStatus);
			}
			break;
		}
		case evt_signalmastdyn:
		{
			uint32_t hlpAct, lastAct = 0;
			uint16_t swiStatus = 0;
			uint8_t dynSwi = 0;
			for (uint8_t i = 0; i < switchAddrListLen; i++) //check for the latest activity
			{
				hlpAct = getLastSwitchActivity(switchAddrList[i]);
				if (hlpAct > lastAct)
				{
					dynSwi = i;
					lastAct = hlpAct;
				}
			}
			if (lastAct != 0) //we have activity
			{
				uint8_t aspectNr = dynSwi * 2;
				if (((getSwiPosition(switchAddrList[dynSwi]) >> 4) & 0x02) > 0)
					aspectNr++; //this is the final aspect #
				if ((extSwiPos != aspectNr)  && aspectList[aspectNr].isUsed)
				{
					targetMove = &aspectList[aspectNr];
					extSwiPos = aspectNr;
					Serial.printf("Updating Aspect %i  for Switch %i\n", aspectNr, switchAddrList[dynSwi]);
				}
			}
			break;
		}
		case evt_signalmastdcc:
			if (getSignalAspect(switchAddrList[0]) != extSwiPos)
			{
				uint16_t newSwiPos = getSignalAspect(switchAddrList[0]);
				int16_t nextVal = -1;
				int16_t nextInd = -1;
				uint8_t distance = 0;
				for (int i = 0; i < aspectListLen; i++)
				{
					if ((newSwiPos <= aspectList[i].aspectID) && (newSwiPos > nextVal))
					{
						nextInd = i;
						nextVal = aspectList[i].aspectID;
					}
				}
				if ((nextInd != extSwiPos) && aspectList[nextInd].isUsed)
				{
					targetMove = &aspectList[nextInd];
					extSwiPos = nextInd;
					Serial.printf("Process Signal %i to Aspect %i Pos %i \n", switchAddrList[0], extSwiPos, targetMove->aspectPos);
				}
			}
			break;
		case evt_analogvalue:
			if (getAnalogValue(switchAddrList[0]) != extSwiPos)
			{
				extSwiPos = getAnalogValue(switchAddrList[0]);
				if (((aspectList[1].isUsed) || (aspectList[2].isUsed)) && (condDataListLen > 0))  //use individual aspects
				{
					switch (extSwiPos)
					{
						case 0:	
							targetMove = &aspectList[0];
							break;
						case 4095:
							targetMove = &aspectList[3];
							break;
						default:
							if (extSwiPos < condDataList[0])
								if (aspectList[1].isUsed)
									targetMove = &aspectList[1];
								else ;
							else
								if (aspectList[2].isUsed)
									targetMove = &aspectList[2];
								else ;
							break;
					}
				}
				else
					if ((aspectList[0].isUsed) && (aspectList[3].isUsed))
					{
						float_t deltaVal = extSwiPos * (aspectList[3].aspectPos - aspectList[0].aspectPos) / 4095;
						aspectList[1].aspectPos = round(deltaVal >= 0 ? aspectList[0].aspectPos + deltaVal: aspectList[3].aspectPos + deltaVal);
						aspectList[1].moveCfg = 0;
						targetMove = &aspectList[1];
						Serial.printf("Process Analog Inp %i to linear Value %i -> %i\n", switchAddrList[0], extSwiPos, aspectList[1].aspectPos);
					}
			}
			break;
		case evt_button: 
			if (getButtonValue(switchAddrList[0]) != extSwiPos)
			{
				if (aspectList[extSwiPos].isUsed)
				{
					extSwiPos = getButtonValue(switchAddrList[0]);
					targetMove = &aspectList[extSwiPos];
					Serial.printf("Process Button %i to Status %i \n", switchAddrList[0], extSwiPos);
				}
			}
			break;
		case evt_blockdetector:
		{
			uint16_t swiStatus = 0;
			for (int8_t i = (switchAddrListLen-1); i >= 0; i--) //check for the latest position
				swiStatus = (2 * swiStatus) + (getBDStatus(switchAddrList[i]) & 0x01);
			if ((swiStatus != extSwiPos) && aspectList[swiStatus].isUsed)
			{
				targetMove = &aspectList[swiStatus];
				extSwiPos = swiStatus;
				Serial.printf("Updating Block Detector %i Sensors  for Status %i  \n", switchAddrListLen, swiStatus);
			}
			break;
		}
		case evt_transponder:
			break;
		default:
//			Serial.printf("Call switchtype %i \n", srcType);
			break;
	}
	if (targetMove)
		if (targetMove->moveCfg == 0)
			processServoSimple();
		else
			processServoComplex();
}

void IoTT_ServoDrive::loadSwitchCfgJSON(JsonObject thisObj)
{
	IoTT_SwitchBase::loadSwitchCfgJSON(thisObj);
//	Serial.println("IoTT_ServoDrive loadSwitchCfgJSON");
}

//----------------------------------------------------------------------------------------------------------------
IoTT_ComboDrive::IoTT_ComboDrive():IoTT_SwitchBase()
{
}

IoTT_ComboDrive::~IoTT_ComboDrive()
{
}

void IoTT_ComboDrive::processSwitch()
{
}

void IoTT_ComboDrive::loadSwitchCfgJSON(JsonObject thisObj)
{
	IoTT_SwitchBase::loadSwitchCfgJSON(thisObj);
//	Serial.println("IoTT_ComboDrive loadSwitchCfgJSON");
}


//----------------------------------------------------------------------------------------------------------------
IoTT_GreenHat::IoTT_GreenHat()
{
}

IoTT_GreenHat::~IoTT_GreenHat()
{
}

void IoTT_GreenHat::begin(IoTT_SwitchList * ownerObj, uint8_t listIndex)
{
	parentObj = ownerObj;
	hatIndex = listIndex;
//	ledAddr = 0x30 - hatIndex;
	ghPWM = new Adafruit_PWMServoDriver(pwmDriverAddr - hatIndex, *parentObj->swiWire);
	ghPWM->begin();
	delay(10);
	ghPWM->setPWMFreq(92); // Analog servos run at ~50 - 200 Hz updates. 92 is the closest I get to 100Hz measured
    for (int i=0; i < 16; i++)
    {
		ghPWM->setPWM(i, 0, 210);
		delay(10);
	}
    myButtons = new IoTT_Mux64Buttons();
	myButtons->initButtonsI2C(&Wire, ioExtAddr - (2 * hatIndex), NULL, true); //use WiFi with buttons (always). ok for pin 36
	buttonHandler = new IoTT_LocoNetButtonList(); 

    myChain = new IoTT_ledChain(&Wire, ledChainAddr, false);// - hatIndex); // set for using I2C Bus address 0x18
    
}

void IoTT_GreenHat::freeObjects()
{
	delete ghPWM;
	delete switchModList; //this should automatically call the destructor of each obj
}

void IoTT_GreenHat::setGreenHatType(greenHatType newType)
{
	modType = newType;
}

void IoTT_GreenHat::loadGreenHatCfgJSON(uint8_t fileNr, JsonObject thisObj, bool resetList)
{
//		"CfgFiles": ["gh/0/switches", "gh/0/btn", "gh/0/btnevt", "gh/0/led"]
	
//	Serial.printf("IoTT_GreenHat::loadGreenHatCfgJSON File Nr %i Reset %i \n", fileNr, resetList);
	
	switch (fileNr)
	{
		case 0: //switches
			//if (thisObj.containsKey("ModuleType"))
			//	modType = getGreenHatTypeByName(thisObj["ModuleType"]);
			if (thisObj.containsKey("Drivers"))
			{
				JsonArray driverMods = thisObj["Drivers"];
				uint16_t newListLen = driverMods.size();
				switchModList = (IoTT_SwitchBase**) realloc (switchModList, (newListLen + switchModListLen) * sizeof(IoTT_SwitchBase*));
				for (int i=0; i < newListLen; i++)
				{
					IoTT_SwitchBase * thisSwiMod = NULL;
					if (modType == servoModule)
						thisSwiMod = new(IoTT_ServoDrive);
					else
						thisSwiMod = new(IoTT_ComboDrive);
					thisSwiMod->begin(this, i);
					thisSwiMod->loadSwitchCfgJSON(driverMods[i]);
					switchModList[switchModListLen + i] = thisSwiMod;
				}
				switchModListLen += newListLen;
			}
			break;
		case 1: //buttons
			if (thisObj.containsKey("Buttons"))
				if (myButtons != NULL)
					myButtons->loadButtonCfgI2CJSONObj(thisObj);
				else
					Serial.println("No Buttons defined");
			else
				Serial.println("No Buttons");
			break;
		case 2: //button handler
			if (thisObj.containsKey("ButtonHandler"))
				if (buttonHandler != NULL)
					buttonHandler->loadButtonCfgJSON(thisObj);
				else
					Serial.println("No Button Handler defined");
			else
				Serial.println("No Button Handler");
			break;
		case 3: //LED
			if (thisObj.containsKey("LEDDefs"))
				if (myChain != NULL)
					myChain->loadLEDChainJSONObj(thisObj);
				else
					Serial.println("No Chain defined");
			else
				Serial.println("No LEDChain");
			break;
	}
}

void IoTT_GreenHat::setPWMValue(uint8_t lineNr, uint16_t pwmVal)
{
//	Serial.printf("Process Servo %i to %i\n", lineNr, pwmVal);
//	yield();
	ghPWM->setPWM(lineNr, 0, pwmVal);
	IoTT_SwitchBase * thisSwiMod = switchModList[lineNr];
	if (pwmVal > 0)
		thisSwiMod->endMoveTimeout = millis() + endMoveDelay;
}

void IoTT_GreenHat::processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
	if (buttonHandler) 
		buttonHandler->processBtnEvent(inputEvent, btnAddr, eventValue); //drives the outgoing buffer and time delayed commands
	if (inputEvent == evt_transponder)
		for (int i=0; i < switchModListLen; i++)
		{
			IoTT_SwitchBase * thisSwiMod = switchModList[i];
			thisSwiMod->processExtEvent(inputEvent, btnAddr, eventValue); //transponder info is not buffered, so we process the event
		}
	
}

bool IoTT_GreenHat::isVerified()
{
	if (myChain)
		return myChain->isVerified();
	else
		return false;
}

void IoTT_GreenHat::processSwitch()
{
	startUpCtr = max(startUpCtr-1,0);
	if (startUpCtr == 0)
	{
		for (int i=0; i < switchModListLen; i++)
		{
			IoTT_SwitchBase * thisSwiMod = switchModList[i];
			thisSwiMod->processSwitch();
		}
		if (buttonHandler) 
			buttonHandler->processButtonHandler(); //drives the outgoing buffer and time delayed commands
		if (myChain)
		{
			if (myChain->isVerified())
				if (myButtons)
					myButtons->processButtons();
			if (millisElapsed(wdtResetTime) >= wdtInterval)
			{
				myChain->resetI2CWDT();
				wdtResetTime = millis();
			}
			myChain->processChain();
		}
	}
}

void IoTT_GreenHat::moveServo(uint8_t servoNr, uint16_t servoPos)
{
	Serial.printf("Servo Nr %i to Pos %i\n", servoNr, servoPos);
	setPWMValue(servoNr, servoPos);
}

//----------------------------------------------------------------------------------------------------------------
IoTT_SwitchList::IoTT_SwitchList()
{
//	Serial.println("construct Switchlist Object");
}

IoTT_SwitchList::~IoTT_SwitchList()
{
	freeObjects();
}

void IoTT_SwitchList::freeObjects()
{
	if (greenHatList)
	{
		delete greenHatList; //this should automatically call the destructor of each obj
		greenHatList = NULL;
		greenHatListLen = 0;
	}
}

void IoTT_SwitchList::begin(TwoWire * newWire)
{
	swiWire = newWire;
}

void IoTT_SwitchList::setMQTTMode(mqttTxFct txFct)
{
}

void IoTT_SwitchList::setLocalMode()
{
}

void IoTT_SwitchList::processLoop()
{
	for (int i=0; i < greenHatListLen; i++)
		greenHatList[i]->processSwitch();
}

bool IoTT_SwitchList::isVerified()
{
	bool verifyOK = true;
	for (int i=0; i < greenHatListLen; i++)
		verifyOK &= greenHatList[i]->isVerified();
	return verifyOK;
}

void IoTT_SwitchList::processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
	for (int i=0; i < greenHatListLen; i++)
		greenHatList[i]->processBtnEvent(inputEvent, btnAddr, eventValue);
}

void IoTT_SwitchList::configModMem(uint8_t numModules)
{
	freeObjects();
	greenHatList = (IoTT_GreenHat**) realloc (greenHatList, numModules * sizeof(IoTT_GreenHat*));
	greenHatListLen = numModules;
    for (int i=0; i < greenHatListLen; i++)
    {
		IoTT_GreenHat * thisGreenHatEntry = new(IoTT_GreenHat);
		thisGreenHatEntry->begin(this, i);
		greenHatList[i] = thisGreenHatEntry;
	}
}

void IoTT_SwitchList::setGreenHatType(uint8_t modNr, greenHatType modType)
{
	greenHatList[modNr]->setGreenHatType(modType);
}

void IoTT_SwitchList::loadSwCfgJSON(uint8_t ghNr, uint8_t fileNr, DynamicJsonDocument doc, bool resetList)
{
	Serial.printf("IoTT_SwitchList::loadSwCfgJSON Module %i File %i\n", ghNr, fileNr);
	JsonObject thisObj = doc.as<JsonObject>();
	greenHatList[ghNr]->loadGreenHatCfgJSON(fileNr, thisObj, resetList);
}

void IoTT_SwitchList::moveServo(uint8_t servoNr, uint16_t servoPos)
{
	uint8_t ghNr = trunc(servoNr / 16);
	Serial.printf("GreenHat Index %i\n", ghNr);
	greenHatList[ghNr]->moveServo(servoNr % 16, servoPos);
}
