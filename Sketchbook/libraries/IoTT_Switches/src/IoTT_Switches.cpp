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

/*
greenHatType getGreenHatTypeByName(String transName)
{
//  if (transName == "servo") return servoModule;
//  if (transName == "combo") return comboModule;
  return servoModule; 
}
*/

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
	parentObj->setPWMValue(modIndex, 0);
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
	hesPoint = 0;
	hesSpeed = 0;
	if (thisObj.containsKey("UseHesi"))
	{
		bool useHesi = (thisObj["UseHesi"]);
		if (useHesi)
		{
			if (thisObj.containsKey("HesPoint"))
				hesPoint = thisObj["HesPoint"];
			if (thisObj.containsKey("HesSpeed"))
				hesSpeed = thisObj["HesSpeed"];
		}
	}
	if (thisObj.containsKey("PowerOff"))
		endMovePwrOff = thisObj["PowerOff"];
	if (thisObj.containsKey("InitPulse"))
		endMoveInitPulse = thisObj["InitPulse"];
	if (!endMovePwrOff)
		endMoveInitPulse = true;



	if (thisObj.containsKey("Positions"))
	{
		JsonArray thisParams = thisObj["Positions"];
        aspectListLen = thisParams.size();
        if (aspectListLen > 0)
			aspectList = (aspectEntry*) realloc (aspectList, aspectListLen * sizeof(aspectEntry));
//		currentPos = 0;
        for (int i=0; i < aspectListLen; i++)
        {
			aspectEntry aspectListEntry;
//			aspectListEntry.aspectID.push_back(0);


			aspectListEntry.isUsed = thisParams[i]["Used"];
			JsonArray aspID = thisParams[i]["AspVal"];
/*			
			if (aspID)
				for (int j=0; j < aspID.size(); j++)
					aspectListEntry.aspectID.push_back((uint16_t)aspID[j]);
			else
			{
				uint16_t newVal = thisParams[i]["AspVal"];
				aspectListEntry.aspectID.push_back(newVal);
			}		
*/			
			if (aspID)
				aspectListEntry.aspectIDLen = aspID.size();
			else
				aspectListEntry.aspectIDLen = 1;
			aspectListEntry.aspectID = (uint16_t*) realloc(aspectListEntry.aspectID, aspectListEntry.aspectIDLen * sizeof(uint16_t));
			if (aspID)
				for (int j=0; j < aspectListEntry.aspectIDLen; j++)
					aspectListEntry.aspectID[j] = aspID[j];
			else
				aspectListEntry.aspectID[0] = thisParams[i]["AspVal"];

			aspectListEntry.aspectPos = thisParams[i]["PosPt"];
			aspectListEntry.moveCfg = thisParams[i]["MoveCfg"];
			aspectList[i] = aspectListEntry;
		}
//		Serial.printf("Init %i with %i\n", switchAddrList[0], currentPos);
	}
}

void IoTT_SwitchBase::saveRunTimeData(File * dataFile)
{
	bool tmpFile = false;
	File diskFile;
	if (dataFile == NULL)
	{
		String diskFileName = servoFileName + String("_mod") + String(modIndex) + String(servoFileExt);
		diskFile = SPIFFS.open(diskFileName, "w");
		dataFile = &diskFile;
		tmpFile = true;
	}
	if (dataFile)
	{
		uint8_t buf[2] = {0,0};
		buf[0] = (currentPos >> 8);
		buf[1] = (currentPos & 0x00FF);
//		Serial.printf("%i %02X %02X\n", currentPos, buf[0], buf[1]);
		dataFile->write(buf, 2);
		buf[0] = (extSwiPos >> 8);
		buf[1] = (extSwiPos & 0x00FF);
//		Serial.printf("%02X %02X %02X \n", extSwiPos, buf[0], buf[1]);
		dataFile->write(buf, 2);
		if (tmpFile)
		{
			dataFile->close();
//			Serial.println("Close File");
		}
		posSaved = true;
//		Serial.printf("Write Curr: %02X ext: %02X\n", currentPos, extSwiPos);
	}
}

