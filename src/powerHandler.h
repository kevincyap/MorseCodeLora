#pragma once

#include <Arduino.h>

// Call once in setup() to disable WiFi/BT and configure wake sources.
void powerSetup();

// Call every loop iteration. Manages idle → dim → deep sleep transitions.
// Pass true if there was recent user/radio activity this tick.
void powerUpdate(bool activity);

// Immediately enter deep sleep (for force-sleep chord).
void powerDeepSleep();
