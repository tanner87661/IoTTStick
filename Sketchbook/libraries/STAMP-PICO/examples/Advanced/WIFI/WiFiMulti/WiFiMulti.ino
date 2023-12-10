/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/STAMP-PICO
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/STAMP-PICO
*
* describe：WIFI Multi.  多wifi择优
* date：2021/10/1
*******************************************************************************
 *  Connect to the best AP based on a given wifi list
 *  根据给定wifi的列表连接到最好的AP
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

void setup()
{
  Serial.begin(115200);
  wifiMulti.addAP("wifi1", "213123");  //Storage wifi configuration information 1.  存储wifi配置信息1
  wifiMulti.addAP("M5wifi", "213431241234");
  wifiMulti.addAP("aaa", "sadf");
  Serial.print("Connecting Wifi..."); //Serial port format output string.  串口格式化输出字符串
}

void loop()
{
  if(wifiMulti.run() == WL_CONNECTED) { //If the connection to wifi is established successfully.  如果与wifi成功建立连接
    Serial.print("WiFi connected\n\nSSID:");
    Serial.println(WiFi.SSID());  //Output Network name.  输出网络名称
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());  //Output signal strength.  输出信号强度
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); //Output IP Address.  输出IP地址
    delay(1000);
  }else{
    //If the connection to wifi is not established successfully.  如果没有与wifi成功建立连接
    Serial.print(".");
    delay(1000);
  }
}