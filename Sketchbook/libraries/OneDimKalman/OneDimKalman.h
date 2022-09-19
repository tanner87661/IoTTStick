/*
 * OneDimKalman.h
 *
 *  Created on: 2019/11/006
 *      Author: hrt
 *      Version : 0.1
 */	

#ifndef OneDimKalman_H_
#define OneDimKalman_H_
#include <Arduino.h>

class OneDimKalman {
public:
	OneDimKalman(double initErrMeasure=8, double initGain=10, double initErrEst=10, double initEst=10);
	OneDimKalman();
	~OneDimKalman();
	void setInitValues(double initErrMeasure=8, double initGain=10, double initErrEst=10, double initEst=10);
	void updateErrorRange(double newVal);
	double getEstimate(double Measurement);
	double getCurrVal();
private:
	double kGain = 10;
	double errEst = 10;
	double errMeasure = 8;
	double currEst = 10;
};


#endif
