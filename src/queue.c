#include "queue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

QueueEntry *queue_entry_new(const char *packet, seq_t seq, size_t size, ulong_t rtx_usecs, int rtx_count)
{
    QueueEntry *entry = malloc(sizeof(QueueEntry));

    memcpy(entry->packet, packet, size);
    entry->rtx_usecs = rtx_usecs;
    entry->rtx_count = rtx_count;
    entry->size = size;
    entry->seq = seq;

    entry->next = NULL;

    return entry;
}

void queue_init(Queue *queue, size_t sz)
{
    queue->top = NULL;
    queue->last = NULL;

    sem_init(&queue->free, 0, sz);
    pthread_mutex_init(&queue->mutex, NULL);
}

void queue_insert_ordered(Queue *queue, QueueEntry *entry)
{
    sem_wait(&queue->free);

    pthread_mutex_lock(&queue->mutex);

    if (!queue->last) {
        queue->last = entry;
        queue->top = entry;
    } else {
        queue->last->next = entry;
        queue->last = entry;
    }

    pthread_mutex_unlock(&queue->mutex);
}

void queue_remove_before(Queue *queue, seq_t seq)
{
    pthread_mutex_lock(&queue->mutex);

    QueueEntry *cur = queue->top;
    QueueEntry *temp = NULL;

    while (cur) {
        if (cur->seq > seq) break;
        if (cur == queue->last) {
            queue->last = NULL;
        }

        temp = cur->next;
        sem_post(&queue->free);
        free(cur);
        cur = temp;
    }

    queue->top = cur;

    pthread_mutex_unlock(&queue->mutex);
}

void queue_remove(Queue *queue, seq_t seq)
{
    pthread_mutex_lock(&queue->mutex);

    QueueEntry *cur = queue->top;
    QueueEntry *prev = NULL;

    while (cur) {
        if (cur->seq == seq) {
            if (prev) {
                prev->next = cur->next;
            } else {
                queue->top = cur->next;
            }

            free(cur);
            sem_post(&queue->free);
            pthread_mutex_unlock(&queue->mutex);
            return;
        }

        prev = cur;
        cur = cur->next;
    }

    pthread_mutex_unlock(&queue->mutex);
}

QueueEntry *queue_get(Queue *queue, seq_t seq)
{
    QueueEntry *cur = queue->top;

    while (cur) {
        if (cur->seq == seq) {
            return cur;
        }

        cur = cur->next;
    }

    return NULL;
}

void queue_print(Queue *q)
{
    QueueEntry *c = q->top;

    printf("QUEUE : ");
    while (c) {
        printf("%d ", c->seq);
        c = c->next;
    }
    printf("\n");
}
