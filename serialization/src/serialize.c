#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ftw.h>
#include "../inc/serialize_api.h"

const char* room_in = "\"room\":\"[^\"]\"\n \"backlog\":%d\n\"activeClients\":%d\n";
const char* room_out = "\"room\":\"%s\"\n \"backlog\":%d\n\"activeClients\":%d\n";


const char* peer_in = "\t\"username\":\"\"[^\"]\"\";\"peeraddr\":\"\"%s\"\";\"peerport\":%d;\"refport\":%d;\"uid\":%d;\"keyhash\":\"[^\"]\"\n";
const char* peer_out = "\t\"username\":\"%s\";\"peeraddr\":\"%s\";\"peerport\":%d;\"refport\":%d;\"uid\":%d;\"keyhash\":\"%s\"\n";

const char* pubkey_in = "\"modulus\":\n\t\"\"[^\"]\"\n\"exponent\":\n\t\"[^\"]\"";;
const char* pubkey_out = "\"modulus\":\n\t\"%s\"\n\"exponent\":\n\t\"%s\" ";

const char* prikey_in = "\"modulus\":\n\t\"\"[^\"]\"\n\"private_d\":\n\t\"[^\"]\"";;
const char* prikey_out = "\"modulus\":\n\t\"%s\"\n\"private_d\":\n\t\"%s\"";

char* write_path = "../../dat/";
char* rooms_path = "../dat/rooms.dat";
const char* client_keys = "../dat/keys/clients/";

void CreateRoom(char* roomname,int backlog, int activeClients,char* usrname,char* peer_addr,uint16_t peer_port, uint16_t refresh_port)
{
    struct room r;
    r.backlog = backlog;
    r.activeClients = activeClients;
    strcpy(r.roomname,roomname);

    FILE* file;
    const char* path = strcat(write_path,"rooms.dat");

    file = fopen(path,"a");

    fprintf(file,room_out,r.roomname,r.backlog,r.activeClients);

    WritePeer(usrname,peer_addr,peer_port,refresh_port);
    // fprintf(file,peer_out,)
}

void WritePeer(char* usrname,char* peer_addr,uint16_t peer_port, uint16_t refresh_port)
{
    struct peer p;
    strcpy(p.usrname,usrname);
    strcpy(p.peer_addr,peer_addr);
    p.peer_port = peer_port;
    p.refresh_port = refresh_port;

    FILE* file;

    const char* path = strcat(write_path,"rooms.dat");
    file = fopen(path,"a");

    fprintf(file,peer_out,p.usrname,p.peer_addr,p.peer_port,p.refresh_port);

    fclose(file);
}

int CheckKeyExistence(char* uid)
{
    FILE *file;
    const char *filename = "example.txt";
    char tmpPath[32];

    file = fopen(filename, "r");
    if (file != NULL) {
        return 0;
    } else {
        file = fopen(filename, "w");
        fclose(file);
        return 1;
    }
}

void WriteKeys(uint16_t uid,char* client_modulus,char* client_exponent,char* server_modulus,char* server_exponent,char* private_d)
{
  char dir[12];
  bzero(&dir,sizeof(dir));

  sprintf(dir,"%d",uid);

  FILE* file;

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

  char path1[40];
  bzero(&path1,sizeof(path1));
  strcpy(path1,rooms_path);

    // printf("\n%s",path1);
    // fflush(stdout);

  FILE* file = fopen(path1, "r");

  if(file == NULL)
  {
    printf("gg");
    fflush(stdout);
  }

    char line[256];
    char room_line[256];
    snprintf(room_line, sizeof(room_line), "\"room\": \"%s\"", roomname);

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, room_line) != NULL) {
            fclose(file);
            return 0; // Room found
        }
    }

    // FIX THIS ROOM IS NOT GETTING FOUND

    fclose(file);
    return 1; // Room not found
}

