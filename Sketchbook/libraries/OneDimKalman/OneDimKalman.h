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
	OneDimKalman(double_t initErrMeasure=8, double_t initGain=10, double_t initErrEst=10, double_t initEst=10);
	OneDimKalman();
	~OneDimKalman();
	void setInitValues(double_t initErrMeasure=8, double_t initGain=10, double_t initErrEst=10, double_t initEst=10);
	void updateErrorRange(double_t newVal);
	double_t getEstimate(double_t Measurement);
	double_t getCurrVal();
private:
	double_t kGain = 10;
	double_t errEst = 10;
	double_t errMeasure = 8;
	double_t currEst = 10;
};


#endif
