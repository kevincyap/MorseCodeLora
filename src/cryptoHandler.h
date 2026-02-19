#pragma once

#include <Arduino.h>

// Encrypt plaintext using AES-128-CTR with a random 4-byte nonce.
// Output: [nonce:4][ciphertext:len]. Returns total output length, or 0 on error.
uint8_t cryptoEncrypt(const uint8_t *plain, uint8_t len, uint8_t *out, uint8_t outBufSize);

// Decrypt data produced by cryptoEncrypt.
// Input: [nonce:4][ciphertext:N]. Returns plaintext length, or 0 on error.
uint8_t cryptoDecrypt(const uint8_t *in, uint8_t inLen, uint8_t *out, uint8_t outBufSize);
