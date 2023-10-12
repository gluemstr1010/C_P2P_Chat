#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include "server_api.h"

// make hash tables
client_info servers[1000];
client_info clients[1000];


void make_alloc_res(SERV_MSG alloc_req,int client_sockfd)
{
   uint8_t transcid[12];
   memcpy(transcid,alloc_req.trasaction_id,sizeof(alloc_req.trasaction_id));


   int16_t port = ntohs(*(int16_t*)(&alloc_req.attributes[7]));  
   char *chatname = malloc(13);
   char *usrname = malloc(13);

   char let;
   uint8_t temp_add[4];

   int chatnamelen = alloc_req.attributes[14];
   
   
   for( int i = 15; i < 15 + chatnamelen ; i++)
   {
      let = (char)alloc_req.attributes[i];
      chatname[i-15] = let;
   }
   
   int usrnamelen = alloc_req.attributes[16 + chatnamelen];
   
   for(int i = 17 + chatnamelen; i < 17 + chatnamelen + usrnamelen ; i++ )
   {
      let = (char)alloc_req.attributes[i];
      usrname[i-(17+chatnamelen)] = let;
   }

    for(int j = 9; j < 13; j++)
    {
        uint8_t a = alloc_req.attributes[j];
        temp_add[j - 9] = a;
    }
   
   
   for(int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
   {
      int cs = canbe_server(alloc_req,i,chatname,temp_add,port);
      printf("%d\n",cs);
      fflush(stdout);
      if(cs == 1)
      {
         break;
      }    
      memcpy(servers[i].client_addr,temp_add,sizeof(servers[i].client_addr));
      servers[i].client_port = port;
      servers[i].chatroom = chatname;
      servers[i].usrname = usrname;
      
      break; 
   }

   close(client_sockfd);
}

int canbe_server(SERV_MSG alloc_req,int i,char *chatname,uint8_t temp_add[],int16_t port)
{
    
    char *ss = servers[i].chatroom;
        if(ss!=NULL )
        {
            if(strcmp(ss,chatname) == 0)
            {
                printf("\nThis room name is taken");
                return 1;
            }
        }

        int aresameIPs;
        int aresamePorts = 1;
        for(int j = 0; j < 4; j++)
        {
            aresameIPs = 1;
            if(temp_add[j] != servers[i].client_addr[j] )
            {
                return 0;
            }
            if(port == servers[i].client_port)
            {
                aresamePorts = 0;
            }
            aresameIPs = 0;
        }
        
        if(aresameIPs == 0 && aresamePorts == 0)
        {
            return 1;
        }

    return 0;
}