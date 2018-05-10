#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>

#include "types.h"
#include "consts.h"

struct queue_entry
{
    char    packet[PACKET_SIZE];
    int     seq;
    size_t  size;

    struct timeval tx_time;
    ulong_t rtx_usecs;
    int     rtx_count;

    struct queue_entry *next;
};

typedef struct queue_entry QueueEntry;

struct queue
{
    QueueEntry *top;
    pthread_mutex_t mutex;
};

typedef struct queue Queue;

QueueEntry *queue_entry_new(const char *packet, int seq, size_t size,
                            ulong_t tx_time, int tx_count);

QueueEntry *queue_get(Queue *queue, int seq);

void queue_init(Queue *queue);
void queue_insert_ordered(Queue *queue, QueueEntry *entry);
void queue_remove(Queue *queue, int seq);

void queue_print(Queue *queue);

#endif
