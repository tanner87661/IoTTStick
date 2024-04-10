'''!
  @file DFRobot_GP8XXX.py
  @brief GP8XXX series DAC driver library (GP8101, GP8211S, GP8413, GP8501, GP8503, GP8512, GP8403, GP8302 driver method is implemented)
  @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license     The MIT License (MIT)
  @author      [fary](feng.yang@dfrobot.com)
  @version  V1.0
  @date  2023-05-10
  @url https://github.com/DFRobot/DFRobot_GP8XXX
'''

from __future__ import print_function
import time
import datetime
import RPi.GPIO as GPIO
from abc import ABC, abstractmethod
## i2c address
GP8XXX_I2C_DEVICE_ADDR              = 0x58

class DFRobot_GP8XXX(ABC):
  ##Select DAC output voltage of 0-2.5V
  OUTPUT_RANGE_2_5V = 0
  ##Select DAC output voltage of 0-5V
  OUTPUT_RANGE_5V = 1
  ##Select DAC output voltage of 0-10V
  OUTPUT_RANGE_10V = 2
  ##Select DAC output voltage of 0-VCC
  OUTPUT_RANGE_VCC = 3
  RESOLUTION_12_BIT =0x0FFF
  RESOLUTION_15_BIT =0x7FFF
  def __init__(self):
    '''!
      @brief Module init
    '''
    pass
  def begin(self):
    pass
  
  def set_dac_out_voltage(self,voltage,channel):
    pass

