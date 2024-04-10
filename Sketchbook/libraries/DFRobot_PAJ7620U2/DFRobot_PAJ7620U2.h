/*!
 * @file DFRobot_PAJ7620U2.h
 * @brief Define the basic structure of the class DFRobot_PAJ7620 gesture sensor 
 * @n The PAC7620 integrates gesture recognition function with general I2C interface into a single chip forming an image analytic sensor
 * @n system. It can recognize 9 human hand gesticulations such as moving up, down, left, right, forward, backward, circle-clockwise, 
 * @n circle-counter Key Parameters clockwise, and waving. It also offers built-in proximity detection in sensing approaching or
 * @n departing object from the sensor. The PAC7620 is designed with great flexibility in power-saving mechanism, well suit for low 
 * @n power battery operated HMI devices. The PAJ7620 is packaged into module form in-built with IR LED and optics lens as a complete
 * @n sensor solution.
 
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      [Alexander](ouki.wang@dfrobot.com)
 * @maintainer  [fary](feng.yang@dfrobot.com)
 * @version  V1.0
 * @date  2021-10-20
 * @url https://github.com/DFRobot/DFRobot_PAJ7620U2
 */

#ifndef __DFROBOT_PAJ7620U2_H
#define __DFROBOT_PAJ7620U2_H

#include "Arduino.h"
#include <Wire.h>

//Open this macro to check the detailed running process of the program.
//#define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

// DEVICE ID
#define PAJ7620_IIC_ADDR  0x73
#define PAJ7620_PARTID  0x7620

// REGISTER BANK SELECT
#define PAJ7620_REGITER_BANK_SEL  (0xEF)

// REGISTER BANK 0
#define PAJ7620_ADDR_PART_ID_LOW         0x00
#define PAJ7620_ADDR_PART_ID_HIGH        0x01
#define PAJ7620_ADDR_VERSION_ID          0x01
#define PAJ7620_ADDR_SUSPEND_CMD         0x03
#define PAJ7620_ADDR_GES_PS_DET_MASK_0   0x41
#define PAJ7620_ADDR_GES_PS_DET_MASK_1   0x42
#define PAJ7620_ADDR_GES_PS_DET_FLAG_0   0x43
#define PAJ7620_ADDR_GES_PS_DET_FLAG_1   0x44
#define PAJ7620_ADDR_STATE_INDICATOR     0x45
#define PAJ7620_ADDR_PS_HIGH_THRESHOLD   0x69
#define PAJ7620_ADDR_PS_LOW_THRESHOLD    0x6A
#define PAJ7620_ADDR_PS_APPROACH_STATE   0x6B
#define PAJ7620_ADDR_PS_RAW_DATA         0x6C

// REGISTER BANK 1
#define PAJ7620_ADDR_PS_GAIN             0x44
#define PAJ7620_ADDR_IDLE_S1_STEP_0      0x67
#define PAJ7620_ADDR_IDLE_S1_STEP_1      0x68
#define PAJ7620_ADDR_IDLE_S2_STEP_0      0x69
#define PAJ7620_ADDR_IDLE_S2_STEP_1      0x6A
#define PAJ7620_ADDR_OP_TO_S1_STEP_0     0x6B
#define PAJ7620_ADDR_OP_TO_S1_STEP_1     0x6C
#define PAJ7620_ADDR_OP_TO_S2_STEP_0     0x6D
#define PAJ7620_ADDR_OP_TO_S2_STEP_1     0x6E
#define PAJ7620_ADDR_OPERATION_ENABLE    0x72

#define PAJ7620_BANK0 0
#define PAJ7620_BANK1 1

// PAJ7620_ADDR_SUSPEND_CMD
#define PAJ7620_I2C_WAKEUP	0x01
#define PAJ7620_I2C_SUSPEND	0x00

// PAJ7620_ADDR_OPERATION_ENABLE
#define PAJ7620_ENABLE	0x01
#define PAJ7620_DISABLE	0x00

#define GES_REACTION_TIME  50    ///< You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME     2000  ///< When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s). 
#define GES_QUIT_TIME      1000 

class DFRobot_PAJ7620U2
{
public:
  #define ERR_OK             0      ///< OK
  #define ERR_DATA_BUS      -1      ///< Error in Data Bus 
  #define ERR_IC_VERSION    -2      ///< IC version mismatch
  /**
   * @enum  eGesture_t
   * @brief gesture information
   */
  typedef enum{
    eGestureNone = 0x00,                                                    /**< no gestures detected */
    eGestureRight = 0x01<<0,                                                /**< move from left to right */
    eGestureLeft  = 0x01<<1,                                                /**< move from right to left */
    eGestureUp    = 0x01<<2,                                                /**< move from down to up */
    eGestureDown  = 0x01<<3,                                                /**< move form up to down */
    eGestureForward   = 0x01<<4,                                            /**< starts far, move close to sensor */
    eGestureBackward  = 0x01<<5,                                            /**< starts near, move far to sensor */
    eGestureClockwise = 0x01<<6,                                            /**< clockwise */
    eGestureAntiClockwise = 0x01<<7,                                        /**< anti-clockwise */
    eGestureWave = 0x01<<8,                                                 /**< wave quickly */
    eGestureWaveSlowlyDisorder = 0x01<<9,                                   /**< wave randomly and slowly */
    eGestureWaveSlowlyLeftRight = eGestureLeft + eGestureRight,             /**< slowly move left and right */
    eGestureWaveSlowlyUpDown = eGestureUp + eGestureDown,                   /**< slowly move up and down */
    eGestureWaveSlowlyForwardBackward = eGestureForward + eGestureBackward, /**< slowly move forward and backward */
    eGestureAll = 0xff                                                      /**< support all gestures, no practical meaning, only suitable for writing abstract program logic. */
  }eGesture_t;
  /**
   * @enum  eBank_t
   * @brief Register Bank
   */
  typedef enum {
    eBank0 = 0, /**< some registers are located in Bank0*/ 
    eBank1 = 1, /**< some registers are located in Bank1*/
  }eBank_t;

