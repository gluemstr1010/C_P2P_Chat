#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include <time.h>
#include <curl/curl.h>
#include <errno.h>
#include "client_api.h"

#define PACKETSTOSEND 10
#define REFRESH_INTERVAL 60

void make_find_req(CLIENT_MSG find_req, int clientfd,char roomname[],char username[], struct sockaddr_in address, int address_len)
{
    bzero(&find_req, sizeof(find_req));
     time_t t;
    srand((unsigned)time(&t));

    find_req.message_type = 0x01;
     for (int i = 0; i < sizeof(find_req.trasaction_id) / sizeof(find_req.trasaction_id[0]); i++)
    {
        find_req.trasaction_id[i] = rand() % 256;
    }
    find_req.attributes[1] = 0x01;

    find_req.attributes[13] = 0x01;
    find_req.attributes[14] = strlen(roomname);
    
    char let;
    int a;

    for(int i = 15; i < 14 + strlen(roomname); i++)
    {
        let = roomname[i - 15];
        a = (int)let;
        find_req.attributes[i] = a;
    }

    find_req.attributes[16 + strlen(roomname)] = strlen(username);
    for (int i = 17 + strlen(roomname); i < 17 + strlen(roomname) + strlen(username); i++)
    {
        let = username[i - (17 + strlen(roomname))];
        a = (int)let;
        find_req.attributes[i] = a;
    }

    u_int16_t Value_size = 0;

     find_req.message_length = htons(sizeof(find_req.attributes));
     find_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;

   int hh = sendto(clientfd,&find_req,sizeof(find_req),MSG_WAITALL,(struct sockaddr*)&address,address_len);
   if( hh < 0)
   {
      printf("\n Last error was: %s",strerror(errno));
      fflush(stdout);
   }
    
}

void make_alloc_req(CLIENT_MSG alloc_req, int clientfd, char roomname[], char username[], int backlog, struct sockaddr_in address, int address_len)
{
    bzero(&alloc_req, sizeof(alloc_req));
    time_t t;
    srand((unsigned)time(&t));

    alloc_req.message_type = 0x02;
     for (int i = 0; i < sizeof(alloc_req.trasaction_id) / sizeof(alloc_req.trasaction_id[0]); i++)
    {
        alloc_req.trasaction_id[i] = rand() % 256;
    }
    alloc_req.attributes[1] = 0x02;

    alloc_req.attributes[13] = 0x02;
    alloc_req.attributes[14] = strlen(roomname);
    
    char let;
    int a;

    for(int i = 15; i < 14 + strlen(roomname); i++)
    {
        let = roomname[i - 15];
        a = (int)let;
        alloc_req.attributes[i] = a;
    }

    alloc_req.attributes[16 + strlen(roomname)] = strlen(username);
    for (int i = 17 + strlen(roomname); i < 17 + strlen(roomname) + strlen(username); i++)
    {
        let = username[i - (17 + strlen(roomname))];
        a = (int)let;
        alloc_req.attributes[i] = a;
    }
       alloc_req.attributes[18 + strlen(roomname) + strlen(username)] = 0x11;
       alloc_req.attributes[19 + strlen(roomname) + strlen(username)] = backlog;

       u_int16_t Value_size = 0;

       alloc_req.message_length = htons(sizeof(alloc_req.attributes));
       alloc_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;
    

      sendto(clientfd,&alloc_req,sizeof(alloc_req),MSG_WAITALL,(struct sockaddr*)&address,address_len);
}

void* refresh_NAT_entry(void* arg)
{
    struct RefreshThreadParams* params = (struct RefreshThreadParams*)arg;

    params->msg.message_type = 0x88;
    while(1)
    {
        for(int i = 0; i < PACKETSTOSEND; i++)
        {
            int q;
            if((q = sendto(params->clientfd,&params->msg,sizeof(params->msg),0,(struct sockaddr*)&params->address,params->address_len)) <= 0)
            {
                printf("\n Last error was: %s",strerror(errno));
                fflush(stdout);
            }
            printf("send this much %d\n",q);
            fflush(stdout);
            sleep(REFRESH_INTERVAL);
        }
    }
}

