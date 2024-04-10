/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/STAMP-PICO
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/STAMP-PICO
*
* describe：WIFI Smart Config.  WIFI智能配网
* date：2021/10/1
*******************************************************************************
 *  Fill in WIFI configuration information through mobile APP to connect STAMP-PICO to relevant WIFI
 *  通过手机APP填写WIFI配置信息使 STAMP-PICO连接至相关WIFI
 *  APP Download Address: https://www.espressif.com/en/products/software/esp-touch/resources
 *  APP下载地址: https://www.espressif.com/zh-hans/products/software/esp-touch/resources
 */
#include <Arduino.h>
#include "WiFi.h"

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA); // Set the wifi mode to the mode compatible with the AP and Station, and start intelligent network configuration
  WiFi.beginSmartConfig();  // 设置wifi模式为AP 与 Station 兼容模式,并开始智能配网

  //Wait for the STAMP-PICO to receive network information from the phone
  //等待STAMP-PICO接收到来自手机的配网信息
  Serial.print("\nWaiting for Phone SmartConfig."); //Screen print format string.  屏幕打印格式化字符串
  while (!WiFi.smartConfigDone()) { //If the smart network is not completed.  若智能配网没有完成
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nSmartConfig received.");

  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) { //STAMP-PICO will connect automatically upon receipt of the configuration information, and return true if the connection is successful.  收到配网信息后STAMP-PICO将自动连接，若连接成功将返回true
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi Connect To: ");
  Serial.println(WiFi.SSID());  //Output Network name.  输出网络名称
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //Output IP Address.  输出IP地址
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());  //Output signal strength.  输出信号强度
}

void loop() {
}