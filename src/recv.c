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
    Socket *sock = self->socket;
    //FILE* fp = fopen ("log.txt", "w+");
    QueueEntry *entry;
    int ack;
    struct timeval tv;
    ulong_t rtt;
    int ack_redondance=0;
    while (self->running) {
        ack = recv_ack(sock);
        printf("WHILE\n");

        if (ack < sock->snd_una || ack >= sock->snd_una + sock->snd_nxt) { // ack déjà reçu ou d'un segment non envoyé
            printf("invalid ack : %d\n", ack);
            continue;
        }

#ifndef NO_FAST_RETRANSMIT
        if(ack == sock->previous_ack){
          ack_redondance+=1;
        }else{
          ack_redondance=0;
          sock->previous_ack = ack;
        }

        if(ack_redondance>=3){
          entry = queue_get(&sock->queue, ack+1);
          sock -> snd_wnd += 1;
          if(entry){
            send(sock->fd, entry->packet, entry->size, 0);
          }

          continue;
      }
#endif

        printf("ack : %d\n", ack);

        pthread_mutex_lock(&sock->queue.mutex);

        gettimeofday(&tv, NULL); // calcul du rtt, srtt
        entry = queue_get(&sock->queue, ack);
        rtt = (tv.tv_usec - entry->tx_time.tv_usec) +
              (tv.tv_sec - entry->tx_time.tv_sec) * 1000000L;

        if (sock->srtt == INITRTT) {
            sock->srtt = rtt;
            sock->rttvar = rtt / 2;
        } else {
            if (sock->srtt > rtt)
                sock->rttvar = ((1 - BETA) * sock->rttvar) + (BETA * (sock->srtt - rtt))+10000;
            else
                sock->rttvar = ((1 - BETA) * sock->rttvar) + (BETA * (rtt - sock->srtt))+10000;
            sock->srtt = (sock->srtt * (1 - ALPHA)) + (ALPHA * rtt);
        }

        printf("RTT=%lld, SRTT=%lld\n", rtt, sock->srtt);

        /*sock->snd_wnd++;
        printf("INCREASE\n");*/
        pthread_mutex_unlock(&sock->queue.mutex);

        queue_remove_before(&sock->queue, ack);

#ifndef NO_CONGESTION
        if(sock->snd_wnd < sock -> ssthresh){
           //fprintf(fp," ** THE WINDOW SS ** : %d and the ack gap : %d and the thresh : %d\n\n", sock->snd_wnd+1, ack-sock-> snd_una, sock->ssthresh);
           sock -> snd_wnd += ack+1-sock->snd_una;
        }else{
           //fprintf(fp," ** THE WINDOW N ** : %d and the ack gap : %d and the thresh : %d\n\n", sock->snd_wnd+1, ack-sock-> snd_una, sock->ssthresh);
           sock -> snd_wnd += 1;
        }
#endif

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
