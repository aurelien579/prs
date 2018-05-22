#ifndef SEND_H
#define SEND_H

#include "queue.h"

#include <pthread.h>

struct send_thread
{
    pthread_t   pthread;
    int         running;

    int         fd;
    Queue       *queue;
};

typedef struct send_thread SendThread;

void send_thread_init(SendThread *thread, int fd, Queue *queue);
void send_thread_exit(SendThread *thread);

#endif
