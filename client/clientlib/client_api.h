#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>

#define MINLENGTH 56

struct cm
{
    int16_t message_type;
    int16_t message_length;
    uint8_t trasaction_id[12];
    uint8_t attributes[MINLENGTH];
};
typedef struct cm CLIENT_MSG;

void make_find_req(CLIENT_MSG find_req, int serverfd);
void make_alloc_req(CLIENT_MSG alloc_req, int serverfd);

#endif