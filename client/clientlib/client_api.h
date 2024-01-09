#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>

#define MINLENGTH 56

struct cm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t trasaction_id[12];
    uint8_t attributes[MINLENGTH];
};
typedef struct cm CLIENT_MSG;

struct si
{
    uint8_t client_addr[4];
    uint16_t client_port;
    char *chatroom; 
    char *usrname;
};
typedef struct si CLIENT;

struct RefreshThreadParams {
    CLIENT_MSG msg;
    int clientfd;
    struct sockaddr_in address;
    int address_len;
};

void make_find_req(CLIENT_MSG find_req, int clientfd,char roomname[],char username[], struct sockaddr_in address, int address_len );
void make_alloc_req(CLIENT_MSG alloc_req, int clientfd, char roomname[], char username[],int backlog, struct sockaddr_in address, int address_len);

void refresh_NAT_entry(void* arg);

void prcess_find_resp(CLIENT_MSG find_res);

#endif