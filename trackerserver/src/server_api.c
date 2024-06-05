#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include <errno.h>
#include <gmp.h>
#include "server_api.h"
#include "../../enc/inc/rsa_api.h"
#include "../../serialization/inc/serialize_api.h"

// make hash tables
server_info servers[10];
// int activeServers = 0;

void make_alloc_res(SEND_REQ alloc_req,int client_sockfd, char* sourceIP, u_int16_t port, struct sockaddr_in address,char* roomname,char* usrname,uint16_t uid)
{

    int backlog = alloc_req.backlog;

    socklen_t addrsiz = sizeof(address);

    SERV_MSG resp;
    memset(&resp,0,sizeof(resp));
   
   

    int checkRoom = CheckRoomExistence(roomname);

    if(checkRoom == 0)
    {
        resp.message_type  = 0x0003;
        sendto(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));
    }
    else
    {
         resp.message_type = 0x0004;

        char chruid[12];
        memset(chruid,0,sizeof(chruid));
        sprintf(chruid,"%d",uid);
        strcpy(resp.attribute,chruid);

        sendto(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));

        bzero(&resp,sizeof(resp));

	    bzero(&address,sizeof(address));
	
	    int k = recvfrom(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,&addrsiz);       

	    if(k < 0)
        {
            printf("\n Last error was 2 -: %s",strerror(errno));   
    	}	   
    	
        uint16_t refport = ntohs(address.sin_port);
        

        CreateRoom(roomname,backlog,1,usrname,sourceIP,port,refport,uid);
    }

        // printf("%s\n",roomname);
    // printf("%s\n",usrname);
    // fflush(stdout);
  
//    uint8_t transcid[12];
//    memcpy(transcid,alloc_req.trasaction_id,sizeof(alloc_req.trasaction_id));

    //    int chatnamelen = alloc_req.attributes[14];
//    int usrnamelen = alloc_req.attributes[16 + chatnamelen];

//    int backlog = alloc_req.attributes[19 + chatnamelen + usrnamelen];

//    process_req(alloc_req,chatname,usrname,chatnamelen,usrnamelen);

//    for(size_t i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
//    {
//       int cs = canbe_server(i,roomname,temp_add,port);
      
//       fflush(stdout);
//       if(cs == 1)
//       {
//         bzero(&resp,sizeof(resp));
//         resp.message_type = 0x0004;
//         //  memcpy(resp.trasaction_id,transcid,sizeof(resp.trasaction_id));
//          resp.attributes[1] = 0x04;
         
         
//          char errmsg[] = "roomname or ip already exists";
//          resp.attributes[3] = strlen(errmsg);
//          for(size_t i = 4; i < 4 + strlen(errmsg); i++  )
//          {
//              let = errmsg[i - 4];
//             a = (int)let;
//             resp.attributes[i] = a;
//          }
//           sendto(client_sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&address,sizeof(address));
//          break;
//       }
//       if(servers[i].server_port == 0)
//       {
//         for(int j = 0; j < 4; j++)
//         {
//          servers[i].server_addr[j] = temp_add[j];
//         }
//         servers[i].server_port = port;
//         servers[i].chatroom = roomname;
//         servers[i].usrname = usrname;
//         servers[i].backlog = backlog;
//         servers[i].activeClients = 0;
//         servers[i].clients = (struct ci*)malloc(backlog * sizeof(struct ci));

//         printf("\n%s",servers[i].chatroom);
//         printf("\n%s",servers[i].usrname);
//         printf("\n%d",servers[i].backlog);
//         printf("\n%d",servers[i].server_addr[0]);
//         printf("%d",servers[i].server_addr[1]);
//         printf("%d",servers[i].server_addr[2]);
//         printf("%d",servers[i].server_addr[3]);
//         printf("::%d",servers[i].server_port);
//         fflush(stdout);

//         activeServers++;

//         bzero(&resp,sizeof(resp));
//         resp.message_type = 0x0003;
//         // memcpy(resp.trasaction_id,transcid,sizeof(resp.trasaction_id));
//         resp.attributes[1] = 0x03;
        
//          char succmsg[] = "Server_addded";
//          resp.attributes[3] = strlen(succmsg);
//          for(size_t i = 4; i < 4 + strlen(succmsg); i++  )
//          {
//              let = succmsg[i - 4];
//             a = (int)let;
//             resp.attributes[i] = a;
//          }
//          int k = 0;
//         k = sendto(client_sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&address,sizeof(address));
//         if(k < 0)
//         {
//             printf("\n Last error was 1 -: %s",strerror(errno));   
//     	}
        
