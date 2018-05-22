#ifndef SEND_H
#define SEND_H

#include "queue.h"

#include <pthread.h>

struct send_thread
{
    int         fd;
    int         running;
    pthread_t   pthread;
    ulong_t     sleep;
    int         count;
    Queue       *queue;
};

typedef struct send_thread SendThread;

void send_thread_init(SendThread *thread, int fd, Queue *queue, ulong_t sleep, int count);
void send_thread_exit(SendThread *thread);

#endif
