#pragma once

#include <Arduino.h>

// Get the device's 1-byte ID (stored in NVS). Assigns a random one on first boot.
uint8_t deviceGetID();

// Set a new device ID (persists to NVS).
void deviceSetID(uint8_t id);

// Get the device name (stored in NVS). Returns "Device XX" if not set.
String deviceGetName();

// Set the device name (persists to NVS). Max 16 characters.
void deviceSetName(const String &name);