class DFRobot_GP8XXX_IIC(DFRobot_GP8XXX):

  GP8XXX_CONFIG_CURRENT_REG           = 0x02
  ## Store function timing start head        
  GP8XXX_STORE_TIMING_HEAD            = 0x02
  ## The first address for entering store timing        
  GP8XXX_STORE_TIMING_ADDR            = 0x10 
  ## The command 1 to enter store timing    
  GP8XXX_STORE_TIMING_CMD1            = 0x03
  ## The command 2 to enter store timing  
  GP8XXX_STORE_TIMING_CMD2            = 0x00  
  ## Store procedure interval delay time: 10ms, more than 7ms
  GP8XXX_STORE_TIMING_DELAY           = 0.01
  ## Total I2C communication cycle   
  I2C_CYCLE_TOTAL                     = 0.000005   
  ## The first half cycle 2 of the total I2C communication cycle  
  I2C_CYCLE_BEFORE                    = 0.000001     
  ## The second half cycle 3 of the total I2C communication cycle
  I2C_CYCLE_AFTER                     = 0.000002     
 
  def __init__(self,sda,scl,resolution,device_addr=GP8XXX_I2C_DEVICE_ADDR):
    '''!
      @brief I2C class initialization
      @param SDA I2C data pin
      @param SCL I2C clock pin
      @param Resolution resolution
      @param device_addr the I2C device address 
    '''
    self._resolution = resolution
    self._device_addr = device_addr
    self._scl = scl
    self._sda = sda

  def begin(self):
    '''!
      @brief Initialize the function
      @return returns 0 for success, and other values for failure 
    '''
    GPIO.setwarnings(False) # masks warnings
    GPIO.setmode(GPIO.BCM) # Set the pin mode BCM
    GPIO.setup(self._scl, GPIO.OUT)
    GPIO.setup(self._sda, GPIO.OUT)
    self._start_signal()
    ret = self._send_byte(self._device_addr << 1, 1)
    self._stop_signal()
    return ret

  def set_dac_outrange(self, range):
    '''!
      @brief Set the DAC output range
      @param range DAC output range
      @n     eOutputRange0_5V(0-5V)
      @n     eOutputRange0_10V(0-10V)
      @return NONE
    '''
    if range == self.OUTPUT_RANGE_5V :
      list =[0x00]
      self._write_reg(self.GP8XXX_CONFIG_CURRENT_REG>>1,list,1)
    elif range == self.OUTPUT_RANGE_10V :
      list = [0x11]
      self._write_reg(self.GP8XXX_CONFIG_CURRENT_REG>>1,list,1)

  def set_dac_out_voltage(self, data, channel=0):
    '''!
      @brief Set different channel output DAC values
      @param data value corresponding to the data voltage value
      @param channel output channel
      @n 0: Channel 0 (valid when PWM0 output is configured)
      @n 1: Channel 1 (effective when PWM1 output is configured)
      @n 2: All channels (valid when configuring dual channel outputs)
      @return NONE
    '''
    if data > self._resolution :
      data = self._resolution
    if self._resolution == self.RESOLUTION_12_BIT:
      data = data << 4
    elif self._resolution == self.RESOLUTION_15_BIT:
      data = data << 1
    self._send_data(data, channel)

  def store(self):
    '''!
      @brief Save the set voltage inside the chip
      @return NONE
    '''
    self._start_signal()
    self._send_byte(self.GP8XXX_STORE_TIMING_HEAD, 0, 3, False)
    self._stop_signal()
    self._start_signal()
    self._send_byte(self.GP8XXX_STORE_TIMING_ADDR)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD1)
    self._stop_signal()

    self._start_signal()
    self._send_byte(self._device_addr << 1, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2, 1)
    self._stop_signal()

    time.sleep(self.GP8XXX_STORE_TIMING_DELAY)

    self._start_signal()
    self._send_byte(self.GP8XXX_STORE_TIMING_HEAD, 0, 3, False)
    self._stop_signal()
    self._start_signal()
    self._send_byte(self.GP8XXX_STORE_TIMING_ADDR)
    self._send_byte(self.GP8XXX_STORE_TIMING_CMD2)
    self._stop_signal()

  def _send_data(self, data, channel):
    '''!
      @brief Set the IIC input value
      @param data input value to be set (0-fff)
      @param channel output channel
      @n 0: Channel 0 (valid when PWM0 output is configured)
      @n 1: Channel 1 (effective when PWM1 output is configured)
      @n 2: All channels (valid when configuring dual channel outputs)
      @return NONE
    '''
    buf = [data & 0xff,(data >> 8 ) & 0xff ,data & 0xff,(data >> 8 ) & 0xff]
    if channel == 0 :
      self._write_reg(self.GP8XXX_CONFIG_CURRENT_REG, buf,2)
    elif channel == 1 :
      self._write_reg(self.GP8XXX_CONFIG_CURRENT_REG << 1, buf,2)
    elif channel == 2 :
      self._write_reg(self.GP8XXX_CONFIG_CURRENT_REG , buf,4)

  def _write_reg(self, reg, data,size):
    '''!
      @brief writes a value to the device register
      @param reg register address
      @param data The data to be written
      @param size data length
    '''
    self._start_signal()
    self._send_byte(self._device_addr << 1, 1)
    self._send_byte(reg, 1)
    for i in range(0,size):
      self._send_byte(data[i], 1)
    self._stop_signal()

  def _start_signal(self):
    '''!
      @brief I2C start signal
    '''
    GPIO.output(self._scl, GPIO.HIGH)
    GPIO.output(self._sda, GPIO.HIGH)
    time.sleep(self.I2C_CYCLE_BEFORE)
    GPIO.output(self._sda, GPIO.LOW)
    time.sleep(self.I2C_CYCLE_AFTER)
    GPIO.output(self._scl, GPIO.LOW)
    time.sleep(self.I2C_CYCLE_TOTAL)

  def _stop_signal(self):
    '''!
      @brief I2C stop signal
    '''
    GPIO.output(self._sda, GPIO.LOW)
    time.sleep(self.I2C_CYCLE_BEFORE)
    GPIO.output(self._scl, GPIO.HIGH)
    time.sleep(self.I2C_CYCLE_TOTAL)
    GPIO.output(self._sda, GPIO.HIGH)
    time.sleep(self.I2C_CYCLE_TOTAL)

  def _recv_ack(self, ack=0):
    '''!
      @brief Receive an answer
      @param acknowledgment signal to be received by ack
      @return Acknowledgment signal
    '''
    ack_ = 0
    error_time = 0
    GPIO.setup(self._sda, GPIO.IN)
    time.sleep(self.I2C_CYCLE_BEFORE)
    GPIO.output(self._scl, GPIO.HIGH)
    time.sleep(self.I2C_CYCLE_AFTER)
    while GPIO.input(self._sda) != ack:
      time.sleep(0.000001)
      error_time += 1
      if error_time > 250:
        break
    ack_ = GPIO.input(self._sda)
    time.sleep(self.I2C_CYCLE_BEFORE)
    GPIO.output(self._scl, GPIO.LOW)
    time.sleep(self.I2C_CYCLE_AFTER)
    GPIO.setup(self._sda, GPIO.OUT)
    return ack_

  def _send_byte(self, data, ack=0, bits=8, flag=True):
    '''!
      @brief Software I2C sends data
      @param data The data to send
      @param ACK acknowledgement signal
      @param how many bits to send
      @param whether the flag receives a reply
      @return Acknowledgment signal
    '''
    i = bits
    data = data & 0xFF
    while i > 0:
      i -= 1
      if data & (1 << i):
        GPIO.output(self._sda, GPIO.HIGH)
      else:
        GPIO.output(self._sda, GPIO.LOW)
      time.sleep(self.I2C_CYCLE_BEFORE)
      GPIO.output(self._scl, GPIO.HIGH)
      time.sleep(self.I2C_CYCLE_TOTAL)
      GPIO.output(self._scl, GPIO.LOW)
      time.sleep(self.I2C_CYCLE_AFTER)
    if flag:
      return self._recv_ack(ack)
    else:
      GPIO.output(self._sda, GPIO.LOW)
      GPIO.output(self._scl, GPIO.HIGH)
    return ack


