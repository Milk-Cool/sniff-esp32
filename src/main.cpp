#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "sniffer.h"
#include "api.h"
#include "config.h"

Preferences prefs;

String inp = "";
void read_input() {
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
                ? "apikey"
                : "api",
                inp.substring(1));
            inp = "";
        } else inp += c;
    }
}
void setup() {
    prefs.begin("sniff");
    Serial.begin(115200);
    Serial.println("Starting up");

    WiFi.mode(WIFI_STA);

    if(!prefs.isKey("ssid") || !prefs.isKey("pass") || !prefs.isKey("api") || !prefs.isKey("apikey")) {
        Serial.println("Waiting 10s to allow for config changes...");
        for(int i = 0; i < 10; i++) {
            read_input();
            delay(1000);
        }
        return;
    }
}

void loop() {
    delay(1);

    read_input();

    WiFi.disconnect();
    auto sniffed = sniffer();

    if(!prefs.isKey("ssid") || !prefs.isKey("pass") || !prefs.isKey("apikey") || !prefs.isKey("api")) {
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
    api_send_sniff(prefs.getString("api"), prefs.getString("apikey"), sniffed);
}