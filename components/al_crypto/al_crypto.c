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

// key string from config
char key_string[65] = CONFIG_AES_256_KEY;

// aes context needed for init
mbedtls_aes_context ctx;

/** Print the numbers 0 to length with spaces separated

**Parameters**
    - length : up to which number is counted (length-1)

**Description**
    Outputs a nicely spaces index of decimal numbers with
    2 digits.
*/
void print_index(int length) {
    printf("index: ");
    // print the index number above
    for (int i = 0; i < length; i++) {
        printf("%2d ", i);
    }
    printf("\n");
}

/** Print bytes in hex format

**Parameters**
    - *bytes : array of bytes
    - length : length of the byte array

**Description**
    Outputs a nicely spaces hex numbers of the bytes.
*/
void print_bytes(byte_t* bytes, int length) {
    byte_t byte;
    printf("bytes: ");
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

/** Print char string

**Parameters**
    - *chars : array of chars
    - length : length of the char array

**Description**
    Outputs a nicely spaces chars.
*/
void print_chars(char* chars, int length) {
    printf("chars: ");
    for (int i = 0; i < length; i++) {
        printf(" %c ", chars[i]);
    }
    printf("\n");
}

/** Copy a byte array

**Parameters**
    - *old : byte array containing the data
    - *new : byte array where the data is copied into
    - len : length of byte arrays

**Description**
    In a loop copy the values from one byte array to the
    other one. Only the first len bytes are copied regard
    less of the length of the arrays.
*/
void copy_bytes(byte_t* old, byte_t* new, int len) {
    for (int i = 0; i < len; ++i) {
        new[i] = old[i];
    }
}

// convert the hex string in chars into length bytes
// 2 hex digits make one byte

/** Convert a hex string of chars into an array of bytes

**Parameters**
    - *chars : hex string that will be converted
    - *bytes : array of bytes for the converted data
    - length :
        length of the byte array, half length of the char
        array

**Requirements**
    Char array must have an even number of hex digits.

**Description**
    Read in the hex string and convert two consecutive
    hex digits into one byte.
*/
void convert_hex2bytes(char* chars, byte_t* bytes, int length) {
    char buff[5];
    buff[0] = '0';
    buff[1] = 'x';
    buff[4] = '\0';
    for (int i = 0; i < length; i++) {
        buff[2] = chars[2 * i];
        buff[3] = chars[2 * i + 1];
        bytes[i] = strtol(buff, NULL, 16);
        // printf("i = %d : ", i);
        // printf("buff: %c%c%c%c, value: 0x%X\n",
        //    buff[0], buff[1], buff[2], buff[3], bytes[i]);
        // print_chars(buff, 4);
    }
}

void al_crypto_padding_message(byte_t* message, int max_len) {
    size_t len = strlen((char*)message);
    for (size_t i = len; i < max_len; ++i) {
        message[i] = 0x0;
    }
}

void al_crypto_init() {
    byte_t key_bytes[32];
    int seed = (int)get_seed();

    convert_hex2bytes(key_string, key_bytes, 32);
    ESP_LOGV(TAG, "key string: %s", key_string);
    ESP_LOGV(TAG, "key");
    print_index(32);
    print_bytes(key_bytes, 32);

    ESP_LOGV(TAG, "seed: %d", seed);
    srand(seed);

    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key_bytes, 256);
    // mbedtls_aes_setkey_dec(&ctx, key, 256);

    ESP_LOGI(TAG, "init finished");
}

void al_crypto_generate_iv(byte_t* iv, int len) {
    for (int i = 0; i < len; ++i) {
        iv[i] = rand() % 256;
    }
}

void al_crypto_encrypt(byte_t* plaintext,
                       int len,
                       byte_t* iv,
                       byte_t* ciphertext) {
    ESP_LOGD(TAG, "encrypting");
    byte_t iv2[16];
    // need to create a copy to prevent destruction.
    copy_bytes(iv, iv2, 16);

    ESP_LOGV(TAG, "iv");
    print_index(16);
    print_bytes(iv, 16);
    print_chars((char*)iv, 16);

    ESP_LOGV(TAG, "plaintext");
    print_index(len);
    print_bytes(plaintext, len);
    print_chars((char*)plaintext, len);

    mbedtls_aes_crypt_cbc(&ctx,
                          ESP_AES_ENCRYPT,
                          len,
                          iv2,
                          plaintext,
                          ciphertext);

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
    // need to create a copy to prevent destruction.
    copy_bytes(iv, iv2, 16);

    ESP_LOGV(TAG, "iv");
    print_index(16);
    print_bytes(iv, 16);
    print_chars((char*)iv, 16);

    ESP_LOGV(TAG, "ciphertext:");
    print_index(len);
    print_bytes(ciphertext, len);
    print_chars((char*)ciphertext, len);

    mbedtls_aes_crypt_cbc(&ctx,
                          ESP_AES_DECRYPT,
                          len,
                          iv2,
                          ciphertext,
                          plaintext);

    ESP_LOGV(TAG, "plaintext:");
    print_index(len);
    print_bytes(plaintext, len);
    print_chars((char*)plaintext, len);
}

