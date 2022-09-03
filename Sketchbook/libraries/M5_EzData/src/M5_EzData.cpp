#include <M5_EzData.h>

WiFiClient wifi_client;
HTTPClient http_client;

const char* host = "https://ezdata.m5stack.com/api/store";

//Wifi连接
int setupWifi(const char* ssid,const char* password){
  delay(10);
  Serial.printf("\nConnecting to %s",ssid);
  WiFi.begin(ssid, password);
  while (!WiFi.isConnected()){
    Serial.print(".");
    delay(500);
  }
  return 1;
  Serial.println("\nWifi connected ok!");
}

int setUpHttpClient(String req){
  if (http_client.begin(req)){
    Serial.println("HTTP client setup done!");
    return 1;
  }else{
    Serial.println("HTTP client setup error!");
    return 0;
  }
}

//Save data to the top of the specified topic queue.  保存数据至指定topic队列首位
int setData(const char *token,const char *topic,int val){
  String ouput, req = (String)host + '/' + token + '/' + topic;
  setUpHttpClient(req);
  StaticJsonDocument<500> payload;
  payload["value"] = val;
  serializeJsonPretty(payload, ouput);  //构造序列化json，格式化输出
  // Specify content-type header
  http_client.addHeader("Content-Type", "application/json");
  // Send HTTP POST request
  int httpResponseCode = http_client.POST((uint8_t*)ouput.c_str(),ouput.length());
  if(httpResponseCode == HTTP_CODE_OK){
    Serial.printf("Success sending data to %s",topic);
    Serial.println(ouput);
    return 1;
  }else {
    Serial.printf("Fail to save data,response code:%d\n",httpResponseCode);
    return 0;
  }
  // Free resources
  http_client.end();
  delay(20);
}

//Save data to the top of the specified data list.  保存数据至指定数据列表首位
int addToList(const char *token,const char *list,int val){
  String ouput, req = (String)host + '/' + token + "/list" + '/' + list;
  setUpHttpClient(req);
  StaticJsonDocument<500> payload;
  payload["payload"] = val;
  serializeJsonPretty(payload, ouput);  //构造序列化json，格式化输出
  // Specify content-type header
  http_client.addHeader("Content-Type", "application/json");
  // Send HTTP POST request
  int httpResponseCode = http_client.POST((uint8_t*)ouput.c_str(),ouput.length());
  if(httpResponseCode == HTTP_CODE_OK){
    Serial.printf("Sending data to %s",list);
    Serial.println(ouput);
    return 1;
  }else{
    Serial.printf("Fail to save data,response code:%d\n",httpResponseCode);
    return 0;
  }
  // Free resources
  http_client.end();
  delay(20);
}

//Delete topic or list, and clear the queue data.  删除指定topic或list，并清空数据
int removeData(const char *token, const char *field){
  String req = (String)host + '/' + token + "/delete" + '/' + field;
  setUpHttpClient(req);
  http_client.addHeader("Content-Type", "application/json");
  int httpResponseCode = http_client.POST("");
  if(httpResponseCode == HTTP_CODE_OK){
    Serial.printf("Successfully remove the data in '%s'\n",field);
    return 1;
  }else{
    Serial.printf("Fail to remove data,response code:%d\n",httpResponseCode);
    return 0;
  }
  // Free resources
  http_client.end();
}

/*
Get a set of data from the specified data list and store it in the specified array
从指定的数据列表中获取一组数据并存储在指定数组里
list:  list name.  列表名称
offset: the offset relative to the first position of the data list.  相对于数据列表首位的偏移
count: the number of read data.  读取数据个数 
*/
int* getData(const char *token, const char *list, int *Array, int offset, int count){
  String req = (String)host + '/' + token + '/' + list + "?offset=" + (String)offset + "&count=" + (String)count;
  setUpHttpClient(req);
  int httpResponseCode = http_client.GET();
  if (httpResponseCode == HTTP_CODE_OK){
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http_client.getStream());
    for(int i = offset; i < count; i++){
      Array[i-offset] = doc["data"][i].as<int>();
    }
    Serial.print("Success to get data\n");
    return Array; //返回数组的地址
  }else{
    Serial.printf("Fail to get data,response code:%d\n", httpResponseCode);
    return 0;
  }
  http_client.end();
}

//Get data from the top of the specified topic queue.  从指定的topic队列首位获取一个数据
int getData(const char *token, const char *topic, int& result){
  String req = (String)host + '/' + token + '/' + topic + "?offset=0&count=1";
  setUpHttpClient(req);
  int httpResponseCode = http_client.GET();
  if (httpResponseCode == HTTP_CODE_OK){
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http_client.getString());
    Serial.print("Success to get data\n");
    result = doc["data"].as<int>();
    return 1;
  }else{
    Serial.printf("Fail to get data,response code:%d\n", httpResponseCode);
    return 0;
  }
  http_client.end();
}