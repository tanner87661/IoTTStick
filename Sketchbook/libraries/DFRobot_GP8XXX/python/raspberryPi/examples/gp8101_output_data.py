# -*- coding:utf-8 -*-
'''!
  @file  gp8101_output_data.py
  @brief Set the data values corresponding to the output voltage and convert the PWM signal into a single-channel analog voltage signal ranging from 0-5V or 0-10V.
  @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license  The MIT License (MIT)
  @author  [fary](feng.yang@dfrobot.com)
  @version  V1.0
  @date  2023-05-10
  @url  https://github.com/DFRobot/DFRobot_GP8XXX
'''


from __future__ import print_function
import sys
import os
import time

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from DFRobot_GP8XXX import *

# Pass the pin number in BCM encoding
DAC = DFRobot_GP8101(pin0=17)

def setup():

  DAC.begin()

  '''
    @brief Set the output DAC value
    @param data PWM duty cycle (0-100)
    @n (0-100) corresponds to (0-5V) or (0-10V), the specific voltage range depends on the module's voltage selection switch
  '''
  DAC.set_dac_out_voltage(100)

def loop():
  pass

if __name__ == "__main__":
  setup()
  while True:
    loop()