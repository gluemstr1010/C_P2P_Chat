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
#include <sys/time.h>
#include <pthread.h>
#include "client_api.h"


void process_err_resp(CLIENT_MSG resp)
{
        int l = resp.attributes[3];
        char *ms = malloc(l);
        char let;
        for (int i = 4; i < 4 + l; i++)
        {
            let = (char)resp.attributes[i];
            ms[i - (4)] = let;
        }
        printf("%s\n", ms);
        fflush(stdout);
        free(ms);
}

void client_stage(CLIENT_MSG resp , CLIENT *clients, int arrclientlen ,int clientfd,char roomname[] , int roomname_len,struct sockaddr_in address, socklen_t addrsize )
{

    int backlog = resp.attributes[14];
    struct timeval recvtimeout;      
    recvtimeout.tv_sec = 5;
    recvtimeout.tv_usec = 0;

    if(setsockopt( clientfd,SOL_SOCKET,SO_RCVTIMEO,&recvtimeout,sizeof(recvtimeout)) < 0)
    {
                printf("\n Error when setiing timeout, exiting...");
                exit(EXIT_FAILURE);
    }

    uint16_t p;

    if(resp.message_type == 0x06)
    {
        process_err_resp(resp);
    }else if(resp.message_type == 0x05)
    {
        
        if(resp.attributes[1] == 0x55)
        {
            clients[0].client_addr[0] = resp.attributes[9];
            clients[0].client_addr[1] = resp.attributes[10];
            clients[0].client_addr[2] = resp.attributes[11];
            clients[0].client_addr[3] = resp.attributes[12];

             p = ntohs(*(uint16_t *)(&resp.attributes[7]));
            clients[0].client_port = p;
            
            char a;
            int usrnemlen = resp.attributes[16];
            char tempusrnem[ usrnemlen ];
            for(int i = 17; i < 16 + usrnemlen; i++)
            {
                  a = (char)resp.attributes[i];
                  tempusrnem[ i - 17 ] = a;
            }
            
            clients[0].usrname = (char *)malloc( usrnemlen );
            clients[0].chatroom = (char *)malloc(roomname_len);
            strcpy(clients[0].usrname,tempusrnem);
            strcpy(clients[0].chatroom,roomname);
            
            int i = 1;
             while(true)
             {
                if(i == backlog-2)
                {
                    break;
                }

                bzero(&resp,sizeof(resp));
                if( recvfrom(clientfd,&resp,sizeof(resp),0,(struct sockaddr*)&address, &addrsize) < 0)
                {
                    if(errno == EAGAIN)
                    {
                        break;
                    }else
                    {
                         printf("\n Error when receiving...");
                        printf("\n Last error was: %s",strerror(errno));
                        exit(EXIT_FAILURE); 
                    }
                   
                }

                printf("\n%d - cport",clients[i].client_port);
                printf("\n%d - i",i);                    
                if(clients[i].client_port == 0 )
                {
                    
                    clients[i].client_addr[0] = resp.attributes[9];
                    clients[i].client_addr[1] = resp.attributes[10];
                    clients[i].client_addr[2] = resp.attributes[11];
                    clients[i].client_addr[3] = resp.attributes[12];

                     p = ntohs(*(uint16_t *)(&resp.attributes[7]));
                    clients[i].client_port = p;

                    printf("\n ip bgn - %d",resp.attributes[9]);
                    printf("%d",resp.attributes[10]);
                    printf("%d",resp.attributes[11]);
                    printf("%d::",resp.attributes[12]);
                    printf("%d",p);
                    fflush(stdout);

                    char a;
                    int clientusrnemlen = resp.attributes[16];
                    char tempclientusrnem[ clientusrnemlen ];
                    for(int i = 17; i < 16 + clientusrnemlen; i++)
                    {
                        a = (char)resp.attributes[i];
                        tempclientusrnem[ i - 17 ] = a;
                    }
                    printf("\n%s",tempclientusrnem);
                    
                    clients[i].usrname = (char *)malloc( usrnemlen );
                    clients[i].chatroom = (char *)malloc(roomname_len);
                    strcpy(clients[i].usrname,tempclientusrnem);
                    strcpy(clients[i].chatroom,roomname);
                }

                i++;

             }

            
            

            // clients = (CLIENT *)malloc(resp.attributes[14] * sizeof(CLIENT));
            // clients[0]->client_port = ntohs(*(int16_t *)(&resp.attributes[7]));

            


            // repeat:
            // while(true)
            // {
            //     bzero(&resp,sizeof(resp));
            //     if( recvfrom(clientfd,&resp,sizeof(resp),0,(struct sockaddr*)&address, &addrsize) < 0)
            //     {
            //         break;
            //     }
            //     else
            //     {
            //         printf("\n");
            //         printf("%d",resp.attributes[9]);
            //         printf("%d",resp.attributes[10]);
            //         printf("%d",resp.attributes[11]);
            //         printf("%d",resp.attributes[12]);
            //         goto repeat;
            //     }
            // }
            
        }
    }
}

