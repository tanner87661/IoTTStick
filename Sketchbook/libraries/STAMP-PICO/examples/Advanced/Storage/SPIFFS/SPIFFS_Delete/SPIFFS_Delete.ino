/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with STAMP-PICO sample source code
*                          配套  STAMP-PICO 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/stamp_pico
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/stamp_pico
*
* describe：SPIFFS Delete
* date：2021/9/25
*******************************************************************************
*/
#include <Arduino.h>
#include <SPIFFS.h>

String file_name = "/M5Stack/notes.txt"; //Sets the location and name of the file to be operated on.  设置被操作的文件位置和名称
void setup() {
  Serial.begin(115200);
  if(SPIFFS.begin()){ // Start SPIFFS, return 1 on success.  启动闪存文件系统,若成功返回1
    Serial.println("\nSPIFFS Started.");  //Screen prints format String.  屏幕打印格式化字符串
  }else{
    Serial.println("SPIFFS Failed to Start.");
  }

  if(SPIFFS.remove(file_name)){  //Delete file_name file from flash, return 1 on success.  从闪存中删除file_name文件,如果成功返回1
    Serial.print(file_name);
    Serial.println(" Remove sucess");
  }else{
    Serial.print(file_name);
    Serial.println(" Remove fail");
  }
}

void loop() {
}