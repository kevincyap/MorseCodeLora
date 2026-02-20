#pragma once

#include <Arduino.h>
#include "config.h"
#include "packetProtocol.h"

enum class TransmitStatus : uint8_t {
    Ok,
    Failed,
    NoAck,
    AlreadyTransmitting,
    NoPacket,
    TransmitFailed
};

// Initializes the SX1262 with range-optimized parameters from config.h.
bool radioInit();

// Arms the radio for continuous receive; safe to call repeatedly.
bool radioStartReceive();

// Non-blocking check for received packet. Filters by localID (accepts broadcast + own address).
bool radioReceivePacket(Packet &outPkt, int16_t &outRssi, uint8_t localID);

// Transmit a packet. Optionally waits for ACK (blocks up to ACK_TIMEOUT_MS).
TransmitStatus radioTransmitPacket(const Packet &pkt, bool waitAck = false, uint8_t localID = 0);

// Indicates whether the radio is idle.
bool radioIdle();

// Returns RSSI of last received packet.
int16_t radioLastRssi();

// Put the radio into RX duty cycle mode (low-power periodic listen).
// The SX1262 autonomously wakes, checks for preamble, and sleeps.
// DIO1 fires when a packet is received.
bool radioStartDutyCycle();

// Disable external PA for deep sleep.
void radioPaSleep();