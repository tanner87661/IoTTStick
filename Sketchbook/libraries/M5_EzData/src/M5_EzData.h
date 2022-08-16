/***************************************************************************
Title: M5_EzData
date: 2021/10/15
***************************************************************************/

#ifndef M5_EzData_h
#define M5_EzData_h

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

//Wifi连接
int setupWifi(const char* ssid,const char* password);

int setUpHttpClient(String req);

//Save data to the top of the specified topic queue.  保存数据至指定topic队列首位
int setData(const char *token,const char *topic,int val);

//Get data from the top of the specified topic queue.  从指定的topic队列首位获取一个数据
int getData(const char *token,const char *topic, int& result);

//Save data to the top of the specified data list.  保存数据至指定list首位
int addToList(const char *token,const char *list,int val);

/*
Get a set of data from the specified data list and store it in the specified array
从指定的数据列表中获取一组数据并存储在指定数组里
list:  list name.  列表名称
offset: the offset relative to the first position of the data list.  相对于数据列表首位的偏移
count: the number of read data.  读取数据个数
*/
int* getData(const char *token,const char *list,int *Array,int offset,int count);

//Delete topic or list, and clear the queue data.  删除指定topic或list，并清空数据
int removeData(const char *token,const char *field);

#endif
