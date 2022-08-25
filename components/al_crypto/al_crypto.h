// APPLICATION LAYER
// Header file of the Crypto component.

#ifndef _AL_CRYPTO_H_
#define _AL_CRYPTO_H_

typedef unsigned char byte_t;

void al_crypto_log_ciphertext(byte_t* ciphertext);

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
    - *ciphertext : byte array of the cipher text

**Requirements**
    Component al_cypto must be initialized with
    `al_crypto_init()`. The allocated length of the 
    ciphertext must be strlen(plaintext) + 17.

**Description**
    Generate and IV. Encrypt the plaintext in AES-CBC mode 
    with the specified initialization vector (IV). The key 
    was set during initialization.
*/
void al_crypto_encrypt(byte_t* plaintext,
                       byte_t* ciphertext);

/* Decrypt the cipher text with AES-CBC mode
**Parameters**
    - *ciphertext : byte array of the cipher text
    - len : length of the ciphertext in bytes
    - *iv : initilization vector (length 16 bytes)
    - *plaintext : byte array for the plain text

**Requirements**
    Component al_cypto must be initialized with
    `al_crypto_init()`. The iv must be 16 bytes long and
    received with the cipher text. Plaintext and ciphertext
    must have the same length and this must be a multiple of
    16 bytes.

**Description**
    Copyt the iv because it will be modified during the
    encryption. Decrypt the ciphertext in AES-CBC mode with
    the specified initialization vector (IV). The key was
    set during initialization.
*/
void al_crypto_decrypt(byte_t* ciphertext,
                       int len,
                       byte_t* iv,
                       byte_t* plaintext);

#endif