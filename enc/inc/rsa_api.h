#include <gmp.h>

#ifndef RSA_API_H
#define RSA_API_H

char* generate_prime(gmp_randstate_t state, int bits);
char* get_d(char* eul,char* e);

void encrypt(char message[],char* e,char* n,char encrypted_msg[20][280]);
void decrypt(char encrypted_msg[20][280],char* dd,char* mod,char* decrypted_msg, int msglen);

#endif