// 	bzero(&resp,sizeof(resp));
// 	bzero(&address,sizeof(address));
	
// 	k = recvfrom(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,&addrsiz);       
// 	if(k < 0)
//         {
//             printf("\n Last error was 2 -: %s",strerror(errno));   
//     	}	   
    	
// 	servers[i].refresh_port = ntohs(address.sin_port);
// 	printf("\n %d - alloc_refresh_port",servers[i].refresh_port);
// 	fflush(stdout);

// 	break;
//       }    
//    }
}

// int canbe_server(int i,char *chatname,uint8_t temp_add[],int16_t port)
// {
    
//     char *ss = servers[i].chatroom;
//         if(ss!=NULL )
//         {
//             if(strcmp(ss,chatname) == 0)
//             {
//                 printf("\n This room name is taken");
//                 return 1;
//             }
//         }

//         int aresameIPs;
//         int aresamePorts = 1;
//         for(int j = 0; j < 4; j++)
//         {
//             aresameIPs = 1;
//             if(temp_add[j] != servers[i].server_addr[j] )
//             {
//                 return 0;
//             }
//             if(port == servers[i].server_port)
//             {
//                 aresamePorts = 0;
//             }
//             aresameIPs = 0;
//         }
        
//         if(aresameIPs == 0 && aresamePorts == 0)
//         {
//             return 1;
//         }
//     free(ss);
//     return 0;
// }

void make_find_res(int client_sockfd,  char* sourceIP, u_int16_t port, struct sockaddr_in address, char* roomname,char* usrname,char* client_e, char* client_m,uint16_t uid)
{
        SERV_MSG resp = {0};

        socklen_t addrsize = sizeof(struct sockaddr_in);

        char tempusrnem[20];
        bzero(&tempusrnem,sizeof(tempusrnem));

        char tempipaddr[17];
        bzero(&tempipaddr,sizeof(tempipaddr));

        const char* pos = GetPeerPos(roomname);

        if(pos == NULL)
        {
            printf("something bad happened");    
        }
        resp.message_type = 0x0004;
        char chruid[12];
        memset(chruid,0,sizeof(chruid));
        sprintf(chruid,"%d",uid);

        strcpy(resp.attribute,chruid);

        sendto(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));

        sleep(1);

        ReadClients(client_sockfd,pos,client_e,client_m,address);

        recvfrom(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,&addrsize);

        uint16_t refport = ntohs(address.sin_port);

        WritePeer(roomname,usrname,sourceIP,port,refport,uid);

}

uint8_t* process_srcIP(char* srcIP,uint8_t client_ipadd[])
{
    
    char* token = strtok(srcIP,".");
 
    
    int i = 0;
    while (token != NULL) {
        client_ipadd[i] = atoi(token);
        i++;
        token = strtok(NULL,".");
    }
    free(token);

    return client_ipadd;
}

