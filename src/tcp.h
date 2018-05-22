#ifndef TCP_H
#define TCP_H

#include "types.h"
#include "queue.h"
#include "clock.h"
#include "recv.h"

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

struct tcp_socket
{
    int fd;

    int snd_nxt;        /* Next sequence number to send */
    int snd_una;        /* First unacknoledged sequence number */
    int snd_wnd;        /* Send window size => snd_una + snd_wnd < snd_nxt */
    int ssthresh;
    int previous_ack;

    int que_nxt;

    Queue queue;
    Clock clock;

    RecvThread recv_thread;

    ulong_t srtt;       /* Smoothed RTT in usecs */
    ulong_t rttvar;
};

typedef struct tcp_socket Socket;

int recv_ack(Socket *s);

Socket *tcp_socket();
void tcp_bind(Socket *sock, const char *ip, u16 port);
void tcp_close(Socket *s);

void tcp_start_transfer(Socket *sock);
Socket *tcp_accept(Socket *sock, struct sockaddr_in *distant_addr);

void tcp_send(Socket *s, const char *buffer, size_t sz);
ssize_t tcp_recv(Socket *s, char *out, size_t sz);

void tcp_output(Socket *sock);

void tcp_wait(Socket *sock);

#endif
