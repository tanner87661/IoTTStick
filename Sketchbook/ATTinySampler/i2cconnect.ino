void initI2C()
{
  i2cConnection.begin();
}

void sendValue(uint16_t currVal)
{
  i2cConnection.beginTransmission(deviceAddr);
//  i2cConnection.send(deviceAddr);
  i2cConnection.send(currVal>>8);
  i2cConnection.send(currVal & 0x00FF);
  if(i2cConnection.endTransmission() != 0){}
}

void verifyI2CConfig()
{
  
}
