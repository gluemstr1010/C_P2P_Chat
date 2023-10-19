#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include "server_api.h"

// make hash tables
server_info servers[1000];

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
   int usrnamelen = alloc_req.attributes[16 + chatnamelen];

   int backlog = alloc_req.attributes[19 + chatnamelen + usrnamelen];

   process_req(alloc_req,chatname,usrname,let,temp_add,chatnamelen);

   
   for(int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
   {
      int cs = canbe_server(alloc_req,i,chatname,temp_add,port);
      
      fflush(stdout);
      if(cs == 1)
      {
         break;
      }    
  
      for(int j = 0; j < 4; j++)
      {
         servers[i].server_addr[j] = temp_add[j];
      }
      servers[i].server_port = port;
      servers[i].chatroom = chatname;
      servers[i].usrname = usrname;
      servers[i].clients = (struct ci*)malloc(backlog * sizeof(struct ci));;

      
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
            if(temp_add[j] != servers[i].server_addr[j] )
            {
                return 0;
            }
            if(port == servers[i].server_port)
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

void make_find_res(SERV_MSG find_req,int client_sockfd)
{
  uint8_t transcid[12];
   memcpy(transcid,find_req.trasaction_id,sizeof(find_req.trasaction_id));

   int16_t server_port = ntohs(*(int16_t*)(&find_req.attributes[7]));  
   char *chatname = malloc(13);
   char *client_usrname = malloc(13);

   char let;
   uint8_t server_add[4];
   uint8_t client_add[4];

   int chatnamelen = find_req.attributes[14];
   int usrnamelen = find_req.attributes[16 + chatnamelen];

   process_req(find_req,chatname,client_usrname,let,server_add,chatnamelen);

   int16_t client_port = ntohs(*(int16_t*)(&find_req.attributes[19 + chatnamelen + usrnamelen]));
   
   for(int i = 21 + chatnamelen + usrnamelen; i < 25 + chatnamelen + usrnamelen; i++)
   {
      uint8_t addrindx = find_req.attributes[i];
      client_add[i - (21 + chatnamelen + usrnamelen)] = addrindx;
   }

   for(int i = 0; i < sizeof(servers->clients) / sizeof(servers->clients[0]); i++)
   {
      int dce = does_client_exist(find_req,i,client_add,client_port);

      printf("%d\n",dce);
      fflush(stdout);
      if(dce == 1)
      {
         break;
      }    

      
      break; 
   }
}

int does_client_exist(SERV_MSG find_req,int i,uint8_t temp_add[],int16_t port)
{
        int aresameIPs;
        int aresamePorts = 1;
        for(int j = 0; j < 4; j++)
        {
            aresameIPs = 1;
            printf("%d",servers[i].clients[j].client_port);
            
            if(temp_add[j] != servers[i].clients[i].client_addr[j] )
            {
                return 0;
            }
            if(port == servers[i].clients[i].client_port)
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

void process_req(SERV_MSG req,char *chatname, char *usrname,char let,uint8_t temp_add[] ,int chatnamelen )
{
     for( int i = 15; i < 15 + chatnamelen ; i++)
   {
      let = (char)req.attributes[i];
      chatname[i- (15)] = let;
   }
   
   int usrnamelen = req.attributes[16 + chatnamelen];
   
   for(int i = 17 + chatnamelen; i < 17 + chatnamelen + usrnamelen ; i++ )
   {
      let = (char)req.attributes[i];
      usrname[i- (17 + chatnamelen)] = let;
   }

    for(int j = 9; j < 13; j++)
    {
        uint8_t a = req.attributes[j];
        temp_add[j - (9)] = a;
    }
}