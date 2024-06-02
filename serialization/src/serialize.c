#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include "../inc/serialize_api.h"
#include "../../trackerserver/trackerlib/server_api.h"
#include "../../enc/inc/rsa_api.h"

const char* room_in = "\"room\":\"[^\"]\"\n\"backlog\":%d\n\"activeClients\":%d\n";
const char* room_out = "\"room\":\"%s\"\n\"backlog\":%d\n\"activeClients\":%d\n";


const char* peer_in = "\t\"username\":\"\"[^\"]\"\";\"peeraddr\":\"\"%s\"\";\"peerport\":%d;\"refport\":%d;\"uid\":%d;\"keyhash\":\"[^\"]\"\n";
const char* peer_out = "\t\"username\":\"%s\";\"peeraddr\":\"%s\";\"peerport\":%d;\"refport\":%d;\"uid\":%d;\"keyhash\":\"%s\"\n";

const char* pubkey_in = "\"modulus\":\n\t\"%[^\"]\"\n\"exponent\":\n\t\"%[^\"]\"";
const char* pubkey_out = "\"modulus\":\n\t\"%s\"\n\"exponent\":\n\t\"%s\"";

const char* prikey_in = "\"modulus\":\n\t\"\"[^\"]\"\n\"private_d\":\n\t\"[^\"]\"";;
const char* prikey_out = "\"modulus\":\n\t\"%s\"\n\"private_d\":\n\t\"%s\"";

char* write_path = "../../dat/";
const char* rooms_path = "../dat/rooms.dat";
const char* client_keys = "../dat/keys/clients/";

void CreateRoom(char* roomname,int backlog, int activeClients,char* usrname,char* peer_addr,uint16_t peer_port, uint16_t refresh_port,uint16_t uid,char* keyhash)
{

    FILE* file;

    file = fopen(rooms_path,"a");

    fprintf(file,room_out,roomname,backlog,activeClients);

    fclose(file);

    WritePeer(roomname,usrname,peer_addr,peer_port,refresh_port,uid,keyhash);

    // fprintf(file,peer_out,)
}

void WritePeer(char* roomname,char* usrname,char* peer_addr,uint16_t peer_port, uint16_t refresh_port,uint16_t uid,char* keyhash)
{
     FILE *file = fopen(rooms_path, "r+");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    // Find the room section
    char room_section[100];
    sprintf(room_section, "\"room\":\"%s\"", roomname);
    char *room_pos = strstr(buffer, room_section);
    if (room_pos == NULL) {
        printf("Room %s not found\n", roomname);
        free(buffer);
        fclose(file);
        return;
    }

    // Find the end of the activeClients section
    char *active_clients_pos = strstr(room_pos, "activeClients");
    if (active_clients_pos == NULL) {
        printf("activeClients section not found for room %s\n", roomname);
        free(buffer);
        fclose(file);
        return;
    }
    char *end_of_clients_pos = strchr(active_clients_pos, '\n');
    if (end_of_clients_pos == NULL) {
        printf("Could not find end of activeClients section\n");
        free(buffer);
        fclose(file);
        return;
    }

    end_of_clients_pos++;

    long insert_pos = end_of_clients_pos - buffer;

    char new_peer_entry[500];
    sprintf(new_peer_entry, peer_out,usrname,peer_addr,peer_port,refresh_port,uid,keyhash);

    fseek(file, insert_pos, SEEK_SET);

    long remainder_size = file_size - insert_pos;
    char *remainder_buffer = (char*)malloc(remainder_size + 1);
    if (remainder_buffer == NULL) {
        perror("Failed to allocate memory");
        free(buffer);
        fclose(file);
        return;
    }
    fread(remainder_buffer, 1, remainder_size, file);
    remainder_buffer[remainder_size] = '\0';
    
    fseek(file, insert_pos, SEEK_SET);

    fwrite(new_peer_entry, 1, strlen(new_peer_entry), file);
    fwrite(remainder_buffer, 1, remainder_size, file);

    free(buffer);
    fclose(file);


    // FILE* file;
    // file = fopen(rooms_path,"a");

    // fprintf(file,peer_out,usrname,peer_addr,peer_port,refresh_port,uid,keyhash);

    // fclose(file);
}

