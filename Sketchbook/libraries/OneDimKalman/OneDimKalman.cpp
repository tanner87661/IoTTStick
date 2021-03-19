/*
 * OneDimKalman.h
 *
 *  Created on: 2019/11/006
 *      Author: hrt
 *      Version : 0.1
 */	

#include "OneDimKalman.h"
#include <Arduino.h>


OneDimKalman::OneDimKalman(double_t initErrMeasure, double_t initGain, double_t initErrEst, double_t initEst)
{
	setInitValues(initErrMeasure, initGain, initErrEst, initEst);
}

OneDimKalman::OneDimKalman()
{
//	setInitValues();
}

OneDimKalman::~OneDimKalman() 
{
}

void OneDimKalman::updateErrorRange(double_t newVal)
{
	errMeasure = newVal;
}

void OneDimKalman::setInitValues(double_t initErrMeasure, double_t initGain, double_t initErrEst, double_t initEst)
{
	kGain = initGain;
	currEst = initEst;
	errMeasure = initErrMeasure;
	errEst = initErrEst;
}

double_t OneDimKalman::getEstimate(double_t Measurement)
{
	double_t lastEst = currEst;
	kGain = errEst / (errEst + errMeasure);
	currEst = currEst + (kGain * (Measurement - currEst));
	errEst = (1.0 - kGain) * errEst + fabs(lastEst - currEst);
	errEst = errEst * (1 - kGain);
//	Serial.printf("Gain: %f EstErr %f InVal %f OutVal %f ErrMeas %f LastEst %f \n", kGain, errEst, Measurement, currEst, errMeasure, lastEst);
	return currEst;
}

double_t OneDimKalman::getCurrVal()
{
	return currEst;
}
