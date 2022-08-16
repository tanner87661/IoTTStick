/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Series sample source code
*                          配套  M5Series 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/rtc
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/rtc
*
* Describe: Unit RTC.
* Date: 2022/7/15
*******************************************************************************
  Real-time time output on serial port every one second
  每个一秒在串口输出实时时间
*/
#include "Unit_RTC.h"

Unit_RTC RTC;

rtc_time_type rtc_time;
rtc_date_type rtc_date;

char str_buffer[64];

void showlog(rtc_time_type *rtc_time, rtc_date_type *rtc_date) {
    sprintf(str_buffer, "RTC Time Now is %02d:%02d:%02d", rtc_time->Hours,
            rtc_time->Minutes, rtc_time->Seconds);
    Serial.println(str_buffer);
    sprintf(str_buffer, "RTC Date Now is %02d:%02d:%02d WeekDay:%02d",
            rtc_date->Year, rtc_date->Month, rtc_date->Date, rtc_date->WeekDay);
    Serial.println(str_buffer);
}

void setup() {
    RTC.begin();
    Serial.begin(115200);
    delay(10);
    rtc_time.Hours   = 14;
    rtc_time.Minutes = 40;
    rtc_time.Seconds = 5;

    rtc_date.WeekDay = 4;
    rtc_date.Month   = 7;
    rtc_date.Date    = 15;
    rtc_date.Year    = 2021;

    RTC.setTime(&rtc_time);
    RTC.setDate(&rtc_date);
}

void loop() {
    RTC.getTime(&rtc_time);
    RTC.getDate(&rtc_date);

    showlog(&rtc_time, &rtc_date);
    delay(300);
}
