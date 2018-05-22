#include "queue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void queue_init(Queue *q)
{
    q->r = 1;
    q->w = 1;

    sem_init(&q->free, 0, QUEUE_SIZE);
    pthread_spin_init(&q->lock, 0);
}

void queue_add_entry(Queue *q, const char *packet, int seq, size_t size,
                     ulong_t rtx_usecs, int rtx_count)
{
    sem_wait(&q->free);

    QueueEntry *entry = &q->data[q->w % QUEUE_SIZE];

    memcpy(entry->packet, packet, size);
    entry->rtx_usecs = rtx_usecs;
    entry->rtx_count = rtx_count;
    entry->size = size;
    entry->seq = seq;

    q->w++;
}

void queue_clear(Queue *q, int c)
{
    q->r += c;
    for (int i = 0; i < c; i++) sem_post(&q->free);
}

QueueEntry *queue_get(Queue *q, int seq)
{
    return &q->data[seq % QUEUE_SIZE];
}

void queue_print(Queue *q)
{
    printf("QUEUE : ");
    for (int i = q->r; i < q->w; i++)
        printf("%d ", q->data[i % QUEUE_SIZE].seq);
    printf("\n");
}
