#include "Unit_UHF_RFID.h"
#include "CMD.h"

String hex2str(uint8_t num) {
    if (num > 0xf) {
        return String(num, HEX);
    } else {
        return ("0" + String(num, HEX));
    }
}

/*! @brief Initialize the Unit UHF_RFID.*/
void Unit_UHF_RFID::begin(HardwareSerial *serial, int baud, uint8_t RX,
                          uint8_t TX, bool debug) {
    _debug  = debug;
    _serial = serial;
    _serial->begin(baud, SERIAL_8N1, RX, TX);
}

/*! @brief Clear the buffer.*/
void Unit_UHF_RFID::cleanBuffer() {
    for (int i = 0; i < 200; i++) {
        buffer[i] = 0;
    }
}

/*! @brief Clear the card's data buffer.*/
void Unit_UHF_RFID::cleanCardsBuffer() {
    for (int i = 0; i < 200; i++) {
        cards[i] = {
            rssi : 0,
            pc : {0},
            epc : {0},
            rssi_str : "",
            pc_str : "",
            epc_str : ""
        };
    }
}

/*! @brief Waiting for a period of time to receive a message
    @return True if the message is available at the specified time and in the
   specified range, otherwise false..*/
bool Unit_UHF_RFID::waitMsg(unsigned long time) {
    unsigned long start = millis();
    uint8_t i           = 0;
    cleanBuffer();
    while (_serial->available() || (millis() - start) < time) {
        if (_serial->available()) {
            uint8_t b = _serial->read();
            buffer[i] = b;
            i++;
            if (b == 0x7e) {
                break;
            }
        }
    }
    if (buffer[0] == 0xbb && buffer[i - 1] == 0x7e) {
        return true;
    } else {
        return false;
    }
}

/*! @brief Send command.*/
void Unit_UHF_RFID::sendCMD(uint8_t *data, size_t size) {
    _serial->write(data, size);
}

/*! @brief Filter the received message.*/
bool Unit_UHF_RFID::filterCardInfo(String epc) {
    for (int i = 0; i < 200; i++) {
        if (epc == cards[i].epc_str) {
            return false;
        }
    }
    return true;
}

/*! @brief Svae the card information.*/
bool Unit_UHF_RFID::saveCardInfo(CARD *card) {
    String rssi = hex2str(buffer[5]);
    String pc   = hex2str(buffer[6]) + hex2str(buffer[7]);
    String epc  = "";

    for (uint8_t i = 8; i < 20; i++) {
        epc += hex2str(buffer[i]);
    }

    if (!filterCardInfo(epc)) {
        return false;
    }

    for (uint8_t i = 8; i < 20; i++) {
        card->epc[i - 8] = buffer[i];
    }

    card->rssi  = buffer[5];
    card->pc[0] = buffer[6];
    card->pc[1] = buffer[7];

    card->rssi_str = rssi;
    card->pc_str   = pc;
    card->epc_str  = epc;

    if (_debug) {
        Serial.println("pc: " + pc);
        Serial.println("rssi: " + rssi);
        Serial.println("epc: " + epc);
        for (uint8_t i = 0; i < 24; i++) {
            Serial.print(hex2str(buffer[i]));
        }
        Serial.println(" ");
    }
    return true;
}

uint8_t Unit_UHF_RFID::pollingOnce() {
    cleanCardsBuffer();
    sendCMD((uint8_t *)POLLING_ONCE_CMD, sizeof(POLLING_ONCE_CMD));
    uint8_t count = 0;
    while (waitMsg()) {
        if (buffer[23] == 0x7e) {
            if (count < 200) {
                if (saveCardInfo(&cards[count])) {
                    count++;
                }
            } else {
                return 200;
            }
        }
    }
    return count;
}

