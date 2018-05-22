#include "recv.h"

#include "tcp.h"
#include "utils.h"
#include "consts.h"

#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

static void *__recv(void *_self)
{
    RecvThread *self = _self;
    int ack;
    int gap;

    while (self->running) {
        ack = recv_ack(self->socket);
        gap = ack - self->socket->snd_una + 1;

        if (gap <= 0) continue;

        pthread_spin_lock(&self->socket->queue.lock);
        queue_clear(&self->socket->queue, gap);
        pthread_spin_unlock(&self->socket->queue.lock);

        self->socket->snd_una = ack + 1;
    }

    return NULL;
}

void recv_thread_init(RecvThread *thread, Socket *socket)
{
    thread->socket = socket;
    thread->running = 1;
    pthread_create(&thread->pthread, NULL, __recv, thread);
}

void recv_thread_stop(RecvThread *thread)
{
    thread->running = 0;
    pthread_cancel(thread->pthread);
}
