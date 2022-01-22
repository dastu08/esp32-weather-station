// APPLICATION LAYER
// Source file of the Crypto component.

#include "al_crypto.h"

#include "esp_log.h"
#include "mbedtls/aes.h"

static const char* TAG = "al_crypto";

mbedtls_aes_context ctx;

void al_crypto_init() {
    char* key = "123456789abcdef";
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, &key, 128);
    mbedtls_aes_setkey_enc(&ctx, &key, 128);
}

void al_crypto_encrypt(char* plaintext, int len, char* ciphertext) {
    unsigned char iv[16] = "1111111111111111";
    unsigned char in[16];
    unsigned char out[16];
    for (int i = 0; (i < len) && (i < 15); i++) {
        in[i] = plaintext[i];
    }
    in[15] = '\0';
    ESP_LOGV(TAG, "input is: %s", in);
    mbedtls_aes_crypt_cbc(&ctx, ESP_AES_ENCRYPT, 16, iv, in, out);
    ESP_LOGV(TAG, "output is: %s", out);
}

// void myencrypt(int key, char* msg, char* out) {
//     out = msg;
// }
