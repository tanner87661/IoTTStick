/*
 * OneDimKalman.h
 *
 *  Created on: 2019/11/006
 *      Author: hrt
 *      Version : 0.1
 */	

#include "OneDimKalman.h"
#include <Arduino.h>


OneDimKalman::OneDimKalman(double initErrMeasure, double initGain, double initErrEst, double initEst)
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

void OneDimKalman::updateErrorRange(double newVal)
{
	errMeasure = newVal;
}

void OneDimKalman::setInitValues(double initErrMeasure, double initGain, double initErrEst, double initEst)
{
	kGain = initGain;
	currEst = initEst;
	errMeasure = initErrMeasure;
	errEst = initErrEst;
}

double OneDimKalman::getEstimate(double Measurement)
{
	double lastEst = currEst;
	kGain = errEst / (errEst + errMeasure);
	currEst = currEst + (kGain * (Measurement - currEst));
	errEst = (1.0 - kGain) * errEst + fabs(lastEst - currEst);
	errEst = errEst * (1 - kGain);
//	Serial.printf("Gain: %f EstErr %f InVal %f OutVal %f ErrMeas %f LastEst %f \n", kGain, errEst, Measurement, currEst, errMeasure, lastEst);
	return currEst;
}

double OneDimKalman::getCurrVal()
{
	return currEst;
}