class DFRobot_GP8503(DFRobot_GP8XXX_IIC):
  def __init__(self,i2c_sda,i2c_scl):
    return super().__init__(sda=i2c_sda,scl=i2c_scl,resolution=self.RESOLUTION_12_BIT)

class DFRobot_GP8211S(DFRobot_GP8XXX_IIC):
  def __init__(self,i2c_sda,i2c_scl):
    return super().__init__(sda=i2c_sda,scl=i2c_scl,resolution=self.RESOLUTION_15_BIT)

class DFRobot_GP8512(DFRobot_GP8XXX_IIC):
  def __init__(self,i2c_sda,i2c_scl):
    return super().__init__(sda=i2c_sda,scl=i2c_scl,resolution=self.RESOLUTION_15_BIT)
  def set_dac_out_voltage(self, data, channel=0):
    '''!
      @brief Set different channel output DAC values
      @param data value corresponding to the data voltage value
      @param channel output channel
      @n 0: Channel 0 (valid when PWM0 output is configured)
      @n 1: Channel 1 (effective when PWM1 output is configured)
      @n 2: All channels (valid when configuring dual channel outputs)
      @return NONE
    '''
    if data > self._resolution :
      data = self._resolution
    self._send_data(data, channel)

class DFRobot_GP8413(DFRobot_GP8XXX_IIC):
  def __init__(self,i2c_sda,i2c_scl,i2c_addr=0x58):
    return super().__init__(sda=i2c_sda,scl=i2c_scl,resolution=self.RESOLUTION_15_BIT,device_addr=i2c_addr)

class DFRobot_GP8403(DFRobot_GP8XXX_IIC):
  def __init__(self,i2c_sda,i2c_scl,i2c_addr=0x58):
    return super().__init__(sda=i2c_sda,scl=i2c_scl,resolution=self.RESOLUTION_12_BIT,device_addr=i2c_addr)

class DFRobot_GP8302(DFRobot_GP8XXX_IIC):
  def __init__(self,i2c_sda,i2c_scl,i2c_addr=0x58):
    return super().__init__(sda=i2c_sda,scl=i2c_scl,resolution=self.RESOLUTION_12_BIT,device_addr=i2c_addr)
  def set_dac_out_electric_current(self,data):
    return self.set_dac_out_voltage(data)

class DFRobot_GP8XXX_PWM(DFRobot_GP8XXX):

  def __init__(self,pin0=-1,pin1=-1):
    '''!
      @brief PWM class initialization
      @param pin 0 PWM channel 0 corresponds to the pin 
      @param pin corresponding to pin1 PWM channel 1 
    '''
    self._pin0 = pin0
    self._pin1 = pin1

  def begin(self):
    '''!
      @brief Initialize the function
      @return 0
    '''
    GPIO.setwarnings(False)  # Block warnings
    GPIO.setmode(GPIO.BCM)   # Set the pin mode BCM
    if self._pin0 != -1 :
      GPIO.setup(self._pin0, GPIO.OUT)  # Set pin R0 direction output
      self._PWM0 = GPIO.PWM(self._pin0, 100)
      self._PWM0.start(0)
    if self._pin1 != -1 :
      GPIO.setup(self._pin1, GPIO.OUT)  # Set pin R1 direction output
      self._PWM1 = GPIO.PWM(self._pin1,100)
      self._PWM1.start(0)

  def set_dac_out_voltage(self, data, channel=0):
    '''!
      @brief Set different channel output DAC values
      @param data pwm pulse width
      @param channel output channel
      @n 0: Channel 0 (valid when PWM0 output is configured)
      @n 1: Channel 1 (effective when PWM1 output is configured)
      @n 2: All channels (valid when configuring dual channel outputs)
      @return NONE
    '''
    if data > 100:
      data = 100
    if data < 0:
      data = 0
    if (channel == 0) and (self._pin0 != -1) :
      self._PWM0.ChangeDutyCycle(data)
    elif (channel == 1) and (self._pin1 != -1) :
      self._PWM1.ChangeDutyCycle(data)
    elif (channel == 2) and (self._pin0 != -1) and (self._pin1 != -1) :
      self._PWM0.ChangeDutyCycle(data)
      self._PWM1.ChangeDutyCycle(data)

class DFRobot_GP8501(DFRobot_GP8XXX_PWM):
  def __init__(self,pin0=-1,pin1=-1):
    return super().__init__(pin0,pin1)

class DFRobot_GP8101(DFRobot_GP8XXX_PWM):
  def __init__(self,pin0=-1):
    return super().__init__(pin0,-1)
  
class DFRobot_GP8101S(DFRobot_GP8XXX_PWM):
  def __init__(self,pin0=-1):
    return super().__init__(pin0,-1)