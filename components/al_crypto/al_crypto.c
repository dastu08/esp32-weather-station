// APPLICATION LAYER
// Source file of the Crypto component.

#include "al_crypto.h"

#include "../general/general.h"
#include "esp_log.h"
#include "mbedtls/aes.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static const char* TAG = "al_crypto";

mbedtls_aes_context ctx;

void print_index(int length) {
    // print the index number above
    for (int i = 0; i < length; i++) {
        printf("%2d ", i);
    }
    printf("\n");
}

void print_bytes(byte_t* bytes, int length) {
    byte_t byte;
    for (int i = 0; i < length; i++) {
        byte = bytes[i];
        if (byte < 0x10) {
            printf("0%X ", byte);
        } else {
            printf("%X ", byte);
        }
    }
    printf("\n");
}

void print_chars(char* chars, int length) {
    for (int i = 0; i < length; i++) {
        printf(" %c ", chars[i]);
    }
    printf("\n");
}

void copy_bytes(byte_t* old, byte_t* new, int len) {
    for (int i = 0; i < len; ++i) {
        new[i] = old[i];
    }
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

void padding_message(byte_t* message, int max_len) {
    size_t len = strlen((char*)message);
    for (size_t i = len; i < max_len; ++i) {
        message[i] = 0x0;
    }
}

void al_crypto_init() {
    char key_hex[65] = "9018321f988274f6a4eaf29c82df2614a296f9c06ca5776a893e1d0c9e35e1f9";
    unsigned char key[32];
    // print_chars(key_hex, 64);
    convert_hex2bytes(key_hex, key, 32);
    ESP_LOGV(TAG, "key");
    print_index(32);
    print_bytes(key, 32);
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key, 256);
    // mbedtls_aes_setkey_dec(&ctx, key, 256);

    ESP_LOGI(TAG, "init finished");
}

void al_crypto_encrypt(byte_t* plaintext,
                       int len,
                       byte_t* iv,
                       byte_t* ciphertext) {
    ESP_LOGD(TAG, "encrypting");
    byte_t iv2[16];
    copy_bytes(iv, iv2, 16);
    // byte_t in[len];
    // byte_t out[len];

    ESP_LOGV(TAG, "iv");
    print_index(16);
    print_bytes(iv, 16);
    print_chars((char*)iv, 16);

    // convert the char to byte_t
    // for (int i = 0; (i < len); i++) {
    //     in[i] = plaintext[i];
    // }
    // in[16] = '\0';
    ESP_LOGV(TAG, "plaintext");
    print_index(len);
    print_bytes(plaintext, len);
    print_chars((char*)plaintext, len);
    mbedtls_aes_crypt_cbc(&ctx, ESP_AES_ENCRYPT, len, iv2, plaintext, ciphertext);
    // out[16] = '\0';
    ESP_LOGV(TAG, "iv");
    print_index(16);
    print_bytes(iv, 16);
    print_chars((char*)iv, 16);

    ESP_LOGV(TAG, "ciphertext");
    print_index(len);
    print_bytes(ciphertext, len);
    print_chars((char*)ciphertext, len);
}

void al_crypto_decrypt(byte_t* ciphertext,
                       int len,
                       byte_t* iv,
                       byte_t* plaintext) {
    ESP_LOGD(TAG, "decrypting");
    byte_t iv2[16];
    copy_bytes(iv, iv2, 16);
    // byte_t in[16];
    // byte_t out[16];
    ESP_LOGV(TAG, "iv");
    print_index(16);
    print_bytes(iv, 16);
    print_chars((char*)iv, 16);

    // for (int i = 0; (i < len) && (i < 16); i++) {
    //     in[i] = ciphertext[i];
    // }
    // in[16] = '\0';
    ESP_LOGV(TAG, "ciphertext:");
    print_index(len);
    print_bytes(ciphertext, len);
    print_chars((char*)ciphertext, len);
    mbedtls_aes_crypt_cbc(&ctx, ESP_AES_DECRYPT, len, iv2, ciphertext, plaintext);
    // out[16] = '\0';
    ESP_LOGV(TAG, "plaintext:");
    print_index(len);
    print_bytes(plaintext, len);
    print_chars((char*)plaintext, len);
}

// void myencrypt(int key, char* msg, char* out) {
//     out = msg;
// }
