#pragma once

#include <Arduino.h>
#include "config.h"

// =============================================================================
// Packet Protocol for MorseCodeLora
//
// Wire format:
//   [flags:1] [srcID:1] [dstID:1] [seqNum:1] [textPayload\0]
//
// RadioLib handles radio-level CRC.
// =============================================================================

struct Packet {
    uint8_t flags   = 0;
    uint8_t srcID   = 0;
    uint8_t dstID   = BROADCAST_ADDR;
    uint8_t seqNum  = 0;
    String  text;           // decoded ASCII message

    bool isAckRequest() const  { return flags & PACKET_FLAG_ACK_REQ; }
    bool isAck() const         { return flags & PACKET_FLAG_IS_ACK; }
    bool isBroadcast() const   { return flags & PACKET_FLAG_BROADCAST; }
};

// Serialize a Packet into a byte buffer for transmission.
// Returns the number of bytes written, or 0 on error.
uint8_t packetSerialize(const Packet &pkt, uint8_t *buf, uint8_t bufSize);

// Deserialize a byte buffer into a Packet.
// Returns true on success.
bool packetDeserialize(const uint8_t *buf, uint8_t len, Packet &outPkt);

// Convenience: build an ACK packet in response to a received packet.
Packet packetMakeAck(const Packet &received, uint8_t localID);
