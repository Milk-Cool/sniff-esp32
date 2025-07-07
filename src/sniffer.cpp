#include "sniffer.h"
#include <WiFi.h>
#include "config.h"
#include "esp_wifi.h"

static std::vector<Sniff> ret;
static const int channels[] = { 1, 6, 11 };
static const wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT
};

static Sniff search_for;
static bool exists(Sniff sniff) {
    return memcmp(sniff.mac, search_for.mac, 6) == 0; // do not check rssi
}

static void cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*) buf;
    if(type != WIFI_PKT_MGMT) return;
    Sniff sniff;
    memcpy(sniff.mac, packet->payload + 10, 6);
    sniff.rssi = packet->rx_ctrl.rssi;

    search_for = sniff;
    if(std::find_if(ret.begin(), ret.end(), exists) == ret.end())
        ret.push_back(sniff);
}

std::vector<Sniff> sniffer() {
    Serial.println("Starting sniffing...");
    ret.erase(ret.begin(), ret.end());

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filter); // remove if it doesn't work
    esp_wifi_set_promiscuous_rx_cb(cb);

    for(int i = 0; i < sizeof(channels) / sizeof(channels[0]); i++) {
        Serial.printf("Channel %d\n", i);
        esp_wifi_set_channel(channels[i], WIFI_SECOND_CHAN_NONE);
        delay(CHANNEL_DELAY);
    }

    esp_wifi_set_promiscuous(false);

    return ret;
}