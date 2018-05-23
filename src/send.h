#ifndef SEND_H
#define SEND_H

#include "queue.h"

#include <pthread.h>

struct sender
{
    int         fd;
    int         running;
    pthread_t   pthread;
    ulong_t     sleep;
    int         count;
    Queue       *queue;
};

typedef struct sender Sender;

void sender_init(Sender *self, int fd, Queue *queue, ulong_t sleep, int count);
void sender_stop(Sender *self);

#endif
