#pragma once

#include <Arduino.h>

// Get the device's 1-byte ID (stored in NVS). Assigns a random one on first boot.
uint8_t deviceGetID();

// Set a new device ID (persists to NVS).
void deviceSetID(uint8_t id);
