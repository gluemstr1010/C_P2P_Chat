#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>

#define MINLENGTH 56
#define SEND_KEY_MSG_MINLENGTH 270
#define MAXLEN_ROOM 20
#define MAXLEN_USER 20
#define MAXENCLETLEN 260

struct cm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t trasaction_id[12];
    uint8_t attributes[MINLENGTH];
};
typedef struct cm CLIENT_MSG;

struct skm
{
    int16_t message_type;
    int16_t message_length;
    uint16_t attributes[SEND_KEY_MSG_MINLENGTH];
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
    int refreshfd;
    struct sockaddr_in address;
    int address_len;
};

struct UpdateThreadParams {
    CLIENT_MSG msg;
    int refreshfd;
    struct sockaddr_in address;
    int address_len;
    CLIENT *clients;
    int arrsize;
};

struct RecvThreadParams{
    CLIENT_MSG msg;
    int clientfd;
    struct sockaddr_in address;
    int address_len;
};

struct SendThreadParams{
    CLIENT_MSG msg;
    int clientfd;
    CLIENT *clients;
    int arrsize;
};

void make_find_req(SEND_REQ find_req, int clientfd,char roomname[],char username[], struct sockaddr_in address, int address_len,char* m,char* e );
void make_alloc_req(SEND_REQ alloc_req, int clientfd, char roomname[], char username[],int backlog, struct sockaddr_in address, int address_len,char* m,char* e);

void* refresh_NAT_entry(void* arg);
void* listen_for_Update(void* arg);
void* recv_msg(void* arg);
void* send_msg(void* arg);

void handle_key(SEND_KEY_MSG msg, char* server_mod, char* server_e);

void prcess_find_resp(CLIENT_MSG find_res);

char* convert_IP_tochar(uint8_t ipadd[]);

#endif
