#include "api.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

void api_send_sniff(String base, std::vector<Sniff> sniffs) {
    if(base.charAt(base.length() - 1) == '/')
        base = base.substring(0, base.length() - 1);
    base += "/sniff";

    Serial.println("Sending data to server...");

    HTTPClient http;
    http.begin(base);

    JsonArray arr;
    for(Sniff sn : sniffs) {
        JsonDocument doc;
        JsonArray mac;
        for(int i = 0; i < 6; i++)
            mac.add(sn.mac[i]);
        doc["mac"] = mac;
        doc["rssi"] = sn.rssi;
        arr.add(doc);
    }
    String out;
    serializeJson(arr, out);

    http.addHeader("Content-Type", "application/json");
    int code = http.POST(out);

    Serial.printf("Code = %d\n", code);

    http.end();
}