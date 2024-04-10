# -*- coding:utf-8 -*-
'''!
  @file  gp8403_output_data.py
  @brief Set output channels, voltage values, and ranges to convert I2C signals into 2-channel analog voltage signals (0-5V or 0-10V).
  @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license  The MIT License (MIT)
  @author  [fary](feng.yang@dfrobot.com)
  @version  V1.0
  @date  2023-05-24
  @url  https://github.com/DFRobot/DFRobot_GP8XXX
'''

from __future__ import print_function
import sys
import os
import time

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from DFRobot_GP8XXX import *

'''
----------------------------
| A0 |  A1 | A2 | i2c_addr |
----------------------------
| 0  |  0  | 0  |   0x58   |
----------------------------
| 1  |  0  | 0  |   0x59   |
----------------------------
| 0  |  1  | 0  |   0x5A   |
----------------------------
| 1  |  1  | 0  |   0x5B   |
----------------------------
| 0  |  0  | 1  |   0x5C   |
----------------------------
| 1  |  0  | 1  |   0x5D   |
----------------------------
| 0  |  1  | 1  |   0x5E   |
----------------------------
| 1  |  1  | 1  |   0x5F   |
----------------------------
'''
# This example uses software I2C driver. It is not recommended to pass in hardware I2C pin interfaces. The pin numbers should be provided in BCM encoding.
DAC = DFRobot_GP8403(i2c_sda=17,i2c_scl=27,i2c_addr=0x58)

def setup():

  while DAC.begin()!=0:
    print("Communication with the device failed. Please check if the connection is correct and ensure that the device address is set correctly.")
    time.sleep(1)

  '''
    @brief Set DAC output range
    @param mode DAC output range
    @n     OUTPUT_RANGE_5V(0-5V)
    @n     OUTPUT_RANGE_10V(0-10V)
  '''
  DAC.set_dac_outrange(DAC.OUTPUT_RANGE_10V)

  '''
    @brief Set different channel output DAC values
    @param data The data value corresponding to the voltage
    @n (0 - 4095) This module is a 15-bit precision DAC module, so (0 - 4095) corresponds to (0-5V) or (0-10V), and the specific voltage range depends on the fluctuation switch selected according to the module voltage
    @param channel Output channel
    @n 0: Channel 0
    @n 1: Channel 1
    @n 2: All channels
  '''
  DAC.set_dac_out_voltage(4095, 2)

  # Save the set voltage in the chip's internal memory for power-off retention.
  # DAC.store()

def loop():
  pass

if __name__ == "__main__":
  setup()
  while True:
    loop()