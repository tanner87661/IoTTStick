#include "TMAG5273.h"

TMAG5273::TMAG5273(TwoWire * newWire)
{
	magWire = newWire;
}

bool TMAG5273::setDeviceConfig(uint8_t mountStyle)
{
	uint16_t devID = readI2CData(0x0E, 2);
//	Serial.printf("ID %i\n", devID);
	if (devID != 0x4954)
		return false;
	uint8_t data = 0x05 << 2;
	writeI2CData(0, &data, 1); //32x average, 0.4ksamples 
	data = 0x02;
	writeI2CData(1, &data, 1); //device config 1 Continuous operating mode, 

/*
 * Vert Mount Settings: 2:0x30   x,y
 * 						3:0x04
 * Horz Mount Settings: 2:0x50   x,z
 * 						3:0x0C
*/	
	switch (mountStyle)
	{
		case 0 : data = 0x50; //X,Z flat
			break;
		case 1 : data = 0x30; //X,Y vertical
			break;
		case 2 : data = 0x70; //X,Y,Z not used
			break;
	}
	writeI2CData(2, &data, 1); //0x70 XYZ channels active
	
	switch (mountStyle)
	{
		case 0 : data = 0x0C; //X,Z flat
			break;
		case 1 : data = 0x04; //X,Y vertical
			break;
		case 2 : data = 0x08; //Y,Z not used
			break;
	}
	writeI2CData(3, &data, 1); //angle calculation on X,Z axis for PCB version
	return true;
}

void TMAG5273::collectCalibData()
{
	for (uint8_t i = 0; i < 3; i++)
	{
		float fluxRes = getFluxDensity(i);
		if ((fluxRes < calibFlux[2 * i]) || (calibFlux[2 * i] == 0))
			calibFlux[2 * i] = fluxRes;
		if ((fluxRes > calibFlux[(2 * i)+1]) || (calibFlux[(2 * i) + 1] == 0))
			calibFlux[(2 * i)+1] = fluxRes;
	}
}

void TMAG5273::setFluxOffset()
{
	uint8_t axisConfig = (readI2CData(3, 1) & 0x0C) >> 2;
	//0h = No angle calculation, magnitude gain, and offset correction enabled
	//1h = X 1st, Y 2nd
	//2h = Y 1st, Z 2nd
	//3h = X 1st, Z 2nd
	switch (axisConfig)
	{
		case 1: 
			calcFluxOffset(0, 0x0A);
			calcFluxOffset(1, 0x0B);
			break;
		case 2: 
			calcFluxOffset(1, 0x0A);
			calcFluxOffset(2, 0x0B);
			break;
		case 3: 
			calcFluxOffset(0, 0x0A);
			calcFluxOffset(2, 0x0B);
			break;
		default:
			break; //no angle calculation set
			
	}
}

void TMAG5273::calcFluxOffset(uint8_t srcAxis, uint8_t trgtRegister)
{
	
	float_t magOffset = (calibFlux[(2 * srcAxis) - 1] + calibFlux[2 * srcAxis]) / 2;
	uint8_t magOffsetAdj = abs(round((4096 * magOffset)/80)); //formula from data sheet page 24
	if (magOffset < 0)
		magOffsetAdj = 256 - magOffsetAdj;
//	writeI2CData(trgtRegister, &magOffsetAdj, 1);
	Serial.printf("Flux Adjustment Axis %i Min %f Max %f Offset %f Adj %i\n", srcAxis, calibFlux[(2 * srcAxis)], calibFlux[(2 * srcAxis) - 1], magOffset, magOffsetAdj);
}

float TMAG5273::getTempData()
{
	return (25.0 + ((float)(readI2CData(0x10, 2) - 17508) / 60.1)); //formula from data sheet page 23
}

float TMAG5273::getAngleData()
{
	uint16_t rotVal = readI2CData(0x19, 2);
	float retVal = (float)(rotVal>> 4);
//	Serial.println(retVal);
	return retVal;
	return (float)((rotVal & 0x1FF0) >> 4) + ((float)(rotVal & 0x000F)/16); //formula from data sheet page 23
	
}

float TMAG5273::getFluxDensity(uint8_t ofAxis) //0:X 1:Y 2:Z
{
	uint16_t magData = readI2CData(0x12 + (2 * ofAxis), 2);
	return (40 * (((float)(magData & 0x7FFF) - (float)(magData & 0x8000)) / 0x10000)); //formula from data sheet page 22
}

void TMAG5273::writeI2CData(uint8_t startReg, uint8_t * regData, uint8_t numBytes)
{
	magWire->beginTransmission(I2CReg);
	magWire->write(startReg);
	for (uint8_t i = 0; i < numBytes; i++)
	{
//		Serial.println(*regData, 16);
		magWire->write(*regData);
		regData++;
	}
	magWire->endTransmission();
}

uint32_t TMAG5273::readI2CData(uint8_t startReg, uint8_t numBytes) //max 4 bytes
{
	writeI2CData(startReg, NULL, 0);
	magWire->requestFrom(I2CReg, numBytes);
	uint32_t readRes = 0;
	for (uint8_t i = 0; i < numBytes; i++)
		readRes = (readRes<<8) + magWire->read();
	return readRes;
}