void IoTT_SwitchBase::loadRunTimeData(File * dataFile)
{
	bool tmpFile = false;
	File diskFile;
	if (dataFile == NULL)
	{
		String diskFileName = servoFileName + String("_mod") + String(modIndex) + String(servoFileExt);
		if (SPIFFS.exists(diskFileName))
		{
			diskFile = SPIFFS.open(diskFileName, "r");
			if (diskFile.size() == 4)
			{
				dataFile = &diskFile;
				tmpFile = true;
			}
		}
	}
	if (dataFile)
	{
		uint8_t buf[2];
		dataFile->read(buf,2);
		currentPos = (int16_t)((buf[0]<<8) + buf[1]); 
//		Serial.printf("%i %02X %02X Size %i\n", currentPos, buf[0], buf[1], dataFile->size());
		if (endMoveInitPulse)
			currentPos++; //incr by 1 to make sure it is processed during startup
		dataFile->read(buf,2);
		extSwiPos = (buf[0]<<8) + buf[1];
//		Serial.printf("%02X %02X %02X \n", extSwiPos, buf[0], buf[1]);
		if (tmpFile)
			dataFile->close();
//		Serial.printf("Read Curr Pos %02X Ext Pos %02X\n", currentPos, extSwiPos);
		switch (srcType) //set digitrax buffers accordingly to avoid init moves
		{
			case evt_trackswitch: //single switch address, just initialize address
			{
//				Serial.printf("%i Track Switch to pos %i\n", modIndex, extSwiPos);
				digitraxBuffer->setSwiStatus(switchAddrList[0], extSwiPos, 0);
				break; 
			}
			case evt_signalmastdyn: //multiple addresses, initialize last acording to extSwiPos
			{
				digitraxBuffer->setSwiStatus(switchAddrList[extSwiPos>>1], extSwiPos, 0);
				uint32_t hlpAct = digitraxBuffer->getLastSwiActivity(switchAddrList[extSwiPos>>1]);
//				Serial.printf("%i Dyn Signal %i to aspect  %i\n", modIndex, switchAddrList[extSwiPos>>1], extSwiPos);
				break; 
			}
			case evt_signalmastdcc: //single address, set aspect
			{
//				Serial.printf("%i DCC Signal %i to aspect %i\n", modIndex, switchAddrList[0], extSwiPos);
				
				digitraxBuffer->setSignalAspect(switchAddrList[0], extSwiPos);
				break; 
			}
		}
	}
	else
	{
//		Serial.println("No data file, InitPos");
		currentPos = (int16_t) initPos;
		extSwiPos = 0;
	}
}

void IoTT_SwitchBase::processExtEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
}

