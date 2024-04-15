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
#include <time.h>
#include <pthread.h>
#include "client_api.h"


void process_err_resp(CLIENT_MSG resp)
{
        int l = resp.attributes[3];
        char *ms = malloc(l);
        char let = 'l';
        for (int i = 4; i < 4 + l; i++)
        {
            let = (char)resp.attributes[i];
            ms[i - (4)] = let;
        }
        printf("%s\n", ms);
        fflush(stdout);
        free(ms);
}

void handle_key(SEND_KEY_MSG msg)
{
    for(size_t i = 0; i < 100; i++ )
    {
        printf("%d",msg.attributes[i]);
        fflush(stdout);
    }

    // for(int i = 2; i < (2 + e_len); i++)
    // {
    //     int c = (char)msg.attributes[i];
    //     printf("%d",c);
    //     fflush(stdout);
    // }



    // for(int i = (4 + e_len); i < e_len + m_len ; i++ )
    // {
    //     sm[(4 + e_len)] = (char)msg.attributes[i];
    // }
   
    // printf("%s\n",se);
    // // printf("%s\n",sm);
    // fflush(stdout);
}

void client_stage(CLIENT_MSG resp , CLIENT *clients ,int clientfd,char roomname[] , int roomname_len,struct sockaddr_in address, socklen_t addrsize )
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

    if(resp.message_type == 0x0006)
    {
        process_err_resp(resp);
    }else if(resp.message_type == 0x0005)
    {
        char a = 'a';
        if(resp.attributes[1] == 0x55)
        {
            clients[0].client_addr[0] = resp.attributes[9];
            clients[0].client_addr[1] = resp.attributes[10];
            clients[0].client_addr[2] = resp.attributes[11];
            clients[0].client_addr[3] = resp.attributes[12];

             p = ntohs(*(uint16_t *)(&resp.attributes[7]));
            clients[0].client_port = p;
            
            
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
                fflush(stdout);      
                if(clients[i].client_port == 0 )
                {
                    
                    clients[i].client_addr[0] = resp.attributes[9];
                    clients[i].client_addr[1] = resp.attributes[10];
                    clients[i].client_addr[2] = resp.attributes[11];
                    clients[i].client_addr[3] = resp.attributes[12];

                     p = ntohs(*(uint16_t *)(&resp.attributes[7]));
                    clients[i].client_port = p;

                    // printf("\n ip bgn - %d",resp.attributes[9]);
                    // printf("%d",resp.attributes[10]);
                    // printf("%d",resp.attributes[11]);
                    // printf("%d::",resp.attributes[12]);
                    // printf("%d",p);
                    // fflush(stdout);

                    
                    int clientusrnemlen = resp.attributes[16];
                    char tempclientusrnem[ clientusrnemlen ];
                    for(int u = 17; u < 16 + clientusrnemlen; u++)
                    {
                        a = (char)resp.attributes[u];
                        tempclientusrnem[ u - 17 ] = a;
                    }
                    printf("\n%s",tempclientusrnem);
                    
                    clients[i].usrname = (char *)malloc( usrnemlen );
                    clients[i].chatroom = (char *)malloc(roomname_len);
                    strcpy(clients[i].usrname,tempclientusrnem);
                    strcpy(clients[i].chatroom,roomname);
                }

                i++;

             }
                        
        }
    }
}

