#pragma once

#include <Arduino.h>

enum class TransmitStatus {
    Ok,
    Failed,
    NoAck,
};

// Initializes the SX1262-based RadioLib instance.
bool radioInit(float frequencyMHz);

// Arms the radio for continuous receive; safe to call repeatedly.
bool radioStartReceive();

// Checks if a packet has been received and returns it when available.
bool radioReceiveAvailable(String &outMessage, int16_t &outRssi, bool sendAck = false);

// Sends a packet using the configured LoRa parameters.
TransmitStatus radioTransmit(String &payload, bool shouldWaitForAck = false);

// Indicates whether the radio is idle (not currently transmitting).
bool radioIdle();

// Waits for an acknowledgment packet within the specified timeout (ms).
bool waitForAck(unsigned long timeoutMs);