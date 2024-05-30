#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <time.h>
#include <gmp.h>
#include "../inc/rsa_api.h"

// SOMEHOW EDIT THIS, SO IT WILL BE MINE
bool is_prime(mpz_t n, gmp_randstate_t state, int iterations) {
    if (mpz_cmp_ui(n, 2) < 0) return false;
    if (mpz_cmp_ui(n, 2) == 0) return true;
    if (mpz_even_p(n)) return false; // Even numbers > 2 are not prime

    mpz_t n_minus_1, a, x, exp, n_minus_3, tmp;
    mpz_inits(n_minus_1, a, x, exp, n_minus_3, tmp, NULL);

    mpz_sub_ui(n_minus_1, n, 1);
    mpz_sub_ui(n_minus_3, n, 3);

    for (int i = 0; i < iterations; ++i) {
        // Generate a random base 'a' in the range [2, n - 2]
        mpz_urandomm(a, state, n_minus_3); // Avoid 0 and n-1
        mpz_add_ui(a, a, 2); // Adjust to [2, n - 2]

        // Calculate x = a^(n-1) mod n using modular exponentiation
        mpz_powm(x, a, n_minus_1, n);

        // Check if x == 1 or x == n - 1
        if (mpz_cmp_ui(x, 1) != 0 && mpz_cmp(x, n_minus_1) != 0) {
            mpz_clears(n_minus_1, a, x, exp, n_minus_3, tmp, NULL);
            return false; // Definitely composite
        }
    }

    mpz_clears(n_minus_1, a, x, exp, n_minus_3, tmp, NULL);
    return true; // Probably prime
}
// SOMEHOW EDIT THIS, SO IT WILL BE MINE
char* generate_prime(gmp_randstate_t state, int bits) {
    mpz_t rnd;
    mpz_init(rnd);

    while (true) {
        // Generate a random number of the specified bits
        mpz_urandomb(rnd, state, bits);

        // Ensure the number is odd
        mpz_setbit(rnd, 0);

        // Set the most significant bit to ensure the number has 'bits' length
        mpz_setbit(rnd, bits - 1);

        // Check if the number is prime using Miller-Rabin with 20 iterations
        if (is_prime(rnd, state, 20)) {
            break;
        }
    }

    char* random = mpz_get_str(NULL, 10, rnd);

    mpz_clear(rnd);
    return random;
}

char* gcd(char* num1,char* num2)
{
    mpz_t high, low, remainder, numm1,numm2;
    mpz_inits(high, low, remainder,numm1,numm2,NULL);

    mpz_set_str(numm1,num1,10);
    mpz_set_str(numm2,num2,10);

    if(mpz_cmp(numm1,numm2) > 0)
    {
        mpz_set(high,numm1);
        mpz_set(low,numm2);
    }
    else if(num2 > num1)
    {
        mpz_set(high,numm2);
        mpz_set(low,numm1);
    }

    mpz_clear(numm1);
    mpz_clear(numm2);

    while(1)
    {
        mpz_mod(remainder,high,low);
        if(mpz_cmp_ui(remainder,0) == 0)
        {
            break;
        }
        else
        {
            mpz_set(high,low);
            mpz_set(low,remainder);
        }
    }
    char* res = mpz_get_str(NULL,10,low);

    mpz_clear(high);
    mpz_clear(low);
    mpz_clear(remainder);

    return res;
}

char* generate_exponent(gmp_randstate_t state,char* euler)
{
    mpz_t rnd,ccp;
    mpz_init(rnd);
    mpz_init(ccp);

    repet:
    mpz_urandomb(rnd, state, 256);

    char* temp_e = mpz_get_str(NULL,10,rnd);

    char* checkcoprime = gcd(euler,temp_e);
    mpz_set_str(ccp,checkcoprime,10);

    if( mpz_cmp_ui(ccp,1) != 0 )
    {
        goto repet;
    }

    char* e = mpz_get_str(NULL,10,rnd);

    mpz_clear(rnd);
    mpz_clear(ccp);
    return e;
}

