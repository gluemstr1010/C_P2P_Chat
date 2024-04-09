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
#include "../../encryption/rsa/rsa.c"

// make hash tables
server_info servers[1000];
int activeServers = 0;

void make_alloc_res(SERV_MSG alloc_req,int client_sockfd, char* sourceIP, u_int16_t port, struct sockaddr_in address, int address_len)
{
   uint8_t transcid[12];
   memcpy(transcid,alloc_req.trasaction_id,sizeof(alloc_req.trasaction_id));
  
   char *chatname = malloc(13);
   char *usrname = malloc(13);

   char let;
   int a;
   socklen_t addrsiz = sizeof(address);
   uint8_t temp_add[4];

   
   process_srcIP(sourceIP,temp_add);

   int chatnamelen = alloc_req.attributes[14];
   int usrnamelen = alloc_req.attributes[16 + chatnamelen];

   int backlog = alloc_req.attributes[19 + chatnamelen + usrnamelen];

   process_req(alloc_req,chatname,usrname,let,chatnamelen);
   SERV_MSG resp;
   
   for(int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
   {
      int cs = canbe_server(alloc_req,i,chatname,temp_add,port);
      
      fflush(stdout);
      if(cs == 1)
      {
        bzero(&resp,sizeof(resp));
        resp.message_type = 0x0004;
         memcpy(resp.trasaction_id,transcid,sizeof(resp.trasaction_id));
         resp.attributes[1] = 0x04;
         
         
         char errmsg[] = "roomname or ip already exists";
         resp.attributes[3] = strlen(errmsg);
         for(int i = 4; i < 4 + strlen(errmsg); i++  )
         {
             let = errmsg[i - 4];
            a = (int)let;
            resp.attributes[i] = a;
         }
          sendto(client_sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&address,address_len);
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
        servers[i].clients = (struct ci*)malloc(backlog * sizeof(struct ci));

        printf("\n%s",servers[i].chatroom);
        printf("\n%s",servers[i].usrname);
        printf("\n%d",servers[i].backlog);
        printf("\n%d",servers[i].server_addr[0]);
        printf("%d",servers[i].server_addr[1]);
        printf("%d",servers[i].server_addr[2]);
        printf("%d",servers[i].server_addr[3]);
        printf("::%d",servers[i].server_port);
        fflush(stdout);

        activeServers++;

        bzero(&resp,sizeof(resp));
        resp.message_type = 0x0003;
        memcpy(resp.trasaction_id,transcid,sizeof(resp.trasaction_id));
        resp.attributes[1] = 0x03;
        char let;
         int a;
         char succmsg[] = "Server_addded";
         resp.attributes[3] = strlen(succmsg);
         for(int i = 4; i < 4 + strlen(succmsg); i++  )
         {
             let = succmsg[i - 4];
            a = (int)let;
            resp.attributes[i] = a;
         }
         int k;
        k = sendto(client_sockfd,&resp,sizeof(resp),MSG_WAITALL,(struct sockaddr*)&address,address_len);
        if(k < 0)
        {
            printf("\n Last error was 1 -: %s",strerror(errno));   
    	}
        
	bzero(&resp,sizeof(resp));
	bzero(&address,sizeof(address));
	
	k = recvfrom(client_sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,&addrsiz);       
	if(k < 0)
        {
            printf("\n Last error was 2 -: %s",strerror(errno));   
    	}	   
    	
	servers[i].refresh_port = ntohs(address.sin_port);
	printf("\n %d - alloc_refresh_port",servers[i].refresh_port);
	fflush(stdout);

	break;
      }    
   }
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
    free(ss);
    return 0;
}

void make_find_res(SERV_MSG find_req,int client_sockfd,  char* sourceIP, u_int16_t port, struct sockaddr_in address, int address_len)
{
  uint8_t transcid[12];
   memcpy(transcid,find_req.trasaction_id,sizeof(find_req.trasaction_id));
  
   char *chtname = malloc(13);
   char *client_usrname = malloc(13);

   char let;
   int a;
   socklen_t addrsiz = sizeof(address);
   uint8_t client_add[4];

   process_srcIP(sourceIP,client_add);

   uint8_t server_add[4];
   int16_t server_port;
   uint8_t server_backlog;

   int chatnamelen = find_req.attributes[14];
   int usrnamelen = find_req.attributes[16 + chatnamelen];

   process_req(find_req,chtname,client_usrname,let,chatnamelen);

    SERV_MSG response;
   char err_msg[30];
   int err = 0;

   printf("\n%d\n",port);

   int x, y = 0;
   
   for(int i = 0; i < activeServers ; i++)
   {    
        if(strcmp(servers[i].chatroom,chtname) == 0)
        {   
            bzero(&response,sizeof(response));
            for(int s = 0; s < 4; s++)
            {
                server_add[s] = servers[i].server_addr[s];
            }
            server_port = servers[i].server_port;
             server_backlog = servers[i].backlog;

             response.message_type = 0x0005;
            memcpy(response.trasaction_id,transcid,sizeof(response.trasaction_id));
            response.attributes[1] = 0x55;
            response.attributes[7] = (server_port >> 8) & 0xFF;
            response.attributes[8] = server_port & 0xFF;
            response.attributes[9] =  server_add[0];
            response.attributes[10] = server_add[1];
            response.attributes[11] = server_add[2];
            response.attributes[12] = server_add[3];
            response.attributes[13] = 0x05;
            response.attributes[14] = server_backlog;
            response.attributes[15] = 0x05;
            u_int8_t usernamelen = strlen(servers[i].usrname);
            response.attributes[16] = usernamelen;
            char tempusrnem[usernamelen];
            strcpy(tempusrnem,servers[i].usrname);
            for(int i = 17; i < 16 + usernamelen; i++)
            {
                let = tempusrnem[i - 17];
                a = (int)let;
                response.attributes[i] = a;
            }


            u_int16_t Value_size = 0;
        
             response.message_length = sizeof(response.attributes);

                response.attributes[3] = ( Value_size >> 8 ) & 0xFF;
                response.attributes[4] = Value_size & 0xFF;
            sleep(5);
            int cc = sendto(client_sockfd,&response,sizeof(response),0,(struct sockaddr*)&address,address_len);
            fflush(stdout);
                for(int j = 0; j <= servers[i].activeClients ; j++)
                {
                    if( servers[i].clients[j].client_port == 0 )
                    {
                       
                        for(int k = 0; k < 4; k++)
                        {
                            servers[i].clients[j].client_addr[k] = client_add[k];
                        }
                        


                        servers[i].clients[j].client_port = port;
                        servers[i].clients[j].chatroom = chtname;
                        servers[i].clients[j].usrname = client_usrname;
                        servers[i].activeClients++;
                	
			x = i;
			y = j;	
                        // printf("\n%d",servers[i].clients[j].client_addr[0]);
                        // printf("%d",servers[i].clients[j].client_addr[1]);
                        // printf("%d",servers[i].clients[j].client_addr[2]);
                        // printf("%d",servers[i].clients[j].client_addr[3]);
                        // printf(":%d",servers[i].clients[j].client_port);
                        // fflush(stdout);
                        
                        err = 1;
                        break;
                    }
                    sleep(4);
                    if (servers[i].clients[j].client_port != 0)
                    {
                         bzero(&response,sizeof(response));
                        response.message_type = 0x0005;
                        memcpy(response.trasaction_id,transcid,sizeof(response.trasaction_id));
                        response.attributes[1] = 0x05;
                        response.attributes[7] = (servers[i].clients[j].client_port >> 8) & 0xFF;
                        response.attributes[8] = servers[i].clients[j].client_port & 0xFF;
                        response.attributes[9] =  servers[i].clients[j].client_addr[0];
                        response.attributes[10] = servers[i].clients[j].client_addr[1];
                        response.attributes[11] = servers[i].clients[j].client_addr[2];
                        response.attributes[12] = servers[i].clients[j].client_addr[3];
                         response.attributes[15] = 0x05;
                        u_int8_t clientusernamelen = strlen(servers[i].clients[j].usrname);
                        response.attributes[16] = clientusernamelen;
                        char tempusrnem[clientusernamelen];
                        strcpy(tempusrnem,servers[i].clients[j].usrname);
                        for(int i = 17; i < 16 + clientusernamelen; i++)
                        {
                            let = tempusrnem[i - 17];
                            a = (int)let;
                            response.attributes[i] = a;
                        }

                        u_int16_t Value_size = 0;
                        
                        find_req.message_length = htons(sizeof(find_req.attributes));

                            int j = sendto(client_sockfd,&response,sizeof(response),MSG_WAITALL,(struct sockaddr*)&address,address_len);
                            if(j < 0)
                            {
                                printf("\n Last error was: %s - 11",strerror(errno));
                                fflush(stdout);
                            }
                            bzero(&response,sizeof(response));

			    j = recvfrom(client_sockfd,&response,sizeof(response),0,(struct sockaddr*)&address,&addrsiz);
			    if(j < 0)
                            {
                                printf("\n Last error was: %s - 22",strerror(errno));
                                fflush(stdout);
                            }
                            printf( "\n %d - recv",j ); 
			    
			    servers[x].clients[y].refresh_port = ntohs(address.sin_port);
			    printf("\n  %d - find refresh port",ntohs(address.sin_port));
			    fflush(stdout);
                    }

              }
        }      
   }    

   if(err == 0)
   {
        strcpy(err_msg,"no_room_found");
         printf("notgut");
        fflush(stdout);
        bzero(&response,sizeof(response));
        response.message_type = 0x0006;
        memcpy(response.trasaction_id,transcid,sizeof(response.trasaction_id));
        response.attributes[1] = 0x06;
        response.attributes[3] = strlen(err_msg);
        int a;
        for(int i = 4 ; i < 4 + strlen(err_msg); i++ )
        {
            let = err_msg[i - 4];
            a = (int)let;
            response.attributes[i] = a;
        }
        int h = sendto(client_sockfd,&response,sizeof(response),MSG_WAITALL,(struct sockaddr*)&address,address_len);

        if(h < 0)
            {
                printf("\n Last error was: %s",strerror(errno));
                fflush(stdout);
            }
            
            fflush(stdout);
   }

}

void process_req(SERV_MSG req,char *chatname, char *usrname,char let ,int chatnamelen )
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
    struct sockaddr_in activeCLient;
    socklen_t addr_size;
    addr_size = sizeof(activeCLient);

    uint8_t new_client[4];

    process_srcIP(sourceaddr,new_client);

    SERV_MSG msg;
    msg.message_type = 0x0011;
    msg.attributes[1] = 0x11;
    msg.attributes[7] = (port >> 8) & 0xFF;
    msg.attributes[8] = port & 0xFF;
     msg.attributes[9] = new_client[0];
    msg.attributes[10] = new_client[1];
    msg.attributes[11] = new_client[2];   
    msg.attributes[12] = new_client[3];
    msg.attributes[13] = 0x11;
    msg.attributes[14] = strlen(roomname);
    char let;
    int a;

    for(int i = 15; i < 15 + strlen(roomname); i++)
    {
        let = roomname[i - 15];
        a = (int)let;
        msg.attributes[i] = a;
    }

    msg.attributes[16 + strlen(roomname)] = strlen(usrname);
    for (int i = 17 + strlen(roomname); i < 17 + strlen(roomname) + strlen(usrname); i++)
    {
        let = usrname[i - (17 + strlen(roomname))];
        a = (int)let;
        msg.attributes[i] = a;
    }

    for (int i = 0; i < sizeof(msg.trasaction_id) / sizeof(msg.trasaction_id[0]); i++)
    {
        msg.trasaction_id[i] = rand() % 256;
    }

    for(int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
    {
        if(servers[i].server_port != 0)
        {
            if(strcmp(servers[i].chatroom,roomname) == 0)
            {
                bzero(&activeCLient,sizeof(activeCLient));
                activeCLient.sin_family = AF_INET;
                activeCLient.sin_port = htons(servers[i].refresh_port);
                inet_pton(AF_INET,convert_IP_tochar(servers[i].server_addr),&(activeCLient.sin_addr));
                    // inet_pton(AF_INET,"127.0.0.1",&(activeCLient.sin_addr)); 
		printf(" \n%d ", servers[i].refresh_port);
                fflush(stdout);

                int e = sendto(sockfd, &msg, sizeof(msg), 0,(struct sockaddr*)&activeCLient, sizeof(activeCLient));
                    
                for(int j = 0; j < servers[i].activeClients-1 ; j++)
                {
                    bzero(&activeCLient,sizeof(activeCLient));
                    activeCLient.sin_family = AF_INET;
                    activeCLient.sin_port = htons(servers[i].clients[j].refresh_port);
                    inet_pton(AF_INET,convert_IP_tochar(servers[i].clients[j].client_addr),&(activeCLient.sin_addr));
                    // inet_pton(AF_INET,"127.0.0.1",&(activeCLient.sin_addr)); 
                
                    
                    int h = sendto(sockfd, &msg, sizeof(msg), 0,(struct sockaddr*)&activeCLient, sizeof(activeCLient));   
                    
                    printf("\n%d - c",h);
                    fflush(stdout);
                }
            }
        }
        
    }
}

void send_key(int serv_sockfd,struct sockaddr_in address, int address_len,gmp_randstate_t state)
{
    char* p = generate_prime(state);
    char* q = generate_prime(state);
   
    mpz_t n,pp,qq, eul;
    mpz_init(n);
    mpz_init(pp);
    mpz_init(qq);
    mpz_init(eul);

    mpz_set_str(pp,p,10);
    mpz_set_str(qq,q,10);

    mpz_mul(n,pp,qq);

    char* mod = mpz_get_str(NULL,10,n);

    mpz_sub_ui(pp,pp,1);
    mpz_sub_ui(qq,qq,1);

    mpz_mul(eul,pp,qq);

    char* euler = mpz_get_str(NULL,10,eul);

    mpz_clears(n,eul,pp,qq,NULL);

    char* exponent = malloc(6);
    strcpy(exponent,"65537");

    char* pd = get_d(euler,exponent);

    SEND_KEY_MSG ack;
    bzero(&ack,sizeof(ack));

    ack.message_type = 0x2020;

    ack.attributes[0] = 0x1;

    char c = 'c';
    int a = 0;

    for(size_t i = 1; i < 1 + strlen(exponent); i++)
    {
        char c = exponent[i-1];
        a = (int)c;
        ack.attributes[i] = a;
        // printf("%d",a);
        // fflush(stdout);
    }
    char* isitthesame = calloc(16,sizeof(char));

    for(size_t i = 1; i < 1 + strlen(exponent); i++)
    {
        c = (char)ack.attributes[i];
        isitthesame[i-1] = c; 
        printf("%c",isitthesame[i-1]);
        fflush(stdout);
    }

    // place modulus into message, send message to client

    free(isitthesame);
    free(p);
    free(q);
    free(pd);
    free(euler);
    free(mod);
    free(exponent);
}

char* convert_IP_tochar(uint8_t ipadd[])
{
    char *ip = malloc(17);
    char temp[17];
    char pom[33];
    bzero(&temp,sizeof(temp));
    bzero(&pom,sizeof(pom));


    char let;
    int k = 0;
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
