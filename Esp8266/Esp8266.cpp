/**
 * Arduino - Wifi driver
 *
 * @author Dalmir da Silva <dalmirdasilva@gmail.com>
 */

#ifndef __ARDUINO_WIFI_ESP8266_CPP__
#define __ARDUINO_WIFI_ESP8266_CPP__ 1

#include <Arduino.h>
#include "Esp8266.h"

Esp8266::Esp8266(unsigned char receivePin, unsigned char transmitPin)
        : SoftwareSerialAttentionDevice(receivePin, transmitPin), echo(true) {
}

unsigned char Esp8266::begin(long bound) {
    SoftwareSerial::begin(bound);
    setEcho(false);
    return sendCommandExpecting("AT", "OK");
}

void Esp8266::setEcho(bool echo) {
    this->echo = echo;
    char command[] = "E0";
    if (echo) {
        command[1] = '1';
    }
    sendCommand(command, true, 100);
}

bool Esp8266::connectToAp(const char *ssid, const char *pwd, bool tmp) {
    write("AT+CWJAP_");
    write(tmp ? "CUR" : "DEF");
    write("=\"");
    write(ssid);
    write("\",\"");
    write(pwd);
    return sendCommandExpecting("\"", "OK") || waitUntilReceive("OK", ESP8266_CWJAP_TIMEOUT) >= 0;
}

bool Esp8266::disconnectFromAp() {
    return sendCommand("+CWQAP", true);
}

bool Esp8266::getStationIp(unsigned char ip[4]) {
    const char *at = NULL;
    if (!sendCommandExpecting("+CIFSR", "OK", true) || (at = findInResponse("STAIP")) == NULL) {
        return false;
    }
    return parseIp(at, ip) == 4;
}

unsigned char Esp8266::parseIp(const char *buf, unsigned char ip[4]) {
    const char *p = buf;
    unsigned char j, i = 0, n = 0, part[4] = { 0 };
    while (*p != '\0' && n < 4) {
        if (*p >= '0' && *p <= '9') {
            part[i++ % 3] = *p;
        }
        if (*p == '.') {
            ip[n++] = (unsigned char) atoi((const char*) part);
            for (j = 0; j < 4; j++) {
                part[j] = '\0';
            }
            i = 0;
        }
        p++;
    }
    if (i > 0) {
        ip[n++] = (unsigned char) atoi((const char*) part);
    }
    return n;
}

#endif /* __ARDUINO_WIFI_ESP8266_CPP__ */
