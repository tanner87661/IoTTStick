# -*- coding:utf-8 -*-
'''!
  @file  gp8211s_output_data.py
  @brief Set the data values corresponding to the output voltage and convert the I2C signal into a single-channel analog voltage signal ranging from 0-10V.
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
# This example uses software I2C driver and it is not recommended to pass the hardware I2C pin interfaces. Instead, the pin numbers should be passed in BCM encoding.
DAC = DFRobot_GP8503(i2c_sda=17,i2c_scl=27)

def setup():
  
  while DAC.begin()!=0:
    print("Communication with the device failed. Please check if the connection is correct and ensure that the device address is set correctly.")
    time.sleep(1)


  '''
    @brief Set the DAC output range
    @param mode DAC output range
    @n     OUTPUT_RANGE_5V (0-5V)
    @n     OUTPUT_RANGE_10V (0-10V)
  '''
  DAC.set_dac_outrange(DAC.OUTPUT_RANGE_10V)

  '''
    @brief Set channel output DAC values
    @param data Data value corresponding to the voltage
    @n (0 - 32767) This module is a 15-bit precision DAC module, so (0 - 32767) corresponds to (0-10V)
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