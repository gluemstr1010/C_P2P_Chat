#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 19302

struct clientserver
{
    int16_t client_port;
    int32_t client_addr;
    char *chatroom; 
};

struct clientserver_Req
{
    uint16_t message_type;
    uint16_t message_len;
    uint32_t magic_cookie;
    u_int8_t transaction_id[12];
    uint8_t attributes[18];
};

int main()
{
    int server_sockfd;

    if( (server_sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0 )
    {
        printf("\n Socket creation failed! Exiting ...");
        printf("\n Last error was: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);  // Bind at port 19302
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any incoming address

    int conn;

    if( (conn = bind(server_sockfd,(struct sockaddr*)&server_addr, sizeof(server_addr))) < 0 )
    {
        printf("\n Socket binding failed! Exiting ...");
        printf("\n Last error was: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct clientserver_Req request;
    bzero(request.attributes,sizeof(request.attributes));
    request.message_type = htons(0x001);
    
    long size = sizeof(request.transaction_id) / sizeof(request.transaction_id[0]);
    for(int i = 0; i < size; i++)
    {
        request.transaction_id[i] = rand() % 256;
    }

   request.attributes[1] = 0x02;
   int16_t port = 0x317A;
   request.attributes[7] = port & 0xFF;
   request.attributes[8] = (port >> 8) & 0xFF;
   uint32_t ipadd = 0x4E50C3EA;
   uint16_t lowpart = ipadd & 0xFFFF;
   uint16_t highpart = (ipadd >> 16) & 0xFFFF;
   request.attributes[9] = (highpart >> 8) & 0xFF;
   request.attributes[10] = highpart & 0xFF;
   request.attributes[11] = (lowpart >> 8) & 0xFF;
   request.attributes[12] = lowpart & 0xFF;

   u_int16_t Value_size;

   for(int i = 7; i < 13; i++)
   {
      long int s = sizeof(request.attributes[i]);
      Value_size += s;
   }
   
   request.message_len = htons(sizeof(request.attributes));
   request.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;
   struct sockaddr_in destaddr;
   bzero(&destaddr,sizeof(destaddr));
   destaddr.sin_family = AF_INET;
   destaddr.sin_addr.s_addr = inet_addr("10.0.0.138");
   destaddr.sin_port = htons(19302);
   
   for(int i = 0; i < 20; i++)
   {
        conn =  sendto(server_sockfd,&request,sizeof(request),0,(struct sockaddr*)&destaddr, sizeof(destaddr));
   }


    return 0;
}