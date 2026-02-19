#include "packetProtocol.h"
#include <string.h>

static constexpr uint8_t HEADER_SIZE = 4;  // flags + srcID + dstID + seqNum

uint8_t packetSerialize(const Packet &pkt, uint8_t *buf, uint8_t bufSize) {
    // Use uint16_t to avoid overflow in size calculation
    uint16_t needed = HEADER_SIZE + pkt.text.length() + 1;
    if (needed > bufSize || needed > 255) {
        return 0;
    }

    uint8_t pos = 0;
    buf[pos++] = pkt.flags;
    buf[pos++] = pkt.srcID;
    buf[pos++] = pkt.dstID;
    buf[pos++] = pkt.seqNum;

    memcpy(&buf[pos], pkt.text.c_str(), pkt.text.length() + 1);
    pos += pkt.text.length() + 1;

    return pos;
}

bool packetDeserialize(const uint8_t *buf, uint8_t len, Packet &outPkt) {
    if (len < HEADER_SIZE + 1) {  // at least header + one null terminator
        return false;
    }

    outPkt.flags  = buf[0];
    outPkt.srcID  = buf[1];
    outPkt.dstID  = buf[2];
    outPkt.seqNum = buf[3];

    const char *textStart = reinterpret_cast<const char *>(&buf[HEADER_SIZE]);
    const char *bufEnd    = reinterpret_cast<const char *>(&buf[len]);

    const char *textEnd = static_cast<const char *>(memchr(textStart, '\0', bufEnd - textStart));
    if (!textEnd) {
        return false;
    }
    outPkt.text = String(textStart);

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