void life_cycle(int sockfd,int refreshsock,struct sockaddr_in server_addr, CLIENT clients[], int bcklog)
{
     pthread_t UpdateThread;
    CLIENT_MSG update = {0};
    bzero(&update,sizeof(update));
    int len = sizeof(server_addr);

    struct UpdateThreadParams ut = {
        .msg = update,
        .refreshfd = refreshsock,
        .address = server_addr,
        .address_len = len,
        .clients = clients,
        .arrsize = bcklog
    };

    // pthread_t refreshNatEntryThread;
    CLIENT_MSG msg;
    bzero(&msg,sizeof(msg));

    // struct RefreshThreadParams rtp = {
    //     .msg = msg,
    //     .clientfd = sockfd,
	// .refreshfd = refreshsock,
    //     .address = server_addr,
    //     .address_len = len
    // };

    pthread_t recvThread;
    
    struct RecvThreadParams rcvtp = {
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

  //  if (pthread_create(&refreshNatEntryThread, NULL,&refresh_NAT_entry,(void*)&rtp) != 0) {
  //      perror("Thread creation failed");
  //      exit(EXIT_FAILURE);
  //  }

     if (pthread_create(&recvThread, NULL,&recv_msg,(void*)&rcvtp) != 0) {
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

   // if (pthread_join(refreshNatEntryThread, NULL) != 0) {
   //     perror("Thread join failed");
   //     exit(EXIT_FAILURE);
   // }

    if (pthread_join(recvThread, NULL) != 0) {
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

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int refreshsock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in client_addr, refresh_addr, server_addr;

     struct timeval reftimeout;      
    reftimeout.tv_sec = 15;
    reftimeout.tv_usec = 0;

    if (sockfd < 0)
    {
        printf("Socket failed lol");
    }

    if (refreshsock < 0)
    {
        printf("Socket failed lol");
    }

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(19302);
    client_addr.sin_addr.s_addr = INADDR_ANY;

        bzero(&refresh_addr, sizeof(refresh_addr));
    refresh_addr.sin_family = AF_INET;
    refresh_addr.sin_port = htons(12080);
    refresh_addr.sin_addr.s_addr = INADDR_ANY;

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(21504);
    // inet_pton(AF_INET,"1.1.1.1",&server_addr.sin_addr);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if( bind(sockfd,(struct sockaddr*)&client_addr,sizeof(client_addr)) < 0 )
    {
        printf("bind failed - 1");
        exit(EXIT_FAILURE);
    }

    if( bind(refreshsock,(struct sockaddr*)&refresh_addr,sizeof(refresh_addr)) < 0 )
    {
        printf("\n Last error was: %s",strerror(errno));
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if(setsockopt( refreshsock,SOL_SOCKET,SO_RCVTIMEO,&reftimeout,sizeof(reftimeout)) < 0)
    {
                printf("\n Error when setiing timeout, exiting...");
                exit(EXIT_FAILURE);
    }


    socklen_t addrsize = sizeof(struct sockaddr_in);

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

    // CLIENT_MSG req;
    // bzero(&req,sizeof(req));
    // CLIENT_MSG resp;
    // bzero(&resp,sizeof(resp));
    int len = sizeof(server_addr);

    // CLIENT_MSG refreshsockf_info;
    // bzero(&refreshsockf_info,sizeof(refreshsockf_info));
    // refreshsockf_info.message_type = htons(0x0044);

    // CLIENT_MSG refreshsockf_rsp;
    // bzero(&refreshsockf_rsp,sizeof(refreshsockf_rsp));

    CLIENT_MSG message = {0};
    bzero(&message,sizeof(message));

    SEND_KEY_MSG sendkeymsg = {0};
    bzero(&sendkeymsg,sizeof(sendkeymsg));


    // char* server_modulus = (char*)calloc(158, sizeof(char));
    // char* server_exponent = (char*)calloc(6, sizeof(char));

    if(choice == 1)
    {
        make_find_req(message,sockfd,roomname,username,server_addr,len);
        bzero(&message,sizeof(message));
         recvfrom(sockfd,&message,sizeof(message),MSG_WAITALL,(struct sockaddr*)&server_addr,&addrsize);        
         int bcklg = message.attributes[14];
         CLIENT clients[bcklg];
         bzero(&clients,sizeof(clients));     
         client_stage(message,clients,sockfd,roomname,strlen(roomname),server_addr,addrsize);

	while(true)
	{
        bzero(&message,sizeof(message));
        message.message_type = htons(0x0044);         
	     sendto(refreshsock,&message,sizeof(message),0,(struct sockaddr*)&server_addr,addrsize);
	     
	     bzero(&message,sizeof(message));
        int rfshrsp = recvfrom(refreshsock,&message,sizeof(message),MSG_WAITALL,(struct sockaddr*)&server_addr,&addrsize);
	     
	    if(rfshrsp > 0)
	    {
		printf("%d",ntohs(message.message_type));
		fflush(stdout);
		if(ntohs(message.message_type) == 0x0404)
		{
		    break;
		}
 	    }
	} 
         life_cycle(sockfd,refreshsock,server_addr,clients,bcklg);

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
        message.message_type = htons(0x0022);
        sendto(sockfd,&message,sizeof(message),0,(struct sockaddr*)&server_addr,sizeof(server_addr));        
        // printf("0x%02X",req.message_type);
        // perror("sendto");
        // fflush(stdout);
        recvfrom(sockfd,&sendkeymsg,sizeof(sendkeymsg),MSG_WAITALL,(struct sockaddr*)&server_addr,&addrsize);
        
        handle_key(sendkeymsg);


    //     CLIENT clients[10];
    //     bzero(&clients,sizeof(clients));
    //     make_alloc_req(req,sockfd,roomname,username,10, server_addr,len);
    //     recvfrom(sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&server_addr,&addrsize);
        
    //     while(true)
	//     {
	//      sendto(refreshsock,&refreshsockf_info,sizeof(refreshsockf_info),0,(struct sockaddr*)&server_addr,addrsize);
	     
	//      bzero(&refreshsockf_rsp,sizeof(refreshsockf_rsp));
    //          int rfshrsp = recvfrom(refreshsock,&refreshsockf_rsp,sizeof(refreshsockf_rsp),MSG_WAITALL,NULL,NULL);
	     
	//     if(rfshrsp > 0)
	//     {
	// 	if(ntohs(refreshsockf_rsp.message_type) == 0x0404)
	// 	{
	// 	    break;
	// 	}
 	//     }
	// }
        
	// life_cycle(sockfd,refreshsock,server_addr,clients,10);
    }else
    {
        exit(EXIT_FAILURE);
    }

    // free(server_modulus);
    // free(server_exponent);
    
    close(refreshsock);
    close(sockfd);

    return 0;
}