void IoTT_SwitchBase::processSwitch(bool extPwrOK)
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
		uint16_t currPos = currentPos;
		if (targetMove)
		{
//			Serial.println("Target");
			if (targetMove->aspectPos != currPos)
			{
				if (microsElapsed(lastMoveTime) > nextMoveWait)
				{
					lastMoveTime = micros();
					//analyze the current status for further decision making
					bool moveUp = targetMove->aspectPos > currPos;
					uint8_t adjMode = targetMove->moveCfg & 0x0F;
//					uint8_t adjMode = moveUp ? (targetMove->moveCfg & 0x0F) : ((targetMove->moveCfg & 0xF0) >> 4);
					float_t linSpeed = moveUp ? (float_t) upSpeed : (float_t)(-1) * downSpeed; //set linSpeed to incr/s
					bool correctDir = (sgn(currSpeed) == sgn(linSpeed)) || (currSpeed == 0.0);
					bool softStop = ((adjMode & 0x03) == 1); //soft stop
					bool softStart = (adjMode & 0x04); //overshoot or soft stop
					bool beforeHesitate = hesPoint > 0 ? (moveUp ? currPos < hesPoint : currPos > hesPoint) : false; 
					uint16_t moveEndPoint = beforeHesitate ? hesPoint : targetMove->aspectPos; //end point or hesitate
					float moveEndSpeed = (beforeHesitate ? (sgn(linSpeed) * hesSpeed) : (adjMode & 0x02 ? (linSpeed) : (adjMode == 0 ? (linSpeed) : (0)))); //speed when arriving at end point, incr/s
					int32_t breakDistance = round(sq(linSpeed - moveEndSpeed) / (2 * (int32_t)decelRate)); //s = v2 /2a
					uint16_t breakPoint = moveEndPoint - (sgn(linSpeed) * breakDistance);
					bool beforeBreakPoint = ((int)(breakPoint - currPos) * (int)sgn(linSpeed)) > 0;

					//calculate dynamic data for step duration and width 
					uint32_t stepDelay = currSpeed == 0 ? refreshInterval : round(1000000 / abs(currSpeed)); //calculating the duration of 1 step in micros/incr
					float stepFactor = currSpeed == 0 ? 1 : 1 + (refreshInterval / stepDelay);  //calculate how many steps to take assuming X ms cycle time

//					Serial.printf("Correct Dir: %i Lin Speed %f Curr Sp %f before break: %i going to %i \n", correctDir, linSpeed, currSpeed, beforeBreakPoint, moveEndPoint);
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
							currentPos += round(stepFactor * sgn(currSpeed)); //set the position
//							Serial.printf("New Pos %i Speed curr %f lin %f StepFactor %f\n", currentPos, currSpeed, linSpeed, stepFactor);
							nextMoveWait = round(stepFactor * stepDelay); //set the delay time
						}
						else
							nextMoveWait = refreshInterval; //standard 1ms wait
					}
					parentObj->setPWMValue(modIndex, round(currentPos));
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
	}
	else //oscillator mode
	{
//Serial.println("end move");
		bool moveUp = currentPos == targetMove->aspectPos;
		uint8_t adjMode = targetMove->moveCfg & 0x0F;
		float thisLambda = lambda; //(float)((moveUp ? (lambda & 0x0F) : ((lambda & 0xF0) >> 4)) + 1) / 2;
		float thisFreq = frequency; //(float)((moveUp ? (frequency & 0x0F) : ((frequency & 0xF0) >> 4)) + 1) / 2;
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
				pwmVal = moveUp ? pwmVal - currVal : pwmVal + currVal;
			else //2, overshoot
				//PWM to targetPos - abs(newAmpl)
				pwmVal = moveUp ? pwmVal + currVal : pwmVal - currVal;
			parentObj->setPWMValue(modIndex, pwmVal);
//			endMoveTimeout = millis() + endMoveDelay;
		} 
		else //done, back to linear mode
		{
			if (endMovePwrOff)
				parentObj->setPWMValue(modIndex, 0);
			else
				parentObj->setPWMValue(modIndex, currentPos);
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
//		Serial.printf("s %i\n", targetMove->moveCfg);
				lastMoveTime = micros();
				bool moveUp = targetMove->aspectPos > currentPos;
				uint16_t stepSpeed = moveUp ? upSpeed : downSpeed; //incr/s 
				if (stepSpeed > 0) //valid speed settings
				{
					uint32_t stepDelay = round(1000000 / stepSpeed); //calculating the duration of 1 step
					float stepFactor = 1 + (refreshInterval / stepDelay);  //calculate how many steps to take assuming 5ms cycle time
					currentPos += round(stepFactor * (moveUp ? 1 : (-1))); //set the position
					nextMoveWait = round(stepFactor * stepDelay); //set the delay time
					bool nextDir = targetMove->aspectPos > currentPos;
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
				parentObj->setPWMValue(modIndex, currentPos);
//				endMoveTimeout = millis() + endMoveDelay;
			}
		}
		else
			if ((endMovePwrOff) && (endMoveTimeout < millis()) && (endMoveTimeout > 0))
			{
				endMoveTimeout = 0;
				parentObj->setPWMValue(modIndex, 0);
			}
}

//------------2----------------------------------------------------------------------------------------------------
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
	{
		if (targetMove->moveCfg == 0)
			processServoSimple();
		else
			processServoComplex();
	}
}