  /**
   * @enum  eRateMode_t
   * @brief Gesture Update Rate
   */
  typedef enum {
    eNormalRate = 0, /**< Gesture Update Rate is 120HZ, Gesture speed is 60°/s - 600°/s*/
    eGamingRate = 1, /**< Gesture Update Rate is 240HZ,Gesture speed is 60°/s - 1200°/s*/
  }eRateMode_t;

  /**
   * @struct  sGestureDescription_t
   * @brief Gesture Description
   */
  typedef struct{
    eGesture_t gesture;       /**< Gesture enumeration variable X */
    const char * description; /**< Description about the gesture enumeration variable X */
  }sGestureDescription_t;

public:
  /**
   * @fn DFRobot_PAJ7620U2
   * @brief Constuctor
   * @param pWire TwoWire
   */
  DFRobot_PAJ7620U2(TwoWire *pWire=&Wire);

  /**
   * @fn begin
   * @brief init function
   * @return return 0 if initialization succeeds, otherwise return non-zero. 
   */
  int begin(void);

  /**
   * @fn setGestureHighRate
   * @brief Set gesture detection mode 
   * @param b true Set to fast detection mode, recognize gestures quickly and return. 
   * @n  false Set to slow detection mode, system will do more judgements. 
   * @n  In fast detection mode, the sensor can recognize 9 gestures: move left, right, up, down,
   * @n  forward, backward, clockwise, counter-clockwise, wave. 
   * @n  To detect the combination of these gestures, like wave left, right and left quickly, users need to design their own 
   * @n  algorithms logic.
   * @n  Since users only use limited gestures, we didn't integrate too much expanded gestures in the library. 
   * @n  If necessary, you can complete the algorithm logic in the ino file by yourself.
   * @n
   * @n
   * @n  In slow detection mode, the sensor recognize one gesture every 2 seconds, and we have integrated the expanded gestures 
   * @n  inside the library, which is convenient for the beginners to use.
   * @n  The slow mode can recognize 9  basic gestures and 4 expanded gestures: move left, right, up, down, forward, backward, 
   * @n  clockwise, counter-clockwise, wave, slowly move left and right, slowly move up and down, slowly move forward and backward, 
   * @n  wave slowly and randomly.
   */
  void setGestureHighRate(bool b);

  /**
   * @fn gestureDescription
   * @brief Get the string descritpion corresponding to the gesture number.
   * @param gesture Gesture number inlcuded in the eGesture_t
   * @return Textual description corresponding to the gesture number:if the gesture input in the gesture table doesn't exist, 
   * @n return null string.
   */
  String gestureDescription(eGesture_t gesture);

  /**
   * @fn getGesture
   * @brief Get gesture
   * @return Return gesture, could be any value except eGestureAll in eGesture_t.
   */
  eGesture_t getGesture(void);

private:
  /**
   * @fn selectBank
   * @brief Switch Bank
   * @param bank  The bank you will switch to, eBank0 or eBank1
   * @return Return 0 if switching successfully, otherwise return non-zero. 
   */
  int selectBank(eBank_t bank);

  /**
   * @fn setNormalOrGamingMode
   * @brief Set rate mode of the module, the API is disabled currently.
   * @param mode The mode users can configure, eNormalRate or eGamingRate
   * @return Return 0 if setting is successful, otherwise return non-zero. 
   */
  int setNormalOrGamingMode(eRateMode_t mode);
  
  /**
   * @fn writeReg
   * @brief Write register function 
   * @param reg  register address 8bits
   * @param pBuf Storage cache of the data to be written into 
   * @param size Length of the data to be written into 
   */
  void writeReg(uint8_t reg, const void* pBuf, size_t size);

  /**
   * @fn readReg
   * @brief Read register function 
   * @param reg  register address 8bits
   * @param pBuf Storage cache of the data to be read
   * @param size Length of the data to be read
   * @return Return the actually read length, fails to read if return 0.  
   */
  uint8_t readReg(uint8_t reg, void* pBuf, size_t size);

private:
  /*! _pWire is TwoWire type pointer from apllication program */ 
  TwoWire *_pWire;
  /*! init configuration table */
  static const uint8_t /*PROGMEM*/ initRegisterArray[219][2]; 
  /*! Table of gesture number corresponding to string description */
  static const sGestureDescription_t /*PROGMEM*/ gestureDescriptionsTable[14]; 
  /*! IIC address of PAJ7620U2, cannot be revised.*/
  const uint8_t _deviceAddr = PAJ7620_IIC_ADDR;
  /*! Whether it is set to fast detection mode, refer to setGestureHighRate for details*/
  bool _gestureHighRate = true;
  /*! Current gesture*/  
  eGesture_t _gesture;
};

#endif
