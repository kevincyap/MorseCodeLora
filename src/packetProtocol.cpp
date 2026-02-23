#include "packetProtocol.h"
#include "cryptoHandler.h"
#include <string.h>

static constexpr uint8_t HEADER_SIZE = 4;  // flags + srcID + dstID + seqNum

uint8_t packetSerialize(const Packet &pkt, uint8_t *buf, uint16_t bufSize) {
    uint8_t pos = 0;
    if (bufSize < HEADER_SIZE) return 0;

    buf[pos++] = pkt.flags;
    buf[pos++] = pkt.srcID;
    buf[pos++] = pkt.dstID;
    buf[pos++] = pkt.seqNum;

    // ACK packets have no payload — skip encryption
    if (pkt.isAck()) {
        return pos;
    }

    // Encrypt text payload: produces [nonce:4][ciphertext:N]
    const uint8_t *plain = reinterpret_cast<const uint8_t *>(pkt.text.c_str());
    uint8_t plainLen = pkt.text.length();
    if (plainLen == 0) return 0;

    uint8_t encLen = cryptoEncrypt(plain, plainLen, &buf[pos], bufSize - pos);
    if (encLen == 0) return 0;

    return pos + encLen;
}

bool packetDeserialize(const uint8_t *buf, uint8_t len, Packet &outPkt) {
    if (len < HEADER_SIZE) return false;

    outPkt.flags  = buf[0];
    outPkt.srcID  = buf[1];
    outPkt.dstID  = buf[2];
    outPkt.seqNum = buf[3];

    // ACK packets have no payload
    if (outPkt.isAck()) {
        return true;
    }

    uint8_t payloadLen = len - HEADER_SIZE;
    if (payloadLen <= CRYPTO_NONCE_SIZE) return false;

    // Decrypt: input is [nonce:4][ciphertext:N]
    uint8_t plainBuf[240];
    uint8_t plainLen = cryptoDecrypt(&buf[HEADER_SIZE], payloadLen, plainBuf, sizeof(plainBuf));
    if (plainLen == 0) return false;

    // Null-terminate and assign
    plainBuf[plainLen] = '\0';
    outPkt.text = String(reinterpret_cast<char *>(plainBuf));

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
