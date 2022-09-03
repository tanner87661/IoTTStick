#ifndef __M5_BM8563_RTC_H__
#define __M5_BM8563_RTC_H__

#include "Arduino.h"
#include <Wire.h>

#define DEVICE_ADDR 0x51

struct rtc_time_type
{
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
  rtc_time_type(uint8_t hours = 0, uint8_t minutes = 0, uint8_t seconds = 0)
  : Hours   { hours   }
  , Minutes { minutes }
  , Seconds { seconds }
  {}
};

struct rtc_date_type
{
  uint16_t Year;
  uint8_t Month;
  uint8_t Date;
  uint8_t WeekDay;
  rtc_date_type(uint16_t year = 2000, uint8_t month = 0, uint8_t date = 0, uint8_t weekDay = 0)
  : Year    { year    }
  , Month   { month   }
  , Date    { date    }
  , WeekDay { weekDay }
  {}
};


class BM8563 { 
  
private:
  void Bcd2asc(void);
  void DataMask();
  void Str2Time(void);
  void writeReg(uint8_t reg, uint8_t data);
  uint8_t ReadReg(uint8_t reg);
  uint8_t Bcd2ToByte(uint8_t Value);
  uint8_t ByteToBcd2(uint8_t Value);
  
   
private:

  /*定义数组用来存储读取的时间数据 */
  uint8_t _trdata[7]; 
  /*定义数组用来存储转换的 asc 码时间数据*/
  //uint8_t asc[14]; 
  uint8_t _addr;
  TwoWire *_wire;

public:

  BM8563();
  BM8563(uint8_t addr);
  void begin();
  void begin(TwoWire *wire);
  void begin(TwoWire *wire, uint8_t scl, uint8_t sda, uint32_t i2c_freq);
  void getBm8563Time(void);

  void setTime(rtc_time_type* RTC_TimeStruct);
  void setDate(rtc_date_type* RTC_DateStruct);

  void getTime(rtc_time_type* RTC_TimeStruct);
  void getDate(rtc_date_type* RTC_DateStruct); 

  int setAlarmIRQ(int afterSeconds);
  int setAlarmIRQ( const rtc_time_type &RTC_TimeStruct);
  int setAlarmIRQ( const rtc_date_type &RTC_DateStruct, const rtc_time_type &RTC_TimeStruct);
  
  void clearIRQ();
  void disableIRQ();

public:
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Week;
  uint8_t Day;
  uint8_t Month;
  uint8_t Year;
  uint8_t DateString[9];
  uint8_t TimeString[9];

  uint8_t asc[14];

};

#endif
