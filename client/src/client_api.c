#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <gmp.h>
#include "client_api.h"
#include "../../enc/inc/rsa_api.h"

#define PACKETSTOSEND 10
#define REFRESH_INTERVAL 60
#define MAXENCLETLEN 260

void make_find_req(SEND_REQ find_req, int clientfd,char roomname[],char username[], struct sockaddr_in address, int address_len,char* m,char* e)
{
//      time_t t;
//     srand((unsigned)time(&t));

//     find_req.message_type = 0x0001;
//      for (size_t i = 0; i < sizeof(find_req.trasaction_id) / sizeof(find_req.trasaction_id[0]); i++)
//     {
//         find_req.trasaction_id[i] = rand() % 256;
//     }
//     find_req.attributes[1] = 0x01;

//     find_req.attributes[13] = 0x01;
//     find_req.attributes[14] = strlen(roomname);
    
//     char let;
//     int a;

//     for(size_t i = 15; i < 14 + strlen(roomname); i++)
//     {
//         let = roomname[i - 15];
//         a = (int)let;
//         find_req.attributes[i] = a;
//     }

//     find_req.attributes[16 + strlen(roomname)] = strlen(username);
//     for (size_t i = 17 + strlen(roomname); i < 17 + strlen(roomname) + strlen(username); i++)
//     {
//         let = username[i - (17 + strlen(roomname))];
//         a = (int)let;
//         find_req.attributes[i] = a;
//     }

//     u_int16_t Value_size = 0;

//      find_req.message_length = htons(sizeof(find_req.attributes));
//      find_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;

//    int hh = sendto(clientfd,&find_req,sizeof(find_req),MSG_WAITALL,(struct sockaddr*)&address,address_len);
//    if( hh < 0)
//    {
//       printf("\n Last error was: %s",strerror(errno));
//       fflush(stdout);
//    }
    
}

void make_alloc_req(SEND_REQ alloc_req, int clientfd, char roomname[], char username[], int backlog, struct sockaddr_in address, int address_len, char* m,char* e)
{
    bzero(&alloc_req, sizeof(alloc_req));
    // time_t t;
    // srand((unsigned)time(&t));

    alloc_req.message_type = 0x0002;
    alloc_req.xlen_r = strlen(roomname);
    alloc_req.xlen_u = strlen(username);
    //  for (size_t i = 0; i < sizeof(alloc_req.trasaction_id) / sizeof(alloc_req.trasaction_id[0]); i++)
    // {
    //     alloc_req.trasaction_id[i] = rand() % 256;
    // }

    char enc_room[strlen(roomname)][MAXENCLETLEN];
    memset(enc_room, '\0', sizeof(enc_room));

     char enc_usr[strlen(username)][MAXENCLETLEN];
    memset(enc_usr, '\0', sizeof(enc_usr));   

    encrypt(roomname,e,m,enc_room);
    encrypt(username,e,m,enc_usr);     

    alloc_req.backlog = backlog;

     sleep(1);
    sendto(clientfd,&alloc_req,sizeof(alloc_req),MSG_WAITALL,(struct sockaddr*)&address,address_len);
     
}

void* refresh_NAT_entry(void* arg)
{
    struct RefreshThreadParams* params = (struct RefreshThreadParams*)arg;

    params->msg.message_type = 0x0088;
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
	    

	    if((q = sendto(params->refreshfd,&params->msg,sizeof(params->msg),0,(struct sockaddr*)&params->address,params->address_len)) <= 0)
            {
                printf("\n Last error was: %s",strerror(errno));
                fflush(stdout);
            }
            // printf("\n send this much %d",q);
            // fflush(stdout);
            sleep(REFRESH_INTERVAL);
        }
    }
    free(params);
}

void* listen_for_Update(void* arg)
{
    struct UpdateThreadParams* params = (struct UpdateThreadParams*)arg;
    
    socklen_t sz = sizeof(params->address);
     struct timeval recvtimeout;      
    recvtimeout.tv_sec = 0;
    recvtimeout.tv_usec = 100000;

    if(setsockopt( params->refreshfd,SOL_SOCKET,SO_RCVTIMEO,&recvtimeout,sizeof(recvtimeout)) < 0)
    {
                printf("\n Error when setiing timeout, exiting...");
                exit(EXIT_FAILURE);
    }

    while (1)
    {
        recvfrom(params->refreshfd,&params->msg,sizeof(params->msg),0,(struct sockaddr*)&params->address,&sz);
        // if( ( <= 0 )
        // {
        //     printf("\n Last error was: %s",strerror(errno));
        //     fflush(stdout);
        // }

        if(params->msg.message_type == 0x0011)
        {
            for(int i = 0; i < params->arrsize; i++)
            {
                if(params->clients[i].chatroom == NULL)
                {
                    for(int k = 9; k < 13; k++)
                    {
                        params->clients[i].client_addr[k-9] = params->msg.attributes[k];
                    }
                    params->clients[i].client_port = ntohs(*(uint16_t *)(&params->msg.attributes[7]));
                    
                    char a;
                    int roomnamelen = params->msg.attributes[14];
                    char temproomname[ roomnamelen ];
                    bzero(&temproomname,sizeof(temproomname));
                    for(int g = 15; g < 15 + roomnamelen; g++)
                    {
                        a = (char)params->msg.attributes[g];
                        temproomname[ g - 15 ] = a;
                    }

                    int usrnemlen = params->msg.attributes[16 + roomnamelen];
                    char tempusrnem[ usrnemlen ];
                    bzero(&tempusrnem,sizeof(tempusrnem));
                    for(int g = 17 + roomnamelen; g < 17 + usrnemlen + roomnamelen; g++)
                    {
                        a = (char)params->msg.attributes[i];
                        tempusrnem[ g - (17 + roomnamelen) ] = a;
                    }

                    params->clients[i].usrname = (char *)malloc( usrnemlen );
                    params->clients[i].chatroom = (char *)malloc(roomnamelen);
                    strcpy(params->clients[i].chatroom,temproomname);
                    strcpy(params->clients[i].usrname,tempusrnem);
                    
                    // printf("\n%d",params->clients[i].client_addr[0]);
                    // printf("%d",params->clients[i].client_addr[1]);
                    // printf("%d",params->clients[i].client_addr[2]);
                    // printf("%d::",params->clients[i].client_addr[3]);
                    // printf("%d",params->clients[i].client_port);
                    break;
                }
            }
        }
       // if(params->msg.message_type != 0)
       // {
       //     printf("\n 0x%02X - upd",params->msg.message_type);
       //     fflush(stdout);
       // }
        

        bzero(&params->msg,sizeof(params->msg));
        fflush(stdout);
    }
    free(params);
}

