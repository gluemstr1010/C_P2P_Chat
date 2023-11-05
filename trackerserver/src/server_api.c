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
int activeServers = 0;

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
      if(servers[i].server_port == 0)
      {
        for(int j = 0; j < 4; j++)
        {
         servers[i].server_addr[j] = temp_add[j];
        }
        servers[i].server_port = port;
        servers[i].chatroom = chatname;
        servers[i].usrname = usrname;
        servers[i].backlog = backlog;
        servers[i].activeClients = 0;
        servers[i].clients = (struct ci*)malloc(backlog * sizeof(struct ci));;

        activeServers++;
        
        break; 
      }    
  
      
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
                printf("\n This room name is taken");
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

   int16_t client_port = ntohs(*(int16_t*)(&find_req.attributes[7]));  
   char *chatname = malloc(13);
   char *client_usrname = malloc(13);

   char let;
   uint8_t client_add[4];
   
   uint8_t server_add[4];
   int16_t server_port;

   int chatnamelen = find_req.attributes[14];
   int usrnamelen = find_req.attributes[16 + chatnamelen];

   process_req(find_req,chatname,client_usrname,let,client_add,chatnamelen);
   
   for(int i = 0; i < activeServers ; i++)
   {
        if(strcmp(servers[i].chatroom,chatname) == 0)
        {
              for(int j = 0; j <= servers[i].activeClients ; j++)
              {
                    int ds = does_client_exist(find_req,i,j,client_add,client_port);
                    
                    if(ds != 0)
                    {
                        printf("Client is already connected");
                        goto jump;
                    }

                    if( servers[i].clients[j].client_port == 0 )
                    {
                        for(int k = 0; k < 4; k++)
                        {
                            servers[i].clients[j].client_addr[k] = client_add[k];
                            server_add[k] = servers[i].server_addr[k];
                        }
                        server_port = servers[i].server_port;


                        servers[i].clients[j].client_port = client_port;
                        servers[i].clients[j].chatroom = chatname;
                        servers[i].clients[j].usrname = client_usrname;
                        servers[i].activeClients++;

                        printf("0x%02X",servers[i].clients[j].client_port);
                        fflush(stdout);
                        
                        break;
                    }
              }
        }      
   }
   SERV_MSG response;
   response.message_type = 0x03;
   memcpy(response.trasaction_id,transcid,sizeof(response.trasaction_id));
   response.attributes[1] = 0x02;
   response.attributes[7] = (server_port >> 8) & 0xFF;
   response.attributes[8] = server_port & 0xFF;
   response.attributes[9] =  server_add[1];
   response.attributes[10] = server_add[0];
   response.attributes[11] = server_add[3];
   response.attributes[12] = server_add[2];

   u_int16_t Value_size = 0;
   
   find_req.message_length = htons(sizeof(find_req.attributes));
   find_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;

   send(client_sockfd,&response,sizeof(response),0);

   close(client_sockfd);
   

   jump:
        SERV_MSG err_resp;
        err_resp.message_type = 0x04;
        memcpy(err_resp.trasaction_id,transcid,sizeof(err_resp.trasaction_id));
        err_resp.attributes[1] = 0x08;
        char err_msg[15] = "address_exists";
        err_resp.attributes[7] = strlen(chatname);
        int a;
        for(int i = 8 ; i < 8 + strlen(chatname); i++ )
        {
            let = err_msg[i - 8];
            a = (int)let;
            err_resp.attributes[i] = a;
        }
        close(client_sockfd);
}

int does_client_exist(SERV_MSG find_req,int i,int k ,uint8_t temp_add[],int16_t port)
{
        int aresameIPs;
        int aresamePorts = 1;
        for(int j = 0; j < 4; j++)
        {
            aresameIPs = 1;
            
            if(temp_add[j] != servers[i].clients[k].client_addr[j] )
            {
                return 0;
            }
            if(port == servers[i].clients[k].client_port)
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