// void roomserver_stage(CLIENT_MSG resp ,int clientfd, struct sockaddr_in address, int address_len)
// {
 

//     if(resp.message_type == 0x04)
//     {
//         process_err_resp(resp);
//         exit(EXIT_FAILURE);
//     }else if(resp.message_type != 0x03)
//     {
//         // 
//          exit(EXIT_FAILURE);
//     }
// }

void life_cycle(int sockfd,struct sockaddr_in server_addr, CLIENT clients[], int bcklog)
{
     pthread_t UpdateThread;
    CLIENT_MSG update;
    bzero(&update,sizeof(update));
    int len = sizeof(server_addr);

    struct UpdateThreadParams ut = {
        .msg = update,
        .clientfd = sockfd,
        .address = server_addr,
        .address_len = len,
        .clients = clients,
        .arrsize = bcklog
    };

    pthread_t refreshNatEntryThread;
    CLIENT_MSG msg;
    bzero(&msg,sizeof(msg));

    struct RefreshThreadParams rtp = {
        .msg = msg,
        .clientfd = sockfd,
        .address = server_addr,
        .address_len = len
    };

    pthread_t sendThread;
    CLIENT_MSG messageHolder;
     bzero(&messageHolder,sizeof(messageHolder));
    struct SendThreadParams stp = {
        .msg = messageHolder,
        .clientfd = sockfd,
        .clients = clients,
        .arrsize = bcklog
    };

    if (pthread_create(&UpdateThread, NULL,&listen_for_Update,(void*)&ut) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&refreshNatEntryThread, NULL,&refresh_NAT_entry,(void*)&rtp) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

     if (pthread_create(&sendThread, NULL,&send_msg,(void*)&stp) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(UpdateThread, NULL) != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(refreshNatEntryThread, NULL) != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(sendThread, NULL) != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }

}

