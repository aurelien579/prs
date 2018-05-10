#include "recv.h"

#include "tcp.h"

#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

static void *__recv(void *_self)
{
    RecvThread *self = _self;
    Socket *sock = self->socket;

    QueueEntry *entry;
    int ack;
    struct timeval tv;
    ulong_t rtt;

    while (self->running) {
        ack = recv_ack(sock);

        if (ack < sock->snd_una || ack >= sock->snd_una + sock->snd_nxt) {
            printf("invalid ack : %d\n", ack);
            continue;
        }

        printf("ack : %d\n", ack);

        pthread_mutex_lock(&sock->queue.mutex);

        gettimeofday(&tv, NULL);
        entry = queue_get(&sock->queue, ack);
        rtt = (tv.tv_usec - entry->tx_time.tv_usec) +
              (tv.tv_sec - entry->tx_time.tv_sec) * 1000000L;

        if (sock->srtt == INITRTT) {
            sock->srtt = rtt;
            sock->rttvar = rtt / 2;
        } else {
            if (sock->srtt > rtt)
                sock->rttvar = ((1 - BETA) * sock->rttvar) + (BETA * (sock->srtt - rtt));
            else
                sock->rttvar = ((1 - BETA) * sock->rttvar) + (BETA * (rtt - sock->srtt));
            sock->srtt = (sock->srtt * (1 - ALPHA)) + (ALPHA * rtt);
        }

        printf("RTT=%lld, SRTT=%lld\n", rtt, sock->srtt);

        /*sock->snd_wnd++;
        printf("INCREASE\n");*/

        for (int i = sock->snd_una; i <= ack; i++) {
            printf("remove %d\n", i);
            queue_remove(&sock->queue, i);
        }

        pthread_mutex_unlock(&sock->queue.mutex);

        sock->snd_una = ack + 1;

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
