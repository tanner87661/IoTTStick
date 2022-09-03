# EzData

## Function Description

EzData is an IoT cloud data storage service provided by M5Stack. Different devices can insert or extract data into the storage queue through the unique token to realize data sharing.

<img src="https://static-cdn.m5stack.com/resource/docs/static/image/iotservice/ezdata/ezdata_01.webp" width="100%">

Precautions：<br>1. All the following operations depend on the `unique token`, which is fixed in the same browser environment. Please copy the token before use.
<br>2. If no data operation is performed within half a year, the data queue corresponding to the token will be cleared.
<br>3. The data will be sorted in descending order according to the inserted time (the last inserted data is at the top of the list), and the data will be accumulated and saved.

`setData(const char *token, const char *topic, int val)`
- Save data to the top of the specified topic queue

`getData(const char *token, const char *topic, int& result)`

- Get data from the top of the specified topic queue.

`addToList(const char *token, const char *list, int val)`

- Save data to the top of the specified data list

`getData(const char *token, const char *list, int *Array, int offset, int count)`

-Obtain a set of data from a specified data list. The advantage of using list storage is that it supports the specified data index offset and can obtain multiple data at a time, and the return value is a list.

-list: list name

-offset: the offset relative to the first position of the data list

-count: the number of read data

`removeData(const char *token, const char *field)`

- Delete topic or list, and clear the queue data.

