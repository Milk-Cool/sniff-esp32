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
    return memcmp(sniff.mac, search_for.mac, 6) == 0 && search_for.ssid == sniff.ssid; // do not check rssi
}

static void cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*) buf;
    if(type != WIFI_PKT_MGMT) return;
    Sniff sniff;
    memcpy(sniff.mac, packet->payload + 10, 6);
    sniff.rssi = packet->rx_ctrl.rssi;

    if((packet->payload[0] & 0xf0) == 0x40) {
        int pos = 24;
        while(pos < packet->rx_ctrl.sig_len) {
            uint8_t tag = packet->payload[pos];
            uint8_t len = packet->payload[pos + 1];
            if(tag == 0x00) {
                char ssid[33];
                memset(ssid, 0, 33);
                memcpy(ssid, &packet->payload[pos + 2], len > 32 ? 32 : len);
                for(int j = 0; j < 32 && ssid[j] != 0; j++)
                    if(ssid[j] >= 0x20 && ssid[j] != 0x7f) // Check printable
                        sniff.ssid += ssid[j];
                break;
            }
            pos += len + 2;
        }
    }

    search_for = sniff;
    if(std::find_if(ret.begin(), ret.end(), exists) != ret.end()) return;

    ret.push_back(sniff);
    Serial.printf("Found %02x:%02x:%02x:%02x:%02x:%02x\n",
        sniff.mac[0], sniff.mac[1], sniff.mac[2],
        sniff.mac[3], sniff.mac[4], sniff.mac[5]);
    if(sniff.ssid != "0")
        Serial.printf("with SSID %s\n", sniff.ssid.c_str());
}

std::vector<Sniff> sniffer() {
    Serial.println("Starting sniffing...");
    ret.erase(ret.begin(), ret.end());

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filter); // remove if it doesn't work
    esp_wifi_set_promiscuous_rx_cb(cb);

    for(int i = 0; i < sizeof(channels) / sizeof(channels[0]); i++) {
        Serial.printf("Channel %d\n", channels[i]);
        esp_wifi_set_channel(channels[i], WIFI_SECOND_CHAN_NONE);
        delay(CHANNEL_DELAY);
    }

    esp_wifi_set_promiscuous(false);

    return ret;
}