void broadcast_new_client(int sockfd, u_int16_t port,char *sourceaddr, char *usrname, char *roomname){
    
    const char* pos = GetPeerPos(roomname);

    struct sockaddr_in activeCLient;

     char client_m[270];
    char client_e[12];

    memset(client_m,'\0',sizeof(client_m));
    memset(client_e,'\0',sizeof(client_e));


    int uid = 0;
    char tempipadd[17];
    memset(tempipadd,0,sizeof(tempipadd));
       uint16_t peerport = 0;

    CLIENT_INFO_MSG msg = {0};
    msg.message_type = 0x0011;

    strcpy(msg.client_addr,sourceaddr);
    msg.port = port;

    msg.xlen_r = strlen(roomname);
    msg.xlen_u = strlen(usrname);

    char enc_room[MAXLEN_ROOM][MAXENCLETLEN];
    char enc_usr[MAXLEN_USER][MAXENCLETLEN];
    
     while ((pos = strstr(pos, "\"username\":")) != NULL) {
        sscanf(pos, "\"username\":\"%*[^\"]\";\"peeraddr\":\"%17[^\"]\";\"peerport\":%hd;\"refport\":%*d;\"uid\":%d",
               tempipadd,
               &peerport,
               &uid);

        GetPeerPublicKey(uid,client_e,client_m);

        if(strlen(client_m) > 1 && strlen(client_e) > 1)
        {
           
            memset(enc_room,'\0',sizeof(enc_room));
            memset(enc_usr,'\0',sizeof(enc_usr));

            encrypt(roomname,client_e,client_m,enc_room);
            encrypt(usrname,client_e,client_m,enc_usr);

            for(int i = 0; i < MAXLEN_ROOM; i++ )
            {
                for(int j = 0; j < MAXENCLETLEN; j++)
                {
                    msg.enc_room[i][j] = enc_room[i][j];
                }
            }

            for(int i = 0; i < MAXLEN_USER; i++ )
            {
                for(int j = 0; j < MAXENCLETLEN; j++)
                {
                    msg.enc_usr[i][j] = enc_usr[i][j];
                }
            }

            activeCLient.sin_family = AF_INET;
            activeCLient.sin_port = htons(peerport);
            inet_pton(AF_INET,tempipadd,&(activeCLient.sin_addr));

            sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr*)&activeCLient,sizeof(activeCLient));

        }

        memset(client_m,0,sizeof(client_m));
        memset(client_e,0,sizeof(client_e));
        memset(msg.enc_room,0,sizeof(msg.enc_room));
        memset(msg.enc_usr,0,sizeof(msg.enc_usr));

        pos++;
    }   

    // struct sockaddr_in activeCLient;

    // uint8_t new_client[4];

    // process_srcIP(sourceaddr,new_client);

    // SERV_MSG msg;
    // msg.message_type = 0x0011;
    // msg.attributes[1] = 0x11;
    // msg.attributes[7] = (port >> 8) & 0xFF;
    // msg.attributes[8] = port & 0xFF;
    //  msg.attributes[9] = new_client[0];
    // msg.attributes[10] = new_client[1];
    // msg.attributes[11] = new_client[2];   
    // msg.attributes[12] = new_client[3];
    // msg.attributes[13] = 0x11;
    // msg.attributes[14] = strlen(roomname);
    // char let;
    // int a;

    // for(size_t i = 15; i < 15 + strlen(roomname); i++)
    // {
    //     let = roomname[i - 15];
    //     a = (int)let;
    //     msg.attributes[i] = a;
    // }

    // msg.attributes[16 + strlen(roomname)] = strlen(usrname);
    // for (size_t i = 17 + strlen(roomname); i < 17 + strlen(roomname) + strlen(usrname); i++)
    // {
    //     let = usrname[i - (17 + strlen(roomname))];
    //     a = (int)let;
    //     msg.attributes[i] = a;
    // }

    // for (size_t i = 0; i < sizeof(msg.trasaction_id) / sizeof(msg.trasaction_id[0]); i++)
    // {
    //     msg.trasaction_id[i] = rand() % 256;
    // }

    // for(size_t i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
    // {
    //     if(servers[i].server_port != 0)
    //     {
    //         if(strcmp(servers[i].chatroom,roomname) == 0)
    //         {
    //             bzero(&activeCLient,sizeof(activeCLient));
    //             activeCLient.sin_family = AF_INET;
    //             activeCLient.sin_port = htons(servers[i].refresh_port);
    //             inet_pton(AF_INET,convert_IP_tochar(servers[i].server_addr),&(activeCLient.sin_addr));
    //                 // inet_pton(AF_INET,"127.0.0.1",&(activeCLient.sin_addr)); 
	// 	printf(" \n%d ", servers[i].refresh_port);
    //             fflush(stdout);

    //             sendto(sockfd, &msg, sizeof(msg), 0,(struct sockaddr*)&activeCLient, sizeof(activeCLient));
                    
    //             for(int j = 0; j < servers[i].activeClients-1 ; j++)
    //             {
    //                 bzero(&activeCLient,sizeof(activeCLient));
    //                 activeCLient.sin_family = AF_INET;
    //                 activeCLient.sin_port = htons(servers[i].clients[j].refresh_port);
    //                 inet_pton(AF_INET,convert_IP_tochar(servers[i].clients[j].client_addr),&(activeCLient.sin_addr));
    //                 // inet_pton(AF_INET,"127.0.0.1",&(activeCLient.sin_addr)); 
                
                    
    //                 int h = sendto(sockfd, &msg, sizeof(msg), 0,(struct sockaddr*)&activeCLient, sizeof(activeCLient));   
                    
    //                 printf("\n%d - c",h);
    //                 fflush(stdout);
    //             }
    //         }
    //     }
        
    // }
}

void send_key(int serv_sockfd,struct sockaddr_in address,char* mod,char* exponent,uint16_t uid)
{
    SEND_KEY_MSG ack;
    bzero(&ack,sizeof(ack));

    ack.message_type = htons(0x1022);

    ack.uid = uid;

    strcpy(ack.attribute1,mod);
    strcpy(ack.attribute2,exponent);

    sendto(serv_sockfd,&ack,sizeof(ack),0,(struct sockaddr*)&address,sizeof(address));

}

char* convert_IP_tochar(uint8_t ipadd[])
{
    char *ip = malloc(17);
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
