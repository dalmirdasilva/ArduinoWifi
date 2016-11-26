/**
 * Arduino - Esp8266 driver
 *
 * @author Dalmir da Silva <dalmirdasilva@gmail.com>
 */

#ifndef __ARDUINO_WIFI_ESP8266_H__
#define __ARDUINO_WIFI_ESP8266_H__ 1

#include <Arduino.h>
#include <SoftwareSerialAttentionDevice.h>

#define ESP8266_CWJAP_TIMEOUT           20000UL

class Esp8266: public SoftwareSerialAttentionDevice {

    /**
     * Using echo.
     */
    bool echo;

public:

    /**
     * Public constructor.
     *
     * @param serial
     */
    Esp8266(unsigned char receivePin, unsigned char transmitPin);

    /**
     * Initializes the device.
     *
     * @param           The bound rate to be used.
     * @return          0 if not success, > 0 otherwise.
     */
    unsigned char begin(long bound);

    /**
     * Configures echo mode
     *
     * @param echo
     */
    void setEcho(bool echo);

    /**
     * Join access point
     *
     * Command:
     * AT+CWJAP_[CUR|DEF]="SSID","PASSWORD"
     *
     * @param   ssid            Ssid.
     * @param   pwd             Password.
     * @return  bool            Successfulness.
     */
    bool connectToAp(const char *ssid, const char *pwd, bool tmp = true);

    /**
     * Disconnects from access point.
     *
     * Command: AT+CWQAP
     *
     * @return  bool            Successfulness.
     */
    bool disconnectFromAp();

    /**
     * Get IP address of ESP8266 station.
     */
    bool getStationIp(unsigned char ip[4]);

    /**
     * AT+CWAUTOCONN
     */

    /**
     * Tries to parse an IP from string.
     *
     * @param           buf should have the following format: [0-9]{1,4}.[0-9]{1,4}.[0-9]{1,4}.[0-9]{1,4}
     * @param           ip  whre to store the parsed ip, 4 bytes.
     */
    unsigned char static parseIp(const char *buf, unsigned char ip[4]);
};

#endif /* __ARDUINO_WIFI_ESP8266_H__ */
