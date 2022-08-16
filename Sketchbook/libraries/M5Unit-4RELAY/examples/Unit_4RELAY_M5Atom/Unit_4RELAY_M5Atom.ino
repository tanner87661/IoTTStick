/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with Atom-Lite/Matrix sample source code
*                          配套  Atom-Lite/Matrix 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/4relay
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/4relay
*
* Product: Unit 4RELAY.
* Date: 2022/7/22
*******************************************************************************
  Please connect to port ,Control 4 relays and demonstrate the asynchronous
  control relay LED 请连接端口，控制4继电器，并演示异步控制继电器LED
-------------------------------------------------------------------------------
  RELAY control reg           | 0x10
  -----------------------------------------------------------------------------
  Relay_ctrl_mode_reg[0]      | R/W | System control
                              | 7 | 6 | 5 | 4 | 3 | 2 | 1 |     0     |
                              | R | R | R | R | R | R | R | Sync Mode |
                              | -Sync Mode:0 LED&Relay Async
                              | -Sync Mode:1 LED&Relay Sync
  ---------------------------------------------------------------------------------
  Relay_ctrl_mode_reg[1]      | R/W | Relay & LED control
                              |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
                              | LED1| LED2| LED3| LED4| RLY1| RLY2| RLY3| RLY4|
-------------------------------------------------------------------------------*/
// Note: The relay can only be controlled in synchronous mode, if the relay is
// controlled in asynchronous mode, it will be invalid.
// 注意:只能在同步模式下控制继电器,如果在异步模式下对继电器进行控制将无效,.

#include <M5Atom.h>
#include "Unit_4RELAY.h"

UNIT_4RELAY relay;

void setup() {
    M5.begin();  // Init M5Atom.  初始化 M5Atom
    Serial.print(relay.begin(&Wire, 26, 32));
    relay.Init(1);  // Set the lamp and relay to synchronous mode(Async =
                    // 0,Sync = 1).  将灯和继电器设为同步模式
    Serial.printf("Please pressed Btn. \n");
    relay.relayAll(0);
}

char count_i = 0;

void loop() {
    relay.relayAll(1);
    delay(1000);
    relay.relayAll(0);
    delay(1000);

    if (M5.Btn.wasPressed()) {  // If button A is pressed.  如果按键A按下
        if ((count_i < 4)) {    // Control relays turn on/off in sequence.
                                // 控制继电器依次接通/断开
            Serial.printf("%d ON\n", count_i);
            relay.relayWrite(count_i, 1);
        } else if ((count_i >= 4)) {
            Serial.printf("%d OFF\n", (count_i - 4));
            relay.relayWrite((count_i - 4), 0);
        }
        count_i++;
        if (count_i >= 8) count_i = 0;
    }
    M5.update();
}