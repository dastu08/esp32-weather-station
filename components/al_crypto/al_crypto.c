// APPLICATION LAYER
// Source file of the Crypto component.

#include "al_crypto.h"

#include "../general/general.h"
#include "esp_log.h"
#include "mbedtls/aes.h"
#include "stdio.h"
#include "stdlib.h"

static const char* TAG = "al_crypto";

typedef unsigned char byte_t;

mbedtls_aes_context ctx;

void print_index(int length) {
    // print the index number above
    for (int i = 0; i < length; i++) {
        printf("%2d ", i);
    }
    printf("\n");
}

void print_bytes(byte_t* bytes, int length) {
    for (int i = 0; i < length; i++) {
        printf("%X ", bytes[i]);
    }
    printf("\n");
}

void print_chars(char* chars, int length) {
    for (int i = 0; i < length; i++) {
        printf(" %c ", chars[i]);
    }
    printf("\n");
}

void convert_hex2bytes(char* chars, byte_t* bytes, int length) {
    char buff[4];
    buff[0] = '0';
    buff[1] = 'x';
    for (int i = 0; i < length; i++) {
        buff[2] = chars[2 * i];
        buff[3] = chars[2 * i + 1];
        // printf("i = %d : ", i);
        // print_chars(buff, 4);
        bytes[i] = strtol(buff, NULL, 16);
    }
}

void al_crypto_init() {
    char key_hex[65] = "9018321f988274f6a4eaf29c82df2614a296f9c06ca5776a893e1d0c9e35e1f9";
    unsigned char key[32];
    // print_chars(key_hex, 64);
    convert_hex2bytes(key_hex, key, 32);
    // print_bytes(key, 32);
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key, 256);
    mbedtls_aes_setkey_dec(&ctx, &key, 256);

    ESP_LOGI(TAG, "init finished");
}

void al_crypto_encrypt(char* plaintext,
                       int len,
                       char* ciphertext) {
    byte_t iv[16] = "1111111111111111";
    byte_t in[16];
    byte_t out[16];

    for (int i = 0; (i < len) && (i < 16); i++) {
        in[i] = plaintext[i];
    }
    // in[16] = '\0';
    ESP_LOGV(TAG, "plaintext");
    print_index(16);
    print_bytes(in, 16);
    print_chars((char*)in, 16);
    mbedtls_aes_crypt_cbc(&ctx, ESP_AES_ENCRYPT, 16, iv, in, out);
    // out[16] = '\0';
    ESP_LOGV(TAG, "ciphertext");
    print_index(16);
    print_bytes(out, 16);
    print_chars((char*)out, 16);
}

void al_crypto_decrypt(char* ciphertext,
                       int len,
                       char* plaintext) {
    byte_t iv[16] = "1111111111111111";
    byte_t in[16];
    byte_t out[16];

    for (int i = 0; (i < len) && (i < 16); i++) {
        in[i] = ciphertext[i];
    }
    // in[16] = '\0';
    ESP_LOGV(TAG, "ciphertext:");
    print_bytes(in, 16);
    print_chars((char*)in, 16);
    mbedtls_aes_crypt_cbc(&ctx, ESP_AES_DECRYPT, 16, iv, in, out);
    // out[16] = '\0';
    ESP_LOGV(TAG, "plaintext:");
    print_bytes(out, 16);
    print_chars((char*)out, 16);
}

// void myencrypt(int key, char* msg, char* out) {
//     out = msg;
// }
