/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/STAMP-PICO
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/STAMP-PICO
*
* describe：WIFI TCP.
* date：2021/10/1
*******************************************************************************
  STAMP-PICO will sends a message to a TCP server
  STAMP-PICO 将向TCP服务器发送一条数据
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

// Set these to your desired credentials.  设置你的热点名称和密码
const char* ssid     = "Explore-F";
const char* password = "xingchentansuo123";
WiFiMulti WiFiMulti;

void setup()
{
  int sum=0;
  Serial.begin(115200);
  WiFiMulti.addAP(ssid, password);  //Add wifi configuration information.  添加wifi配置信息
  Serial.printf("Waiting connect to WiFi: %s ...",ssid); //Serial port output format string.  串口输出格式化字符串
  while(WiFiMulti.run() != WL_CONNECTED) {  //If the connection to wifi is not established successfully.  如果没有与wifi成功建立连接
    Serial.print(".");
    delay(1000);
    sum+=1;
    if(sum==8) Serial.print("Conncet failed!");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //The serial port outputs the IP address of the STAMP-PICO.  串口输出STAMP-PICO的IP地址
  delay(500);
}

void loop()
{
  const char * host = "www.baidu.com"; //Set the IP address or DNS of the TCP server.  设置TCP服务器的ip或dns
  const uint16_t port = 80; //The port of the TCP server is specified.  设置TCP服务器的端口
  Serial.printf("Connecting to: %s\n",host);

  WiFiClient client;
  if (!client.connect(host, port)) {  //Connect to the server. 0 is returned if the connection fails.  连接服务器,若连接失败返回0
    Serial.print("Connection failed.\nWaiting 5 seconds before retrying...\n");
    delay(5000);
    return;
  }
  //send an arbitrary string to the server.  发送一个字符串到上边连接的服务器
  client.print("Send this data to the server");
  //send a basic document request to the server.  向服务器发送一个基本的文档请求.
  client.print("GET /index.html HTTP/1.1\n\n");
  int maxloops = 0;

  //wait for the server's reply to become available
  //等待服务器的回复
  while (!client.available() && maxloops < 1000){
    maxloops++;
    delay(1); //delay 1 msec
  }
  if (client.available() > 0){  //Detects whether data is received.  检测是否接收到数据
    String line = client.readStringUntil('\r'); //Read information from data received by the device until \r is read.  从设备接收到的数据中读取信息,直至读取到\r时
    Serial.println(line); //String received by serial port output.  串口输出接收到的字符串
  }else{
    Serial.println("client.available() timed out ");
  }
    Serial.println("Closing connection.");
    client.stop();
    Serial.println("Waiting 5 seconds before restarting...");
    delay(5000);
}