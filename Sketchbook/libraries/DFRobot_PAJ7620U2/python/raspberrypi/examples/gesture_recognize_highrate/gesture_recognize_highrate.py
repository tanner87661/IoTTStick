#-*- coding: utf-8 -*-
'''!
  @file GestureRecognize_HighRate.ino
  @brief Present the 9 built-in gestures data the sensor supports. 
  @n Wave your hand above the sensor (within 0~20cm), it can recognize 9 kinds of gestures: move up, down, left, right, forward,
  @n backward, clockwise, anti-clockwise, wave.
  @n For more usages of the sensor, refer to the description about setGestureHighRate in function setup.
  @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @licence     The MIT License (MIT)
  @author      Alexander(ouki.wang@dfrobot.com)
  @maintainer  [fary](feng.yang@dfrobot.com)
  @version  V1.0
  @date  2019-07-16
  @url https://github.com/DFRobot/DFRobot_PAJ7620U2
'''
import sys
sys.path.append('../../')
import time
from DFRobot_PAJ7620U2 import *

paj = DFRobot_PAJ7620U2(bus=1)


print("Gesture recognition system base on PAJ7620U2")
while(paj.begin() != 0):
    print("initial PAJ7620U2 failure! Please check if all the connections are fine, or if the wire sequence is correct?")
    time.sleep(0.5)

print("PAJ7620U2 init finished, start to test the gesture recognition function.")

'''Set to fast detection mode 
 # If the parameter is set to false, the module enters slow detection mode, and it detects one gesture every 2s. We have integrated
 # some gestures inside the module to make it convenient for beginners.   
 # The slow mode can recognize 9  basic gestures and 4 expanded gestures: move left, right, up, down, forward, backward, clockwise,
 # counter-clockwise, wave. slowly move left and right, slowly move up and down, slowly move forward and backward, wave slowly and 
 # randomly.
 #
 # If the parameter is set to true, the module enters fast detection mode. 
 # The fast mode can recognize 9 gestures: move left, right, up, down, forward, backward, clockwise, counter-clockwise, wave
 # To detect the combination of these gestures, like wave left, right and left quickly, users needs to design their own algorithms logic.
 # Since users only use limited gestures in this mode, we are not going to integrate too much expanded gestures in the library.
 # If necessary, you can complete the algorithm logic in the ino file by yourself.
 '''
paj.set_gesture_highrate(True)

def main():
  while True:
    '''
    Read gesture number（return eGesture_t enumerated type）
    GESTURE_NONE  GESTURE_RIGHT  GESTURE_LEFT  GESTURE_UP  GESTURE_DOWN  GESTURE_FORWARD
    GESTURE_BACKWARD  GESTURE_CLOCKWISE  GESTURE_ANTI_CLOCKWISE  GESTURE_WAVE  GESTURE_WAVE_SLOWLY_DISORDER
    GESTURE_WAVE_SLOWLY_LEFT_RIGHT  GESTURE_WAVE_SLOWLY_UP_DOWN  GESTURE_WAVE_SLOWLY_FORWARD_BACKWARD
    '''
    gesture = paj.get_gesture()
    if gesture != paj.GESTURE_NONE:
      '''
      Get the string descritpion corresponding to the gesture number.
      The string description could be 
      "None","Right","Left", "Up", "Down", "Forward", "Backward", "Clockwise", "Anti-Clockwise", "Wave",
      "WaveSlowlyDisorder", "WaveSlowlyLeftRight", "WaveSlowlyUpDown", "WaveSlowlyForwardBackward"
      '''
      description  = paj.gesture_description(gesture);#Convert gesture number into string description
      print("--------------Gesture Recognition System---------------------------")
      print("gesture code        = %d"%(gesture))
      print("gesture description  = "+description)

if __name__ == "__main__":
    main()