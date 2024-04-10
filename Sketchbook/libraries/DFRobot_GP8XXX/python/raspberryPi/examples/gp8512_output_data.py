# -*- coding:utf-8 -*-
'''!
  @file  gp8512_output_data.py
  @brief Set the output channels and voltage values to convert the I2C signal into two channels of analog voltage signals ranging from 0-2.5V or 0-VCC (2.7V-5.5V).
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
# This example uses software I2C driver. It is not recommended to pass the hardware I2C pin interface. The pin number passed should use the BCM coding.
DAC = DFRobot_GP8512(i2c_sda=17,i2c_scl=27)

def setup():

  while DAC.begin()!=0:
    print("Communication with the device failed. Please check if the connection is correct and ensure that the device address is set correctly.")
    time.sleep(1)

  '''!
    @brief Set the output DAC value
    @param data value corresponding to the data voltage value
    @n (0 - 32767) This module is a DAC module with 15-bit accuracy, so (0 - 32767) corresponds to (0-2.5V) or (0-VCC), respectively, and the specific voltage range is determined by the module voltage selection fluctuation switch
  '''
  DAC.set_dac_out_voltage(32767)

  # Save the set voltage in the chip's internal memory for power-off retention.
  # DAC.store()

def loop():
  pass

if __name__ == "__main__":
  setup()
  while True:
    loop()