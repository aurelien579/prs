#ifndef TCP_H
#define TCP_H

#include "types.h"
#include "queue.h"
#include "recv.h"
#include "send.h"


#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>


struct tcp_socket
{
    int     fd;
    int     snd_una;
    int     que_nxt;

#ifdef SRV2
    int     snd_wnd;
    int     snd_nxt;
    int     ssthresh;
    ulong_t srtt;
    ulong_t rttvar;
    int     previous_ack;
#endif

    Queue   queue;
    Recver  recver;
    Sender  sender;
};

typedef struct tcp_socket Socket;

int recv_ack(Socket *s);

Socket *tcp_socket();
void tcp_close(Socket *s);

Socket *tcp_accept(Socket *sock, struct sockaddr_in *distant_addr);
void tcp_bind(Socket *sock, const char *ip, u16 port);

void tcp_start_transfer(Socket *sock, ulong_t sleep, int count);

void tcp_send(Socket *s, const char *buffer, size_t sz);
ssize_t tcp_recv(Socket *s, char *out, size_t sz);

#ifdef SRV2
void tcp_output(Socket *sock);
#endif

void tcp_wait(Socket *sock);

#endif
