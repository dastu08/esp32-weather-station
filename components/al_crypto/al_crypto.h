// APPLICATION LAYER
// Header file of the Crypto component.

#ifndef _AL_CRYPTO_H_
#define _AL_CRYPTO_H_

typedef unsigned char byte_t;

/** Initialize the al_crypto component

**Requirements**
    A key string of length 64 hex digits must be specified
    in the config.

**Description**
    Convert the key hex string into bytes and initialize
    the mbedtls aes stuff with the key.
*/
void al_crypto_init();

/** Encrypt the plain text with AES-CBC mode

**Parameters**
    - *plaintext : byte array of the plain text

**Returns**
    - *ciphertext : byte array of the cipher text

**Requirements**
    Component al_cypto must be initialized with
    `al_crypto_init()`. The length of the plaintext has to
    be less than the maximum buffer length.

**Description**
    Generate an IV. Encrypt the plaintext in AES-CBC mode
    with the specified initialization vector (IV). The key
    was set during initialization. Prepend the IV to the
    ciphertext. Log the ciphertext.
*/
byte_t* al_crypto_encrypt(byte_t* plaintext);

/** Decrypt the cipher text with AES-CBC mode

**Parameters**
    - *ciphertext : byte array of the cipher text
    - length : length of the ciphertext in bytes

**Returns** 
    - *plaintext : byte array for the plain text

**Requirements**
    Component al_cypto must be initialized with
    `al_crypto_init()`. The IV must be 16 bytes long and
    the first block of the cipher text. Ciphertext must be
    a multiple of 16 bytes and not exced the buffer length
    + 16 bytes for IV.

**Description**
    Copy the IV from the ciphertext. Decrypt the ciphertext
    in AES-CBC mode with the specified initialization vector
    (IV). The key was set during initialization.
*/
byte_t* al_crypto_decrypt(byte_t* ciphertext, int length);

/** Log the ciphertext

**Parameters**
    - *ciphertext : byte array of the ciphertext with IV

**Prerequisites**
    The ciphertext must be at least 3 blocks of 16 bytes
    long.

**Description**
    Convert the first 16 byte block (IV) and then the
    first and last 16 byte blocks of the ciphertext into
    a hex string represantation. Then DEBUG log the blocks
    separated by spaces.
*/
void al_crypto_log_ciphertext(byte_t* ciphertext);

#endif