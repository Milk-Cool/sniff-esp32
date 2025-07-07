#include "api.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_wifi.h"

String get_mac_str(uint8_t* mac) {
    char mac_str[18];
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    mac_str[17] = '\0';
    return String(mac_str);
}

String get_mac() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    return get_mac_str(mac);
}

void api_send_sniff(String base, std::vector<Sniff> sniffs) {
    if(base.charAt(base.length() - 1) == '/')
        base = base.substring(0, base.length() - 1);
    base += "/sniff";

    Serial.println("Sending data to server...");

    HTTPClient http;
    http.begin(base);

    JsonDocument data;
    for(Sniff sn : sniffs) {
        JsonDocument doc;
        JsonArray mac;
        doc["mac"] = get_mac_str(sn.mac);
        doc["rssi"] = sn.rssi;
        data["sniffs"].add(doc);
    }

    data["mac"] = get_mac();

    String out;
    serializeJson(data, out);

    http.addHeader("Content-Type", "application/json");
    int code = http.POST(out);

    Serial.printf("Code = %d\n", code);

    http.end();
}