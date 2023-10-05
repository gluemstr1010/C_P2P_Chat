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
#include <time.h>

#define MINLENGTH 12

struct sm
{
    int16_t message_type;
    int16_t message_length;
    int8_t trasaction_id[12];
    int8_t attributes[MINLENGTH];
};

int main(){
    int sockfd;
    struct sockaddr_in client_addr, server_addr;
    time_t t; 
	srand((unsigned)time(&t));


    if( ( sockfd = socket(AF_INET,SOCK_STREAM,0) ) < 0)
    {
        printf("Socket failed lol");
    }

    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(12780);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(19302);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    int conn;

    if( ( conn = connect(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) ) < 0)
    {
        printf("\n Connection to server failed! Exiting ...");
        printf("\n Last error was: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sm find_req;
    bzero(&find_req,sizeof(find_req));

    find_req.message_type = htons(0x0001);
    for(int i = 0; i < sizeof(find_req.trasaction_id) / sizeof(find_req.trasaction_id[0]); i++)
    {
        find_req.trasaction_id[i] = rand() % 256;
    }

    find_req.attributes[1] = 0x02;
    
    int16_t port = 0x317A;
   find_req.attributes[7] = port & 0xFF;
   find_req.attributes[8] = (port >> 8) & 0xFF;
   uint32_t ipadd = 0x4E50C3EA;
   uint16_t lowpart = ipadd & 0xFFFF;
   uint16_t highpart = (ipadd >> 16) & 0xFFFF;
   find_req.attributes[9] = (highpart >> 8) & 0xFF;
   find_req.attributes[10] = highpart & 0xFF;
   find_req.attributes[11] = (lowpart >> 8) & 0xFF;
   find_req.attributes[12] = lowpart & 0xFF;
u_int16_t Value_size;

   for(int i = 7; i < 13; i++)
   {
      long int s = sizeof(find_req.attributes[i]);
      Value_size += s;
   }
   
   find_req.message_length = htons(sizeof(find_req.attributes));
   find_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;

   send(sockfd,&find_req,sizeof(find_req),0);


    return 0;
}