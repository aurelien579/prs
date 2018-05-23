#include "send.h"
#include "utils.h"
#include "tcp.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#ifdef SRV1
static void *__send(void *_thread)
{
    Sender      *self = _thread;
    int         count = 0;
    Queue       *q = &self->sock->queue;
    QueueEntry  *entry;

    while (self->running) {
        pthread_spin_lock(&q->lock);

        count = min(self->count, queue_readable(q));

        for (int i = 0; i < count; i++) {
            entry = queue_get(q, q->r + i);
            send(self->sock->fd, entry->packet, entry->size, 0);
        }

        pthread_spin_unlock(&q->lock);

        usleep(self->sleep);
    }

    return NULL;
}
#endif

#ifdef SRV2
static void *__send(void *_self)
{
    Sender      *self = _self;
    Queue       *q = &self->sock->queue;
    QueueEntry  *entry;
    int         should_send = 0;

    while (self->running) {
        pthread_spin_lock(&q->lock);


        for (int i = 0; i < queue_readable(q); i++) {
            entry = queue_get(q, q->r + i);

            if (entry->rtx_usecs > self->sleep) {
                entry->rtx_usecs -= self->sleep;
            } else {
                entry->rtx_usecs = 0;
                if (entry->rtx_count > 0)
                    should_send = 1;
            }
        }

        pthread_spin_unlock(&q->lock);

        if (should_send) {
            tcp_output(self->sock);
            should_send = 0;
        }

        usleep(self->sleep);
    }

    return NULL;
}
#endif

void sender_init(Sender *self, Socket *sock, ulong_t sleep, int count)
{
    self->running = 1;
    self->sock = sock;
    self->sleep = sleep;
    self->count = count;

    pthread_create(&self->pthread, NULL, __send, self);
}

void sender_stop(Sender *self)
{
    self->running = 0;
    pthread_cancel(self->pthread);
}
