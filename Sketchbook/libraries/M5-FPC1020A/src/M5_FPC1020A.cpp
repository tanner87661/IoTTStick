#include "M5_FPC1020A.h"

FingerPrint::FingerPrint() {
}

/*! @brief Initialize the FPC1020A.*/
void FingerPrint::begin(HardwareSerial *_Serial, uint8_t rx, uint8_t tx) {
    _serial = _Serial;
    _serial->begin(19200, SERIAL_8N1, rx, tx);
}

/*! @brief Send a message and wait for the specified time until the return value
 is received.*/
uint8_t FingerPrint::fpm_sendAndReceive(uint16_t timeout) {
    uint8_t i, j;
    uint8_t checkSum = 0;

    memset(RxBuf, 0, 9);

    TxBuf[5] = 0;

    _serial->write(CMD_HEAD);
    for (i = 1; i < 6; i++) {
        _serial->write(TxBuf[i]);
        checkSum ^= TxBuf[i];
    }
    _serial->write(checkSum);
    _serial->write(CMD_TAIL);

    uint8_t ch;
    unsigned long start = millis();
    while (_serial->available() || (millis() - start) < timeout) {
        for (uint8_t i = 0; i < 8; i++) {
            ch       = _serial->read();
            RxBuf[i] = ch;
            if (RxBuf[0] != 0xf5) break;
            delay(1);
        }
        if (RxBuf[0] == 0xf5 && RxBuf[7] == 0xf5) {
            break;
        }
    }
    if (RxBuf[HEAD] != CMD_HEAD) return ACK_FAIL;
    if (RxBuf[TAIL] != CMD_TAIL) return ACK_FAIL;
    if (RxBuf[CMD] != (TxBuf[CMD])) return ACK_FAIL;

    checkSum = 0;
    for (j = 1; j < CHK; j++) {
        checkSum ^= RxBuf[j];
    }
    if (checkSum != RxBuf[CHK]) {
        return ACK_FAIL;
    }
    return ACK_SUCCESS;
}

/*! @brief Putting the chip to sleep.*/
uint8_t FingerPrint::fpm_sleep(void) {
    uint8_t res;

    TxBuf[CMD] = CMD_SLEEP_MODE;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = 0;
    TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(1500);

    if (res == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

/*! @brief Set mode to add mode.*/
uint8_t FingerPrint::fpm_setAddMode(uint8_t fpm_mode) {
    uint8_t res;

    TxBuf[CMD] = CMD_ADD_MODE;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = fpm_mode;
    TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(1200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_readAddMode(void) {
    TxBuf[CMD] = CMD_ADD_MODE;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = 0;
    TxBuf[P3]  = 0X01;

    fpm_sendAndReceive(1200);

    return RxBuf[Q2];
}

/*! @brief Get the user's number.*/
uint16_t FingerPrint::fpm_getUserNum(void) {
    uint8_t res;

    TxBuf[CMD] = CMD_USER_CNT;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = 0;
    TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(1200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return RxBuf[Q2];
    } else {
        return 0XFF;
    }
}

/*! @brief Initialize the EXTIO2.*/
uint8_t FingerPrint::fpm_deleteAllUser(void) {
    uint8_t res;

    TxBuf[CMD] = CMD_DEL_ALL;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = 0;
    TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(1200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

/*! @brief Delete all user information.*/
uint8_t FingerPrint::fpm_deleteUser(uint8_t userNum) {
    uint8_t res;

    TxBuf[CMD] = CMD_DEL;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = userNum;
    TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(1200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

/*! @brief Add User.*/
uint8_t FingerPrint::fpm_addUser(uint8_t userNum, uint8_t userPermission) {
    uint8_t res;

    TxBuf[CMD] = CMD_ADD_1;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = userNum;
    TxBuf[P3]  = userPermission;

    res = fpm_sendAndReceive(5000);

    if (res == ACK_SUCCESS) {
        if (RxBuf[Q3] == ACK_SUCCESS) {
            TxBuf[CMD] = CMD_ADD_2;

            res = fpm_sendAndReceive(5000);

            if (res == ACK_SUCCESS) {
                if (RxBuf[Q3] == ACK_SUCCESS) {
                    TxBuf[CMD] = CMD_ADD_3;

                    res = fpm_sendAndReceive(5000);

                    if (res == ACK_SUCCESS) {
                        return RxBuf[Q3];
                    }
                }
            }
        }
    }
    return res;
}

/*! @brief Compare fingerprint information.*/
uint8_t FingerPrint::fpm_compareFinger(void) {
    uint8_t res;

    TxBuf[CMD] = CMD_MATCH;
    TxBuf[P1]  = 0;
    TxBuf[P2]  = 0;
    TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(8000);

    if (res == ACK_SUCCESS) {
        if (RxBuf[Q3] == ACK_NOUSER) {
            return ACK_NOUSER;
        }
        if (RxBuf[Q3] == ACK_TIMEOUT) {
            return ACK_TIMEOUT;
        }
        if ((RxBuf[Q2] != 0) &&
            (RxBuf[Q3] == 1 || RxBuf[Q3] == 2 || RxBuf[Q3] == 3)) {
            return ACK_SUCCESS;
        }
    }
    return res;
}

/*! @brief Get the user's id.*/
uint8_t FingerPrint::fpm_getUserId(void) {
    return RxBuf[Q1] << 8 | RxBuf[Q2];
}
