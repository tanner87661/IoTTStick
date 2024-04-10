# DFRobot_PAJ7620
- [English Version](./README.md)

DFRobot_PAJ7620手势识别传感器是一款强大的3D手势识别交互式传感器；在最远20cm范围内，最多可以识别13种手势。具备良好的手势识别稳定性和节能机制，总是能够在恰当的时候偷偷帮你节省电源；采用Gravity接口，零件的连接就不再是一件麻烦的事。目前有两种工作模式：高速模式下可以识别手的上\下\左\右\前\后\顺时针\逆时针移动以及快速挥动9种手势；低速模式下可以此前9种基础上再加乱序\缓慢左右\缓慢前后\缓慢上下移动4种手势。显然这款传感器可以有广泛的运用范围，设想一下，挥挥手控制电视空调；挥挥手调整灯光和音乐；挥挥手去操纵游戏...会不会很有意思呢？

<br>
<img src="../../resources/images/SEN0315.png">
<br>


## 产品链接(https://www.dfrobot.com.cn/goods-2677.html)

    SKU：SEN0315

## 目录

* [概述](#概述)
* [库安装](#库安装)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [创作者](#创作者)
 
## 概述

在这个Arduino库中，我们将向您展示PAJ7620手势传感器的基本用法。你可以使用内置的例子来体验下面的功能:
1. 快速模式下识别9个手势功能。
2. 慢速模式下识别9个基本手势和4个扩展手势的功能。
3. 一个快速模式下的手势序列识别的例子，我们称之为手势密码。

## 库安装

这里提供两种使用本库的方法：
1. 打开Arduino IDE,在状态栏中的Tools--->Manager Libraries 搜索"DFRobot_VEML6075"并安装本库.
2. 首先下载库文件,将其粘贴到\Arduino\libraries目录中,然后打开examples文件夹并在该文件夹中运行演示.


## 方法

```python
  def begin(self):
    '''!
      @brief 初始化函数
      @return 返回0代表初始化成功，否则失败
    '''

  def set_gesture_highrate(self,v):
    '''!
      @brief 设置手势检测模式
      @param b true 设置为快速检测模式，快速识别手势并返回。
      @n  false 设置为慢速检测模式，系统将做更多的判断。
      @n  在快速检测模式下，传感器可识别左、右、上、下、前、后、顺、逆时针、波浪9种手势。
      @n  为了检测这些手势的组合，比如快速的左右摆动，用户需要设计自己的算法逻辑。
      @n  由于用户只使用有限的手势，我们没有在库中集成太多扩展的手势。如果需要，可以自己完成ino文件中的算法逻辑。
      @n
      @n
      @n  在慢检测模式下，传感器每2秒识别一个手势，我们将扩展的手势集成到库中，方便初学者使用。
      @n  慢速模式可识别9种基本手势和4种扩展手势:左、右、上、下、前进、后退、顺时针、逆时针、挥手、缓慢左右、缓慢上下、缓慢前后、缓慢随意挥手。
    '''

  def gesture_description(self,gesture):
    '''!
      @brief 获取与手势数对应的字符串描述。
      @param gesture eGesture_t中包含的手势数
      @return 返回与手势对应的文本描述:如果手势表中的手势输入不存在，则返回空字符串。
    '''
  
  def get_gesture(self):
    '''!
      @brief 识别手势
      @return 返回手势，可以是除eGesture_t中的eGestureAll之外的任何值。
    '''
```

## 兼容性

* RaspberryPi 版本

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |           |            |    √     |         |
| RaspberryPi3 |     √     |            |          |         |
| RaspberryPi4 |           |            |    √     |         |

* Python 版本

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |     √     |            |          |         |
| Python3 |     √     |            |          |         |


## 历史

- 2021/10/20 - 1.0.1 版本
- 2019/07/16 - 1.0.0 版本

## 创作者

Written by yangfeng(feng.yang@dfrobot.com), 2021. (Welcome to our [website](https://www.dfrobot.com/))
