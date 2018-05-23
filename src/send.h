#ifndef SEND_H
#define SEND_H

#include "queue.h"

#include <pthread.h>

struct sender
{
    int         running;
    pthread_t   pthread;
    ulong_t     sleep;
    int         count;
    Socket      *sock;
};

typedef struct sender Sender;

void sender_init(Sender *self, Socket *sock, ulong_t sleep, int count);
void sender_stop(Sender *self);

#endif
