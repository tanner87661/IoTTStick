
#ifndef _MODULE_GRBL_H_
#define _MODULE_GRBL_H_

#include <Arduino.h>

class GRBL
{
    private:
        void sendByte(byte b);
        void sendBytes(uint8_t *data, size_t size);
        TwoWire *_Wire;
        uint8_t _addr;
    public:
        GRBL(uint8_t addr=0x70);
        void Init();
        void Init(TwoWire *Wire);
        void Init(TwoWire *Wire, uint32_t x_step, uint32_t y_step,uint32_t z_step,uint32_t acc);
        int addr;
        String mode;
        void sendGcode(char *c);
        void unLock();
        void readClean();
        void waitIdle();
        void setMotor(int x=0, int y=0, int z=0, int speed=300);
        void setMode(String mode);
        String readLine();
        String readStatus();
        bool readIdle();
        bool inLock();
};

#endif