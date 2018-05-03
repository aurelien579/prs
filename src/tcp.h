#ifndef TCP_H
#define TCP_H

#include "types.h"
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

struct tcp_socket
{
    int fd;

    u16 snd_nxt;        /* Next sequence number to send */
    u16 snd_una;        /* First unacknoledged sequence number */
    u16 snd_wnd;        /* Send window size => snd_una + snd_wnd < snd_nxt */
};

typedef struct tcp_socket Socket;

Socket *tcp_socket();
void tcp_bind(Socket *sock, const char *ip, u16 port);
void tcp_close();

Socket *tcp_accept(Socket *sock, struct sockaddr_in *distant_addr);

ssize_t tcp_send(Socket *s, const char *buffer, size_t sz);
ssize_t tcp_recv(Socket *s, char *out, size_t sz);

#endif
