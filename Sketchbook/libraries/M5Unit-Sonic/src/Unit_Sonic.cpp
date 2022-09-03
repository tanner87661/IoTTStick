#include "Unit_Sonic.h"

/*! @brief Initialize the Sonic. */
void SONIC_I2C::begin(TwoWire* wire, uint8_t addr, uint8_t sda, uint8_t scl,
                      uint32_t speed) {
    _wire  = wire;
    _addr  = addr;
    _sda   = sda;
    _scl   = scl;
    _speed = speed;
    _wire->begin((int)_sda, (int)_scl, (uint32_t)_speed);
}

/*! @brief Get distance data. */
float SONIC_I2C::getDistance() {
    uint32_t data;
    _wire->beginTransmission(_addr);  // Transfer data to 0x57. 将数据传输到0x57
    _wire->write(0x01);
    _wire->endTransmission();  // Stop data transmission with the Ultrasonic
                               // Unit. 停止与Ultrasonic Unit的数据传输
    delay(120);
    _wire->requestFrom(_addr,
                       (uint8_t)3);  // Request 3 bytes from Ultrasonic Unit.
                                     // 向Ultrasonic Unit请求3个字节。
    data = _wire->read();
    data <<= 8;
    data |= _wire->read();
    data <<= 8;
    data |= _wire->read();
    float Distance = float(data) / 1000;
    if (Distance > 4500.00) {
        return 4500.00;
    } else {
        return Distance;
    }
}

/*! @brief Initialize the Sonic. */
void SONIC_IO::begin(uint8_t trig, uint8_t echo) {
    _trig = trig;
    _echo = echo;
    pinMode(_trig,
            OUTPUT);  // Sets the trigPin as an Output. 将 TrigPin 设置为输出
    pinMode(_echo,
            INPUT);  // Sets the echoPin as an Input. 将 echoPin 设置为输入
}

float SONIC_IO::getDuration() {
    digitalWrite(_trig, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds.
    // 将TrigPin设定为高电平状态，持续10微秒。

    digitalWrite(_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trig, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    //读取echoPin，返回以微秒为单位的声波移动时间
    float duration = pulseIn(_echo, HIGH);

    return duration;
}

/*! @brief Get distance data. */
float SONIC_IO::getDistance() {
    // Calculating the distance
    float Distance = getDuration() * 0.34 / 2;
    if (Distance > 4500.00) {
        return 4500.00;
    } else {
        return Distance;
    }
}