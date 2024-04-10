# DFRobot_GP8XXX
* [English Version](./README.md)

GP8XXX 系列在树莓派上的驱动库（目前已经兼容GP8101，GP8101S，GP8211S，GP8413，GP8501，GP8503，GP8512，GP9403，GP8302）

## 产品链接([www.dfrobot.com](www.dfrobot.com))

SUK：

1. DFR1034 --- GP8503
2. DFR1035 --- GP8512
3. DFR1036 --- GP8101S
4. DFR1037 --- GP8501
5. DFR1071 --- GP8211S
6. DFR1073 --- GP8413
7. DFR0971 --- GP8403
8. DFR0972 --- GP8302

## 目录

* [概述](#概述)
* [库安装](#库安装)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [创作者](#创作者)

## 概述

  此库已经争对目前兼容的6款产品分别提供了示例demo,用户可以根据名称进行对应使用

## 库安装

1. 下载库至树莓派，要使用这个库，首先要将库下载到Raspberry Pi，命令下载方法如下:<br>
```python
sudo git clone https://github.com/DFRobot/DFRobot_GP8403
```
2. 打开并运行例程，要执行一个例程demo_x.py，请在命令行中输入python demo_x.py。

## 方法

```python

#################I2C系列#################
  def begin(self)
    '''!
      @brief 初始化函数
      @return 返回0表示成功，其他值表示失败 
    '''

  def set_dac_outrange(self, range)
    '''!
      @brief 设置DAC输出范围
      @param range DAC输出范围
      @n     eOutputRange0_5V(0-5V)
      @n     eOutputRange0_10V(0-10V)
      @return NONE
    '''

  def set_dac_out_voltage(self, data, channel=0)
    '''!
      @brief 设置不同通道输出DAC值
      @param data 电压值对应的数据值
      @param channel 输出通道
      @n  0:通道0  (配置PWM0输出时有效)
      @n  1:通道1  (配置PWM1输出时有效)
      @n  2:全部通道 (配置双通道输出时有效)
      @return NONE
    '''

  def store(self)
    '''!
      @brief 将设置的电压保存在芯片内部
      @return NONE
    '''


#################PWM系列#################

  def begin(self)
    '''!
      @brief 初始化函数
      @return 0
    '''

  def set_dac_out_voltage(self, data, channel=0)
    '''!
      @brief 设置不同通道输出DAC值
      @param data pwm 脉宽
      @param channel 输出通道
      @n  0:通道0  (配置PWM0输出时有效)
      @n  1:通道1  (配置PWM1输出时有效)
      @n  2:全部通道 (配置双通道输出时有效)
      @return NONE
    '''

```

## 兼容性

| 主板         | 通过 | 未通过 | 未测试 | 备注 |
| ------------ | :--: | :----: | :----: | :--: |
| RaspberryPi2 |      |        |   √    |      |
| RaspberryPi3 |      |        |   √    |      |
| RaspberryPi4 |  √   |        |        |      |

* Python 版本

| Python  | 通过 | 未通过 | 未测试 | 备注 |
| ------- | :--: | :----: | :----: | ---- |
| Python2 |  √   |        |        |      |
| Python3 |  √   |        |        |      |


## 历史

- 2023/5/10 - 1.0.0 版本

## 创作者

- Written by fary( feng.yang@dfrobot.com), 2023. (Welcome to our [website](https://www.dfrobot.com/))

