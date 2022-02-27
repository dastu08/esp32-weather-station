// APPLICATION LAYER
// Header file of the Crypto component.

#ifndef _AL_CRYPTO_H_
#define _AL_CRYPTO_H_

void al_crypto_init();
void al_crypto_encrypt(char* plaintext,
                       int len,
                       char* ciphertext);
void al_crypto_decrypt(char* ciphertext,
                       int len,
                       char* plaintext);
// void myencrypt(int key, char* msg, char* out);

#endif