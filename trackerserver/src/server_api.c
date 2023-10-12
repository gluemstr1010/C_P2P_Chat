#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <uchar.h>
#include "server_api.h"

// make hash tables
client_info servers[1000];
client_info clients[1000];


void make_alloc_res(SERV_MSG alloc_req)
{
   uint8_t transcid[12];
   memcpy(transcid,alloc_req.trasaction_id,sizeof(alloc_req.trasaction_id));


   int16_t port = ntohs(*(int16_t*)(&alloc_req.attributes[7]));  

   int chatnamelen = alloc_req.attributes[14];
   char *chatname = malloc(13);
   char let;
   for( int i = 15; i < 15 + chatnamelen ; i++)
   {
      let = (char)alloc_req.attributes[i];
      chatname[i-15] = let;
   }
   char *usrname = malloc(13);
   int usrnamelen = alloc_req.attributes[16 + chatnamelen];
   
   for(int i = 17 + chatnamelen; i < 17 + chatnamelen + usrnamelen ; i++ )
   {
      let = (char)alloc_req.attributes[i];
      usrname[i-(17+chatnamelen)] = let;
      printf("%c",let);
   }
   

   
//    for(int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
//    {
//         if( servers->client_port == 0 )
//         {
//                 for(int j = 9; j < 13; j++)
//                 {
//                     uint8_t a = alloc_req.attributes[j];
//                     servers[i].client_addr[j - 9] = a;
//                 }
       
//                 break;
//         }
//    }


}

int canbe_server(client_info info, SERV_MSG alloc_req)
{
    for(int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
    {
        if( servers->client_port == 0 )
        {
            for(int j = 9; j < 13; j++)
            {
                        uint8_t a = alloc_req.attributes[j];
                        servers[i].client_addr[j - 9] = a;
            }
        
            break;
        }
    }
}