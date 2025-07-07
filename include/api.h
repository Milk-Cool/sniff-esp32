#pragma once
#include <Arduino.h>
#include <vector>
#include "sniff.h"

void api_send_sniff(String base, std::vector<Sniff> sniffs);