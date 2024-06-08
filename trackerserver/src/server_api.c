#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include <errno.h>
#include <gmp.h>
#include "server_api.h"
#include "../../enc/inc/rsa_api.h"
#include "../../serialization/inc/serialize_api.h"

// make hash tables
server_info servers[10];
// int activeServers = 0;

void make_alloc_res(SEND_REQ alloc_req,int client_sockfd, char* sourceIP, u_int16_t port, struct sockaddr_in address,char* roomname,char* usrname,uint16_t uid)
{

    int backlog = alloc_req.backlog;

    socklen_t addrsiz = sizeof(address);

    SERV_MSG resp;
    memset(&resp,0,sizeof(resp));

    int checkRoom = CheckRoomExistence(roomname);

    if(checkRoom == 0)
    {
        resp.message_type  = 0x0003;
        sendto(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));
    }
    else
    {
         resp.message_type = 0x0004;

        char chruid[12];
        memset(chruid,0,sizeof(chruid));
        sprintf(chruid,"%d",uid);
        strcpy(resp.attribute,chruid);

        sendto(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));

        bzero(&resp,sizeof(resp));

	    bzero(&address,sizeof(address));

        again:
	
	    int k = recvfrom(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,&addrsiz);       

	    if (k < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                goto again;
            } 
        }
    	
        uint16_t refport = ntohs(address.sin_port);   

        CreateRoom(roomname,backlog,1,usrname,sourceIP,port,refport,uid);
    }

 
}


void make_find_res(int client_sockfd,  char* sourceIP, u_int16_t port, struct sockaddr_in address, char* roomname,char* usrname,char* client_e, char* client_m,uint16_t uid)
{
        SERV_MSG resp = {0};

        socklen_t addrsize = sizeof(struct sockaddr_in);

        char tempusrnem[20];
        bzero(&tempusrnem,sizeof(tempusrnem));

        char tempipaddr[17];
        bzero(&tempipaddr,sizeof(tempipaddr));

        const char* pos = GetPeerPos(roomname);

        if(pos == NULL)
        {
            printf("something bad happened");    
        }
        resp.message_type = 0x0004;
        char chruid[12];
        memset(chruid,0,sizeof(chruid));
        sprintf(chruid,"%d",uid);

        strcpy(resp.attribute,chruid);

        again:

        sendto(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));

        sleep(1);

        ReadClients(client_sockfd,pos,client_e,client_m,address);

        int r = recvfrom(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,&addrsize);

        if (r < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                goto again;
            } 
        }
        uint16_t refport = ntohs(address.sin_port);

        WritePeer(roomname,usrname,sourceIP,port,refport,uid);

}

uint8_t* process_srcIP(char* srcIP,uint8_t client_ipadd[])
{
    
    char* token = strtok(srcIP,".");
 
    
    int i = 0;
    while (token != NULL) {
        client_ipadd[i] = atoi(token);
        i++;
        token = strtok(NULL,".");
    }
    free(token);

    return client_ipadd;
}

void broadcast_new_client(int sockfd, u_int16_t port,char *sourceaddr, char *usrname, char *roomname){
    
    const char* pos = GetPeerPos(roomname);

    struct sockaddr_in activeCLient;

     char client_m[270];
    char client_e[12];

    memset(client_m,'\0',sizeof(client_m));
    memset(client_e,'\0',sizeof(client_e));


    int uid = 0;
    char tempipadd[17];
    memset(tempipadd,0,sizeof(tempipadd));
       uint16_t peerport = 0;

    CLIENT_INFO_MSG msg = {0};
    msg.message_type = 0x0011;

    strcpy(msg.client_addr,sourceaddr);
    msg.port = port;

    msg.xlen_r = strlen(roomname);
    msg.xlen_u = strlen(usrname);

    char enc_room[MAXLEN_ROOM][MAXENCLETLEN];
    char enc_usr[MAXLEN_USER][MAXENCLETLEN];
    
     while ((pos = strstr(pos, "\"username\":")) != NULL) {
        sscanf(pos, "\"username\":\"%*[^\"]\";\"peeraddr\":\"%17[^\"]\";\"peerport\":%hd;\"refport\":%*d;\"uid\":%d",
               tempipadd,
               &peerport,
               &uid);

        GetPeerPublicKey(uid,client_e,client_m);

        if(strlen(client_m) > 1 && strlen(client_e) > 1)
        {
           
            memset(enc_room,'\0',sizeof(enc_room));
            memset(enc_usr,'\0',sizeof(enc_usr));

            encrypt(roomname,client_e,client_m,enc_room);
            encrypt(usrname,client_e,client_m,enc_usr);

            for(int i = 0; i < MAXLEN_ROOM; i++ )
            {
                for(int j = 0; j < MAXENCLETLEN; j++)
                {
                    msg.enc_room[i][j] = enc_room[i][j];
                }
            }

            for(int i = 0; i < MAXLEN_USER; i++ )
            {
                for(int j = 0; j < MAXENCLETLEN; j++)
                {
                    msg.enc_usr[i][j] = enc_usr[i][j];
                }
            }

            activeCLient.sin_family = AF_INET;
            activeCLient.sin_port = htons(peerport);
            inet_pton(AF_INET,tempipadd,&(activeCLient.sin_addr));

            sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&activeCLient,sizeof(activeCLient));

        }

        memset(client_m,0,sizeof(client_m));
        memset(client_e,0,sizeof(client_e));
        memset(msg.enc_room,0,sizeof(msg.enc_room));
        memset(msg.enc_usr,0,sizeof(msg.enc_usr));

        pos++;
    }   

}

void send_key(int serv_sockfd,struct sockaddr_in address,char* mod,char* exponent,uint16_t uid)
{
    SEND_KEY_MSG ack;
    bzero(&ack,sizeof(ack));

    ack.message_type = htons(0x1022);

    ack.uid = uid;

    strcpy(ack.attribute1,mod);
    strcpy(ack.attribute2,exponent);

    sendto(serv_sockfd,&ack,sizeof(ack),0,(struct sockaddr*)&address,sizeof(address));

}

char* convert_IP_tochar(uint8_t ipadd[])
{
    char *ip = malloc(17);
    char temp[17];
    char pom[33];
    bzero(&temp,sizeof(temp));
    bzero(&pom,sizeof(pom));

    for (int i = 0; i < 4; i++)
    {
        int a = ipadd[i];
        sprintf(temp,"%d",a);
        
        if(i < 3)
        {
            strcat(temp,".");
        }

        strcat(pom,temp);
    }

    strcpy(ip,pom);
    return ip;
}