void* recv_msg(void* arg)
{
    struct RecvThreadParams* params = (struct RecvThreadParams*)arg;

   while(1)
   {
	 recvfrom(params->clientfd,&params->msg,sizeof(params->msg),0,NULL,NULL);

 //   if(params->msg.message_type != 0)
 //   {
 //           printf("\n 0x%02X - recv",params->msg.message_type);
 //           fflush(stdout);
 //   }

    if(params->msg.message_type == 0x76)
        {
            int msglen = params->msg.attributes[1];
            char let;
            char *mesg = malloc(msglen + 1);
            bzero(mesg,msglen);
            
            for(int i = 2; i < 2 + msglen; i++)
            {
                 let = (char)params->msg.attributes[i];
                 if(let != '\0')
                 {
                    mesg[i-2] = let;
                 }
            }
            printf("\n%s",mesg);
            fflush(stdout);
        }
   }
   free(params);
}

char* convert_IP_tochar(uint8_t ipadd[])
{
    char *ip = malloc(17);
    bzero(ip,17);
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

void* send_msg(void* arg)
{
     struct SendThreadParams* params = (struct SendThreadParams*)arg;
     struct sockaddr_in client;
    while(1)
    {
        bzero(&params->msg,sizeof(params->msg));
        char msg[20];
        printf("\n Send message:");
        fflush(stdout);
        fgets(msg,sizeof(msg),stdin);

        params->msg.message_type = 0x0076;
        for (size_t i = 0; i < sizeof(params->msg.trasaction_id) / sizeof(params->msg.trasaction_id[0]); i++)
        {
            params->msg.trasaction_id[i] = rand() % 256;
        }
        int len = sizeof(msg) / sizeof(msg[0]);
        params->msg.attributes[1] = len;
        char let;
        int a;

        for(size_t i = 2; i < 2 + strlen(msg); i++)
        {
            let = msg[i - 2];
            a = (int)let;
            params->msg.attributes[i] = a;
        }

        for(int i = 0; i < params->arrsize; i++)
        {
	    printf("%d",params->clients[i].client_port);
	    fflush(stdout);
            if(params->clients[i].client_port != 0)
            {
                 bzero(&client,sizeof(client));
                client.sin_family = AF_INET;
                client.sin_port = htons(params->clients[i].client_port);
                inet_pton(AF_INET,convert_IP_tochar(params->clients[i].client_addr),&(client.sin_addr));
                sendto(params->clientfd,&params->msg,sizeof(params->msg),0,(struct sockaddr*)&client,sizeof(client));

                char ipAddress[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN is the maximum size for IPv4 addresses

                // Convert the binary IP address to a string
                if (inet_ntop(AF_INET, &(client.sin_addr), ipAddress, INET_ADDRSTRLEN) == NULL) {
                    perror("Error converting IP address to string");
                }

                
            //     printf("\nIP Address: %s\n", ipAddress);
            //     printf("%u",client.sin_port);

            //     perror("sendto");
            //     printf("\n%d",o);
            //      printf("\n Last error was: %s",strerror(errno));
            //   fflush(stdout);
            }
        }

        
    }
    free(params);
}


void handle_key(SEND_KEY_MSG msg, char* sm, char* se)
{
    
    int16_t e_len = msg.attributes[1];
    for(int i = 2; i < (2 + e_len); i++)
    {
        char c = (char)msg.attributes[i];
        se[i - 2] = c;
        // printf("%c",(char)msg.attributes[i]);
        // fflush(stdout);
    }

    int m_len = msg.attributes[(3 + e_len )];

    for(int i = (4 + e_len); i < 4 + e_len + m_len ; i++ )
    {
        sm[ i - (4 + e_len)] = (char)msg.attributes[i];
        // printf("%c", (char)msg.attributes[i] );
        // fflush(stdout);
    }

    // printf("%d\n",e_len);
    // printf("%d\n",m_len);
    // fflush(stdout);
}