void ReadClients(int sockfd,const char* pos,char* exponent,char* mod,struct sockaddr_in address)
{
    char* tempusrnem =  (char*)calloc(20, sizeof(char));
    char peeraddr[17];
    bzero(&peeraddr,sizeof(peeraddr));
    uint16_t port = 0;

    CLIENT_INFO_MSG resp = {0};

     char enc_room[MAXLEN_ROOM][MAXENCLETLEN];
    memset(enc_room, '\0', sizeof(enc_room));

     char enc_usr[MAXLEN_USER][MAXENCLETLEN];
    memset(enc_usr, '\0', sizeof(enc_usr));   

    while ((pos = strstr(pos, "\"username\":")) != NULL) {
        sscanf(pos, "\"username\":\"%19[^\"]\";\"peeraddr\":\"%[^\"]\";\"peerport\":%hd;\"refport\":%*d;\"uid\":%*d;\"keyhash\":\"%*[^\"]\"",
               tempusrnem,
               peeraddr,
               &port);

        resp.message_type = 0x0005;

        encrypt(tempusrnem,exponent,mod,enc_usr);

        for(int i = 0; i < MAXLEN_USER; i++ )
        {
            for(int j = 0; j < MAXENCLETLEN; j++)
            {
                resp.enc_usr[i][j] = enc_usr[i][j];
            }
        }

        resp.xlen_u = strlen(tempusrnem);

        strcpy(resp.client_addr,peeraddr);

        resp.port = port;

        sendto(sockfd,&resp,sizeof(resp),0,(struct sockaddr*)&address,sizeof(address));

        bzero(&resp,sizeof(resp));
        memset(tempusrnem,0,20);
        memset(peeraddr,0,sizeof(peeraddr));

        sleep(1);
             
        pos++;
    }   
}

void GetPeerPublicKey(int uid,char client_e[12],char client_m[270])
{
    char chruid[11];
    memset(chruid,0,sizeof(chruid));
    sprintf(chruid,"%d",uid);

    char clientdir[40];
    memset(clientdir,0,sizeof(clientdir));
    strcpy(clientdir,client_keys);
    strcat(clientdir,chruid);

    char pubkeypath[50];
    memset(pubkeypath,0,sizeof(pubkeypath));
    strcpy(pubkeypath,clientdir);
    strcat(pubkeypath,"/clientpub.key");

    printf("%s\n",pubkeypath);
    fflush(stdout);

    DIR *dir = opendir(clientdir);
    if (!dir) {
        // perror("Unable to open directory");
        goto skip;
    }

    

    FILE *file = fopen(pubkeypath, "r");
    if (file == NULL) {
        // perror("Unable to open file");
        goto skip;
    }

    if(fscanf(file,pubkey_in,client_m,client_e) != 2)
    {
        // perror("Error reading values");
        goto skip;
    }

    // char buffer[512];
    // if (fgets(buffer, sizeof(buffer), file) != NULL) {
    //     if (sscanf(buffer, peer_in, client_m, client_e) != 2) {
    //         perror("Error reading values");
    //     } 
    //         printf("modulus: %s\n", client_m);
    //         printf("exponent: %s\n", client_e);
    //         fflush(stdout);
        
    // } else {
    //     perror("Error reading line");
    // }

    closedir(dir);
    fclose(file);


    skip:
}

