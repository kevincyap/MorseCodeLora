#include "cryptoHandler.h"
#include "config.h"
#include <mbedtls/aes.h>
#include <esp_random.h>
#include <string.h>

// AES-CTR needs a 16-byte counter block. We build it from the 4-byte nonce
// padded with zeros. For our short messages this is sufficient.
static void buildCounterBlock(const uint8_t *nonce4, uint8_t *counterBlock) {
    memset(counterBlock, 0, 16);
    memcpy(counterBlock, nonce4, CRYPTO_NONCE_SIZE);
}

uint8_t cryptoEncrypt(const uint8_t *plain, uint8_t len, uint8_t *out, uint8_t outBufSize) {
    uint8_t totalLen = CRYPTO_NONCE_SIZE + len;
    if (totalLen > outBufSize || len == 0) {
        return 0;
    }

    // Generate random nonce
    uint32_t rand = esp_random();
    memcpy(out, &rand, CRYPTO_NONCE_SIZE);

    // Set up AES-CTR
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, CRYPTO_KEY, 128);

    uint8_t counterBlock[16];
    buildCounterBlock(out, counterBlock);

    uint8_t streamBlock[16];
    size_t ncOff = 0;

    mbedtls_aes_crypt_ctr(&ctx, len, &ncOff, counterBlock, streamBlock,
                           plain, out + CRYPTO_NONCE_SIZE);

    mbedtls_aes_free(&ctx);
    return totalLen;
}

uint8_t cryptoDecrypt(const uint8_t *in, uint8_t inLen, uint8_t *out, uint8_t outBufSize) {
    if (inLen <= CRYPTO_NONCE_SIZE) {
        return 0;
    }

    uint8_t cipherLen = inLen - CRYPTO_NONCE_SIZE;
    if (cipherLen > outBufSize) {
        return 0;
    }

    // Extract nonce
    const uint8_t *nonce = in;
    const uint8_t *cipher = in + CRYPTO_NONCE_SIZE;

    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, CRYPTO_KEY, 128);

    uint8_t counterBlock[16];
    buildCounterBlock(nonce, counterBlock);

    uint8_t streamBlock[16];
    size_t ncOff = 0;

    mbedtls_aes_crypt_ctr(&ctx, cipherLen, &ncOff, counterBlock, streamBlock,
                           cipher, out);

    mbedtls_aes_free(&ctx);
    return cipherLen;
}
