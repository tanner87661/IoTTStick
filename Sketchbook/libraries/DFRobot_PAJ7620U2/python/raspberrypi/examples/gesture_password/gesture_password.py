#-*- coding: utf-8 -*-
'''!
  @file GesturePassword.py
  @brief Write algorithms in fast mode to realize gesture password. 
  @n Input gesture password in 20sm, if correct, enter the system, otherwise, continue to wait for users to input password.
  @n The timeout period can be adjusted via macro TIMEOUT, unit(mm). 
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

paj = DFRobot_PAJ7620U2(1)

#Input the correct gestures within TIMEOUT period (mm):up up down down left left right right
password = [paj.GESTURE_UP,paj.GESTURE_UP,paj.GESTURE_DOWN,paj.GESTURE_DOWN,paj.GESTURE_LEFT,paj.GESTURE_LEFT,paj.GESTURE_RIGHT,paj.GESTURE_RIGHT]

index = 0        #The number of the correctly input password 
correct = False  #Whether the input password is correct 
TIMEOUT=20000    #Set Timeout period, unit(mm)

def setup():
  print("Gesture recognition system base on PAJ7620U2")
  while(paj.begin() != 0):
    print("initial PAJ7620U2 failure! Please check if all the connections are fine, or if the wire sequence is correct?")
    sleep(0.5)
  
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


def loop():
  '''Read gesture number (return Gesture type)
   # GESTURE_NONE  GESTURE_RIGHT  GESTURE_LEFT  GESTURE_UP  GESTURE_DOWN  GESTURE_FORWARD
   # GESTURE_BACKWARD  GESTURE_CLOCKWISE  GESTURE_ANTI_CLOCKWISE  GESTURE_WAVE  GESTURE_WAVE_SLOWLY_DISORDER
   # GESTURE_WAVE_SLOWLY_LEFT_RIGHT  GESTURE_WAVE_SLOWLY_UP_DOWN  GESTURE_WAVE_SLOWLY_FORWARD_BACKWARD
  '''
  global index
  pd_len = len(password)
  print("password length=%d"%pd_len)
  start_timestamp = time.time()
  print("please input the %d gesture"%(index+1))
  correct = False
  while(correct == False):
    now = time.time()
    if(now - start_timestamp >= TIMEOUT):
        start_timestamp = now
        index = 0
        print("timeout，input again")
        print("please input the %d gesture"(index+1))

    gesture = paj.get_gesture()
    if(gesture == paj.GESTURE_NONE):
      continue
    
    print(paj.gesture_description(gesture))
    if(gesture == password[index]):
      index = index + 1
      print("please input the %d gesture"%(index + 1))
    else:
      start_timestamp = time.time()
      index = 0
      print("gesture password is incorrect, try again")
      print("please input the %d gesture"%(index + 1))
    
    if(index == pd_len):
      correct = True
  
  print("Unlock all gestures successfully, you have entered the system")
  print("To enter the gesture password, you have spent %d seconds"%((time.time()-start_timestamp)/1000))
  
  #TO DO
  while True:
    pass

if __name__ == "__main__":
  setup()
  while True:
    loop()