void IoTT_ServoDrive::processSwitch(bool extPwrOK)
{
	switch (srcType)
	{
		case evt_trackswitch:
		{
			uint16_t swiStatus = 0;
			for (int8_t i = (switchAddrListLen-1); i >= 0; i--) //check for the latest position
				swiStatus = (2 * swiStatus) + ((digitraxBuffer->getSwiPosition(switchAddrList[i]) >> 5) & 0x01);
			if (((swiStatus != extSwiPos) || endMoveInitPulse) && aspectList[swiStatus].isUsed)
			{
//				Serial.printf("Ext Tg Move 475 %i %i\n", extSwiPos, swiStatus);
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
				hlpAct = digitraxBuffer->getLastSwiActivity(switchAddrList[i]);
				if (hlpAct > lastAct)
				{
					dynSwi = i;
					lastAct = hlpAct;
				}
			}
			if (lastAct != 0) //we have activity
			{
				uint8_t aspectNr = dynSwi * 2;
				if (((digitraxBuffer->getSwiPosition(switchAddrList[dynSwi]) >> 4) & 0x02) > 0)
					aspectNr++; //this is the final aspect #
				if (((extSwiPos != aspectNr) || endMoveInitPulse)  && aspectList[aspectNr].isUsed)
				{
//				Serial.println("Ext Tg Move 503");
					targetMove = &aspectList[aspectNr];
					extSwiPos = aspectNr;
//					Serial.printf("Updating Aspect %i  for Switch %i\n", aspectNr, switchAddrList[dynSwi]);
				}
			}
			break;
		}
		case evt_signalmastdcc:
		{
			uint16_t sigAddress = switchAddrList[0];
			uint8_t sigAspect = digitraxBuffer->getSignalAspect(sigAddress);
			if (sigAspect != extSwiPos)
			{
//				Serial.printf("Get %i Ext %i\n", sigAspect, extSwiPos);
				uint16_t newSwiPos = sigAspect;
				int16_t nextVal = -1;
				int16_t nextInd = -1;
				uint8_t distance = 0;
				
				
				
				
				for (int i = 0; i < aspectListLen; i++)
				{
//					Serial.println(aspectList[i].aspectID);
					if ((newSwiPos <= aspectList[i].aspectID[0]) && (newSwiPos > nextVal))
					{
						nextInd = i;
						nextVal = aspectList[i].aspectID[0];
					}
				}
				if ((aspectList[nextInd].aspectID[0] != extSwiPos) || endMoveInitPulse)
				{
					if (aspectList[nextInd].isUsed)
						targetMove = &aspectList[nextInd];
					extSwiPos = sigAspect;
//					Serial.printf("Process Signal %i to Aspect %i Pos %i \n", sigAddress, sigAspect, targetMove->aspectPos);
				}
			}
			break;
		}
		case evt_analogvalue:
			if ((digitraxBuffer->getAnalogValue(switchAddrList[0]) != extSwiPos) || endMoveInitPulse)
			{
				extSwiPos = digitraxBuffer->getAnalogValue(switchAddrList[0]);
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
								{
									targetMove = &aspectList[1];
								}
								else ;
							else
								if (aspectList[2].isUsed)
								{
									targetMove = &aspectList[2];
								}
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
//						Serial.printf("Process Analog Inp %i to linear Value %i -> %i\n", switchAddrList[0], extSwiPos, aspectList[1].aspectPos);
					}
			}
			break;
		case evt_button: 
		{
			uint8_t newBtnVal = digitraxBuffer->getButtonValue(switchAddrList[0]);
			if ((newBtnVal != extSwiPos) || endMoveInitPulse)
			{
				if (newBtnVal <= aspectListLen)
					if (aspectList[newBtnVal].isUsed)
					{
						extSwiPos = newBtnVal;
						targetMove = &aspectList[newBtnVal];
//						Serial.printf("Process Button %i to Status %i \n", switchAddrList[0], extSwiPos);
					}
			}
			break;
		}
		case evt_blockdetector:
		{
			uint16_t swiStatus = 0;
			for (int8_t i = (switchAddrListLen-1); i >= 0; i--) //check for the latest position
				swiStatus = (2 * swiStatus) + (digitraxBuffer->getBDStatus(switchAddrList[i]) & 0x01);
			if (((swiStatus != extSwiPos) || endMoveInitPulse) && aspectList[swiStatus].isUsed)
			{
				targetMove = &aspectList[swiStatus];
				extSwiPos = swiStatus;
//				Serial.printf("Updating Block Detector %i Sensors  for Status %i  \n", switchAddrListLen, swiStatus);
			}
			break;
		}
		case evt_transponder:
		{
			//this is handled in external events
			break;
		}
		default:
//			Serial.printf("Call switchtype %i \n", srcType);
			break;
	}
	endMoveInitPulse = false;
	if (extPwrOK)
		if (targetMove)
		{
			if (targetMove->moveCfg == 0)
				processServoSimple();
			else
				processServoComplex();
		}
		else
		{
			if ((endMovePwrOff) && (endMoveTimeout < millis()) && (endMoveTimeout > 0))
			{
				endMoveTimeout = 0;
				parentObj->setPWMValue(modIndex, 0);
			}
		}
	else
		parentObj->setPWMValue(modIndex, 0);	
}

