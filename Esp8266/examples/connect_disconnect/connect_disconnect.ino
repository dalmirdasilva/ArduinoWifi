#include <Esp8266.h>

Esp8266 esp = Esp8266(2, 3);
unsigned char ip[4];

void setup() {
    Serial.begin(19200);
    Serial.println("Initializing...");
    esp.begin(19200);
    if (esp.connectToAp("SSID", "PASSWORD")) {
        Serial.println("Connected successfully to the AP.");
        Serial.print("IP: ");
        if (esp.getStationIp(ip)) {
            Serial.print(ip[0]);
            Serial.print('.');
            Serial.print(ip[1]);
            Serial.print('.');
            Serial.print(ip[2]);
            Serial.print('.');
            Serial.println(ip[3]);
        }
    }
    if (esp.disconnectFromAp()) {
        Serial.println("Disconnected successfully from the AP.");
    }
    Serial.println("Done.");
}

void loop() {
}