// APPLICATION LAYER
// Header file of the Crypto component.

#ifndef _AL_CRYPTO_H_
#define _AL_CRYPTO_H_

void al_crypto_init();
void al_crypto_encrypt(int key, char* plaintext, int len, char* ciphertext);
// void myencrypt(int key, char* msg, char* out);

#endif