uint8_t Unit_UHF_RFID::pollingMultiple(uint16_t polling_count) {
    cleanCardsBuffer();
    memcpy(buffer, POLLING_MULTIPLE_CMD, sizeof(POLLING_MULTIPLE_CMD));
    buffer[6] = (polling_count >> 88) & 0xff;
    buffer[7] = (polling_count)&0xff;

    uint8_t check = 0;
    for (uint8_t i = 1; i < 8; i++) {
        check += buffer[i];
    }

    buffer[8] = check & 0xff;

    if (_debug) {
        Serial.println("send cmd:");
        for (uint8_t i = 0; i < sizeof(POLLING_MULTIPLE_CMD); i++) {
            Serial.print(hex2str(buffer[i]));
        }
        Serial.println(" ");
    }

    sendCMD(buffer, sizeof(POLLING_MULTIPLE_CMD));

    uint8_t count = 0;
    while (waitMsg()) {
        if (buffer[23] == 0x7e) {
            if (count < 200) {
                if (saveCardInfo(&cards[count])) {
                    count++;
                }
            } else {
                break;
            }
        }
    }
    return count;
}

/*! @brief Get hardware version information.*/
String Unit_UHF_RFID::getVersion() {
    sendCMD((uint8_t *)HARDWARE_VERSION_CMD, sizeof(HARDWARE_VERSION_CMD));
    if (waitMsg()) {
        String info;
        for (uint8_t i = 0; i < 50; i++) {
            info += (char)buffer[6 + i];
            if (buffer[8 + i] == 0x7e) {
                break;
            }
        }
        return info;
    } else {
        return "ERROR";
    }
}

String Unit_UHF_RFID::selectInfo() {
    sendCMD((uint8_t *)GET_SELECT_PARAMETER_CMD,
            sizeof(GET_SELECT_PARAMETER_CMD));
    if (waitMsg()) {
        String Info = "";
        for (uint8_t i = 12; i < 24; i++) {
            Info += hex2str(buffer[i]);
        }
        if (_debug) {
            for (uint8_t i = 0; i < 26; i++) {
                Serial.print(hex2str(buffer[i]));
            }
            Serial.println(" ");
        }
        return Info;
    } else {
        return "ERROR";
    }
}