void IoTT_ServoDrive::loadSwitchCfgJSON(JsonObject thisObj)
{
	IoTT_SwitchBase::loadSwitchCfgJSON(thisObj);
//	Serial.println("IoTT_ServoDrive loadSwitchCfgJSON");
}

//----------------------------------------------------------------------------------------------------------------
/*
IoTT_ComboDrive::IoTT_ComboDrive():IoTT_SwitchBase()
{
}

IoTT_ComboDrive::~IoTT_ComboDrive()
{
}

void IoTT_ComboDrive::processSwitch(bool extPwrOK)
{
}

void IoTT_ComboDrive::loadSwitchCfgJSON(JsonObject thisObj)
{
	IoTT_SwitchBase::loadSwitchCfgJSON(thisObj);
//	Serial.println("IoTT_ComboDrive loadSwitchCfgJSON");
}

*/
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
		ghPWM->setPWM(i, 0, 0); //Servo off
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

/*
void IoTT_GreenHat::setGreenHatType(greenHatType newType)
{
	modType = newType;
}
*/

void IoTT_GreenHat::loadGreenHatCfgJSON(uint8_t fileNr, JsonObject thisObj, bool resetList)
{
//		"CfgFiles": ["gh/0/switches", "gh/0/btn", "gh/0/btnevt", "gh/0/led"]
	
//	Serial.printf("IoTT_GreenHat::loadGreenHatCfgJSON File Nr %i Reset %i \n", fileNr, resetList);
//	if (thisObj.containsKey("Drivers")) Serial.println("Switches");
//	if (thisObj.containsKey("Buttons")) Serial.println("Buttons");
//	if (thisObj.containsKey("ButtonHandler")) Serial.println("ButtonHandler");
//	if (thisObj.containsKey("LEDDefs")) Serial.println("LEDDefs");
	
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
//					if (modType == servoModule)
						thisSwiMod = new(IoTT_ServoDrive);
//					else
//						thisSwiMod = new(IoTT_ComboDrive);
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
					buttonHandler->loadButtonCfgJSONObj(thisObj, false);
				else
					Serial.println("No Button Handler defined");
			else
				Serial.println("No Button Handler");
			break;
		case 3: //LED
			if (thisObj.containsKey("LEDDefs"))
				if (myChain != NULL)
					myChain->loadLEDChainJSONObj(thisObj, false);
				else
					Serial.println("No Chain defined");
			else
				Serial.println("No LEDChain");
			break;
	}
}

void IoTT_GreenHat::setPWMValue(uint8_t lineNr, uint16_t pwmVal)
{
//	if (lineNr == 0)
//		Serial.printf("%i pos %i\n", lineNr, pwmVal);
	ghPWM->setPWM(lineNr, 0, pwrOK ? pwmVal : 0); //if external DC power missing, we shut down servo electrically
	IoTT_SwitchBase * thisSwiMod = switchModList[lineNr];
	if (pwmVal > 0)
		thisSwiMod->endMoveTimeout = millis() + endMoveDelay;
}

void IoTT_GreenHat::processBtnEvent(sourceType inputEvent, uint16_t btnAddr, uint16_t eventValue)
{
//	Serial.println("Btn Evt");
	if (buttonHandler) 
		buttonHandler->processBtnEvent(inputEvent, btnAddr, eventValue); //drives the outgoing buffer and time delayed commands
	if (inputEvent == evt_transponder)
	{
		for (int i=0; i < switchModListLen; i++)
		{
			IoTT_SwitchBase * thisSwiMod = switchModList[i];
			thisSwiMod->processExtEvent(inputEvent, btnAddr, eventValue); //transponder info is not buffered, so we process the event
		}
		if (myChain)
			myChain->processBtnEvent(inputEvent, btnAddr, eventValue);
	}
}

bool IoTT_GreenHat::isVerified()
{
	if (myChain)
		return myChain->isVerified();
	else
		return false;
}

