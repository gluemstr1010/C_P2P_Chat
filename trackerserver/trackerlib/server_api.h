#ifndef SERVER_API_H
#define SERVER_API_H

#include <stdint.h>

#define MINLENGTH 12

struct sm
{
    int16_t message_type;
    int16_t message_length;
    int8_t trasaction_id[12];
    int8_t attributes[MINLENGTH];
};
typedef struct sm SERV_MSG;


#endif

