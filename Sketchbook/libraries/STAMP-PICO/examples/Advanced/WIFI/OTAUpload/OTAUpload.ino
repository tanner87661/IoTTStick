/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/stamp_pico
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/stamp_pico
*
* describe：OTA Upload.  隔空传输程序
* date：2021/10/1
*******************************************************************************
  PC and STAMP-PICO can only be used on the same wifi.  电脑和STAMP-PICO需在同一wifi下才可使用
  When the OTA is ready, restart the Arduino client from Tools > Ports > Network ports to instantly transmit the program wirelessly.
  OTA 准备好后重启Arduino客户端在工具->端口->网络端口,即刻无线传输程序
*/
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// Set the name and password of the wifi to be connected.  配置所连接wifi的名称和密码
const char* ssid     = "Explore-F";
const char* password = "xingchentansuo123";

void setup() {
  Serial.begin(115200);
  pinMode(39,INPUT);
  WiFi.begin(ssid, password); //Connect wifi and return connection status.  连接wifi并返回连接状态
  Serial.print("Waiting Wifi Connect");
  while (WiFi.status() != WL_CONNECTED) { //If the wifi connection fails.  若wifi未连接成功
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("WiFi Connect To: ");
  Serial.println(WiFi.SSID());  //Output Network name.  输出网络名称
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //Output IP Address.  输出IP地址

  ArduinoOTA.setHostname("STAMP-PICO"); //Set the network port name.  设置网络端口名称
  ArduinoOTA.setPassword("666666"); //Set the network port connection password.  设置网络端口连接的密码
  ArduinoOTA.begin(); //Initialize the OTA.  初始化OTA
  Serial.println("OTA ready!");  //Serial port output format string.  串口输出格式化字符串
}

void loop() {
  ArduinoOTA.handle();  //Continuously check for update requests.  持续检测是否有更新请求
  if(!digitalRead(39)){  //if Btn is Pressed.  如果按键按下
    ArduinoOTA.end(); //Ends the ArduinoOTA service.  结束OTA服务
    Serial.println("OTA End!");
    delay(200);
  }
}