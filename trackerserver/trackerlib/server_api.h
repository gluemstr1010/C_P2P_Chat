#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>
#include <gmp.h>

#define SERV_MSG_MINLENGTH 56
#define SEND_KEY_MSG_MINLENGTH 270
#define MAXLEN_ROOM 20
#define MAXLEN_USER 20
#define MAXENCLETLEN 280

struct sm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t trasaction_id[12];
    uint8_t attributes[SERV_MSG_MINLENGTH];
};
typedef struct sm SERV_MSG;

struct cim
{
    int16_t message_type;
    uint16_t port;
    char client_addr[17];
    uint8_t xlen_u;
    char enc_usr[MAXLEN_ROOM][MAXENCLETLEN];
    uint8_t xlen_r;
    char enc_room[MAXLEN_ROOM][MAXENCLETLEN];
};
typedef struct cim CLIENT_INFO_MSG;

// struct skm
// {
//     int16_t message_type;
//     int16_t message_length;
//     uint16_t attributes[SEND_KEY_MSG_MINLENGTH];
// };
// typedef struct skm SEND_KEY_MSG;

// struct skm
// {
//     int16_t message_type;
//     uint8_t exponent_length;
//     char* exponent;
//     uint16_t modulus_length;
//     char* modulus;
// };
// typedef struct skm SEND_KEY_MSG;

struct skm
{
    int16_t message_type;
    uint16_t uid;
    char attribute1[270]; // modulus or or hash
    char attribute2[6]; // exponent
};
typedef struct skm SEND_KEY_MSG;

struct sr
{
    int16_t message_type;
    uint8_t xlen_r;
    uint8_t xlen_u;
    char enc_room[MAXLEN_ROOM][MAXENCLETLEN];
    char enc_usr[MAXLEN_ROOM][MAXENCLETLEN];
    uint8_t backlog;
};
typedef struct sr SEND_REQ;

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

void make_find_res(int serv_sockfd,  char* sourceIP, u_int16_t port, struct sockaddr_in address, char* roomname,char* usrname,char* e, char* m,uint16_t uid,char hash[65],SERV_MSG resp);
void broadcast_new_client(int sockfd,u_int16_t port,char *sourceaddr, char *usrname, char *roomname);

void make_alloc_res(SEND_REQ alloc_req,int serv_sockfd, char* sourceIP, u_int16_t port, struct sockaddr_in address,char* roomname,char* usrname,uint16_t uid,char hash[65]);
int canbe_server(int i,char *chatname,uint8_t temp_add[],int16_t port);

void send_key(int serv_sockfd,struct sockaddr_in address,char* mod,char* exponent,uint16_t uid);

void process_req(SERV_MSG req,char *chatname, char *usrname,int chatnamelen, int usrnemlen);

uint8_t* process_srcIP(char* srcIP,uint8_t client_ipadd[]);
char* convert_IP_tochar(uint8_t ipadd[]);


#endif

