/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/stamp_pico
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/stamp_pico
*
* describe：NTP TIME.
* date：2021/9/25
*******************************************************************************
  This program scans the addresses 1-127 continuosly and shows the devices found on the TFT.
  该程序连续扫描地址 1-127 并显示在外部(内部)I2C发现的设备。
*/
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

// Set the name and password of the wifi to be connected.  配置所连接wifi的名称和密码
const char* ssid       = "M5";
const char* password   = "123456";

const char* ntpServer = "time1.aliyun.com"; //Set the connect NTP server.  设置连接的NTP服务器
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

void printLocalTime(){  //Output current time.  输出当前时间
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){ //Return 1 when the time is successfully obtained.  成功获取到时间返回1
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d \n%Y %H:%M:%S");  //Screen prints date and time.  屏幕打印日期和时间
}

void setup(){
  Serial.begin(115200);
  Serial.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, password); //Connect wifi and return connection status.  连接wifi并返回连接状态
  while(WiFi.status() != WL_CONNECTED) { //If the wifi connection fails.  若wifi未连接成功
    delay(500); //delay 0.5s.  延迟0.5s
    Serial.print(".");
  }
  Serial.println("\nCONNECTED!");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //init and get the time.  初始化并设置NTP
  printLocalTime();
  WiFi.disconnect(true);  //Disconnect wifi.  断开wifi连接
  WiFi.mode(WIFI_OFF);  //Set the wifi mode to off.  设置wifi模式为关闭
  delay(20);
}

void loop(){
  delay(1000);
  printLocalTime();
}