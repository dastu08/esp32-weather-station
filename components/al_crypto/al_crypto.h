// APPLICATION LAYER
// Header file of the Crypto component.

#ifndef _AL_CRYPTO_H_
#define _AL_CRYPTO_H_

typedef unsigned char byte_t;

/** Padding of a message with zeros

**Parameters**
    - *message : byte array of the message
    - max_length : total length in memory of the byte array

**Requirements**
    Message musst be null terminated so strlen on it does
    not return an error.

**Description**
    Determine the length of the message in the array. Then
    fill up the remaining space with zeros until the end.
*/
void al_crypto_padding_message(byte_t* message, int max_len);

/** Initialize the al_crypto component

**Requirements**
    A key string of length 64 hex digits must be specified
    in the config.

**Description**
    Get a seed for the random generator and initialize it.
    Convert the key hex string into bytes and initialize
    the mbedtls aes stuff with the key.
*/
void al_crypto_init();

/** Generate an initialization vector (IV)
**Parameters**
    - *iv : byte array to hold the iv in it
    - len : number of bytes of the iv

**Requirements**
    Initialize the random number generator with srand().

**Description**
    Generate len number of bytes (values 0-255) and store
    them in the iv byte array.
*/
void al_crypto_generate_iv(byte_t* iv, int len);

/** Encrypt the plain text with AES-CBC mode
**Parameters**
    - *plaintext : byte array of the plain text
    - len : length of the plaintext in bytes
    - *iv : initilization vector (length 16 bytes)
    - *ciphertext : byte array for the cipher text

**Requirements**
    Component al_cypto must be initialized with 
    `al_crypto_init()`. The iv must be 16 bytes long and
    randomly generated. Use `al_crypto_generate_iv()`.
    Plaintext and ciphertext must have the same length
    and this must be a multiple of 16 bytes. Plaintext must
    also be padded before. Use 
    `al_crypto_padding_message()`.

**Description**
    Copyt the iv because it will be modified during the
    encryption. Encrypt the plaintext in AES-CBC mode with
    the specified initialization vector (IV). The key was 
    set during initialization.
*/
void al_crypto_encrypt(byte_t* plaintext,
                       int len,
                       byte_t* iv,
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