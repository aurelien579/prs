#include "recv.h"

#include "tcp.h"
#include "utils.h"
#include "consts.h"

#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

#ifdef SRV1
static void *__recv(void *_self)
{
    Recver  *self = _self;
    int     ack, gap;

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
#endif

#ifdef SRV2
static void *__recv(void *_self)
{
    Recver          *self = _self;
    Socket          *sock = self->socket;
    QueueEntry      *entry;
    int             ack, gap, ack_redondance = 0;
    struct timeval  tv;
    ulong_t         rtt;

    while (self->running) {
        ack = recv_ack(sock);
        gap = ack - self->socket->snd_una + 1;

        if (gap <= 0) continue;

        if (ack == sock->previous_ack) {
            ack_redondance += 1;
        } else {
            ack_redondance = 0;
            sock->previous_ack = ack;
        }

        if (ack_redondance>=3) {
            entry = queue_get(&sock->queue, ack + 1);
            sock->snd_wnd += 1;
            if (entry)
                send(sock->fd, entry->packet, entry->size, 0);

            continue;
        }

        pthread_spin_lock(&sock->queue.lock);

        gettimeofday(&tv, NULL); // calcul du rtt, srtt
        entry = queue_get(&sock->queue, ack);
        rtt = (tv.tv_usec - entry->tx_time.tv_usec) +
              (tv.tv_sec - entry->tx_time.tv_sec) * 1000000L;

        if (sock->srtt == INITRTT) {
            sock->srtt = rtt;
            sock->rttvar = rtt / 2;
        } else {
            if (sock->srtt > rtt)
                sock->rttvar = ((1 - BETA) * sock->rttvar) + (BETA * (sock->srtt - rtt))+RTT_OFFSET;
            else
                sock->rttvar = ((1 - BETA) * sock->rttvar) + (BETA * (rtt - sock->srtt))+RTT_OFFSET;
            sock->srtt = (sock->srtt * (1 - ALPHA)) + (ALPHA * rtt);
        }

        pthread_spin_unlock(&sock->queue.lock);

        queue_clear(&sock->queue, gap);

        if (sock->snd_wnd < sock->ssthresh) {
            sock -> snd_wnd += ack+1-sock->snd_una;
        } else {
            sock -> snd_wnd += 1;
        }

        sock->snd_una = ack + 1;

        tcp_output(sock);
    }

    return NULL;
}
#endif

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
