# -*- coding:utf-8 -*-
'''!
  @file  gp8302_output_data.py
  @brief Set the output current value and convert the I2C signal to a single channel 4-20mA analog current output.
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
DAC = DFRobot_GP8302(i2c_sda=17,i2c_scl=27)

def setup():

  while DAC.begin()!=0:
    print("Communication with the device failed. Please check if the connection is correct and ensure that the device address is set correctly.")
    time.sleep(1)

  '''
    @brief Set different channel outputs of the DAC
    @param data The data value corresponding to the current value
    @n (0 - 4095) This module is a 12-bit precision DAC module, so (0 - 4095) corresponds to (0-25mA)
  '''
  DAC.set_dac_out_electric_current(4095)

  # Save the set voltage in the chip's internal memory for power-off retention.
  # DAC.store()

def loop():
  pass

if __name__ == "__main__":
  setup()
  while True:
    loop()