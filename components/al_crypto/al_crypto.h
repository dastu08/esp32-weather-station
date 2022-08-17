// APPLICATION LAYER
// Header file of the Crypto component.

#ifndef _AL_CRYPTO_H_
#define _AL_CRYPTO_H_

typedef unsigned char byte_t;

void padding_message(byte_t* message, int max_len);

void al_crypto_init();
void al_crypto_encrypt(byte_t* plaintext,
                       int len,
                       byte_t* iv,
                       byte_t* ciphertext);
void al_crypto_decrypt(byte_t* ciphertext,
                       int len,
                       byte_t* iv,
                       byte_t* plaintext);
// void myencrypt(int key, char* msg, char* out);

#endif