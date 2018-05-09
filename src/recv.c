#include "recv.h"

#include "tcp.h"

#include <stdio.h>
#include <pthread.h>

static void *__recv(void *_self)
{
    RecvThread *self = _self;
    Socket *sock = self->socket;

    int ack;

    while (self->running) {
        ack = recv_ack(sock);

        if (ack < sock->snd_una || ack >= sock->snd_una + sock->snd_nxt) {
            printf("(ack : %d)\n", ack);
            continue;
        }
        printf("ack : %d\n", ack);

        pthread_mutex_lock(&sock->queue.mutex);

        for (int i = sock->snd_una + 1; i <= ack; i++) {
            printf("remove %d\n", i);
            queue_remove(&sock->queue, i);
        }
        //queue_print(&sock->queue);

        pthread_mutex_unlock(&sock->queue.mutex);

        sock->snd_una = ack;

        tcp_output(sock);
    }

    return NULL;
}

void recv_thread_init(RecvThread *thread, Socket *socket)
{
    thread->socket = socket;
    thread->running = 1;
    pthread_create(&thread->pthread, NULL, __recv, thread);
}
