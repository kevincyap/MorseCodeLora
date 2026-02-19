#include "packetProtocol.h"
#include <string.h>

static constexpr uint8_t HEADER_SIZE = 4;  // flags + srcID + dstID + seqNum

uint8_t packetSerialize(const Packet &pkt, uint8_t *buf, uint8_t bufSize) {
    // Header (4 bytes) + text + '\0' + morse + '\0'
    uint8_t needed = HEADER_SIZE + pkt.text.length() + 1 + pkt.morse.length() + 1;
    if (needed > bufSize) {
        return 0;
    }

    uint8_t pos = 0;
    buf[pos++] = pkt.flags;
    buf[pos++] = pkt.srcID;
    buf[pos++] = pkt.dstID;
    buf[pos++] = pkt.seqNum;

    memcpy(&buf[pos], pkt.text.c_str(), pkt.text.length() + 1);
    pos += pkt.text.length() + 1;

    memcpy(&buf[pos], pkt.morse.c_str(), pkt.morse.length() + 1);
    pos += pkt.morse.length() + 1;

    return pos;
}

bool packetDeserialize(const uint8_t *buf, uint8_t len, Packet &outPkt) {
    if (len < HEADER_SIZE + 2) {  // at least header + two null terminators
        return false;
    }

    outPkt.flags  = buf[0];
    outPkt.srcID  = buf[1];
    outPkt.dstID  = buf[2];
    outPkt.seqNum = buf[3];

    // Find first null terminator (end of text)
    const char *textStart = reinterpret_cast<const char *>(&buf[HEADER_SIZE]);
    const char *bufEnd    = reinterpret_cast<const char *>(&buf[len]);

    const char *textEnd = static_cast<const char *>(memchr(textStart, '\0', bufEnd - textStart));
    if (!textEnd) {
        return false;
    }
    outPkt.text = String(textStart);

    // Morse string follows the first null
    const char *morseStart = textEnd + 1;
    if (morseStart >= bufEnd) {
        return false;
    }
    const char *morseEnd = static_cast<const char *>(memchr(morseStart, '\0', bufEnd - morseStart));
    if (!morseEnd) {
        return false;
    }
    outPkt.morse = String(morseStart);

    return true;
}

Packet packetMakeAck(const Packet &received, uint8_t localID) {
    Packet ack;
    ack.flags  = PACKET_FLAG_IS_ACK;
    ack.srcID  = localID;
    ack.dstID  = received.srcID;
    ack.seqNum = received.seqNum;
    return ack;
}
