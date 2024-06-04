#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <openssl/sha.h>
#include "server_api.h"
#include "../../enc/inc/rsa_api.h"
#include "../../serialization/inc/serialize_api.h"
// #include "../../encryption/rsa/rsa.c"

#define PORT 21504

void createShaHash(const char *str, unsigned char outputBuffer[64])
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str, strlen(str));
    SHA256_Final(outputBuffer, &sha256);
}


void hashToHexString(const unsigned char *hash, char hexString[65]) {
    for (int i = 0; i < 32; i++) {
        sprintf(hexString + (i * 2), "%02x", hash[i]);
    }
    hexString[64] = '\0';  // Null-terminate the string
}

int main()
{
    int server_sockfd;

    if( (server_sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0 )
    {
        printf("\n Socket creation failed! Exiting ...");
        printf("\n Last error was: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);  // Bind at port 21504
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any incoming address

    int conn;

    if( (conn = bind(server_sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr))) < 0 )
    {
        printf("\n Socket binding failed! Exiting ...");
        printf("\n Last error was: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    
    // printf("\n Waiting for incoming connections ...");
    fflush(stdout);
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);
    gmp_randstate_t state;
    gmp_randinit_default(state);

    SEND_REQ send_req = {0};

    SEND_KEY_MSG init = {0};

    u_int16_t port;
    char sourceip[16];
    while(true)
    {
        gmp_randseed_ui(state, time(NULL));
        srand(time(NULL));
         bzero(&init,sizeof(init));
        //  init.attribute1 = (char*)calloc(strlen(270), sizeof(char));
        //  init.attribute2 = (char*)calloc(strlen(6), sizeof(char));
         bzero(&client_addr,sizeof(client_addr));
         bzero(&port,sizeof(port));
         bzero(&sourceip,sizeof(sourceip));

        conn = recvfrom(server_sockfd,&init ,sizeof(init),0,(struct sockaddr*)&client_addr,&addr_size);
        
        if(conn > 0)
        {
            port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &(client_addr.sin_addr),sourceip, INET_ADDRSTRLEN);
        char temp[16];
        bzero(&temp,sizeof(temp));
        strcpy(temp,sourceip);
        // char let;
        
        // printf("0x%02X\n",req.message_type);
        // fflush(stdout);

        if(init.message_type == 0x2200)
        {    
            char* client_modulus = (char*)calloc(270, sizeof(char));
            char* client_exponent = (char*)calloc(6, sizeof(char));   

            strcpy(client_modulus,init.attribute1);
            strcpy(client_exponent,init.attribute2);         

            int buffer_size = snprintf(NULL, 0, "\"modulus\":\n\t\"%s\"\n\"exponent\":\n\t\"%s\"\n", client_modulus,client_exponent) + 1;
    
            // Allocate memory for the final string
            char *formatted_string = (char*)calloc(buffer_size, sizeof(char));
            if (!formatted_string) {
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }

            // Format the string with the variables
            sprintf(formatted_string, "\"modulus\":\n\t\"%s\"\n\"exponent\":\n\t\"%s\"\n", client_modulus, client_exponent);
            unsigned char shaHash[64];
            char getHash[65];
            // bzero(&shaHash,sizeof(shaHash));
            memset(shaHash,0,sizeof(shaHash));

            createShaHash(formatted_string,shaHash);
            hashToHexString(shaHash, getHash);        // GETTING THE HASH

            uint16_t uid = (unsigned long long)rand() * (unsigned long long)RAND_MAX + rand();

            char* p = generate_prime(state,428);
            char* q = generate_prime(state,428); // generate primes
        
            mpz_t n,pp,qq, eul;
            mpz_init(n);
            mpz_init(pp);
            mpz_init(qq);
            mpz_init(eul);

            mpz_set_str(pp,p,10);
            mpz_set_str(qq,q,10);

            mpz_mul(n,pp,qq);

            char* mod = mpz_get_str(NULL,10,n); // get modulus

            mpz_sub_ui(pp,pp,1);
            mpz_sub_ui(qq,qq,1);

            mpz_mul(eul,pp,qq);

            char* euler = mpz_get_str(NULL,10,eul); // get phi of n

            char* exponent = malloc(6);
            strcpy(exponent,"65537"); // exponent

            char* pd = get_d(euler,exponent); // private d

            // // // printf("p:%s\n",p);
            // // // printf("q:%s\n",q);
            // // // printf("phi:%s\n",euler);
            // // // printf("mod:%s\n",mod);
            // // printf("private d:%s\n",pd);

            send_key(server_sockfd,client_addr,mod,exponent,uid);

            recvfrom(server_sockfd,&send_req ,sizeof(send_req),0,(struct sockaddr*)&client_addr,&addr_size); 
            // listen for request

            uint8_t roomnamelen = send_req.xlen_r;
            uint8_t usrnamelen = send_req.xlen_u;
            
            char* roomname = (char*)calloc(roomnamelen+1, sizeof(char));
            char* usrname = (char*)calloc(usrnamelen+1, sizeof(char));

            decrypt(send_req.enc_room,pd,mod,roomname,roomnamelen);
            decrypt(send_req.enc_usr,pd,mod,usrname,usrnamelen);

            // printf("0x%02X",send_req.message_type);
            int checkRoom = 0;
            if(send_req.message_type == 0x02)
            {
                  make_alloc_res(send_req,server_sockfd,sourceip,port,client_addr,roomname,usrname,uid,getHash);
            }
            if(send_req.message_type == 0x01)
            {   
                SERV_MSG resp = {0};
                checkRoom = CheckRoomExistence(roomname);
                if(checkRoom == 1)
                {
                    resp.message_type = 0x0006;
                    
                    sendto(server_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&client_addr,sizeof(client_addr));
                }
                else
                {
                    make_find_res(server_sockfd,sourceip,port,client_addr,roomname,usrname,client_exponent,client_modulus,uid,getHash);
                    broadcast_new_client(server_sockfd,port,temp,usrname,roomname);
                }
                
            }
            if(checkRoom != 1)
            {
                 WriteKeys(uid,client_modulus,client_exponent,mod,exponent,pd); // WRITE KEYS TO FILES
            }

           

            free(client_modulus);
            free(client_exponent);

            // printf("%s",mod);
            // fflush(stdout);          

            mpz_clears(n,eul,pp,qq,NULL);
        }
        }
        
        
    }
    
    return 0;
}

// SERV_MSG req;
    // while(true)
    // {
    //     bzero(&req,sizeof(req));
    //     printf("\n Waiting for incoming connections ...");
    //     int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &addr_size);
    //     recv(client_sockfd,&req,sizeof(req),0);
    //     if(req.message_type == 0x01)
    //     {
    //         make_find_res(req);
    //     }
    //     if(req.message_type == 0x02)
    //     {
    //         make_alloc_res(req);
    //     }
    // }
