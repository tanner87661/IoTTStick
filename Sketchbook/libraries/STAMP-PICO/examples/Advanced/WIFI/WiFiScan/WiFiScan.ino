/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/STAMP-PICO
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/STAMP-PICO
*
* describe：Wifi scan.  wifi扫描
* date：2021/10/1
*******************************************************************************
*/
#include <Arduino.h>
#include "WiFi.h"

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);// Set WiFi to station mode and disconnect from an AP if it was previously connected.  将WiFi设置为站模式，如果之前连接过AP，则断开连接
  WiFi.disconnect();  //Turn off all wifi connections.  关闭所有wifi连接
  delay(100); //100 ms delay.  延迟100ms
  Serial.print("WIFI SCAN"); //print string.  打印字符串
  Serial.println("Please press Btn to scan");
  pinMode(39,INPUT);
}

void loop()
{
  if(!digitalRead(39)){  //If button is pressed.  如果按键按下
    Serial.println("scan start\n");
    int n = WiFi.scanNetworks();  //return the number of networks found.  返回发现的网络数
    if (n == 0){  //If no network is found.  如果没有找到网络
      Serial.println("no networks found");
    }else{  //If have network is found.  找到网络
      Serial.printf("networks list :,found:%d\n",n);
      for(int i=0;i<n;i++){
        // Print SSID and RSSI for each network found.  打印每个找到的网络的SSID和信号强度
        Serial.printf("%d:",i);
        Serial.print(WiFi.SSID(i));
        Serial.printf("(%d)",WiFi.RSSI(i));
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      }
    }
    delay(1000);
    Serial.println("Please press Btn to rescan");
  }
}
