/*!
  * @file DFRobot_GP8XXX.cpp
  * @brief GP8XXX series DAC driver library (GP8101, GP8101S, GP8211S, GP8413, GP8501, GP8503, GP8512, GP8403, GP8302 driver method is implemented)
  * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
  * @license     The MIT License (MIT)
  * @author      [fary](feng.yang@dfrobot.com)
  * @version  V1.0
  * @date  2023-05-10
  * @url https://github.com/DFRobot/DFRobot_GP8XXX
  */
 
#include "DFRobot_GP8XXX.h"

int DFRobot_GP8XXX_IIC::begin(void)
{
  _pWire->begin();
  _pWire->beginTransmission(_deviceAddr);
  return _pWire->endTransmission();
}

void DFRobot_GP8XXX_IIC::setDACOutRange(eOutPutRange_t range)
{
  uint8_t data=0x00;
  switch(range){
    case eOutputRange5V:    
      writeRegister(GP8XXX_CONFIG_CURRENT_REG>>1,&data,1);
      break;
    case eOutputRange10V:  
      data=0x11;
      writeRegister(GP8XXX_CONFIG_CURRENT_REG>>1,&data,1);
      break;
    default:
      break;
  }
}

void DFRobot_GP8XXX_IIC::setDACOutVoltage(uint16_t voltage, uint8_t channel)
{
  if(voltage > _resolution)
    voltage = _resolution;
    
  if(_resolution == RESOLUTION_12_BIT ){
    voltage = voltage << 4;
  }else if(_resolution == RESOLUTION_15_BIT) {
    voltage = voltage << 1;
  }
  sendData(voltage, channel);
}

void DFRobot_GP8512::setDACOutVoltage(uint16_t voltage, uint8_t channel)
{
  if(voltage > _resolution)
    voltage = _resolution;
  sendData(voltage, channel);
}

void DFRobot_GP8XXX_IIC::sendData(uint16_t data, uint8_t channel)
{
  uint8_t buff[4]={ uint8_t(data & 0xff) , uint8_t(data >> 8) , uint8_t(data & 0xff) , uint8_t(data >> 8)};
  if(channel == 0){
    writeRegister(GP8XXX_CONFIG_CURRENT_REG,(void *)buff,2);
  }else if(channel == 1){
    writeRegister(GP8XXX_CONFIG_CURRENT_REG<<1,(void *)buff,2);
  }else if(channel == 2){
    writeRegister(GP8XXX_CONFIG_CURRENT_REG,(void *)buff,4);
  }
}

uint8_t DFRobot_GP8XXX_IIC::writeRegister(uint8_t reg, void* pBuf, size_t size)
{
  if(pBuf == NULL){
	  return 1;
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_deviceAddr);
  _pWire->write(reg);
  for( uint16_t i = 0; i < size; i++ ){
    _pWire->write(_pBuf[i]);
  }
  _pWire->endTransmission();

  return 0;
}

void DFRobot_GP8XXX_IIC::store(void)
{
  #if defined(ESP32)
    _pWire->~TwoWire();
  #elif !defined(ESP8266)
    _pWire->end();
  #endif
  
  pinMode(_scl, OUTPUT);
  pinMode(_sda, OUTPUT);
  digitalWrite(_scl, HIGH);
  digitalWrite(_sda, HIGH);
  startSignal();
  sendByte(GP8XXX_STORE_TIMING_HEAD, 0, 3, false);
  stopSignal();
  startSignal();
  sendByte(GP8XXX_STORE_TIMING_ADDR,0);
  sendByte(GP8XXX_STORE_TIMING_CMD1,0);
  stopSignal();
  
  startSignal();
  sendByte(_deviceAddr<<1, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  sendByte(GP8XXX_STORE_TIMING_CMD2, 1);
  stopSignal();

  delay(GP8XXX_STORE_TIMING_DELAY);

  startSignal();
  sendByte(GP8XXX_STORE_TIMING_HEAD, 0, 3, false);
  stopSignal();
  startSignal();
  sendByte(GP8XXX_STORE_TIMING_ADDR);
  sendByte(GP8XXX_STORE_TIMING_CMD2);
  stopSignal();
  _pWire->begin();
}


void DFRobot_GP8XXX_IIC::startSignal(void)
{
  digitalWrite(_scl,HIGH);
  digitalWrite(_sda,HIGH);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_sda,LOW);
  delayMicroseconds(I2C_CYCLE_AFTER);
  digitalWrite(_scl,LOW);
  delayMicroseconds(I2C_CYCLE_TOTAL);
}


void DFRobot_GP8XXX_IIC::stopSignal(void)
{
  digitalWrite(_sda,LOW);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_scl,HIGH);
  delayMicroseconds(I2C_CYCLE_TOTAL);
  digitalWrite(_sda,HIGH);
  delayMicroseconds(I2C_CYCLE_TOTAL);
}


uint8_t DFRobot_GP8XXX_IIC::sendByte(uint8_t data, uint8_t ack, uint8_t bits, bool flag)
{
  for(int i=bits-1; i>=0;i--){  
    if(data & (1<<i)){
      digitalWrite(_sda,HIGH);
    }else{
      digitalWrite(_sda,LOW);
    }
    delayMicroseconds(I2C_CYCLE_BEFORE); 
    digitalWrite(_scl,HIGH);
    delayMicroseconds(I2C_CYCLE_TOTAL);
    digitalWrite(_scl,LOW);
    delayMicroseconds(I2C_CYCLE_AFTER); 
  }
  if(flag) return recvAck(ack);
  else {
    digitalWrite(_sda,LOW);
    digitalWrite(_scl,HIGH);
    return 0;
  }
}

uint8_t DFRobot_GP8XXX_IIC::recvAck(uint8_t ack)
{
  uint8_t ack_=0;
  uint16_t errorTime = 0;
  pinMode(_sda,INPUT_PULLUP);
  digitalWrite(_sda,HIGH);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_scl,HIGH);
  delayMicroseconds(I2C_CYCLE_AFTER);
  while(digitalRead(_sda) != ack){
      delayMicroseconds(1);
      errorTime++;
      if(errorTime > 250) break;
  }
  ack_=digitalRead(_sda);
  delayMicroseconds(I2C_CYCLE_BEFORE);
  digitalWrite(_scl,LOW);
  delayMicroseconds(I2C_CYCLE_AFTER);
  pinMode(_sda,OUTPUT);
  return ack_;
}



int DFRobot_GP8XXX_PWM::begin()
{
  #if (defined ESP8266)
    analogWriteRange(255);
  #endif  
  if(_pin0 !=-1 ){
    pinMode(_pin0, OUTPUT);
    analogWrite(_pin0,0);
  }
  if(_pin1 !=-1 ){
    pinMode(_pin1, OUTPUT);
    analogWrite(_pin1,0);
  }
  return 0;
}

void DFRobot_GP8XXX_PWM::setDACOutVoltage(uint16_t data , uint8_t channel)
{ 
  sendData(data, channel);
}


void DFRobot_GP8XXX_PWM::sendData(uint8_t data, uint8_t channel)
{
    if( (channel == 0) && (_pin0 != -1) ){
      analogWrite(_pin0, data);
    }else if( (channel == 1) && (_pin1 != -1) ){
      analogWrite(_pin1, data);
    }else if( (channel == 2) && (_pin0 != -1) && (_pin1 != -1) ){
      analogWrite(_pin0, (uint8_t)data);
	    analogWrite(_pin1, (uint8_t)data);
    }
}