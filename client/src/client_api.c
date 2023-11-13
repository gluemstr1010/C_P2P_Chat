#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <uchar.h>
#include <time.h>
#include <curl/curl.h>
#include "client_api.h"

size_t WriteData(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    char *output_buffer = (char *)userp;

    strcat(output_buffer, (char *)contents);

    return total_size;
}

char* get_public_ip()
{
    CURL *curl_handle;
    CURLcode res;
 
     curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, "https://api.ipify.org");
        char output_buffer[20]; 
        output_buffer[0] = '\0'; 
        curl_easy_setopt(curl_handle,CURLOPT_WRITEFUNCTION,WriteData);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, output_buffer);
        res = curl_easy_perform(curl_handle);
 
        if (res != CURLE_OK) {
            printf("Error: %s\n", curl_easy_strerror(res));
            return NULL;
        }

        char *temp = malloc(20);
        temp = output_buffer;
        return temp;
 
        curl_easy_cleanup(curl_handle);
    }
 
    curl_global_cleanup();
    return NULL;
}

void make_find_req(CLIENT_MSG find_req, int clientfd,char roomname[],char username[], uint8_t client_ipadd[])
{
    bzero(&find_req, sizeof(find_req));
     time_t t;
    srand((unsigned)time(&t));

    find_req.message_type = 0x01;
     for (int i = 0; i < sizeof(find_req.trasaction_id) / sizeof(find_req.trasaction_id[0]); i++)
    {
        find_req.trasaction_id[i] = rand() % 256;
    }
    find_req.attributes[1] = 0x01;

    find_req.attributes[7] = 0x7A;
    find_req.attributes[8] = 0x31;   // what port is open will  be made later

    find_req.attributes[9] = client_ipadd[1];
    find_req.attributes[10] = client_ipadd[0];
    find_req.attributes[11] = client_ipadd[3];
    find_req.attributes[12] = client_ipadd[2];

    find_req.attributes[13] = 0x02;
    find_req.attributes[14] = strlen(roomname);
    
    char let;
    int a;

    for(int i = 15; i < 14 + strlen(roomname); i++)
    {
        let = roomname[i - 15];
        a = (int)let;
        find_req.attributes[i] = a;
    }

    u_int16_t Value_size = 0;

     find_req.message_length = htons(sizeof(find_req.attributes));
     find_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;
    
    send(clientfd,&find_req,sizeof(find_req),0);
    
}

void make_alloc_req(CLIENT_MSG alloc_req, int clientfd, char roomname[], char username[], uint8_t client_ipadd[], int backlog)
{
    bzero(&alloc_req, sizeof(alloc_req));
    time_t t;
    srand((unsigned)time(&t));

    alloc_req.message_type = 0x02;
     for (int i = 0; i < sizeof(alloc_req.trasaction_id) / sizeof(alloc_req.trasaction_id[0]); i++)
    {
        alloc_req.trasaction_id[i] = rand() % 256;
    }
    alloc_req.attributes[1] = 0x02;

    alloc_req.attributes[7] = 0x7A;
    alloc_req.attributes[8] = 0x31;   // what port is open will  be made later

    alloc_req.attributes[9] = client_ipadd[0];
    alloc_req.attributes[10] = client_ipadd[1];
    alloc_req.attributes[11] = client_ipadd[2];
    alloc_req.attributes[12] = client_ipadd[3];

    alloc_req.attributes[13] = 0x02;
    alloc_req.attributes[14] = strlen(roomname);
    
    char let;
    int a;

    for(int i = 15; i < 14 + strlen(roomname); i++)
    {
        let = roomname[i - 15];
        a = (int)let;
        alloc_req.attributes[i] = a;
    }

    alloc_req.attributes[16 + strlen(roomname)] = strlen(username);
    for (int i = 17 + strlen(roomname); i < 17 + strlen(roomname) + strlen(username); i++)
    {
        let = username[i - (17 + strlen(roomname))];
        a = (int)let;
        alloc_req.attributes[i] = a;
    }
       alloc_req.attributes[18 + strlen(roomname) + strlen(username)] = 0x11;
       alloc_req.attributes[19 + strlen(roomname) + strlen(username)] = backlog;

       u_int16_t Value_size = 0;

       alloc_req.message_length = htons(sizeof(alloc_req.attributes));
       alloc_req.attributes[3] = ( htons(Value_size) >> 8 ) & 0xFF;
    

    send(clientfd,&alloc_req,sizeof(alloc_req),0);
}

void prcess_find_resp(CLIENT_MSG resp)
{

}