void IoTT_GreenHat::processSwitch(bool extPwrOK)
{
	pwrOK = extPwrOK;
	if (millis() > startupTimer)
	{
//		if (startUpCtr == 0)
//			myChain->refreshAnyway = true;
		startupTimer += startupInterval;
		startUpCtr = max(startUpCtr-1,0);
	}
	for (int i=0; i < switchModListLen; i++)
	{
		if (i >= startUpCtr)
		{
			IoTT_SwitchBase * thisSwiMod = switchModList[i];
			thisSwiMod->processSwitch(extPwrOK);
		}
	}
	if (startUpCtr == 0)
	{
		if (buttonHandler) 
			buttonHandler->processButtonHandler(); //drives the outgoing buffer and time delayed commands
	}
	if (startUpCtr < 10)
	{
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

void IoTT_GreenHat::saveRunTimeData(File * dataFile)
{
	uint8_t buf[2];
	Serial.printf("Saving Servo Data %i Servos, 4 bytes each\n", switchModListLen);
	for (int i=0; i < switchModListLen; i++)
	{
		IoTT_SwitchBase * thisSwiMod = switchModList[i];
		thisSwiMod->saveRunTimeData(dataFile);
	}
}

void IoTT_GreenHat::loadRunTimeData(File * dataFile)
{
	uint8_t buf[2];
	IoTT_SwitchBase * thisSwiMod;
	for (int i=0; i < switchModListLen; i++)
	{
		thisSwiMod = switchModList[i];
		thisSwiMod->loadRunTimeData(dataFile);
	}
}

void IoTT_GreenHat::identifyLED(uint16_t LEDNr)
{
	if (myChain)
		myChain->identifyLED(LEDNr);
}

void IoTT_GreenHat::moveServo(uint8_t servoNr, uint16_t servoPos)
{
//	Serial.printf("Servo Nr %i to Pos %i\n", servoNr, servoPos);
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

void IoTT_SwitchList::saveRunTimeData()
{
	for (int i=0; i < greenHatListLen; i++)
	{
		String diskFileName = servoFileName + String(i) + String(servoFileExt);
		File dataFile = SPIFFS.open(diskFileName, "w");
		if (dataFile)
		{
			greenHatList[i]->saveRunTimeData(&dataFile);
			dataFile.close();
			Serial.println("Writing Servo File " + String(i) + " complete");
		}
		else
		{
			Serial.println("Unable to write Servo File " + String(i));
		}
	}
}

void IoTT_SwitchList::loadRunTimeData(uint8_t ghNr)
{
	String diskFileName = servoFileName + String(ghNr) + String(servoFileExt);
    File dataFile = SPIFFS.open(diskFileName, "r");
//    Serial.println(dataFile.size());
//    Serial.println(greenHatListLen);
    if ((dataFile) && (dataFile.size() >= 64))
    {
		greenHatList[ghNr]->loadRunTimeData(&dataFile);
		dataFile.close();
		Serial.printf("Servo File %i Runtime Data loaded\n", ghNr);
	}
    else
    {
		if (dataFile)
			dataFile.close();
		greenHatList[ghNr]->loadRunTimeData(NULL);
		Serial.println("Unable to read Servo File " + String(ghNr) + " Runtime Data");
	}
}

void IoTT_SwitchList::processLoop(bool extPwrOK)
{
	for (int i=0; i < greenHatListLen; i++)
		greenHatList[i]->processSwitch(extPwrOK);
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

/*
void IoTT_SwitchList::setGreenHatType(uint8_t modNr, greenHatType modType)
{
	greenHatList[modNr]->setGreenHatType(modType);
}
*/

void IoTT_SwitchList::loadSwCfgJSON(uint8_t ghNr, uint8_t fileNr, DynamicJsonDocument doc, bool resetList)
{
//	Serial.printf("IoTT_SwitchList::loadSwCfgJSON Module %i File %i %i\n", ghNr, fileNr, resetList);
	JsonObject thisObj = doc.as<JsonObject>();
	greenHatList[ghNr]->loadGreenHatCfgJSON(fileNr, thisObj, resetList);
	if (fileNr == 0)
		loadRunTimeData(ghNr);
}

void IoTT_SwitchList::identifyLED(uint16_t LEDNr)
{
	uint8_t ghNr = trunc(LEDNr / 33);
	greenHatList[ghNr]->identifyLED(LEDNr % 33);
}

void IoTT_SwitchList::moveServo(uint8_t servoNr, uint16_t servoPos)
{
	uint8_t ghNr = trunc(servoNr / 16);
//	Serial.printf("GreenHat Index %i\n", ghNr);
	greenHatList[ghNr]->moveServo(servoNr % 16, servoPos);
}
