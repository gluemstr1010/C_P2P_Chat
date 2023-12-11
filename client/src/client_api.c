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

void refresh_NAT_entry(CLIENT_MSG msg,int clientfd, struct sockaddr_in address, int address_len)
{
    msg.message_type = 0x88;
    for(int i = 0; i < PACKETSTOSEND; i++)
    {
        if(sendto(clientfd,&msg,sizeof(msg),0,(struct sockaddr*)&address,address_len) < 0)
        {
            printf("\n Last error was: %s",strerror(errno));
            fflush(stdout);
        }
    }
}