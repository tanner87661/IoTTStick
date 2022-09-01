#include "MODULE_GRBL13.2.h"
#include <Wire.h>

GRBL::GRBL(uint8_t addr){
    this->addr = addr;
}

void GRBL::Init()		
{
    _Wire = &Wire1;
}

void GRBL::Init(TwoWire *Wire)		
{
    _Wire = Wire;
}

void GRBL::Init(TwoWire *Wire, uint32_t x_step, uint32_t y_step, uint32_t z_step, uint32_t acc)		
{
	_Wire = Wire;
    if(x_step){
        char code[256];
        sprintf(code,"$0=%d",x_step);
        sendGcode(code);
    }
    if(y_step){
        char code[256];
        sprintf(code,"$1=%d",y_step);
        sendGcode(code);
    }
    if(z_step){
        char code[256];
        sprintf(code,"$2=%d",z_step);
        sendGcode(code);
    }
    if(acc){
        char code[256];
        sprintf(code,"$8=%d",acc);
        sendGcode(code);
    }
}

void GRBL::sendGcode(char *c)
{
  _Wire->beginTransmission(addr);
  while ((*c) != 0) {
    _Wire->write(*c);
    c++;
  }
  _Wire->write(0x0d);
  _Wire->write(0x0a);
  _Wire->endTransmission();
}

void GRBL::sendByte(byte b) {
  _Wire->beginTransmission(addr);
  _Wire->write(b);
  _Wire->endTransmission();
}

void GRBL::sendBytes(uint8_t *data, size_t size) {
  _Wire->beginTransmission(addr);
  _Wire->write(data,size);
  _Wire->endTransmission();
}

void GRBL::readClean(){
    while(1){
        uint8_t i = 0;
        char data[10];
        _Wire->requestFrom(addr, 10);
        while (_Wire->available() > 0) {
            data[i++] = _Wire->read();
        }
        if(data[9] == 0xff) break;
    }
}

void GRBL::unLock() {
  this->sendByte(0x18);
  delay(5);
  char bytes[] = "$X\r\n";
  this->sendBytes((uint8_t *)bytes, 4);
}

void GRBL::setMotor(int x, int y, int z, int speed) {

    char code[256];
    memset(code,0,sizeof(char)*256);
    sprintf(code,"G1 X%dY%dZ%d F%d",x,y,z,speed);
    return this->sendGcode(code);
}

void GRBL::setMode(String mode){
    if(mode == "distance"){
        char bytes[] = "G91\n";
        this->sendBytes((uint8_t *)bytes, 4);
        this->mode = mode;
    }else if(mode == "absolute"){
        char bytes[] = "G90\n";
        this->sendBytes((uint8_t *)bytes, 4);
        this->mode = mode;
    }
}

void GRBL::waitIdle(){
    this->readClean();
    while(1){
        this->sendByte('@');
        char state;
        _Wire->requestFrom(addr, 1);
        if (_Wire->available() > 0) {
            state = _Wire->read();
        }
        if(state == 'I'){
            break;
        }
        delay(5);
    }
}

//read grbl return message

String GRBL::readLine() {
    String Data = ""; 
    while(1){
        uint8_t i = 0;
        char data[10];
        _Wire->requestFrom(addr, 10);
        while (_Wire->available() > 0) {
            data[i] = _Wire->read();
            i++;
        }
        Data += data;
        if (data[9] == 0xff) {
            break;
        }
    }
    return Data;
}

String GRBL::readStatus() {
    this->readClean();
    this->sendByte('@');
    return this->readLine();
}

    // read grbl state
bool GRBL::readIdle() {
    return this->readStatus()[0] == 'I';
}

bool GRBL::inLock() {
    return this->readStatus()[0] == 'A';
}