void* listen_for_Update(void* arg)
{
    struct UpdateThreadParams* params = (struct UpdateThreadParams*)arg;
    int j;
    socklen_t sz = sizeof(params->address);
     struct timeval recvtimeout;      
    recvtimeout.tv_sec = 0;
    recvtimeout.tv_usec = 100000;

    if(setsockopt( params->clientfd,SOL_SOCKET,SO_RCVTIMEO,&recvtimeout,sizeof(recvtimeout)) < 0)
    {
                printf("\n Error when setiing timeout, exiting...");
                exit(EXIT_FAILURE);
    }

    while (1)
    {
        j = recvfrom(params->clientfd,&params->msg,sizeof(params->msg),0,(struct sockaddr*)&params->address,&sz);
        // if( ( <= 0 )
        // {
        //     printf("\n Last error was: %s",strerror(errno));
        //     fflush(stdout);
        // }

        if(params->msg.message_type == 0x11)
        {
            for(int i = 0; i < params->arrsize; i++)
            {
                if(params->clients[i].chatroom == NULL)
                {
                    for(int k = 9; k < 12; k++)
                    {
                        params->clients[i].client_addr[k-9] = params->msg.attributes[k];
                    }
                    params->clients[i].client_port = ntohs(*(uint16_t *)(&params->msg.attributes[7]));
                    
                    char a;
                    int roomnamelen = params->msg.attributes[14];
                    char temproomname[ roomnamelen ];
                    bzero(&temproomname,sizeof(temproomname));
                    for(int i = 15; i < 15 + roomnamelen; i++)
                    {
                        a = (char)params->msg.attributes[i];
                        temproomname[ i - 15 ] = a;
                    }

                    int usrnemlen = params->msg.attributes[16 + roomnamelen];
                    char tempusrnem[ usrnemlen ];
                    bzero(&tempusrnem,sizeof(tempusrnem));
                    for(int i = 17 + roomnamelen; i < 17 + usrnemlen + roomnamelen; i++)
                    {
                        a = (char)params->msg.attributes[i];
                        tempusrnem[ i - (17 + roomnamelen) ] = a;
                    }

                    params->clients[i].usrname = (char *)malloc( usrnemlen );
                    params->clients[i].chatroom = (char *)malloc(roomnamelen);
                    strcpy(params->clients[i].chatroom,temproomname);
                    strcpy(params->clients[i].usrname,tempusrnem);
                   
                    break;
                }
            }
        }

        if(params->msg.message_type == 0x76)
        {
            int msglen = params->msg.attributes[1];
            char let;
            char mesg[msglen];
            bzero(&mesg,sizeof(msglen));
            for(int i = 2; i < 2 + msglen; i++)
            {
                 let = (char)params->msg.attributes[i];
                 mesg[i] = let;
            }
            printf("\n%s",mesg);
        }

        bzero(&params->msg,sizeof(params->msg));
        fflush(stdout);
    }
    
}

char* convert_IP_tochar(uint8_t ipadd[])
{
    char *ip = malloc(17);
    char temp[17];
    char pom[33];
    bzero(&temp,sizeof(temp));
    bzero(&pom,sizeof(pom));


    char let;
    int k = 0;
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

void* send_msg(void* arg)
{
     struct SendThreadParams* params = (struct UpdateThreadParams*)arg;
     struct sockaddr_in client;
    while(1)
    {
        bzero(&params->msg,sizeof(params->msg));
        char msg[20];
        printf("\n Send message:");
        fflush(stdout);
        fgets(msg,sizeof(msg),stdin);

        params->msg.message_type = 0x76;
        for (int i = 0; i < sizeof(params->msg.trasaction_id) / sizeof(params->msg.trasaction_id[0]); i++)
        {
            params->msg.trasaction_id[i] = rand() % 256;
        }
        params->msg.attributes[1] = strlen(msg);
        char let;
        int a;

        for(int i = 2; i < 2 + strlen(msg); i++)
        {
            let = msg[i - 2];
            a = (int)let;
            params->msg.attributes[i] = a;
        }

        for(int i = 0; i < params->arrsize; i++)
        {
            if(params->clients[i].chatroom != NULL)
            {
                 bzero(&client,sizeof(client));
                client.sin_family = AF_INET;
                client.sin_port = htons(params->clients[i].client_port);
                inet_pton(AF_INET,convert_IP_tochar(params->clients[i].client_addr),&(client.sin_addr));
                sendto(params->clientfd,&params->msg,sizeof(params->msg),0,(struct sockaddr*)&client,sizeof(client));
                perror("sendto");
            }
        }

        fgetc(stdin);
    }
}