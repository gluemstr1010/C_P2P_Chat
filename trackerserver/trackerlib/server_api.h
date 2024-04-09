#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>
#include <gmp.h>

#define SERV_MSG_MINLENGTH 56
#define SEND_KEY_MSG_MINLENGTH 1024 

struct sm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t trasaction_id[12];
    uint8_t attributes[SERV_MSG_MINLENGTH];
};
typedef struct sm SERV_MSG;

struct skm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t attributes[SEND_KEY_MSG_MINLENGTH];
};
typedef struct skm SEND_KEY_MSG;

struct ci
{
    uint8_t client_addr[4];
    uint16_t client_port;
    uint16_t refresh_port;
    char *chatroom; 
    char *usrname;
};
typedef struct ci client_info;

struct si
{
    uint8_t server_addr[4];
    uint16_t server_port;
    uint16_t refresh_port;
    char *chatroom; 
    char *usrname;
    int backlog;
    int activeClients;
    client_info* clients;
};
typedef struct si server_info;

void make_find_res(SERV_MSG find_req, int serv_sockfd,  char* sourceIP, u_int16_t port, struct sockaddr_in address, int address_len);
void broadcast_new_client(int sockfd,u_int16_t port,char *sourceaddr, char *usrname, char *roomname);

void make_alloc_res(SERV_MSG alloc_req,int serv_sockfd, char* sourceIP, u_int16_t port, struct sockaddr_in address, int address_len);
int canbe_server(SERV_MSG alloc_req,int i,char *chatname,uint8_t temp_add[],int16_t port);

void send_key(int serv_sockfd,struct sockaddr_in address, int address_len,gmp_randstate_t state);

void process_req(SERV_MSG req,char *chatname, char *usrname,char let,int chatnamelen);

uint8_t* process_srcIP(char* srcIP,uint8_t client_ipadd[]);
char* convert_IP_tochar(uint8_t ipadd[]);


#endif

