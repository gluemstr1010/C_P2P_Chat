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
    uint16_t client_port;
    char *chatroom; 
    char *usrname;
};
typedef struct ci client_info;

struct si
{
    uint8_t server_addr[4];
    uint16_t server_port;
    char *chatroom; 
    char *usrname;
    int backlog;
    int activeClients;
    client_info* clients;
};
typedef struct si server_info;

void make_find_res(SERV_MSG find_req, int serv_sockfd,  char* sourceIP, u_int16_t port, struct sockaddr_in address, int address_len);

void make_alloc_res(SERV_MSG alloc_req,int serv_sockfd, char* sourceIP, u_int16_t port, struct sockaddr_in address, int address_len);
int canbe_server(SERV_MSG alloc_req,int i,char *chatname,uint8_t temp_add[],int16_t port);

void sendtoclientserver(int servsockfd ,uint16_t port,uint8_t server_add[],struct sockaddr_in address, int address_len);
void process_req(SERV_MSG req,char *chatname, char *usrname,char let,int chatnamelen);

uint8_t* process_srcIP(char* srcIP,uint8_t client_ipadd[]);


#endif

