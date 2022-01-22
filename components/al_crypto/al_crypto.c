// APPLICATION LAYER
// Source file of the Crypto component.

#include "al_crypto.h"

#include "mbedtls/aes.h"

mbedtls_aes_context ctx;

void mycryptoinit() {
    // nothing

    mbedtls_aes_init(&ctx);
}

// void myencrypt(int key, char* msg, char* out) {
//     out = msg;
// }
