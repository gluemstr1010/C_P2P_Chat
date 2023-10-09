#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>

#define MINLENGTH 56

struct sm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t trasaction_id[12];
    uint8_t attributes[MINLENGTH];
};
typedef struct sm SERV_MSG;

struct ci
{
    uint8_t client_addr[4];
    int16_t client_port;
    char *chatroom; 
    char *usrname;
};
typedef struct ci client_info;

void make_find_res(SERV_MSG find_req);

void make_alloc_res(SERV_MSG alloc_req);
int canbe_server(client_info info,SERV_MSG alloc_req);


#endif

