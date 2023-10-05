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
#include "server_api.h"

#define PORT 19302

struct ci
{
    int32_t client_addr;
    int16_t client_port;
    char *chatroom; 
    char *usrname;
};
typedef struct ci client_info;

int main()
{
    int server_sockfd;

    if( (server_sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0 )
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

    // client_info servr_arr[1000];
    // client_info clients_arr[1000];

    if( listen(server_sockfd, 2) < 0 )
    {
        printf("\n Server failed when tried to listen! Exiting ...");
        printf("\n Last error was: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);

    SERV_MSG find_req;
     bzero(&find_req,sizeof(find_req));

    printf("\n Waiting for incoming connections ...");

        
     int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &addr_size);
     recv(client_sockfd,&find_req,sizeof(find_req),0);

     printf("0x%02X",find_req.attributes[7]);
     printf("0x%02X",find_req.attributes[8]);

     

    return 0;
}