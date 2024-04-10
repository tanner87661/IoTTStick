/*!
 * @file GesturePassword.ino
 * @brief Write algorithms in fast mode to realize gesture password. 
 * @n Input gesture password in 20sm, if correct, enter the system, otherwise, continue to wait for users to input password.
 * @n The timeout period can be adjusted via macro TIMEOUT, unit(mm). 
 *
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      [Alexander](ouki.wang@dfrobot.com)
 * @maintainer  [fary](feng.yang@dfrobot.com)
 * @version  V1.0
 * @date  2019-07-16
 * @url https://github.com/DFRobot/DFRobot_PAJ7620U2
 */
 
#include <DFRobot_PAJ7620U2.h>

DFRobot_PAJ7620U2 paj;


//Input the correct gestures within TIMEOUT period (mm):up up down down left left right right
DFRobot_PAJ7620U2::eGesture_t password[]={DFRobot_PAJ7620U2::eGestureUp,DFRobot_PAJ7620U2::eGestureUp,DFRobot_PAJ7620U2::eGestureDown,DFRobot_PAJ7620U2::eGestureDown,\
  DFRobot_PAJ7620U2::eGestureLeft,DFRobot_PAJ7620U2::eGestureLeft,DFRobot_PAJ7620U2::eGestureRight,DFRobot_PAJ7620U2::eGestureRight};

static uint8_t index = 0;     //The number of the correctly input password 
bool correct = false;  //Whether the input password is correct 
#define TIMEOUT 20000 //Set Timeout period, unit(mm)

void setup()
{
  Serial.begin(115200);
  delay(300);
  Serial.println("Gesture recognition system base on PAJ7620U2");
  while(paj.begin() != 0){
    Serial.println("initial PAJ7620U2 failure! Please check if all the connections are fine, or if the wire sequence is correct?");
    delay(500);
  }
  Serial.println("PAJ7620U2 init finished, start to test the gesture recognition function.");
  
  /*Set to fast detection mode 
   * If the parameter is set to false, the module enters slow detection mode, and it detects one gesture every 2s. We have integrated
   * some gestures inside the module to make it convenient for beginners.   
   * The slow mode can recognize 9  basic gestures and 4 expanded gestures: move left, right, up, down, forward, backward, clockwise,
   * counter-clockwise, wave. slowly move left and right, slowly move up and down, slowly move forward and backward, wave slowly and 
   * randomly.
   * 
   *
   *
   *
   * If the parameter is set to true, the module enters fast detection mode. 
   * The fast mode can recognize 9 gestures: move left, right, up, down, forward, backward, clockwise, counter-clockwise, wave
   * To detect the combination of these gestures, like wave left, right and left quickly, users needs to design their own algorithms logic.
   * Since users only use limited gestures in this mode, we are not going to integrate too much expanded gestures in the library.
   * If necessary, you can complete the algorithm logic in the ino file by yourself.
   */
  paj.setGestureHighRate(true);
}

void loop()
{
  /* Read gesture number（return eGesture_t enumerated type）
   * eGestureNone  eGestureRight  eGestureLeft  eGestureUp  eGestureDown  eGestureForward
   * eGestureBackward  eGestureClockwise  eGestureAntiClockwise  eGestureWave  eGestureWaveSlowlyDisorder
   * eGestureWaveSlowlyLeftRight  eGestureWaveSlowlyUpDown  eGestureWaveSlowlyForwardBackward
   */
  DFRobot_PAJ7620U2::eGesture_t gesture;
  uint8_t pdLen = sizeof(password)/sizeof(password[0]);
  Serial.print("password length=");Serial.println(pdLen);
  unsigned long startTimeStamp = millis();
  Serial.print("please input the "); Serial.print(index+1); Serial.println(" gesture");
  do{
    unsigned long now = millis();
    if(now - startTimeStamp >= TIMEOUT){
        startTimeStamp = now;
        index = 0;
        Serial.println("timeout，input again");
        Serial.print("please input the "); Serial.print(index+1); Serial.println(" gesture");
    }
    gesture = paj.getGesture();
    if(gesture == paj.eGestureNone){
      continue;
    }
    Serial.println(paj.gestureDescription(gesture));
    if(gesture == password[index]){
      index++;
      Serial.print("please input the "); Serial.print(index+1); Serial.println(" gesture");
    }else{
      startTimeStamp = millis();
      index = 0;
      Serial.println("gesture password is incorrect, try again");
      Serial.print("please input the "); Serial.print(index+1); Serial.println(" gesture");
    }
    if(index == pdLen){
      correct = true;
    }
  }while(!correct);
  Serial.println("Unlock all gestures successfully, you have entered the system");
  Serial.print("To enter the gesture password, you have spent");
  Serial.print((millis()-startTimeStamp)/1000);
  Serial.println(" seconds");
  
  
  //TO DO
  while(1);
}
