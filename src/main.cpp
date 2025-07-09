#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "sniffer.h"
#include "api.h"
#include "config.h"

Preferences prefs;

void setup() {
    prefs.begin("sniff");
    Serial.begin(115200);
    Serial.println("Starting up");

    WiFi.mode(WIFI_STA);

    if(!prefs.isKey("ssid") || !prefs.isKey("pass") || !prefs.isKey("api") || !prefs.isKey("key")) {
        Serial.println("Waiting 10s to allow for config changes...");
        delay(10000); // Serial data is cached anyawy
        return;
    }
}

String inp = "";
void loop() {
    delay(1);

    while(Serial.available()) {
        char c = Serial.read();
        if(c == '\r') continue;
        if(c == '\n') {
            prefs.putString(
                inp[0] == 's'
                ? "ssid"
                : inp[0] == 'p'
                ? "pass"
                : inp[0] == 'k'
                ? "key"
                : "api",
                inp.substring(1));
            inp = "";
        } else inp += c;
    }

    WiFi.disconnect();
    auto sniffed = sniffer();

    if(!prefs.isKey("ssid") || !prefs.isKey("pass") || !prefs.isKey("key") || !prefs.isKey("api")) {
        Serial.println("Skipping uploading because not all preferences were found!");
        return;
    }
    WiFi.begin(prefs.getString("ssid"), prefs.getString("pass"));
    uint64_t last = millis();
    Serial.print("Connecting");
    while(!WiFi.isConnected() && millis() - last < CONNECT_TIMEOUT) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
    if(!WiFi.isConnected()) {
        Serial.println("Couldn't connect!");
        return;
    }
    api_send_sniff(prefs.getString("api"), prefs.getString("key"), sniffed);
}