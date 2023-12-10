# M5 PoE_CAM Ethernet  Arduino 依赖库

中文 | [English](README_cn.md)


<img src="https://static-cdn.m5stack.com/resource/docs/static/assets/img/product_pics/unit/poe_cam/poe_cam_01.webp" alt="M5Core2_P1" width="350" height="350"><img src="https://static-cdn.m5stack.com/resource/docs/products/unit/M5PoECAM-W/img-0e65978b-b66f-41ea-a94f-e0be885ce076.webp" alt="M5Core2_P1" width="350" height="350">

* **如果查看 PoECAM 的详细介绍文档，[点击这里](https://docs.m5stack.com/zh_CN/unit/poe_cam)**

## 描述

**PoECAM**是一款集成`PoE`(Power Over Ethernet)功能的一款开源`可编程网络摄像头`。硬件使用`ESP32`控制核心+`W5500嵌入式以太网控制器`+200w像素图像传感器`OV2640`方案。 搭配`8MB PSRAM` + `16MB Flash` 大内存组合。整机体积紧凑, 供电方式灵活, 仅需两步, `连接网线, 即可实现稳定的图像传输`。非常适用于仓储监控, 定时图像采集等应用。

## 注意事项:

1. PoECAM下载程序需外接ESP32烧录器，你可以[点击此处购买M5官方的ESP32-Downloader套件，内含转接小板，连接会更加的方便](https://shop.m5stack.com/products/esp32-downloader-kit)
2. PoECAM的出厂固件，在连接交换机后将会自动获取IP，并启动web服务器。通过查看PoECAM的串口输出，可以获取到IP地址与图像流URL，同一局域网下用浏览器的访问该URL，既可以实时预览图像。

## 更多信息

**Arduino IDE 环境搭建**: [点击这里](https://docs.m5stack.com/zh_CN/quick_start/poe_cam/arduino)
