#include "send.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

static void *__send(void *_thread)
{
    Sender      *self = _thread;
    int         count = 0;
    Queue       *q = self->queue;
    QueueEntry  *entry;

    while (self->running) {
        pthread_spin_lock(&q->lock);

        count = min(self->count, queue_readable(q));

        for (int i = 0; i < count; i++) {
            entry = queue_get(q, q->r + i);
            send(self->fd, entry->packet, entry->size, 0);
        }

        pthread_spin_unlock(&q->lock);

        usleep(self->sleep);
    }

    return NULL;
}

void sender_init(Sender *self, int fd, Queue *queue, ulong_t sleep, int count)
{
    self->running = 1;
    self->fd = fd;
    self->queue = queue;
    self->sleep = sleep;
    self->count = count;

    pthread_create(&self->pthread, NULL, __send, self);
}

void sender_stop(Sender *self)
{
    self->running = 0;
    pthread_cancel(self->pthread);
}
