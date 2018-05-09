#ifndef RECV_H
#define RECV_H

#include "types.h"

#include <pthread.h>

struct recv_thread
{
    pthread_t pthread;

    int     running;
    Socket *socket;
};

typedef struct recv_thread RecvThread;

void recv_thread_init(RecvThread *thread, Socket *socket);

#endif
