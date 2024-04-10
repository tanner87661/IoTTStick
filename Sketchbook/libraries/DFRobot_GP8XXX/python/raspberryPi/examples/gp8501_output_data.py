# -*- coding:utf-8 -*-
'''!
  @file  gp8501_output_data.py
  @brief Set the output channel and voltage value, and convert the PWM signal to two 0-2.5V or 0-VCC (2.7V-5.5V) analog voltage signals
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
#The incoming pin number is BCM coded
DAC = DFRobot_GP8501(pin0=17,pin1=18)

def setup():

  DAC.begin()

  '''!
    @brief Set different channel outputs for the DAC values
    @param data PWM Duty  (0-100)
    @n  The optional parameters (0-100) correspond to (0-2.5V) or (0-VCC), and the specific corresponding voltage range depends on the module voltage selection wave switch
    @param channel Output channel
    @n 0: Channel 0 (valid when configuring PWM0 output)
    @n 1: Channel 1 (valid when configuring PWM1 output)
    @n 2: All channels (valid when configuring dual-channel output)
  '''
  DAC.set_dac_out_voltage(100, 2)

def loop():
  pass

if __name__ == "__main__":
  setup()
  while True:
    loop()