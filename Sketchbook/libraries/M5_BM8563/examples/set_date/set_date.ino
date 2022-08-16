#include "M5_BM8563.h"

BM8563 RTC;

rtc_time_type rtc_time_unit;
rtc_date_type rtc_date_unit;

char str_buffer[64];

void showlog(rtc_time_type *rtc_time, rtc_date_type *rtc_date) {
    sprintf(str_buffer,"RTC Time Now is %02d:%02d:%02d",
          rtc_time->Hours, rtc_time->Minutes, rtc_time->Seconds);
    Serial.println(str_buffer);
    sprintf(str_buffer,"RTC Date Now is %02d:%02d:%02d WeekDay:%02d",
          rtc_date->Year, rtc_date->Month, rtc_date->Date, rtc_date->WeekDay);
    Serial.println(str_buffer);
}

void setup()
{
  RTC.begin();
  Serial.begin(115200);
  delay(10);
  rtc_time_unit.Hours = 14;
  rtc_time_unit.Minutes = 40;
  rtc_time_unit.Seconds = 5;

  rtc_date_unit.WeekDay = 4;
  rtc_date_unit.Month = 7;
  rtc_date_unit.Date = 15;
  rtc_date_unit.Year = 2021;

  RTC.setTime(&rtc_time_unit);
  RTC.setDate(&rtc_date_unit);
}

void loop()
{

  RTC.getTime(&rtc_time_unit);
  RTC.getDate(&rtc_date_unit);

  showlog(&rtc_time_unit, &rtc_date_unit);
  delay(300);
}
