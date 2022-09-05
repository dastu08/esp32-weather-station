// APPLICATION LAYER
// Source file of the Crypto component.

#include "al_crypto.h"

#include "../general/general.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mbedtls/aes.h"
#include "stdio.h"
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
void convert_hex2bytes(char* chars,
                       byte_t* bytes,
                       int length) {
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

/** Padding of a message with zeros

**Parameters**
    - *message : byte array of the message
    - start : starting index for padding
    - stop : stopping indeex for padding (excluded)

**Requirements**
    Message musst be null terminated so strlen on it does
    not return an error.

**Description**
    Determine the length of the message in the array. Then
    fill up the remaining space with zeros until the end.
*/
void message_padding(byte_t* message, int start, int stop) {
    for (size_t i = start; i < stop; ++i) {
        message[i] = '\0';
    }
    ESP_LOGV(TAG,
             "message padding from %d to %d bytes",
             start,
             stop);
}

/** Generate an initialization vector (IV)

**Parameters**
    - *iv : byte array to hold the iv in it
    - len : number of bytes of the iv

**Description**
    Generate len number of bytes (values 0-255) and store
    them in the iv byte array.
*/
void generate_iv(byte_t* iv, int len) {
    for (int i = 0; i < len; ++i) {
        iv[i] = esp_random() % 256;
    }
    ESP_LOGV(TAG, "generated iv with %d bytes", len);
}

void al_crypto_init() {
    byte_t key_bytes[32];

    // read in key string
    convert_hex2bytes(key_string, key_bytes, 32);
    ESP_LOGD(TAG, "key string: %s", key_string);

    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key_bytes, 256);
    // mbedtls_aes_setkey_dec(&ctx, key, 256);

    ESP_LOGI(TAG, "init finished");
}

void al_crypto_encrypt(byte_t* plaintext,
                       byte_t* ciphertext) {
    const int max_length = 128;
    byte_t iv[16];
    byte_t buffer_in[max_length];
    byte_t buffer_out[max_length];

    // length of the plaintext in bytes
    int length = strlen((char*)plaintext);

    ESP_LOGD(TAG,
             "encrypting text of length %d bytes",
             length);

    // check if the plaintext is too long
    if (length >= max_length) {
        ESP_LOGW(TAG,
                 "Cannot encrypt a message of length %d bytes, max length is %d bytes. Aborting!",
                 length,
                 max_length);
        return;
    }

    generate_iv(iv, 16);

    // copy IV into ciphertext
    for (int i = 0; i < 16; ++i) {
        ciphertext[i] = iv[i];
    }
    ESP_LOGV(TAG, "copied iv into ciphertext");

    // copy message into buffer
    for (int i = 0; i < length; ++i) {
        buffer_in[i] = plaintext[i];
    }
    ESP_LOGV(TAG, "copied plaintext into buffer");

    message_padding(buffer_in,
                    length,
                    max_length);

    ESP_LOGV(TAG, "padding of buffer");

    ESP_LOGV(TAG, "buffer_in: %s", buffer_in);

    // encrypt the message
    mbedtls_aes_crypt_cbc(&ctx,
                          ESP_AES_ENCRYPT,
                          max_length,
                          iv,
                          buffer_in,
                          buffer_out);
    ESP_LOGV(TAG, "encrypted buffer");

    // copy encrypted buffer into ciphertext after IV
    for (int i = 0; i < max_length; ++i) {
        ciphertext[16 + i] = buffer_out[i];
    }
    // null terminate the string
    ciphertext[max_length + 16] = '\0';

    int cipher_length = strlen((char*)ciphertext);
    ESP_LOGV(TAG,
             "ciphertext length: %d bytes, %d words",
             cipher_length,
             cipher_length / 16);

    al_crypto_log_ciphertext(ciphertext);
}

void al_crypto_decrypt(byte_t* ciphertext,
                       byte_t* plaintext) {
    const int max_length = 128;
    byte_t iv[16];
    byte_t buffer_in[max_length];
    byte_t buffer_out[max_length];

    // length of the ciphertext in bytes
    int length = strlen((char*)ciphertext);

    ESP_LOGD(TAG,
             "decrypting text of length %d bytes",
             length);

    // check if the plaintext is too long
    if (length >= max_length) {
        ESP_LOGW(TAG,
                 "Cannot decrypt a message of length %d. Aborting!",
                 length);
        return;
    }

    // read the IV from ciphertext
    for (int i = 0; i < 16; ++i) {
        iv[i] = ciphertext[i];
    }
    ESP_LOGV(TAG, "read iv from ciphertext");

    // copy ciphertext into buffer
    for (int i = 16; i < length; ++i) {
        buffer_in[i - 16] = ciphertext[i];
    }
    ESP_LOGV(TAG,
             "copied ciphertext into buffer %d bytes",
             strlen((char*)buffer_in));

    // decrypt the message
    mbedtls_aes_crypt_cbc(&ctx,
                          ESP_AES_DECRYPT,
                          max_length,
                          iv,
                          buffer_in,
                          buffer_out);
    ESP_LOGV(TAG, "decrypted buffer");

    // copy the buffer into plaintext
    for (int i = 0; i < max_length; i++) {
        plaintext[i] = buffer_out[i];
    }
    // null terminate the string
    plaintext[max_length] = '\0';

    ESP_LOGV(TAG,
             "plaintext length: %d bytes",
             strlen((char*)plaintext));
}

void al_crypto_log_ciphertext(byte_t* ciphertext) {
    byte_t byte;
    char buffer[3];
    char chars1[33];
    char chars2[33];
    char chars3[33];
    int length = strlen((char*)ciphertext);

    // IV
    for (int i = 0; i < 16; ++i) {
        byte = ciphertext[i];
        sprintf(buffer, "%02x", byte);
        chars1[2 * i] = buffer[0];
        chars1[2 * i + 1] = buffer[1];
    }
    chars1[32] = '\0';

    // first 16 bytes of the ciphertext
    for (int i = 0; i < 16; ++i) {
        byte = ciphertext[i + 16];
        sprintf(buffer, "%02x", byte);
        chars2[2 * i] = buffer[0];
        chars2[2 * i + 1] = buffer[1];
    }
    chars2[32] = '\0';

    // last 16 bytes of the ciphertext
    for (int i = 0; i < 16; ++i) {
        byte = ciphertext[length - 16 + i];
        sprintf(buffer, "%02x", byte);
        chars3[2 * i] = buffer[0];
        chars3[2 * i + 1] = buffer[1];
    }
    chars3[32] = '\0';

    ESP_LOGD(TAG,
             "ciphertext: %s %s ... %s",
             chars1,
             chars2,
             chars3);
}
