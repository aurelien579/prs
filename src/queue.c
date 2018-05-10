#include "queue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

QueueEntry *queue_entry_new(const char *packet, int seq, size_t size, ulong_t rtx_usecs, int rtx_count)
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

void queue_init(Queue *queue)
{
    queue->top = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
}

void queue_insert_ordered(Queue *queue, QueueEntry *entry)
{
    QueueEntry *prev = NULL;
    QueueEntry *cur = queue->top;

    if (!cur) {
        queue->top = entry;
        return;
    }

    while (cur) {
        if (cur->seq > entry->seq)
        {
            entry->next = cur;
            if (prev) {
                prev->next = entry;
            } else {
                queue->top = entry;
            }

            return;
        }

        prev = cur;
        cur = cur->next;
    }

    prev->next = entry;
}

void queue_remove(Queue *queue, int seq)
{
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
            return;
        }

        prev = cur;
        cur = cur->next;
    }
}

QueueEntry *queue_get(Queue *queue, int seq)
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
