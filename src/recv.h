#ifndef RECV_H
#define RECV_H

#include "types.h"

#include <pthread.h>

struct recver
{
    pthread_t   pthread;
    int         running;
    Socket      *socket;
};

typedef struct recver Recver;

void recver_init(Recver *self, Socket *socket);
void recver_stop(Recver *self);

#endif
