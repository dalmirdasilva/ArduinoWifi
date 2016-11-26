/**
 * Arduino - Esp8266 driver
 * 
 * @author Dalmir da Silva <dalmirdasilva@gmail.com>
 */

#include "TcpSocket.h"

TcpSocket::TcpSocket(Esp8266 *esp)
        : esp(esp), multiplexed(false) {
}

bool TcpSocket::useMultiplexer(bool use) {
    char command[] = "+CIPMUX=0";
    if (use) {
        command[8] = '1';
    }
    multiplexed = use;
    return esp->sendCommandExpecting(command, "OK", true);
}

unsigned char TcpSocket::status() {
    return status(-1);
}

unsigned char TcpSocket::status(char connection) {
//    ConnectionState state = TcpSocket::ERROR_WHEN_QUERING;
//    int pos;
//    esp->write("AT+CIPSTATUS");
//    if (connection != (char) -1) {
//        esp->write('=');
//        esp->write('0' + connection);
//    }
//    esp->sendCommandExpecting("", "OK");
//    pos = esp->waitUntilReceive("STATE", GPRS_Esp8266_CIPSTATUS_TIMEOUT);
//    if (pos >= 0) {
//        if (esp->doesResponseContains("INITIAL")) {
//            state = TcpSocket::IP_INITIAL;
//        } else if (esp->doesResponseContains("START")) {
//            state = TcpSocket::IP_START;
//        } else if (esp->doesResponseContains("CONFIG")) {
//            state = TcpSocket::IP_CONFIG;
//        } else if (esp->doesResponseContains("GPRSACT")) {
//            state = TcpSocket::IP_GPRSACT;
//        } else if (esp->doesResponseContains("STATUS")) {
//            state = TcpSocket::IP_STATUS;
//        } else if (esp->doesResponseContains("CONNECTING") || esp->doesResponseContains("LISTENING")) {
//            state = TcpSocket::CONNECTING_OR_LISTENING;
//        } else if (esp->doesResponseContains("CONNECT OK")) {
//            state = TcpSocket::CONNECT_OK;
//        } else if (esp->doesResponseContains("CLOSING")) {
//            state = TcpSocket::CLOSING;
//        } else if (esp->doesResponseContains("CLOSED")) {
//            state = TcpSocket::CLOSED;
//        } else if (esp->doesResponseContains("DEACT")) {
//            state = TcpSocket::PDP_DEACT;
//        }
//    }
//    return state;
}

unsigned char TcpSocket::configureDns(const char *primary, const char *secondary) {
    bool expected;
    esp->write("AT+CDNSCFG=\"");
    esp->write(primary);
    esp->write("\",\"");
    esp->write(secondary);
    expected = esp->sendCommandExpecting("\"", "OK");
    return expected ? TcpSocket::OK : TcpSocket::ERROR;
}

unsigned char TcpSocket::open(char connection, const char *mode, const char *address, unsigned int port) {
    int pos;
    esp->write("AT+CIPSTART=");
    if (connection != (char) -1) {
        esp->write('0' + connection);
        esp->write(',');
    }
    esp->write('"');
    esp->write(mode);
    esp->write("\",\"");
    esp->write(address);
    esp->write("\",\"");
    esp->print(port, DEC);
    esp->sendCommand("\"");
    pos = esp->waitUntilReceive("CONNECT", GPRS_Esp8266_CIPSTART_TIMEOUT);
    if (pos >= 0 && !esp->doesResponseContains("FAIL")) {
        return TcpSocket::OK;
    }
    return (unsigned char) TcpSocket::ERROR;
}

unsigned int TcpSocket::send(char connection, unsigned char *buf, unsigned int len) {
    bool ok;
    int pos = -1;
    unsigned int sent = 0;
    esp->write("AT+CIPSEND=");
    if (connection != (char) -1) {
        esp->write('0' + connection);
        esp->write(',');
    }
    esp->print(len, DEC);
    ok = esp->sendCommandExpecting("", ">");
    if (ok) {
        sent = (unsigned int) esp->write((const char *) buf, len);
        pos = esp->waitUntilReceive("SEND OK", GPRS_Esp8266_SEND_TIMEOUT);
    }
    return pos >= 0 ? sent : 0;
}

unsigned char TcpSocket::close(char connection) {
    int pos;
    esp->write("AT+CIPCLOSE=1");
    if (connection != (char) -1) {
        esp->write(',');
        esp->write('0' + connection);
    }
    esp->sendCommand();
    pos = esp->waitUntilReceive("CLOSE OK", GPRS_Esp8266_CIPSTART_TIMEOUT);
    if (pos >= 0) {
        return TcpSocket::OK;
    }
    return TcpSocket::ERROR;
}

unsigned char TcpSocket::close() {
    return close(-1);
}

// TODO
unsigned char TcpSocket::resolve(const char *name, unsigned char ip[4]) {
    OperationResult result = TcpSocket::ERROR;
    bool ok;
    int pos;
    const char* p;
    esp->write("AT+CDNSGIP=\"");
    esp->write(name);
    ok = esp->sendCommandExpecting("\"", "OK");
    if (ok) {
        pos = esp->waitUntilReceive("+CDNSGIP: 1", GPRS_Esp8266_CDNSGIP_TIMEOUT);
        if (pos >= 0) {
            pos = esp->waitUntilReceive("\",\"", GPRS_Esp8266_CDNSGIP_TIMEOUT);
            if (pos >= 0) {
                p = (const char*) esp->getLastResponse();
                if (parseIp(p + pos, ip) == 4) {
                    result = TcpSocket::OK;
                }
            }
        }
    }
    return result;
}

unsigned char TcpSocket::configureServer(unsigned char mode, unsigned int port) {
    mode &= 0x01;
    esp->write("AT+CIPSERVER=");
    esp->print(mode, DEC);
    esp->write(',');
    esp->print(mode, DEC);
    return esp->sendCommandExpecting("", "OK") ? TcpSocket::OK : TcpSocket::ERROR;
}

unsigned char TcpSocket::shutdown() {
    return esp->sendCommandExpecting("AT+CIPSHUT", "SHUT OK") ? TcpSocket::OK : TcpSocket::ERROR;
}

unsigned char TcpSocket::getTransmittingState(char connection, void *stateStruct) {
    int pos;
    unsigned char *response;
    TransmittingState *state = (TransmittingState *) stateStruct;
    esp->write("AT+CIPACK=");
    if (connection != (char) -1) {
        esp->write(',');
        esp->write('0' + connection);
    }
    esp->sendCommand();
    pos = esp->waitUntilReceive("+CIPACK", GPRS_Esp8266_CIPACK_TIMEOUT);
    if (pos >= 0) {
        response = esp->getLastResponse();
        // < +CIPACK: 2,2,0
        sscanf((const char *) (response + pos), "+CIPACK: %d,%d,%d", &state->txlen, &state->acklen, &state->nacklen);
        return TcpSocket::OK;
    } else {
        state->txlen = state->acklen = state->nacklen = 0;
        return TcpSocket::ERROR;
    }
}

unsigned char TcpSocket::parseIp(const char *buf, unsigned char ip[4]) {
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

