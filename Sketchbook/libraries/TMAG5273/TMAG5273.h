#ifndef TMAG5273_H
#define TMAG5273_H
#include <Arduino.h>
#include <Wire.h>

#define I2CReg 0x35
#define readReg 0x6A
#define writeReg 0x6B
#define STAY 0xffff
#define axis_X      0x0
#define axis_Y      0x1
#define axis_Z      0x2

class TMAG5273
{

public:
  // Constructor
	TMAG5273(TwoWire * newWire = NULL);
  //  device config
	bool setDeviceConfig();

	float getTempData();
	float getAngleData();
	float getFluxDensity(uint8_t ofAxis);
	
	void collectCalibData();
	void setFluxOffset();


private:
	TwoWire * magWire = NULL;
	float calibFlux[6] = {0,0,0,0,0,0};
	uint8_t readRegData(byte address);
	void writeI2CData(uint8_t startReg, uint8_t * regData, uint8_t numBytes);
	uint32_t readI2CData(uint8_t startReg, uint8_t numBytes); //max 4 bytes
	bool magCalibrated = false;
	void calcFluxOffset(uint8_t srcAxis, uint8_t trgtRegister);
}; //class TMAG5273

#endif