int main()
{

    int sockfd;
    struct sockaddr_in client_addr, server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Socket failed lol");
    }

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(19452);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(21504);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // inet_pton(AF_INET,"78.80.195.234",&(server_addr.sin_addr));    

    socklen_t addrsize;

    int conn;

     printf("Enter 1 for client, 2 for creating room:");
    short choice;
    scanf("%hd",&choice);

    fgetc(stdin);

    printf("Enter room name:");
    char roomname[13];
    fgets(roomname,sizeof(roomname),stdin);

    printf("\nEnter username name:");
    char username[13];
    fgets(username,sizeof(username),stdin);

    CLIENT_MSG req;
    CLIENT_MSG resp;
    bzero(&req,sizeof(req));
    bzero(&resp,sizeof(resp));
    int len = sizeof(server_addr);

    

    int pom;    

    if(choice == 1)
    {
        make_find_req(req,sockfd,roomname,username,server_addr,len);
         conn = recvfrom(sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&server_addr,&addrsize);
         int bcklg = resp.attributes[14];
         pom = bcklg;
         CLIENT clients[bcklg];
         bzero(&clients,sizeof(clients));     
         client_stage(resp,clients,bcklg,sockfd,roomname,strlen(roomname),server_addr,addrsize);
         life_cycle(sockfd,server_addr,clients,bcklg);

        //  for(int i = 0; i < bcklg -1 ; i++)
        //  {
            
        //         printf("\n%s",clients[i].usrname);
        //         printf("\n%d",clients[i].client_addr[0]);
        //         printf("%d",clients[i].client_addr[1]);
        //         printf("%d",clients[i].client_addr[2]);
        //         printf("%d::",clients[i].client_addr[3]);
        //         printf("%d",clients[i].client_port);
        //         printf("\n");
        //         fflush(stdout);
            
        //  }
    }
    else if(choice == 2)
    {
        CLIENT clients[10];
        make_alloc_req(req,sockfd,roomname,username,10, server_addr,len);
        conn = recvfrom(sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&server_addr,&addrsize);
        life_cycle(sockfd,server_addr,clients,10);
    }else
    {
        exit(EXIT_FAILURE);
    }

    

    // struct sm find_req;
    // bzero(&find_req, sizeof(find_req));

    // find_req.message_type = 0x01;
    // for (int i = 0; i < sizeof(find_req.trasaction_id) / sizeof(find_req.trasaction_id[0]); i++)
    // {
    //     find_req.trasaction_id[i] = rand() % 256;
    // }

    // find_req.attributes[1] = 0x01;

    // int16_t port = 0x317A;
    // find_req.attributes[7] = (port >> 8) & 0xFF;
    // find_req.attributes[8] = port & 0xFF;
    // uint32_t ipadd = 0x4E50C3EA;
    // uint16_t lowpart = ipadd & 0xFFFF;
    // uint16_t highpart = (ipadd >> 16) & 0xFFFF;
    // find_req.attributes[9] = (highpart >> 8) & 0xFF;
    // find_req.attributes[10] = highpart & 0xFF;
    // find_req.attributes[11] = (lowpart >> 8) & 0xFF;
    // find_req.attributes[12] = lowpart & 0xFF;
    // find_req.attributes[13] = 0x02;
    // char *chatname = "Valve";
    // find_req.attributes[14] = strlen(chatname);
    // char let;
    // int a;
    // for (int i = 15; i < 15 + strlen(chatname); i++)
    // {
    //     let = chatname[i - 15];
    //     a = (int)let;
    //     find_req.attributes[i] = a;
    // }
    // char *usrname = "Franta";
    // find_req.attributes[16 + strlen(chatname)] = strlen(usrname);
    // for (int i = 17 + strlen(chatname); i < 17 + strlen(chatname) + strlen(usrname); i++)
//     {
//         let = usrname[i - (17 + strlen(chatname))];
//         a = (int)let;
//         find_req.attributes[i] = a;
//     }
//     //    find_req.attributes[18 + strlen(chatname) + strlen(usrname)] = 0x11;
//     //    find_req.attributes[19 + strlen(chatname) + strlen(usrname)] = 0xA;

//     u_int16_t Value_size = 0;

//        find_req.message_length = htons(sizeof(find_req.attributes));
//        find_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;

//     send(sockfd, &find_req, sizeof(find_req), 0);

//     struct sm response;
//     recv(sockfd, &response, sizeof(response), 0);

//     if (response.message_type == 0x03)
//     {
//         int16_t myport = ntohs(*(int16_t *)(&response.attributes[7]));
//         printf("%d", myport);
//     }
//     if (response.message_type == 0x08)
//     {
//         char *ms = malloc(15);
//         int l = response.attributes[7];
//         for (int i = 8; i < 8 + l; i++)
//         {
//             let = (char)response.attributes[i];
//             ms[i - (8)] = let;
//         }
//         printf("%s\n", ms);
//         free(ms);
//     }
    
    
    close(sockfd);

    return 0;
}