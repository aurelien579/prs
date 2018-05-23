#include "recv.h"

#include "tcp.h"
#include "utils.h"
#include "consts.h"

#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

static void *__recv(void *_self)
{
    Recver  *self = _self;
    int     ack, gap;

    while (self->running) {
        ack = recv_ack(self->socket);
        gap = ack - self->socket->una + 1;

        if (gap <= 0) continue;

        pthread_spin_lock(&self->socket->queue.lock);
        queue_clear(&self->socket->queue, gap);
        pthread_spin_unlock(&self->socket->queue.lock);

        self->socket->una = ack + 1;
    }

    return NULL;
}

void recver_init(Recver *self, Socket *socket)
{
    self->socket = socket;
    self->running = 1;
    pthread_create(&self->pthread, NULL, __recv, self);
}

void recver_stop(Recver *self)
{
    self->running = 0;
    pthread_cancel(self->pthread);
}
