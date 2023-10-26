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

struct si
{
    uint8_t server_addr[4];
    int16_t server_port;
    char *chatroom; 
    char *usrname;
    int backlog;
    int activeClients;
    client_info* clients;
};
typedef struct si server_info;

void make_find_res(SERV_MSG find_req, int client_sockfd);
int does_client_exist(SERV_MSG find_req,int i,int k,uint8_t temp_add[],int16_t port);

void make_alloc_res(SERV_MSG alloc_req,int client_sockfd);
int canbe_server(SERV_MSG alloc_req,int i,char *chatname,uint8_t temp_add[],int16_t port);

void process_req(SERV_MSG req,char *chatname, char *usrname,char let,uint8_t temp_add[] ,int chatnamelen);


#endif

