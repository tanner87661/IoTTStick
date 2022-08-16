#include "Unit_PoESP32.h"

/*! @brief Initialize the Unit PoESP32.*/
void Unit_PoESP32::Init(HardwareSerial* serial, unsigned long baud, uint8_t RX,
                        uint8_t TX) {
    _serial = serial;
    _serial->begin(baud, SERIAL_8N1, RX, TX);
}

/*! @brief Waiting for a period of time to receive a message
    @return Received messages.. */
String Unit_PoESP32::waitMsg(unsigned long time) {
    String restr;
    unsigned long start = millis();
    while (1) {
        if (Serial2.available() || (millis() - start) < time) {
            String str = Serial2.readString();
            restr += str;
        } else {
            break;
        }
    }
    return restr;
}

/*! @brief Send a command */
void Unit_PoESP32::sendCMD(String command) {
    _serial->println(command);
}

/*! @brief Detecting device connection status
    @return True if good connection, false otherwise. */
bool Unit_PoESP32::checkDeviceConnect() {
    sendCMD("AT");
    _readstr = waitMsg(100);
    if (_readstr.indexOf("OK") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief Detecting device ETH connection status
    @return True if good connection, false otherwise. */
bool Unit_PoESP32::checkETHConnect() {
    sendCMD("AT+CIPETH?");
    _readstr = waitMsg(1000);

    if (_readstr.indexOf("192") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief Create a TCP client
    @return True if create successfully, false otherwise. */
bool Unit_PoESP32::createTCPClient(String ip, int port) {
    sendCMD("AT+CIPCLOSE");
    delay(500);
    sendCMD("AT+CIPSTART=\"TCP\",\"" + ip + "\"," + String(port));
    _readstr = waitMsg(1000);
    if (_readstr.indexOf("CONNECT") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief send a TCP data
    @return True if send successfully, false otherwise. */
bool Unit_PoESP32::sendTCPData(uint8_t* buffer, size_t size) {
    sendCMD("AT+CIPSEND=" + String(size) + "");
    delay(500);
    _serial->write(buffer, size);
    _serial->print("");
    _readstr = waitMsg(500);

    if (_readstr.indexOf("SEND OK") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief Create a MQTT client
    @return True if create successfully, false otherwise. */
bool Unit_PoESP32::createMQTTClient(String host, String port, String clientId,
                                    String user_name, String user_pwd) {
    sendCMD("AT+MQTTCLEAN=0");
    delay(500);
    sendCMD("AT+MQTTUSERCFG=0,1,\"" + clientId + "\",\"" + user_name + "\",\"" +
            user_pwd + "\",0,0,\"\"");
    delay(500);
    sendCMD("AT+MQTTCONN=0,\"" + host + "\"," + port + ",0");
    _readstr = waitMsg(4000);
    Serial.print(_readstr);
    if (_readstr.indexOf("+MQTTCONNECTED") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief public a MQTT message
    @return True if public successfully, false otherwise. */
bool Unit_PoESP32::publicMQTTMsg(String topic, String payload, String qos) {
    delay(500);
    _readstr = waitMsg(500);

    sendCMD("AT+MQTTPUB=0,\"" + topic + "\",\"" + payload + "\"," + qos + ",0");
    if (_readstr.indexOf("OK") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief subscribe a MQTT message
    @return True if subscribe successfully, false otherwise. */
bool Unit_PoESP32::subscribeMQTTMsg(String topic, String qos) {
    delay(500);
    sendCMD("AT+MQTTSUB=0,\"" + topic + "\"," + qos + "");
    _readstr = waitMsg(500);

    if (_readstr.indexOf("OK") != -1) {
        return true;
    } else {
        return false;
    }
}

/*! @brief Create a HTTP client
    @return True if create successfully, false otherwise. */
String Unit_PoESP32::createHTTPClient(http_method_t method,
                                      http_content_t content_type, String url,
                                      String payload) {
    if (payload != "") {
        sendCMD("AT+HTTPCLIENT=" + String(method) + "," + String(content_type) +
                ",\"" + url + "\",\"\",\"\",1,\"" + payload + "\"");
    } else {
        sendCMD("AT+HTTPCLIENT=" + String(method) + "," + String(content_type) +
                ",\"" + url + "\",\"\",\"\",1");
    }
    _readstr = waitMsg(4000);
    return _readstr;
}