char* get_d(char* eul,char* e)
{
    // SOURCE : https://www.extendedeuclideanalgorithm.com/code.php#extnr

    mpz_t a, b, q, r, x1, x2, y1, y2, x, y, pom,conseul;
    mpz_inits(a,b,q,r,x1,x2,y1,y2,x,y,pom,conseul,NULL);

    mpz_set_str(a,eul,10);
    mpz_set_str(conseul,eul,10);
    mpz_set_str(b,e,10);

    mpz_set_ui(x1,1);
    mpz_set_ui(x2,0);

    mpz_set_ui(y1,0);
    mpz_set_ui(y2,1);

    mpz_set_ui(x,1);
    mpz_set_ui(y,0);

    mpz_set_ui(r,1);

    while(mpz_cmp_ui(r,0) > 0)
    {
        mpz_tdiv_qr(q,r, a,b);

        
       

        mpz_mul(pom,q,x2);
        mpz_sub(x,x1,pom);
        // int pom1 = mpz_get_ui(pom);

        // printf("%d ",pom1);

        mpz_mul(pom,q,y2);
        mpz_sub(y,y1,pom);

        // int g1 = mpz_get_si(x);
        // int g2 = mpz_get_si(y);

       
        // printf("%d ",g1);
        // printf("%d\n",g2);

        if(mpz_cmp_ui(r,0) > 0)
        {
            mpz_set(a,b);
            mpz_set(b,r);

            mpz_set(x1,x2);
            mpz_set(x2,x);

            mpz_set(y1,y2);
            mpz_set(y2,y);
        }
        
    }
//   char* xx2 = mpz_get_str(NULL,10,x2);  

    if(mpz_cmp_ui(y2,0) < 0)
    {
        mpz_add(y2,conseul,y2);
    }

    char* yy2 = mpz_get_str(NULL,10,y2);

    mpz_clears(a, b, q, r, x1, x2, y1, y2, x, y, pom,conseul,NULL);

    return yy2;
}

void encrypt(char message[],char* e,char* n, char encrypted_msg[20][280] )
{
    mpz_t pom,chrr, modulus,exponent;
    mpz_init(pom);
    mpz_init(chrr);
    mpz_init(modulus);
    mpz_init(exponent);

    mpz_set_str(modulus,n,10);
    mpz_set_str(exponent,e,10);

    for(size_t i = 0; i < strlen(message);i++)
    {
        int chr = (int)message[i];
        // printf("char:%d\n",chr);
        mpz_set_ui(chrr,chr);
        mpz_powm(pom,chrr,exponent,modulus);
        
        char *pomstr = mpz_get_str(NULL,10,pom);
        strcpy(encrypted_msg[i],pomstr);
        // printf("%s\n",pomstr);
        free(pomstr);
    }
    mpz_clear(pom);
    mpz_clear(chrr);
    mpz_clear(modulus);
    mpz_clear(exponent);
}

void decrypt(char encrypted_msg[20][280],char* dd,char* n,char* decrypted_message, int msglen)
{
    mpz_t pom,chrr,d,modulus;
    mpz_init(pom);
    mpz_init(chrr);
    mpz_init(d);
    mpz_init(modulus);

    mpz_set_str(d,dd,10);
    mpz_set_str(modulus,n,10);

    for(int i = 0; i < msglen; i++)
    {
                char *chr = encrypted_msg[i];
                mpz_set_str(chrr,chr,10);
            mpz_powm(pom,chrr,d,modulus);
            // mpz_mod(pom,pom,modulus);

            char *pomstr = mpz_get_str(NULL,10,pom);

            int temp = atoi(pomstr);
            char getchar = (char)temp;
            decrypted_message[i] = getchar;
             free(pomstr);  
    }

    decrypted_message[msglen] = "\0";
    
    mpz_clear(pom);
    mpz_clear(chrr);
    mpz_clear(modulus);
    mpz_clear(d);
}
