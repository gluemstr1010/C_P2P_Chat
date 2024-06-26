#include <stdint.h>
#include <sys/socket.h>
#include "../../trackerserver/trackerlib/server_api.h"
#ifndef SERIALIZE_API_H
#define SERIALIZE_API_H

struct room
{
    char roomname[16];
    int backlog;
    int activeClients;
};

struct peer
{
    char usrname[16];
    char peer_addr[16];
    uint16_t peer_port;
    uint16_t refresh_port;
};

void CreateRoom(char* roomname,int backlog, int activeClients,char* usrname,char* peer_addr,uint16_t peer_port, uint16_t refresh_port,uint16_t uid);
void WritePeer(char* roomname,char* usrname,char* peer_addr,uint16_t peer_port, uint16_t refresh_port,uint16_t uid);

int CheckKeyExistence(char* uid);
void WriteKeys(uint16_t uid,char* client_modulus,char* client_exponent,char* server_modulus,char* server_exponent,char* private_d,char hash[65]);

int CheckDirExistence(uint16_t uid);
int CheckRoomExistence(char* roomname);

const char* GetPeerPos(char* roomname);
void ReadClients(int sockfd,const char* pos,char* exponent,char* mod,struct sockaddr_in address);
const char* RetrieveClientHash(uint16_t uid,char hash[65]);

void GetPeerPublicKey(int uid,char client_e[12],char client_m[270]);
void GetPrivateKey(uint16_t uid,char private_d[260],char modulus[260]);

// path is uid, writing client modulus and exponent

// SAME WILL BE FOR CLIENT, CLIENT WILL NEED TO SAVE KEYS TO MULTIPLE ROOMS

#endif