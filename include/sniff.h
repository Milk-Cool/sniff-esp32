#pragma once
#include <Arduino.h>
#include <stdint.h>

typedef struct {
    uint8_t mac[6];
    int8_t rssi;
    String ssid;
} Sniff;