bool Unit_UHF_RFID::select(uint8_t *epc) {
    memcpy(buffer, SET_SELECT_PARAMETER_CMD, sizeof(SET_SELECT_PARAMETER_CMD));

    if (_debug) {
        Serial.println("---raw---cmd-------------------");
        for (uint8_t i = 0; i < 26; i++) {
            Serial.print(hex2str(buffer[i]));
        }
        Serial.println(" ");
        Serial.println("-------------------------");
    }

    uint8_t check = 0;

    for (uint8_t i = 12; i < 24; i++) {
        buffer[i] = epc[i - 12];
    }

    for (uint8_t i = 1; i < 24; i++) {
        check += buffer[i];
    }

    buffer[24] = check & 0xff;

    if (_debug) {
        Serial.println("------cmd-------------------");
        for (uint8_t i = 0; i < 26; i++) {
            Serial.print(hex2str(buffer[i]));
        }
        Serial.println(" ");
        Serial.println("-------------------------");
    }

    sendCMD(buffer, sizeof(SET_SELECT_PARAMETER_CMD));
    if (waitMsg()) {
        if (_debug) {
            for (uint8_t i = 0; i < 25; i++) {
                Serial.print(hex2str(buffer[i]));
            }
            Serial.println(" ");
            Serial.println("-------------------------");
        }
        for (uint8_t i = 0; i < sizeof(SET_SELECT_OK); i++) {
            if (SET_SELECT_OK[i] != buffer[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

bool Unit_UHF_RFID::writeCard(uint8_t *data, size_t size, uint8_t membank,
                              uint16_t sa, uint32_t access_password) {
    memcpy(buffer, WRITE_STORAGE_CMD, sizeof(WRITE_STORAGE_CMD));
    buffer[5] = (access_password >> 24) & 0xff;
    buffer[6] = (access_password >> 16) & 0xff;
    buffer[7] = (access_password >> 8) & 0xff;
    buffer[8] = access_password & 0xff;

    buffer[9] = membank;

    buffer[10] = (sa >> 8) & 0xff;
    buffer[11] = sa & 0xff;

    uint8_t word = size / 2;

    buffer[12] = (word >> 8) & 0xff;
    buffer[13] = word & 0xff;

    uint8_t offset = 14;
    for (uint8_t i = 0; i < size; i++) {
        buffer[14 + i] = data[i];
        offset++;
    }
    uint8_t check = 0;
    for (uint8_t i = 1; i < offset; i++) {
        check += buffer[i];
    }

    buffer[offset]     = check & 0xff;
    buffer[offset + 1] = 0x7e;

    if (_debug) {
        Serial.println("send cmd:");
        for (uint8_t i = 0; i < (offset + 2); i++) {
            Serial.print(hex2str(buffer[i]));
        }
        Serial.println(" ");
    }

    sendCMD(buffer, offset + 2);
    if (waitMsg()) {
        if (_debug) {
            Serial.println("result:");
            for (uint8_t i = 0; i < 25; i++) {
                Serial.print(hex2str(buffer[i]));
            }
            Serial.println(" ");
        }
        if (WRITE_STORAGE_ERROR[2] == buffer[2]) {
            Serial.println("Write Error");
            return false;
        }
        return true;
    } else {
        return false;
    }
}

bool Unit_UHF_RFID::readCard(uint8_t *data, size_t size, uint8_t membank,
                             uint16_t sa, uint32_t access_password) {
    memcpy(buffer, READ_STORAGE_CMD, sizeof(READ_STORAGE_CMD));
    buffer[5]    = (access_password >> 24) & 0xff;
    buffer[6]    = (access_password >> 16) & 0xff;
    buffer[7]    = (access_password >> 8) & 0xff;
    buffer[8]    = access_password & 0xff;
    buffer[9]    = membank;
    uint8_t word = size / 2;
    buffer[12]   = (word >> 8) & 0xff;
    buffer[13]   = word & 0xff;

    uint8_t check = 0;

    for (uint8_t i = 1; i < 14; i++) {
        check += buffer[i];
    }

    buffer[14] = check & 0xff;

    if (_debug) {
        Serial.println("send cmd:");
        for (uint8_t i = 0; i < sizeof(READ_STORAGE_CMD); i++) {
            Serial.print(hex2str(buffer[i]));
        }
        Serial.println(" ");
        Serial.println("-------------------------");
    }

    sendCMD(buffer, sizeof(READ_STORAGE_CMD));
    if (waitMsg()) {
        if (_debug) {
            Serial.println("result:");
            for (uint8_t i = 0; i < (22 + size); i++) {
                Serial.print(hex2str(buffer[i]));
            }
            Serial.println(" ");
            Serial.println("-------------------------");
        }
        if (READ_STORAGE_ERROR[2] == buffer[2]) {
            return false;
        }

        uint8_t temp[size];
        for (uint8_t i = 0; i < size; i++) {
            temp[i] = buffer[20 + i];
        }
        memcpy(data, temp, size);
        return true;
    } else {
        return false;
    }
}

// 2600 => 26dB
bool Unit_UHF_RFID::setTxPower(uint16_t db) {
    memcpy(buffer, SET_TX_POWER, sizeof(SET_TX_POWER));
    buffer[5] = (db >> 8) & 0xff;
    buffer[6] = db & 0xff;

    uint8_t check = 0;

    for (uint8_t i = 1; i < 7; i++) {
        check += buffer[i];
    }
    buffer[7] = check & 0xff;
    sendCMD(buffer, sizeof(SET_TX_POWER));
    if (waitMsg()) {
        if (buffer[2] != 0xB6) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}