const char* GetPeerPos(char* roomname)
{
    FILE* file = fopen("../dat/rooms.dat","r");

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return '\0';
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    char room_section[100];
    sprintf(room_section, "\"room\":\"%s\"", roomname);
    char *room_pos = strstr(buffer, room_section);
    if (room_pos == NULL) {
        printf("Room %s not found\n", roomname);
        free(buffer);
        fclose(file);
        return '\0';
    }

     char *active_clients_pos = strstr(room_pos, "activeClients");
    if (active_clients_pos == NULL) {
        printf("activeClients section not found for room %s\n", roomname);
        free(buffer);
        return '\0';
    }

    char *peer_info_start = strchr(active_clients_pos, '\n');
    if (peer_info_start == NULL) {
        printf("Could not find start of peer info\n");
        free(buffer);
        return '\0';
    }
    peer_info_start++;

    char *peer_info_end = strstr(peer_info_start, "\"room\":");
    if (peer_info_end == NULL) {
        peer_info_end = buffer + file_size;
    }

    size_t peer_info_len = peer_info_end - peer_info_start;
    char *peer_info = malloc(peer_info_len + 1);
    strncpy(peer_info, peer_info_start, peer_info_len);
    peer_info[peer_info_len] = '\0';

    const char *peer_start = peer_info;
    peer_start = strstr(peer_start, "\"username\":");

    return peer_start;
}

// int CheckKeyExistence(char* uid)
// {
//     FILE *file;
//     const char *filename = "example.txt";
//     // char tmpPath[32];

//     file = fopen(filename, "r");
//     if (file != NULL) {
//         return 0;
//     } else {
//         file = fopen(filename, "w");
//         fclose(file);
//         return 1;
//     }
// }

void WriteKeys(uint16_t uid,char* client_modulus,char* client_exponent,char* server_modulus,char* server_exponent,char* private_d)
{
  int checkDir = CheckDirExistence(uid);

  char dir[12];
  bzero(&dir,sizeof(dir));

  sprintf(dir,"%d",uid);

  char path1[50];
  bzero(&path1,sizeof(path1));
  strcat(path1,client_keys);
  strcat(path1,dir);

  char path2[50];
  bzero(&path2,sizeof(path2));
  strcpy(path2,path1);

  char path3[50];
  bzero(&path3,sizeof(path3));
  strcpy(path3,path1);

  strcat(path1,"/clientpub.key");
  strcat(path2,"/serverpub.key");
  strcat(path3,"/serverpri.key");

//   printf("%s\n",path1);
//   printf("%s\n",path2);
//   printf("%s\n",path3);
//   fflush(stdout);

  FILE* clientpub;
  FILE* serverpub;
  FILE* serverpri;

  clientpub = fopen(path1,"w");
  serverpub = fopen(path2,"w");
  serverpri = fopen(path3,"w");

  fprintf(clientpub,pubkey_out,client_modulus,client_exponent);
  fprintf(serverpub,pubkey_out,server_modulus,server_exponent);
  fprintf(serverpri,prikey_out,server_modulus,private_d);

  fclose(clientpub);
  fclose(serverpub);
  fclose(serverpri);
}

int directory_exists(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && (st.st_mode & S_IFDIR)) {
        return 1; // Directory exists
    }
    return 0; // Directory does not exist
}
int create_directory(const char *path) {
    #if defined(_WIN32)
    return _mkdir(path); // For Windows
    #else
    return mkdir(path, 0700); // For Unix-like systems
    #endif
}

int CheckDirExistence(uint16_t uid)
{

    char dir[12];
    bzero(&dir,sizeof(dir));

    sprintf(dir,"%d",uid);

    char tmp[40];
    bzero(&tmp,sizeof(tmp));

    strcat(tmp,client_keys);
    strcat(tmp,dir);
    
    if (directory_exists(tmp)) {
        return 1;
    } else {
        if (create_directory(tmp) == 0) {
            return 0;
        } else {
            return 2;
        }
    }
}

int CheckRoomExistence(char* roomname)
{
  // ../dat/rooms.dat

  FILE* file = fopen(rooms_path, "r");

  if(file == NULL)
  {
    printf("gg");
    fflush(stdout);
  }

    char line[256];
    char room_line[256];
    snprintf(room_line, sizeof(room_line), "\"room\":\"%s\"", roomname);
    // printf("%s",room_line);
    // fflush(stdout);
    while (fgets(line, sizeof(line), file)) {

        if (strstr(line, room_line) != 0) {
            fclose(file);
            return 0; // Room found
        }
    }

    // FIX THIS ROOM IS NOT GETTING FOUND

    fclose(file);
    return 1; // Room not found
}

