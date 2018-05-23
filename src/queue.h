#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include "types.h"
#include "consts.h"

#define QUEUE_SIZE 65535

struct queue_entry
{
    char            packet[PACKET_SIZE];
    int             seq;
    size_t          size;

    struct timeval  tx_time;
    ulong_t         rtx_usecs;
    int             rtx_count;
};

typedef struct queue_entry QueueEntry;

struct queue
{
    QueueEntry              data[QUEUE_SIZE];
    int                     r;
    int                     w;
    sem_t                   free;
    pthread_spinlock_t      lock;
};

typedef struct queue Queue;

void queue_init(Queue *queue);

void queue_add_entry(Queue *q, const char *packet, int seq, size_t size,
                     ulong_t tx_time, int tx_count);

QueueEntry *queue_get(Queue *q, int seq);

void queue_clear(Queue *q, int c);


static inline int queue_readable(Queue *q)
{
    return q->w - q->r;
}

static inline int queue_writable(Queue *q)
{
    return QUEUE_SIZE - queue_readable(q);
}


#endif
